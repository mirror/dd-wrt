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
	websWrite(wp, "<table cellspacing=\"4\" summary=\"portif\" id=\"portif_table\" class=\"table\"><thead><tr>\n");
	for (i = 0; i < 4; i++) {
		websWrite(wp, "<th width=\"25%%\">%s</th>\n", ifname[i][0] ? ifname[i] : "");
	}
	websWrite(wp, "</tr></thead>\n");
	websWrite(wp, "<tbody>\n");
	websWrite(wp, "<tr>\n");

	char buf[128];
	for (i = 0; i < 4; i++) {
		websWrite(wp, "<td>\n");
		if (ifname[i][0]) {
			websWrite(
				wp,"1000");
		}
		websWrite(wp, "</td>\n");
	}
	websWrite(wp, "</tbody>\n");
	websWrite(wp, "</table>\n");
}
struct portcontext {
	char ifname[4][32];
	int count;
};

static void show_portif(webs_t wp, struct portcontext *ctx, char *ifname)
{
	if (!ctx->count) {
		memset(ctx->ifname, 0, sizeof(ctx->ifname));
	}
	strlcpy(ctx->ifname[ctx->count], ifname, 32);
	ctx->count++;
	if (ctx->count == 4) {
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
	memset(&ctx,0,sizeof(ctx));
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n", tran_string(buf, sizeof(buf), "status_band.h2"));
	getIfLists(eths, sizeof(eths));

	foreach(var, eths, next) {
		if (strstr(var, "lan") || strstr(var, "wan") || strstr(var, "eth"))
		show_portif(wp, &ctx, var);
	}
	if (ctx.count > 0) {
		show_portif_row(wp, ctx.ifname);
	}
	websWrite(wp, "</fieldset>\n");
}
