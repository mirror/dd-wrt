/*
 * bandwidth.c
 *
 * Copyright (C) 2005 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#define PROC_DEV "/proc/net/dev"

#define MAXCOL 8
static void get_ifstat(char *ifname, char *buffer, size_t len)
{
	char line[256];
	FILE *fp;
	struct dev_info {
		unsigned long long int rx_bytes;
		unsigned long long int rx_pks;
		unsigned long long int rx_errs;
		unsigned long long int rx_drops;
		unsigned long long int tx_bytes;
		unsigned long long int tx_pks;
		unsigned long long int tx_errs;
		unsigned long long int tx_drops;
		unsigned long long int tx_colls;
	} info;
	info.rx_pks = info.rx_errs = info.rx_drops = 0;
	info.tx_pks = info.tx_errs = info.tx_drops = info.tx_colls = 0;
	if ((fp = fopen(PROC_DEV, "r")) == NULL) {
		return;
	} else {
		/*
		 * Inter-| Receive | Transmit face |bytes packets errs drop fifo
		 * frame compressed multicast|bytes packets errs drop fifo colls
		 * carrier compressed lo: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 eth0:
		 * 674829 5501 0 0 0 0 0 0 1249130 1831 0 0 0 0 0 0 eth1: 0 0 0 0 0 0 
		 * 0 0 0 0 0 0 0 0 0 0 eth2: 0 0 0 0 0 719 0 0 1974 16 295 0 0 0 0 0
		 * br0: 107114 1078 0 0 0 0 0 0 910094 1304 0 0 0 0 0 0
		 * 
		 */
		while (fgets(line, sizeof(line), fp) != NULL) {
			int ifl = 0;
			if (!strchr(line, ':'))
				continue;
			while (line[ifl] != ':')
				ifl++;
			line[ifl] = 0; /* interface */
			char ifnamecopy[32];
			int l = 0;
			int i;
			int len = strlen(line);
			for (i = 0; i < len; i++) {
				if (line[i] == ' ')
					continue;
				ifnamecopy[l++] = line[i];
			}
			ifnamecopy[l] = 0;
			if (!strcmp(ifnamecopy, ifname)) {
				sscanf(line + ifl + 1,
				       "%llu %llu %llu %llu %*llu %*llu %*llu %*llu %llu %llu %llu %llu %*llu %llu %*llu %*llu",
				       &info.rx_bytes, &info.rx_pks, &info.rx_errs, &info.rx_drops, &info.tx_bytes, &info.tx_pks,
				       &info.tx_errs, &info.tx_drops, &info.tx_colls);
			}
		}
		fclose(fp);
	}
	snprintf(buffer, len, "RX:%lld MiB TX:%lld MiB", info.rx_bytes >> 20, info.tx_bytes >> 20);

	return;
}

static void show_portif_row(webs_t wp, char ifname[MAXCOL][32])
{
	int i;
	int max = 0;
	char buf[128];
	struct portstatus status;
	websWrite(wp, "<table cellspacing=\"4\" summary=\"portif\" id=\"portif_table\" class=\"table\"><thead><tr>\n");
	for (i = 0; i < MAXCOL; i++) {
		if (ifname[i][0]) {
			char *label = nvram_nget("%s_label", ifname[i]);
			websWrite(wp, "<th class=\"center\" width=\"%d%%\">%s</th>\n", 100 / MAXCOL, *label ? label : ifname[i]);
		} else {
			websWrite(wp, "<th class=\"center\" width=\"%d%%\">&nbsp;</th>\n", 100 / MAXCOL);
		}
	}
	websWrite(wp, "</tr></thead>\n");
	websWrite(wp, "<tbody>\n");
	websWrite(wp, "<tr>\n");
	for (i = 0; i < MAXCOL; i++) {
		if (ifname[i][0]) {
			int r = getLanPortStatus(ifname[i], &status);
			if (r) {
				websWrite(wp, "<td class=\"status_red center\">");
				websWrite(wp, "error %d", r);
			} else {
				if (status.link) {
					char buffer[128];
					get_ifstat(ifname[i], buffer, sizeof(buffer));
					if (status.speed == 10)
						websWrite(wp, "<td title=\"%s\" class=\"status_orange center\">", buffer);
					else if (status.speed == 100)
						websWrite(wp, "<td title=\"%s\" class=\"status_yellow center\">", buffer);
					else if (status.speed >= 1000)
						websWrite(wp, "<td title=\"%s\" class=\"status_green center\">", buffer);
					websWrite(wp, "%d%s", status.speed, status.fd ? "HD" : "FD");
				} else {
					websWrite(wp, "<td class=\"status_red center\">Down");
				}
			}
			websWrite(wp, "</td>\n");
		} else {
			websWrite(wp, "<td>&nbsp;</td>\n");
		}
	}
	websWrite(wp, "</tbody>\n");
	websWrite(wp, "</table>\n");
}
struct portcontext {
	char ifname[MAXCOL][32];
	int count;
};

static void show_portif(webs_t wp, struct portcontext *ctx, char *ifname)
{
	if (!ctx->count) {
		memset(ctx->ifname, 0, sizeof(ctx->ifname));
	}
	strlcpy(ctx->ifname[ctx->count], ifname, 32);
	ctx->count++;
	if (ctx->count == MAXCOL) {
		ctx->count = 0;
		show_portif_row(wp, ctx->ifname);
	}
}

void EJ_VISIBLE ej_show_portstatus(webs_t wp, int argc, char_t **argv)
{
	char wan_if_buffer[33];
	char name[180];
	const char *next, *bnext;
	char var[80];
	char eths[512];
	char eths2[512];
	char buf[128];
	char bword[256];
	glob_t globbuf;
	char *globstring;
	int globresult;
	int c;
	struct portcontext ctx;
	memset(&ctx, 0, sizeof(ctx));
	websWrite(wp, "<h2>%s</h2>\n", tran_string(buf, sizeof(buf), "networking.portstatus"));
	websWrite(wp, "<fieldset>\n");
	getIfLists(eths, sizeof(eths));
	foreach(var, eths, next) {
		if (!strncmp(var, "lan", 3) || !strncmp(var, "wan", 3) || !strncmp(var, "eth", 3) || !strncmp(var, "10g", 3))
			show_portif(wp, &ctx, var);
	}
	if (ctx.count > 0) {
		show_portif_row(wp, ctx.ifname);
	}
	websWrite(wp, "</fieldset>\n");
}
