/*
 * Copyright (c) 2005 Evgeniy Polyakov <johnpol@2ka.mxt.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * Based on iptables/utils/nfnl_osf.c.
 */

#include <nft.h>

#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <linux/unistd.h>

#include <libmnl/libmnl.h>

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_osf.h>
#include <mnl.h>
#include <osf.h>

#define OPTDEL			','
#define OSFPDEL 		':'
#define MAXOPTSTRLEN		128

static struct nf_osf_opt IANA_opts[] = {
	{ .kind = 0, .length = 1,},
	{ .kind=1, .length=1,},
	{ .kind=2, .length=4,},
	{ .kind=3, .length=3,},
	{ .kind=4, .length=2,},
	{ .kind=5, .length=1,},		/* SACK length is not defined */
	{ .kind=6, .length=6,},
	{ .kind=7, .length=6,},
	{ .kind=8, .length=10,},
	{ .kind=9, .length=2,},
	{ .kind=10, .length=3,},
	{ .kind=11, .length=1,},		/* CC: Suppose 1 */
	{ .kind=12, .length=1,},		/* the same */
	{ .kind=13, .length=1,},		/* and here too */
	{ .kind=14, .length=3,},
	{ .kind=15, .length=1,},		/* TCP Alternate Checksum Data. Length is not defined */
	{ .kind=16, .length=1,},
	{ .kind=17, .length=1,},
	{ .kind=18, .length=3,},
	{ .kind=19, .length=18,},
	{ .kind=20, .length=1,},
	{ .kind=21, .length=1,},
	{ .kind=22, .length=1,},
	{ .kind=23, .length=1,},
	{ .kind=24, .length=1,},
	{ .kind=25, .length=1,},
	{ .kind=26, .length=1,},
};

static char *nf_osf_strchr(char *ptr, char c)
{
	char *tmp;

	tmp = strchr(ptr, c);
	if (tmp)
		*tmp = '\0';

	while (tmp && isspace(*(tmp + 1)))
		tmp++;

	return tmp;
}

static void nf_osf_parse_opt(struct nf_osf_opt *opt, __u16 *optnum, char *obuf,
			     int olen)
{
	int i, op;
	char *ptr, wc;
	unsigned long val;

	ptr = &obuf[0];
	i = 0;
	while (ptr != NULL && i < olen && *ptr != 0) {
		val = 0;
		wc = OSF_WSS_PLAIN;
		switch (obuf[i]) {
		case 'N':
			op = OSFOPT_NOP;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				*ptr = '\0';
				ptr++;
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		case 'S':
			op = OSFOPT_SACKP;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				*ptr = '\0';
				ptr++;
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		case 'T':
			op = OSFOPT_TS;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				*ptr = '\0';
				ptr++;
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		case 'W':
			op = OSFOPT_WSO;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				switch (obuf[i + 1]) {
				case '%':
					wc = OSF_WSS_MODULO;
					break;
				case 'S':
					wc = OSF_WSS_MSS;
					break;
				case 'T':
					wc = OSF_WSS_MTU;
					break;
				default:
					wc = OSF_WSS_PLAIN;
					break;
				}

				*ptr = '\0';
				ptr++;
				if (wc)
					val = strtoul(&obuf[i + 2], NULL, 10);
				else
					val = strtoul(&obuf[i + 1], NULL, 10);
				i += (int)(ptr - &obuf[i]);

			} else
				i++;
			break;
		case 'M':
			op = OSFOPT_MSS;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				if (obuf[i + 1] == '%')
					wc = OSF_WSS_MODULO;
				*ptr = '\0';
				ptr++;
				if (wc)
					val = strtoul(&obuf[i + 2], NULL, 10);
				else
					val = strtoul(&obuf[i + 1], NULL, 10);
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		case 'E':
			op = OSFOPT_EOL;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				*ptr = '\0';
				ptr++;
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		default:
			op = OSFOPT_EMPTY;
			ptr = nf_osf_strchr(&obuf[i], OPTDEL);
			if (ptr) {
				ptr++;
				i += (int)(ptr - &obuf[i]);
			} else
				i++;
			break;
		}

		if (op != OSFOPT_EMPTY) {
			opt[*optnum].kind = IANA_opts[op].kind;
			opt[*optnum].length = IANA_opts[op].length;
			opt[*optnum].wc.wc = wc;
			opt[*optnum].wc.val = val;
			(*optnum)++;
		}
	}
}

static int osf_load_line(char *buffer, int len, int del,
			 struct netlink_ctx *ctx)
{
	int i, cnt = 0;
	char obuf[MAXOPTSTRLEN];
	struct nf_osf_user_finger f;
	char *pbeg, *pend;
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfg;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	memset(&f, 0, sizeof(struct nf_osf_user_finger));

	if (ctx->nft->debug_mask & NFT_DEBUG_MNL)
		nft_print(&ctx->nft->output, "Loading '%s'.\n", buffer);

	for (i = 0; i < len && buffer[i] != '\0'; ++i) {
		if (buffer[i] == ':')
			cnt++;
	}

	if (cnt != 8) {
		if (ctx->nft->debug_mask & NFT_DEBUG_MNL)
			nft_print(&ctx->nft->output, "Wrong input line '%s': cnt: %d, must be 8, i: %d, must be %d.\n", buffer, cnt, i, len);
		return -EINVAL;
	}

	memset(obuf, 0, sizeof(obuf));

	pbeg = buffer;
	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		if (pbeg[0] == 'S') {
			f.wss.wc = OSF_WSS_MSS;
			if (pbeg[1] == '%')
				f.wss.val = strtoul(&pbeg[2], NULL, 10);
			else if (pbeg[1] == '*')
				f.wss.val = 0;
			else
				f.wss.val = strtoul(&pbeg[1], NULL, 10);
		} else if (pbeg[0] == 'T') {
			f.wss.wc = OSF_WSS_MTU;
			if (pbeg[1] == '%')
				f.wss.val = strtoul(&pbeg[2], NULL, 10);
			else if (pbeg[1] == '*')
				f.wss.val = 0;
			else
				f.wss.val = strtoul(&pbeg[1], NULL, 10);
		} else if (pbeg[0] == '%') {
			f.wss.wc = OSF_WSS_MODULO;
			f.wss.val = strtoul(&pbeg[1], NULL, 10);
		} else if (isdigit(pbeg[0])) {
			f.wss.wc = OSF_WSS_PLAIN;
			f.wss.val = strtoul(&pbeg[0], NULL, 10);
		}

		pbeg = pend + 1;
	}
	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		f.ttl = strtoul(pbeg, NULL, 10);
		pbeg = pend + 1;
	}
	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		f.df = strtoul(pbeg, NULL, 10);
		pbeg = pend + 1;
	}
	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		f.ss = strtoul(pbeg, NULL, 10);
		pbeg = pend + 1;
	}

	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		i = sizeof(obuf);
		snprintf(obuf, i, "%.*s,", i - 2, pbeg);
		pbeg = pend + 1;
	}

	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		i = sizeof(f.genre);
		if (pbeg[0] == '@' || pbeg[0] == '*')
			pbeg++;
		snprintf(f.genre, i, "%.*s", i - 1, pbeg);
		pbeg = pend + 1;
	}

	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		i = sizeof(f.version);
		snprintf(f.version, i, "%.*s", i - 1, pbeg);
		pbeg = pend + 1;
	}

	pend = nf_osf_strchr(pbeg, OSFPDEL);
	if (pend) {
		*pend = '\0';
		i = sizeof(f.subtype);
		snprintf(f.subtype, i, "%.*s", i - 1, pbeg);
		pbeg = pend + 1;
	}

	nf_osf_parse_opt(f.opt, &f.opt_num, obuf, sizeof(obuf));

	memset(buf, 0, sizeof(buf));

	if (del) {
		nlh = mnl_nlmsg_put_header(buf);
		nlh->nlmsg_type = (NFNL_SUBSYS_OSF << 8) | OSF_MSG_REMOVE;
		nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
		nlh->nlmsg_seq = ctx->seqnum;

		nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
		nfg->nfgen_family = AF_UNSPEC;
		nfg->version = NFNETLINK_V0;
		nfg->res_id = 0;
	} else {
		nlh = mnl_nlmsg_put_header(buf);
		nlh->nlmsg_type = (NFNL_SUBSYS_OSF << 8) | OSF_MSG_ADD;
		nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
		nlh->nlmsg_seq = ctx->seqnum;

		nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
		nfg->nfgen_family = AF_UNSPEC;
		nfg->version = NFNETLINK_V0;
		nfg->res_id = 0;

		mnl_attr_put(nlh, OSF_ATTR_FINGER, sizeof(struct nf_osf_user_finger), &f);
	}

	return nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, 0, NULL);
}

#define OS_SIGNATURES DEFAULT_INCLUDE_PATH "/nftables/osf/pf.os"

int nfnl_osf_load_fingerprints(struct netlink_ctx *ctx, int del)
{
	FILE *inf;
	int err = 0;
	char buf[1024];

	if (ctx->nft->debug_mask & NFT_DEBUG_MNL)
		nft_print(&ctx->nft->output, "Opening OS signature file '%s'\n",
			  OS_SIGNATURES);

	inf = fopen(OS_SIGNATURES, "r");
	if (!inf) {
		if (ctx->nft->debug_mask & NFT_DEBUG_MNL)
			nft_print(&ctx->nft->output, "Failed to open file '%s'\n",
				  OS_SIGNATURES);

		return -1;
	}

	while (fgets(buf, sizeof(buf), inf)) {
		int len;

		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
			continue;

		len = strlen(buf) - 1;

		if (len <= 0)
			continue;

		buf[len] = '\0';

		err = osf_load_line(buf, len, del, ctx);
		if (err)
			break;

		memset(buf, 0, sizeof(buf));
	}

	fclose(inf);
	return err;
}
