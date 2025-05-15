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
#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <linux/mii.h>

static int mdio_read(int skfd, struct ifreq *ifr, int location)
{
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr->ifr_data;
	mii->reg_num = location;
	if (ioctl(skfd, SIOCGMIIREG, ifr) < 0) {
		fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr->ifr_name, strerror(errno));
		return -1;
	}
	return mii->val_out;
}

int getLanPortStatus(const char *ifname, struct portstatus *status)
{
	//fallback
	int skfd;
	struct ifreq ifr;
	unsigned bmcr, bmsr, advert, lkpar, bmcr2, lpa2;
	memset(&ifr, 0, sizeof(ifr));
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
		close(skfd);
		return -1;
	}
	bmcr = mdio_read(skfd, &ifr, MII_BMCR);
	bmsr = mdio_read(skfd, &ifr, MII_BMSR);
	advert = mdio_read(skfd, &ifr, MII_ADVERTISE);
	lkpar = mdio_read(skfd, &ifr, MII_LPA);
	bmcr2 = mdio_read(skfd, &ifr, MII_CTRL1000);
	lpa2 = mdio_read(skfd, &ifr, MII_STAT1000);
	status->speed = ((bmcr2 & (ADVERTISE_1000HALF | ADVERTISE_1000FULL)) & lpa2 >> 2) ? 1000 : (bmcr & BMCR_SPEED100) ? 100 : 10;
	status->fd = (bmcr & BMCR_FULLDPLX);
	status->link = (bmsr & BMSR_LSTATUS);
	close(skfd);
	char path[64];
	sprintf(path, "/sys/class/net/%s/speed", ifname);
	FILE *fp = fopen(path, "rb");
	if (fp) {
		char speed[64];
		fgets(speed, sizeof(speed), fp);
		status->speed = atoi(speed);
		fclose(fp);
	}
	return 0;
}

static void show_portif_row(webs_t wp, char ifname[4][32], int max)
{
	int i;
	struct portstatus status;
	websWrite(wp, "<table cellspacing=\"4\" summary=\"portif\" id=\"portif_table\" class=\"table\"><thead><tr>\n");
	for (i = 0; i < max; i++) {
		websWrite(wp, "<th width=\"6,25%%\">%s</th>\n", ifname[i][0] ? ifname[i] : "");
	}
	websWrite(wp, "</tr></thead>\n");
	websWrite(wp, "<tbody>\n");
	websWrite(wp, "<tr>\n");

	char buf[128];
	for (i = 0; i < max; i++) {
		if (ifname[i][0]) {
			int r = getLanPortStatus(ifname[i], &status);
			if (r) {
				websWrite(wp, "<td class=\"status_red center\">\n");
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
					websWrite(wp, "<td class=\"status_red center\">\n");
					websWrite(wp, "down");
				}
				websWrite(wp, "</td>\n");
			}
		}
	}
	websWrite(wp, "</tbody>\n");
	websWrite(wp, "</table>\n");
}
struct portcontext {
	char ifname[16][32];
	int count;
};

static void show_portif(webs_t wp, struct portcontext *ctx, char *ifname, int max)
{
	if (!ctx->count) {
		memset(ctx->ifname, 0, sizeof(ctx->ifname));
	}
	strlcpy(ctx->ifname[ctx->count], ifname, 32);
	ctx->count++;
	if (ctx->count == 16) {
		ctx->count = 0;
		show_portif_row(wp, ctx->ifname, max);
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
	int max = 0;
	foreach(var, eths, next) {
		if (!strncmp(var, "lan", 3) || !strncmp(var, "wan", 3) || !strncmp(var, "eth", 3))
			max++;
	}
	if (max > 16)
		max = 16;

	foreach(var, eths, next) {
		if (!strncmp(var, "lan", 3) || !strncmp(var, "wan", 3) || !strncmp(var, "eth", 3))
			show_portif(wp, &ctx, var, max);
	}
	if (ctx.count > 0) {
		show_portif_row(wp, ctx.ifname, max);
	}
	websWrite(wp, "</fieldset>\n");
}
