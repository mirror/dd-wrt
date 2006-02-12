#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_mark_t.h>

static int mark_supplied;

#define MARK_TARGET  '1'
#define MARK_SETMARK '2'
static struct option opts[] =
{
	{ "mark-target" , required_argument, 0, MARK_TARGET },
	{ "set-mark"    , required_argument, 0, MARK_SETMARK },
	{ 0 }
};

static void print_help()
{
	printf(
	"mark target options:\n"
	" --set-mark value     : Set nfmark value\n"
	" --mark-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;

	markinfo->target = EBT_ACCEPT;
	markinfo->mark = 0;
	mark_supplied = 0;
}

#define OPT_MARK_TARGET   0x01
#define OPT_MARK_SETMARK  0x02
static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)(*target)->data;
	char *end;

	switch (c) {
	case MARK_TARGET:
		check_option(flags, OPT_MARK_TARGET);
		if (FILL_TARGET(optarg, markinfo->target))
			print_error("Illegal --mark-target target");
		break;
	case MARK_SETMARK:
		check_option(flags, OPT_MARK_SETMARK);
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			print_error("Bad MARK value '%s'", optarg);
		mark_supplied = 1;
                break;
	 default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;

	if (time == 0 && mark_supplied == 0)
		print_error("No mark value supplied");
	if (BASE_CHAIN && markinfo->target == EBT_RETURN)
		print_error("--mark-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;

	printf("--set-mark 0x%lx", markinfo->mark);
	if (markinfo->target == EBT_ACCEPT)
		return;
	printf(" --mark-target %s", TARGET_NAME(markinfo->target));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_mark_t_info *markinfo1 =
	   (struct ebt_mark_t_info *)t1->data;
	struct ebt_mark_t_info *markinfo2 =
	   (struct ebt_mark_t_info *)t2->data;

	return markinfo1->target == markinfo2->target &&
	   markinfo1->mark == markinfo2->mark;
}

static struct ebt_u_target mark_target =
{
	.name		= EBT_MARK_TARGET,
	.size		= sizeof(struct ebt_mark_t_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _init(void) __attribute__ ((constructor));
static void _init(void)
{
	register_target(&mark_target);
}
