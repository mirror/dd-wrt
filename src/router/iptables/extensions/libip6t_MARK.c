/* Shared library add-on to iptables to add MARK target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
/* For 64bit kernel / 32bit userspace */
#include "../include/linux/netfilter_ipv4/ipt_MARK.h"

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MARK target v%s options:\n"
"  --set-mark value                   Set nfmark value\n"
"  --and-mark value                   Binary AND the nfmark with value\n"
"  --or-mark  value                   Binary OR  the nfmark with value\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "set-mark", 1, 0, '1' },
	{ "and-mark", 1, 0, '2' },
	{ "or-mark", 1, 0, '3' },
	{ "xor-mark", 1, 0, '4' },
	{ "set-xmark", 1, 0, '5' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ip6t_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse_v0(int c, char **argv, int invert, unsigned int *flags,
	 const struct ip6t_entry *entry,
	 struct ip6t_entry_target **target)
{
	struct ipt_mark_target_info *markinfo
		= (struct ipt_mark_target_info *)(*target)->data;

	switch (c) {
	case '1':
#ifdef KERNEL_64_USERSPACE_32
		if (string_to_number_ll(optarg, 0, 0, 
				     &markinfo->mark))
#else
		if (string_to_number_l(optarg, 0, 0, 
				     &markinfo->mark))
#endif
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "MARK target: Can't specify --set-mark twice");
		*flags = 1;
		break;
	case '2':
		exit_error(PARAMETER_PROBLEM,
			   "MARK target: kernel too old for --and-mark");
	case '3':
		exit_error(PARAMETER_PROBLEM,
			   "MARK target: kernel too old for --or-mark");
	default:
		return 0;
	}

	return 1;
}

static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
		           "MARK target: Parameter --set/and/or-mark"
			   " is required");
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse_v1(int c, char **argv, int invert, unsigned int *flags,
	 const struct ip6t_entry *entry,
	 struct ip6t_entry_target **target)
{
	struct ipt_mark_target_info_v1 *markinfo
		= (struct ipt_mark_target_info_v1 *)(*target)->data;

	switch (c) {
	case '1':
	        markinfo->mode = IPT_MARK_SET;
		break;
	case '2':
	        markinfo->mode = IPT_MARK_AND;
		break;
	case '3':
	        markinfo->mode = IPT_MARK_OR;
		break;
	default:
		return 0;
	}

#ifdef KERNEL_64_USERSPACE_32
	if (string_to_number_ll(optarg, 0, 0,  &markinfo->mark))
#else
	if (string_to_number_l(optarg, 0, 0, &markinfo->mark))
#endif
		exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);

	if (*flags)
		exit_error(PARAMETER_PROBLEM,
			   "MARK target: Can't specify --set-mark twice");

	*flags = 1;
	return 1;
}


static int
parse_v2(int c, char **argv, int invert, unsigned int *flags,
	 const struct ip6t_entry *entry,
	 struct ip6t_entry_target **target)
{
	struct xt_mark_tginfo2 *markinfo
		= (struct xt_mark_tginfo2 *)(*target)->data;

	char *end;
	unsigned long mark = 0;
	unsigned long mask = ~0U;
	unsigned long val;
	mark = strtoul(optarg, &end, 0);
	if (end == optarg)
		exit_error(PARAMETER_PROBLEM, "Bad MARK value (mask) `%s' %d", optarg,mark);
		
	if (*end == '/') {
	    mask = strtoul(end+1,&end,0);
	    }
	if (*end != '\0')
		exit_error(PARAMETER_PROBLEM, "Bad MARK value (garbage at end) `%s'", optarg);
	
	string_to_number_l(optarg, 0, 0, &val);

	switch (c) {
	case '1':
	        markinfo->mark = mark;
	        markinfo->mask = mark | mask;
		break;
	case '2':
	        markinfo->mark = 0;
	        markinfo->mask = ~val;
		break;
	case '3':
	        markinfo->mark = markinfo->mask = val;
		break;
	case '4':
	        markinfo->mark = val;
	        markinfo->mask = 0;
	        break;
	case '5':
	        markinfo->mark = mark;
	        markinfo->mask = mask;
		break;
	default:
		return 0;
	}

	if (*flags)
		exit_error(PARAMETER_PROBLEM,
			   "MARK target: Can't specify --set-mark twice");

	*flags = 1;
	return 1;
}


#ifdef KERNEL_64_USERSPACE_32
static void
print_mark(unsigned long long mark)
{
	printf("0x%llx ", mark);
}
#else
static void
print_mark(unsigned long mark)
{
	printf("0x%lx ", mark);
}
#endif

/* Prints out the targinfo. */
static void
print_v0(const struct ip6t_ip6 *ip,
	 const struct ip6t_entry_target *target,
	 int numeric)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;
	printf("MARK set ");
	print_mark(markinfo->mark);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save_v0(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;

	printf("--set-mark ");
	print_mark(markinfo->mark);
}

/* Prints out the targinfo. */
static void
print_v1(const struct ip6t_ip6 *ip,
	 const struct ip6t_entry_target *target,
	 int numeric)
{
	const struct ipt_mark_target_info_v1 *markinfo =
		(const struct ipt_mark_target_info_v1 *)target->data;

	switch (markinfo->mode) {
	case IPT_MARK_SET:
		printf("MARK set ");
		break;
	case IPT_MARK_AND:
		printf("MARK and ");
		break;
	case IPT_MARK_OR: 
		printf("MARK or ");
		break;
	}
	print_mark(markinfo->mark);
}

static void
print_v2(const struct ip6t_ip6 *ip,
	 const struct ip6t_entry_target *target,
	 int numeric)
{
	const struct xt_mark_tginfo2 *info = (const struct xt_mark_tginfo2 *)target->data;

	if (info->mark == 0)
		printf(" MARK and 0x%x", (unsigned int)(uint32_t)~info->mask);
	else if (info->mark == info->mask)
		printf(" MARK or 0x%x", info->mark);
	else if (info->mask == 0)
		printf(" MARK xor 0x%x", info->mark);
	else if (info->mask == 0xffffffffU)
		printf(" MARK set 0x%x", info->mark);
	else
		printf(" MARK xset 0x%x/0x%x", info->mark, info->mask);
}


/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save_v1(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct ipt_mark_target_info_v1 *markinfo =
		(const struct ipt_mark_target_info_v1 *)target->data;

	switch (markinfo->mode) {
	case IPT_MARK_SET:
		printf("--set-mark ");
		break;
	case IPT_MARK_AND:
		printf("--and-mark ");
		break;
	case IPT_MARK_OR: 
		printf("--or-mark ");
		break;
	}
	print_mark(markinfo->mark);
}

static void
save_v2(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct xt_mark_tginfo2 *info = (const struct xt_mark_tginfo2 *)target->data;

	printf(" --set-xmark 0x%x/0x%x", info->mark, info->mask);
}

static
struct ip6tables_target mark_v0 = {
	.next		= NULL,
	.name		= "MARK",
	.version	= IPTABLES_VERSION,
	.revision	= 0,
	.size		= IP6T_ALIGN(sizeof(struct ipt_mark_target_info)),
	.userspacesize	= IP6T_ALIGN(sizeof(struct ipt_mark_target_info)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse_v0,
	.final_check	= &final_check,
	.print		= &print_v0,
	.save		= &save_v0,
	.extra_opts	= opts
};

static
struct ip6tables_target mark_v1 = {
	.next		= NULL,
	.name		= "MARK",
	.version	= IPTABLES_VERSION,
	.revision	= 1,
	.size		= IP6T_ALIGN(sizeof(struct ipt_mark_target_info_v1)),
	.userspacesize	= IP6T_ALIGN(sizeof(struct ipt_mark_target_info_v1)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse_v1,
	.final_check	= &final_check,
	.print		= &print_v1,
	.save		= &save_v1,
	.extra_opts	= opts
};

static
struct ip6tables_target mark_v2= {
	.next		= NULL,
	.name		= "MARK",
	.version	= IPTABLES_VERSION,
	.revision	= 2,
	.size           = IP6T_ALIGN(sizeof(struct xt_mark_tginfo2)),
	.userspacesize  = IP6T_ALIGN(sizeof(struct xt_mark_tginfo2)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse_v2,
	.final_check	= &final_check,
	.print		= &print_v2,
	.save		= &save_v2,
	.extra_opts	= opts
};

void _init(void)
{
	register_target6(&mark_v0);
	register_target6(&mark_v1);
	register_target6(&mark_v2);
}
