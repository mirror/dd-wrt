/*
 * bonding.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_BONDING
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <services.h>
void stop_bonding(void)
{
	int i;

	for (i = 0; i < 10; i++) {
		char bond[32];

		sprintf(bond, "bond%d", i);
		if (ifexists(bond)) {
			char word[256];
			char *br = getRealBridge(bond, word);

			if (br)
				br_del_interface(br, bond);
			eval("ifconfig", bond, "down");
		}
	}
	rmmod("bonding");
}

void start_bonding(void)
{
	char mode[64];
	char count[64];

	stop_bonding();

	sprintf(mode, "mode=%s", nvram_default_get("bonding_type", "balance-rr"));
	sprintf(count, "max_bonds=%s", nvram_default_get("bonding_number", "1"));

	char word[256];
	char *next, *wordlist;
	int first = 0;
	wordlist = nvram_safe_get("bondings");
	foreach(word, wordlist, next)
	{
		char *port = word;
		char *tag = strsep(&port, ">");

		if (!tag || !port) {
			break;
		}
		if (!strncmp(port, "wlan", 4) && nvram_nmatch("wdsap", "%s_mode", port)) {
			eval("ifconfig", port, "down");
			eval("iwpriv", port, "wdssep", "0");
			eval("ifconfig", port, "up");
		}
		if (!first) {
			eval("insmod", "bonding", "miimon=100", "downdelay=200", "updelay=200", mode, count);
			first = 1;
		}
		eval("ifconfig", tag, "0.0.0.0", "up");
		eval("ifenslave", tag, port);
	}
	int c = nvram_geti("bonding_number");
	int i;

	for (i = 0; i < c; i++) {
		sprintf(word, "bond%d", i);
		char tmp[256];
		char *br = getRealBridge(word, tmp);

		if (br)
			br_add_interface(br, word);
	}
}

int isBond(char *ifname)
{
	char word[256];
	char *next, *wordlist;

	wordlist = nvram_safe_get("bondings");
	foreach(word, wordlist, next)
	{
		char *port = word;
		char *tag = strsep(&port, ">");

		if (!tag || !port) {
			break;
		}
		if (!strcmp(port, ifname))
			return 1;
	}
	return 0;
}

#endif
