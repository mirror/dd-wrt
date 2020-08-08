/* 802_3
 *
 * Author:
 * Chris Vitale <csv@bluetail.com>
 *
 * May 2003
 *
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_802_3.h>

#define _802_3_SAP	'1'
#define _802_3_TYPE	'2'

static const struct option br802_3_opts[] = {
	{ .name = "802_3-sap",	.has_arg = true, .val = _802_3_SAP },
	{ .name = "802_3-type",	.has_arg = true, .val = _802_3_TYPE },
	XT_GETOPT_TABLEEND,
};

static void br802_3_print_help(void)
{
	printf(
"802_3 options:\n"
"--802_3-sap [!] protocol       : 802.3 DSAP/SSAP- 1 byte value (hex)\n"
"  DSAP and SSAP are always the same.  One SAP applies to both fields\n"
"--802_3-type [!] protocol      : 802.3 SNAP Type- 2 byte value (hex)\n"
"  Type implies SAP value 0xaa\n");
}

static void br802_3_init(struct xt_entry_match *match)
{
	struct ebt_802_3_info *info = (struct ebt_802_3_info *)match->data;

	info->invflags = 0;
	info->bitmask = 0;
}

static int
br802_3_parse(int c, char **argv, int invert, unsigned int *flags,
	      const void *entry, struct xt_entry_match **match)
{
	struct ebt_802_3_info *info = (struct ebt_802_3_info *) (*match)->data;
	unsigned int i;
	char *end;

	switch (c) {
	case _802_3_SAP:
		if (invert)
			info->invflags |= EBT_802_3_SAP;
		i = strtoul(optarg, &end, 16);
		if (i > 255 || *end != '\0')
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with specified "
					"sap hex value, %x",i);
		info->sap = i; /* one byte, so no byte order worries */
		info->bitmask |= EBT_802_3_SAP;
		break;
	case _802_3_TYPE:
		if (invert)
			info->invflags |= EBT_802_3_TYPE;
		i = strtoul(optarg, &end, 16);
		if (i > 65535 || *end != '\0') {
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with the specified "
					"type hex value, %x",i);
		}
		info->type = htons(i);
		info->bitmask |= EBT_802_3_TYPE;
		break;
	default:
		return 0;
	}

	*flags |= info->bitmask;
	return 1;
}

static void
br802_3_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void br802_3_print(const void *ip, const struct xt_entry_match *match,
			  int numeric)
{
	struct ebt_802_3_info *info = (struct ebt_802_3_info *)match->data;

	if (info->bitmask & EBT_802_3_SAP) {
		printf("--802_3-sap ");
		if (info->invflags & EBT_802_3_SAP)
			printf("! ");
		printf("0x%.2x ", info->sap);
	}
	if (info->bitmask & EBT_802_3_TYPE) {
		printf("--802_3-type ");
		if (info->invflags & EBT_802_3_TYPE)
			printf("! ");
		printf("0x%.4x ", ntohs(info->type));
	}
}

static struct xtables_match br802_3_match =
{
	.name		= "802_3",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_802_3_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_802_3_info)),
	.init		= br802_3_init,
	.help		= br802_3_print_help,
	.parse		= br802_3_parse,
	.final_check	= br802_3_final_check,
	.print		= br802_3_print,
	.extra_opts	= br802_3_opts,
};

void _init(void)
{
	xtables_register_match(&br802_3_match);
}
