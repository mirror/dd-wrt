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
#include <xtables.h>

#include <linux/version.h>

#define NDPI_IPTABLES_EXT
#include "xt_ndpi.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

static char *prot_short_str[] = { NDPI_PROTOCOL_SHORT_STRING };

static void 
ndpi_mt4_save(const void *entry, const struct xt_entry_match *match)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
        int i,c,l,t;

        for (t = l = c = i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
		if(!strncmp(prot_short_str[i],"badproto_",9)) continue;
		if(i) t++; // skip unknown
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
			if(!l) l = i; c++;
		}
	}
	if(!c) return; // BUG?
	if( c == 1) {
		printf(" %s --%s ",info->invert ? "! ":"", prot_short_str[l]);
		return;
	}
	if( c == t ) { // all
		printf(" %s --all ",info->invert ? "! ":"");
		return;
	}
	printf(" %s --proto ",info->invert ? "! ":"" );
        for (l = i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
                if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
			printf("%s%s",l++ ? ",":"", prot_short_str[i]);
			
                }
        }
}


static void 
ndpi_mt4_print(const void *entry, const struct xt_entry_match *match,
                  int numeric)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
        int i,c,l,t;

        for (t = c = i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
		if(!strncmp(prot_short_str[i],"badproto_",9)) continue;
		if(i) t++; // skip unknown
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) c++;
	}
	if(!c) return;
	if( c == t ) {
		printf(" %sprotocol all",info->invert ? "! ":"");
		return;
	}

	printf(" %sprotocol%s ",info->invert ? "! ":"",c > 1 ? "s":"");

        for (l = i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
                if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
                        printf("%s%s",l++ ? ",":"", prot_short_str[i]);
                }
        }
}


static int 
ndpi_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const void *entry, struct xt_entry_match **match)
{
	struct xt_ndpi_mtinfo *info = (void *)(*match)->data;
        int i;

        *flags = 0;
	info->invert = invert;
	if(c == NDPI_LAST_IMPLEMENTED_PROTOCOL+1) {
		char *np = optarg,*n;
		int num;
		while((n = strtok(np,",")) != NULL) {
			num = -1;
			for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
			    if(!strcmp(prot_short_str[i],n)) {
				    num = i;
				    break; 
			    }
			}
			if(num < 0) {
				printf("Unknown proto '%s'\n",n);
				return false;
			}
			NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,num);
        		*flags |= 1;
			np = NULL;
		}
		return *flags != 0;
	}
	if(c == NDPI_LAST_IMPLEMENTED_PROTOCOL+2) { // all
		for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
	    	    if(strncmp(prot_short_str[i],"badproto_",9))
			NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,i);
		}
        	*flags |= 1;
		return true;
	}
	if(c > NDPI_LAST_IMPLEMENTED_PROTOCOL+2) {
		printf("BUG! c > NDPI_LAST_IMPLEMENTED_PROTOCOL+1\n");
		return false;
	}
        for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                if (c == i){
                        NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags, i);
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

	printf("ndpi match options:\n--all Match any known protocol\n");
        for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
	    if(strncmp(prot_short_str[i],"badproto_",9))
                printf("--%-16s (0x%x) Match for %s protocol packets.\n",
                       prot_short_str[i],i, prot_short_str[i]);
        }
}


static void 
ndpi_mt_init (struct xt_entry_match *match)
{
	/* struct xt_ndpi_mtinfo *info = (void *)match->data; */
	/* inet_pton(PF_INET, "192.0.2.137", &info->dst.in); */
}

/*
 * NDPI_LAST_IMPLEMENTED_PROTOCOL
 * + --unknown
 * + --all
 * + --proto lists_comma_separated
 * + NULL
 */
static struct option ndpi_mt_opts[NDPI_LAST_IMPLEMENTED_PROTOCOL+4];

static struct xtables_match
ndpi_mt4_reg = {
	.version = XTABLES_VERSION,
	.name = "ndpi",
	.revision = 0,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	.family = AF_INET,
#else
	.family = NFPROTO_UNSPEC,
#endif
	.size = XT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.help = ndpi_mt_help,
	.init = ndpi_mt_init,
	.parse = ndpi_mt4_parse,
	.final_check = ndpi_mt_check,
	.print = ndpi_mt4_print,
	.save = ndpi_mt4_save,
	.extra_opts = ndpi_mt_opts,
};
/* target NDPIMARK */
enum {
        O_SET_VALUE=0,
        O_SET_NDPI,
        O_SET_MARK,
        O_SET_CLSF,
        O_ACCEPT,
        F_SET_VALUE = 1 << O_SET_VALUE,
        F_SET_NDPI = 1 << O_SET_NDPI,
        F_SET_MARK = 1 << O_SET_MARK,
        F_SET_CLSF = 1 << O_SET_CLSF,
        F_ACCEPT = 1 << O_ACCEPT,
        F_ANY       = F_SET_VALUE | F_SET_NDPI |
			F_SET_MARK | F_SET_CLSF |
			F_ACCEPT,
};

static void NDPI_help(void)
{
        printf(
"NDPI target options:\n"
"  --value value/mask                  Set value = (value & ~mask) | value\n"
"  --ndpi-id                           Set value = (value & ~proto_mask) | proto_mark\n"
"  --set-mark                          Set nfmark = value\n"
"  --set-clsf                          Set priority = value\n"
"  --accept                            -j ACCEPT\n"
);
}

static const struct xt_option_entry NDPI_opts[] = {
        {.name = "value",    .id = O_SET_VALUE,.type = XTTYPE_MARKMASK32},
        {.name = "ndpi-id",  .id = O_SET_NDPI, .type = XTTYPE_NONE},
        {.name = "set-mark", .id = O_SET_MARK, .type = XTTYPE_NONE},
        {.name = "set-clsf", .id = O_SET_CLSF, .type = XTTYPE_NONE},
        {.name = "accept",   .id = O_ACCEPT,   .type = XTTYPE_NONE},
        XTOPT_TABLEEND,
};
static void NDPI_parse_v0(struct xt_option_call *cb)
{
        struct xt_ndpi_tginfo *markinfo = cb->data;

        xtables_option_parse(cb);
        switch (cb->entry->id) {
        case O_SET_VALUE:
                markinfo->mark = cb->val.mark;
                markinfo->mask = ~cb->val.mask;
                break;
	case O_SET_NDPI:
		markinfo->proto_id = 1;
		break;
	case O_SET_MARK:
		markinfo->t_mark = 1;
		break;
	case O_SET_CLSF:
		markinfo->t_clsf = 1;
		break;
	case O_ACCEPT:
		markinfo->t_accept = 1;
		break;
        default:
                xtables_error(PARAMETER_PROBLEM,
                           "NDPI target: unknown --%s",
                           cb->entry->name);
        }
}

static void NDPI_print_v0(const void *ip,
                          const struct xt_entry_target *target, int numeric)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
char buf[128];
int l;
        l = snprintf(buf,sizeof(buf)-1," NDPI");
	if(info->t_mark)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set MARK ");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set CLSF ");
	if(info->mask || info->mark) {
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1," and 0x%x or 0x%x",
				info->mask,info->mark);
	     else
	        l += snprintf(&buf[l],sizeof(buf)-l-1," set 0x%x", info->mark);
	}
	if(info->proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " and ~0xff or ndpi-id");
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," ACCEPT");
	printf(buf);
}

static void NDPI_save_v0(const void *ip, const struct xt_entry_target *target)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
	char buf[128];
	int l = 0;
	if(info->mask || info->mark) {
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --value 0x%x", info->mark);
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1,"/0x%x",~info->mask);
	}
	if(info->proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id");
	if(info->t_mark)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-mark");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-clsf");
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --accept");
	printf(buf);

}

static void NDPI_check(struct xt_fcheck_call *cb)
{
        if (!(cb->xflags & (F_SET_VALUE|F_SET_NDPI)))
                xtables_error(PARAMETER_PROBLEM,
                           "NDPI target: Parameter --set-mark or --ndpi-id"
                           " is required %x",cb->xflags);
}

static struct xtables_target ndpi_tg_reg[] = {
        {
                .family        = NFPROTO_UNSPEC,
                .name          = "NDPI",
                .version       = XTABLES_VERSION,
                .revision      = 0,
                .size          = XT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
                .userspacesize = XT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
                .help          = NDPI_help,
                .print         = NDPI_print_v0,
                .save          = NDPI_save_v0,
                .x6_parse      = NDPI_parse_v0,
                .x6_fcheck     = NDPI_check,
                .x6_options    = NDPI_opts,
        },
};

void _init(void)
{
        int i;

        for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                ndpi_mt_opts[i].name = prot_short_str[i];
                ndpi_mt_opts[i].flag = NULL;
		ndpi_mt_opts[i].has_arg = 0;
                ndpi_mt_opts[i].val = i;
        }
	ndpi_mt_opts[i].name = "proto";
	ndpi_mt_opts[i].flag = NULL;
	ndpi_mt_opts[i].has_arg = 1;
	ndpi_mt_opts[i].val = i;
	i++;
	ndpi_mt_opts[i].name = "all";
	ndpi_mt_opts[i].flag = NULL;
	ndpi_mt_opts[i].has_arg = 0;
	ndpi_mt_opts[i].val = i;
	i++;
	ndpi_mt_opts[i].name = NULL;
	ndpi_mt_opts[i].flag = NULL;
	ndpi_mt_opts[i].has_arg = 0;
	ndpi_mt_opts[i].val = 0;

	xtables_register_match(&ndpi_mt4_reg);
	xtables_register_targets(ndpi_tg_reg, ARRAY_SIZE(ndpi_tg_reg));
}
