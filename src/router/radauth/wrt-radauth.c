/* Really simple radius authenticator
 *
 * Copyright (c) 2004 Michael Gernoth <michael@gernoth.net>
 * Copyright (c) 2006 Atheros Support by Sebastian Gottschall <s.gottschall@newmedia-net.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <wlutils.h>
#include <bcmnvram.h>
//#include <net/ethernet.h>

#include <unistd.h>
#include <ctype.h>
#include "radius.h"

#ifndef WLC_IOCTL_MAXLEN
#define WLC_IOCTL_MAXLEN	8192
#endif

#define WAIT	300		/* Seconds until a STA expires */

struct sta {
	unsigned char mac[6];
	unsigned char accepted;
	unsigned char changed;
	struct sta *next;	/* Pointer to next STA in linked list */
	time_t lastseen;
};

char *server, *secret;
short port;
short mackey;
short macfmt;
int internal = 0;
#include <shutils.h>

int authmac(unsigned char *mac)
{
	char macstr[32];
	if (internal) {
		sprintf(macstr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#ifdef DEBUG
		printf("mac %s\n", macstr);
#endif

		char *collection = nvram_get_collection("iradius");
		if (collection != NULL) {
			char entry[32];
			char *next;
			int c = 0;
			char smac[32];
			memset(smac, 0, 32);
			char enabled[32];
			memset(enabled, 0, 32);
			char time[32];
			memset(time, 0, 32);
			foreach(entry, collection, next) {

				switch (c % 3) {
				case 0:
					strcpy(smac, entry);
					break;
				case 1:
					strcpy(enabled, entry);
					break;
				case 2:
					strcpy(time, entry);
					if (strlen(smac) > 0) {
#ifdef DEBUG
						printf("mac %s %s %s\n", smac, enabled, time);
#endif
						if (!strcasecmp(smac, macstr)) {
							if (strcmp(enabled, "1") == 0) {

								struct timeval now;
								gettimeofday(&now, NULL);
								long t = atol(time);
								if (t == -1) {
									free(collection);
									return 0;
								}
								t -= now.tv_sec;
								if (t <= 0) {
									free(collection);
									return 0;
								}
								free(collection);
								return 1;
							} else {
								free(collection);
								return 0;
							}
						}
					}
					break;
				}
				c++;
			}
		}
		if (collection == NULL) {
			collection = malloc(32);
			memset(collection, 0, 32);
		} else
			collection = realloc(collection, strlen(collection) + 32);
#ifdef DEBUG
		printf("mac2 %s\n", macstr);
#endif
		strcat(collection, macstr);
		strcat(collection, " 1");
		strcat(collection, " 1800 ");	//time set to not configured
#ifdef DEBUG
		printf("collection %s\n", collection);
#endif
		nvram_store_collection("iradius", collection);

		int radcount = 0;
		char *radc = nvram_get("iradius_count");
		if (radc != NULL)
			radcount = atoi(radc);

		radcount++;
		char count[16];
		sprintf(count, "%d", radcount);
		nvram_set("iradius_count", count);
		nvram_commit();
		free(collection);
		return 0;
	} else {

		switch (macfmt) {
		case 1:	//000000-000000
			sprintf(macstr, "%2.2x%2.2x%2.2x-%2.2x%2.2x%2.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		case 2:	//000000000000
			sprintf(macstr, "%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		case 3:	//00:00:00:00:00:00
			sprintf(macstr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		default:	//00-00-00-00-00-00
			sprintf(macstr, "%2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		if (!mackey)
			return radius(server, port, macstr, macstr, secret);
		else
			return radius(server, port, macstr, secret, secret);
	}

}

int main(int argc, char **argv)
{
	int num, i;
	unsigned char *buf;	/* Buffer for wireless-ioctls MAC lists */
	buf = malloc(WLC_IOCTL_MAXLEN);
	unsigned char *pos;
	char *iface;
	char *maxun;
	int override;
	struct maclist *maclist;
	struct ether_addr *ea;
	int usePortal = 0;
	char macbuild[64];
	struct sta *first;	/* Pointer to first element in linked STA list */

	int val;
	int lastcnt;		/* Number of blacklisted cards in the last loop */
	int statechange;	/* Do we need to push the new blacklist/reset the card? */
	time_t step;
	/* SeG DD-WRT change */
	int unauthenticated_users;	/* count for unauthicated users which can access the AP without radius */
	int maxunauthenticated_users;	/* maxcount for unauthenticated users */

	if (argc < 2) {
	      argerror:;
		fprintf(stderr, "wrt-radauth - A simple radius authenticator\n");
		fprintf(stderr, "(C) 2005 Michael Gernoth\n");
		fprintf(stderr, "(C) 2006 Atheros support Sebastian Gottschall\n");
		fprintf(stderr, "Usage: %s [-nx] interface radiusip radiusport sharedkey radiusoverride mackeytype macunauthusers\n", argv[0]);
		fprintf(stderr, "\t-n1\tUse new MAC address format 'aabbcc-ddeeff' instead of 'AA-BB-CC-DD-EE-FF'\n");
		fprintf(stderr, "\t-n2\tUse really new MAC address format 'aabbccddeeff' instead of 'AA-BB-CC-DD-EE-FF'\n");
		exit(1);
	}
#ifdef DEBUG
	printf("$Id: wrt-radauth.c,v 1.17 2004/09/28 13:15:51 simigern Exp $ coming up...\n");
#endif
	int offset = 1;
	if (argc > 2 && (strcmp(argv[1], "-n1") == 0)) {
		macfmt = 1;
		offset = 2;
	} else if (argc > 2 && (strcmp(argv[1], "-n2") == 0)) {
		macfmt = 2;
		offset = 2;
	} else if (argc > 2 && (strcmp(argv[1], "-n3") == 0)) {
		macfmt = 3;
		offset = 2;
	} else if (argc > 2 && (strcmp(argv[1], "-t") == 0)) {
		macfmt = 0;
		internal = 1;
		offset = 2;
	} else {
		macfmt = 0;
		offset = 1;
	}
	iface = argv[offset++];

	if (argc - offset != 6 && internal == 0) {
		goto argerror;
	}
	if (argc != 3 && internal == 1) {
		goto argerror;
	}
	maxunauthenticated_users = 0;
	if (!internal) {
		usePortal = 0;
		server = argv[offset++];
		port = atoi(argv[offset++]);
		secret = argv[offset++];
		override = atoi(argv[offset++]);
		mackey = atoi(argv[offset++]);
		maxun = argv[offset++];
		if (maxun != NULL && strlen(maxun) > 0)
			maxunauthenticated_users = atoi(maxun);	//read nvram variable
	}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	if (wl_probe(iface)) {
		printf("Interface %s is not broadcom wireless!\n", iface);
	}
#endif

	/* Get configuration from nvram */
/*	server=strdup(nvram_safe_get("wl0_radius_ipaddr"));
	port=atoi(nvram_safe_get("wl0_radius_port"));
	usePortal=atoi(nvram_safe_get("wl_radportal"));
        override=atoi(nvram_safe_get("radius_override"));
*/
	/* SeG DD-WRT change */
//      maxun = nvram_get("max_unauth_users");
//      secret=strdup(nvram_get("wl0_radius_key"));
//      mackey=atoi(nvram_get("wl_radmacpassword"));
#ifdef DEBUG
	printf("Server: %s:%d, Secret: %s\n", server, port, secret);
#endif

	/* Initialize vars */
	lastcnt = 0;

	/* Disable MAC security on card */
	memset(buf, 0, WLC_IOCTL_MAXLEN);
	set_maclist(iface, buf);
	security_disable(iface);

	/* No STAs in list */
	first = NULL;

	while (1) {
		struct sta *currsta, *prev;

		step = time(NULL);

		/* Initialize vars */
		statechange = 0;

		/* Query card for currently associated STAs */
#ifdef DEBUG
		puts("get assoc list");
#endif
		memset(&buf[0], 0, WLC_IOCTL_MAXLEN);
		int cnt = getassoclist(iface, buf);
#ifdef DEBUG
		printf("count %d\n", cnt);

#endif
		pos = buf;
		if (cnt == -1)
			num = 0;
		else
			num = cnt;
//      memcpy (&num, pos, 4);  /* TODO: This really is struct maclist */
		pos += 4;
#ifdef DEBUG
		printf("count %d\n", num);
#endif
		unauthenticated_users = 0;	//reset count for unauthenticated users

		/* Look at the associated STAs */
		for (i = 0; i < num; i++) {
			currsta = first;
			prev = NULL;

			/* Have we already seen this STA */
			while (currsta != NULL) {
				if (memcmp(currsta->mac, pos, 6) == 0) {
					if (currsta->lastseen + WAIT < step) {
						currsta->changed = 1;
						int response = authmac(pos);
						if (response == -10 && override == 1)
							currsta->accepted = 1;
						else if (response > 0) {
							currsta->accepted = 1;
#ifdef DEBUG
							printf
							    ("Reauthenticating STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
							     currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif
						} else {
							currsta->accepted = 0;
						}
						currsta->lastseen = step;
					}

					if (!currsta->accepted) {
						/* SeG DD-WRT Change  */
						if (unauthenticated_users < maxunauthenticated_users) {
							unauthenticated_users++;	//increment unauthenticated user count
							currsta->accepted = 1;
						} else {
							currsta->changed = 1;
#ifdef DEBUG
							printf("Rejecting STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif
						}

					}
					break;
				}
				prev = currsta;
				currsta = currsta->next;
			}

			/* Or is it new? */
			if (currsta == NULL) {
				/* Alloc mem for new STA */
#ifdef DEBUG
				printf("alloc new sta\n");
#endif
				currsta = malloc(sizeof(struct sta));
				if (currsta == NULL) {
					perror("malloc");
					exit(1);
				}
#ifdef DEBUG
				printf("check if first is null\n");
#endif
				if (first == NULL) {
#ifdef DEBUG
					printf("first is null\n");
#endif
					first = currsta;
				} else {
#ifdef DEBUG
					printf("first is not null\n");
#endif
					prev->next = currsta;
				}

				currsta->next = NULL;
				currsta->changed = 1;

#ifdef DEBUG
				printf("copy new mac\n");
#endif
				memcpy(currsta->mac, pos, 6);
#ifdef DEBUG
				if (currsta != NULL)
					printf("checking STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif
				currsta->lastseen = step;
				int response = authmac(currsta->mac);
#ifdef DEBUG
				printf("response of authmac %d", response);
#endif

				if (response == -10 && override == 1)
					currsta->accepted = 1;
				else if (response == 1) {
					currsta->accepted = 1;
#ifdef DEBUG
					printf("Accepting STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif
					if (usePortal) {
						sprintf(macbuild,
							"radiusallow %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
						system(macbuild);

					}

				} else {
					/* SeG DD-WRT Change  */
					if (unauthenticated_users < maxunauthenticated_users) {
						unauthenticated_users++;	//increment unauthenticated user count
						currsta->accepted = 1;
					} else {
						currsta->accepted = 0;
#ifdef DEBUG
						printf("Rejecting STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif
					}
				}
			}
			pos += 6;	/* Jump to next MAC in list */
		}

		/* Expire old STAs from list, free memory */
		currsta = first;
		prev = NULL;
		while (currsta != NULL) {
			if (currsta->lastseen + WAIT < step) {
				struct sta *tmpsta;

				statechange = 1;
#ifdef DEBUG
				printf("Expiring STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", currsta->mac[0], currsta->mac[1], currsta->mac[2], currsta->mac[3], currsta->mac[4], currsta->mac[5]);
#endif

				if (currsta == first) {
					tmpsta = first;
					first = first->next;
					free(tmpsta);
					currsta = first;
				} else {
					tmpsta = currsta;
					prev->next = currsta->next;
					free(currsta);
					currsta = prev->next;
				}

			} else {
				prev = currsta;
				currsta = currsta->next;
			}
		}

		/* Find STAs to kick off */
		memset(buf, 0, sizeof(buf));
		maclist = (struct maclist *)buf;
		maclist->count = 0;
		ea = maclist->ea;
		currsta = first;
		while (currsta != NULL) {
			if (!currsta->accepted) {
				memcpy(ea, currsta->mac, 6);
				ea++;
				maclist->count++;
			}
			if (currsta->changed) {
				statechange = 1;
				currsta->changed = 0;
			}
			currsta = currsta->next;
		}

		/* statechange = Previously unseen/denied STA seen or STA expired */
		if (statechange) {
			if (maclist->count) {
				unsigned char mac[6];

				security_deny(iface);	//deny macs
				set_maclist(iface, buf);	//set maclist to deny

				ea = maclist->ea;
				for (i = 0; i < maclist->count; i++) {
					memcpy(mac, ea, 6);
#ifdef DEBUG
					printf("Disassociating STA %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
					if (usePortal) {
						sprintf(macbuild, "radiusdisallow %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
						system(macbuild);

					} else
						kick_mac(iface, mac);

					ea++;
				}
			} else {
				if (lastcnt != 0) {
					/* The card does not accept any association with an empty
					 * deny-list, so disable MAC-security */
#ifdef DEBUG
					printf("Resetting security\n");
#endif
					security_disable(iface);
				}
			}
			lastcnt = maclist->count;
		}

		/* Immediately continue after a statechange */
		if (!statechange)
			sleep(1);

	}

	exit(0);
}
