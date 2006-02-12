/* Shared library add-on to iptables to add ROUTE target support.
 * Author : Cédric de Launois, <delaunois@info.ucl.ac.be>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <net/if.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_ROUTE.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"ROUTE target v%s options:\n"
"  --iface   name                Send this packet directly through iface name.\n"
"  --ifindex index               Send this packet directly through iface index.\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "iface", 1, 0, '1' },
	{ "ifindex", 1, 0, '2' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

#define IPT_ROUTE_OPT_IF    0x01

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_route_target_info *route_info = 
		(struct ipt_route_target_info*)(*target)->data;

	switch (c) {
		char *end;
	case '1':
		if (*flags & IPT_ROUTE_OPT_IF)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --iface or --ifindex twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --iface");

		if (strlen(optarg) > sizeof(route_info->if_name) - 1)
			exit_error(PARAMETER_PROBLEM,
				   "Maximum interface name length %u",
				   sizeof(route_info->if_name) - 1);

		strcpy(route_info->if_name, optarg);
		route_info->if_index = 0;
		*flags |= IPT_ROUTE_OPT_IF;
		break;

	case '2':
		if (*flags & IPT_ROUTE_OPT_IF)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --iface or --ifindex twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --ifindex");

		route_info->if_name[0] = 0;
		route_info->if_index = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad ROUTE ifindex `%s'",
				   optarg);

		if (route_info->if_index == 0)
			exit_error(PARAMETER_PROBLEM,
				   "Interface index can't be 0 !");

		*flags |= IPT_ROUTE_OPT_IF;
		break;

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
		           "ROUTE target: Parameter --iface is required");
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_route_target_info *route_info
		= (const struct ipt_route_target_info *)target->data;

	printf("ROUTE ");

	if (route_info->if_name[0] != 0)
		printf("iface %s ", route_info->if_name);
	else 
		printf("ifindex %u ", route_info->if_index);
}

static
struct iptables_target route
= { NULL,
    "ROUTE",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_route_target_info)),
    IPT_ALIGN(sizeof(struct ipt_route_target_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    NULL, /* save */
    opts
};

void _init(void)
{
	register_target(&route);
}
