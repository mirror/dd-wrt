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

static void show_portif_row(webs_t wp, char ifname[4][32])
{
	int i;
	int max = 0;
	char buf[128];
	struct portstatus status;
	websWrite(wp, "<table cellspacing=\"4\" summary=\"portif\" id=\"portif_table\" class=\"table\"><thead><tr>\n");
	for (i = 0; i < 12; i++) {
		if (ifname[i][0])
			max++;
	}
	for (i = 0; i < max; i++) {
		websWrite(wp, "<th class=\"center\" width=\"8%%\">%s</th>\n", ifname[i][0] ? ifname[i] : "");
	}
	websWrite(wp, "</tr></thead>\n");
	websWrite(wp, "<tbody>\n");
	websWrite(wp, "<tr>\n");
	for (i = 0; i < max; i++) {
		int r = getLanPortStatus(ifname[i], &status);
		if (r) {
			websWrite(wp, "<td class=\"status_red center\">");
			websWrite(wp, "error %d", r);
		} else {
			if (status.link) {
				if (status.speed == 10)
					websWrite(wp, "<td class=\"status_orange center\">\n");
				else if (status.speed == 100)
					websWrite(wp, "<td class=\"status_yellow center\">\n");
				else if (status.speed >= 1000)
					websWrite(wp, "<td class=\"status_green center\">\n");
				websWrite(wp, "%d%s", status.speed, status.fd ? "HD" : "FD");
			} else {
				websWrite(wp, "<td class=\"status_red center\">");
				websWrite(wp, "down");
			}
			websWrite(wp, "</td>\n");
		}
	}
	websWrite(wp, "</tbody>\n");
	websWrite(wp, "</table>\n");
}
struct portcontext {
	char ifname[12][32];
	int count;
};

static void show_portif(webs_t wp, struct portcontext *ctx, char *ifname)
{
	if (!ctx->count) {
		memset(ctx->ifname, 0, sizeof(ctx->ifname));
	}
	strlcpy(ctx->ifname[ctx->count], ifname, 32);
	ctx->count++;
	if (ctx->count == 12) {
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
		if (!strncmp(var, "lan", 3) || !strncmp(var, "wan", 3) || !strncmp(var, "eth", 3))
			show_portif(wp, &ctx, var);
	}
	if (ctx.count > 0) {
		show_portif_row(wp, ctx.ifname);
	}
	websWrite(wp, "</fieldset>\n");
}
