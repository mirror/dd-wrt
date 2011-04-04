/* 
 * libxt_opendpi.c
 * Copyright (C) 2010 Gerardo E. Gidoni <gerel@gnu.org>
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
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
#include <xtables.h>

#include "xt_opendpi.h"

static char *prot_long_str[] = { IPOQUE_PROTOCOL_LONG_STRING };
static char *prot_short_str[] = { IPOQUE_PROTOCOL_SHORT_STRING };

static void 
opendpi_mt4_save(const void *entry, const struct xt_entry_match *match)
{
	const struct xt_opendpi_mtinfo *info = (const void *)match->data;
        int i;

        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        printf("--%s ", prot_short_str[i]);
                }
        }
}


static void 
opendpi_mt4_print(const void *entry, const struct xt_entry_match *match,
                  int numeric)
{
	const struct xt_opendpi_mtinfo *info = (const void *)match->data;
	int i;

        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        printf("protocol %s ", prot_long_str[i]);
                }
        }
}


static int 
opendpi_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const void *entry, struct xt_entry_match **match)
{
	struct xt_opendpi_mtinfo *info = (void *)(*match)->data;
        int i;

        *flags = 0;
        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                if (c == i){
                        IPOQUE_ADD_PROTOCOL_TO_BITMASK(info->flags, i);
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
opendpi_mt_check (unsigned int flags)
{
	if (flags == 0){
		xtables_error(PARAMETER_PROBLEM, "xt_opendpi: You need to "
                              "specify at least one protocol");
	}
}


static void
opendpi_mt_help(void)
{
        int i;

	printf("opendpi match options:\n");
        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                printf("--%s Match for %s protocol packets.\n",
                       prot_short_str[i], prot_long_str[i]);
        }
}


static void 
opendpi_mt_init (struct xt_entry_match *match)
{
	struct xt_opendpi_mtinfo *info = (void *)match->data;
	/* inet_pton(PF_INET, "192.0.2.137", &info->dst.in); */
}


static struct option opendpi_mt_opts[IPOQUE_MAX_SUPPORTED_PROTOCOLS+1];

static struct xtables_match
opendpi_mt4_reg = {
	.version = XTABLES_VERSION,
	.name = "opendpi",
	.revision = 0,
	.family = AF_INET,
	.size = XT_ALIGN(sizeof(struct xt_opendpi_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_opendpi_mtinfo)),
	.help = opendpi_mt_help,
	.init = opendpi_mt_init,
	.parse = opendpi_mt4_parse,
	.final_check = opendpi_mt_check,
	.print = opendpi_mt4_print,
	.save = opendpi_mt4_save,
	.extra_opts = opendpi_mt_opts,
};

void _init(void)
{
        int i;

        for (i = 0; i < IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                opendpi_mt_opts[i].name = prot_short_str[i+1];
                opendpi_mt_opts[i].has_arg = false;
                opendpi_mt_opts[i].val = i+1;
        }
        opendpi_mt_opts[i].name = NULL;
        opendpi_mt_opts[i].flag = NULL;
        opendpi_mt_opts[i].has_arg = 0;
        opendpi_mt_opts[i].val = 0;

	xtables_register_match(&opendpi_mt4_reg);
}
