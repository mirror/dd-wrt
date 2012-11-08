/* 
 * libxt_ndpi.c
 * Copyright (C) 2010-2012 G. Elian Gidoni <geg@gnu.org>
 *               2012 Ed Wildgoose <lists@wildgooses.com>
 * 
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <iptables.h>
#include "ndpi_define.h"
#include "ndpi_macros.h"
#include "ndpi_protocols_osdpi.h"
#include "ndpi_api.h"
#include "ndpi_protocol_history.h"
#include "ndpi_structs.h"

//#include <linux/version.h>

#include "xt_ndpi.h"


#define true 1
#define false 0

static char *prot_long_str[] = { NDPI_PROTOCOL_LONG_STRING };
static char *prot_short_str[] = { NDPI_PROTOCOL_SHORT_STRING };

static void 
ndpi_mt4_save(const struct ipt_ip *entry, const struct ipt_entry_match *match)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
        int i;

        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        printf("--%s ", prot_short_str[i]);
                }
        }
}


static void 
ndpi_mt4_print(const struct ipt_ip *entry, const struct ipt_entry_match *match,
                  int numeric)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
	int i;

        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        printf("protocol %s ", prot_long_str[i]);
                }
        }
}


static int 
ndpi_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const struct ipt_entry *entry,unsigned int *nfcache, struct ipt_entry_match **match)
{
	struct xt_ndpi_mtinfo *info = (void *)(*match)->data;
        int i;

        *flags = 0;
        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                if (c == i){
                        NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags, i);
                        /*printf("Parameter detected as protocol %s.\n",
                          prot_long_str[i]);*/
                        *flags = 1;
                        return true;
                }
        }

	return false;
}

#ifndef xtables_error
#define xtables_error exit_error
#endif

static void
ndpi_mt_check (unsigned int flags)
{
	if (flags == 0){
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: You need to "
                              "specify at least one protocol");
	}
}


static void
ndpi_mt_help(void)
{
        int i;

	printf("ndpi match options:\n");
        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                printf("--%s Match for %s protocol packets.\n",
                       prot_short_str[i], prot_long_str[i]);
        }
}


static void 
ndpi_mt_init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	struct xt_ndpi_mtinfo *info = (void *)m->data;
	/* inet_pton(PF_INET, "192.0.2.137", &info->dst.in); */
}


static struct option ndpi_mt_opts[NDPI_LAST_IMPLEMENTED_PROTOCOL+1];

static struct iptables_match
ndpi_mt4_reg = {
	.version = IPTABLES_VERSION,
	.name = "ndpi",
	.revision = 0,
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
//	.family = AF_INET,
//#else
//	.family = NFPROTO_IPV4,
//#endif
	.size = IPT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.userspacesize = IPT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
//	.help = ndpi_mt_help,
	.init = ndpi_mt_init,
	.parse = ndpi_mt4_parse,
	.final_check = ndpi_mt_check,
	.print = ndpi_mt4_print,
	.save = ndpi_mt4_save,
	.extra_opts = ndpi_mt_opts,
};

void _init(void)
{
        int i;

        for (i = 0; i < NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                ndpi_mt_opts[i].name = prot_short_str[i+1];
                ndpi_mt_opts[i].has_arg = false;
                ndpi_mt_opts[i].val = i+1;
        }
        ndpi_mt_opts[i].name = NULL;
        ndpi_mt_opts[i].flag = NULL;
        ndpi_mt_opts[i].has_arg = 0;
        ndpi_mt_opts[i].val = 0;

	register_match(&ndpi_mt4_reg);
}
