/* Shared library add-on to iptables to add multiple TCP port support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6t_multiport.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"multiport v%s options:\n"
" --source-ports port[,port,port...]\n"
" --sports ...\n"
"				match source port(s)\n"
" --destination-ports port[,port,port...]\n"
" --dports ...\n"
"				match destination port(s)\n"
" --ports port[,port,port]\n"
"				match both source and destination port(s)\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "source-ports", 1, 0, '1' },
	{ "sports", 1, 0, '1' }, /* synonym */
	{ "destination-ports", 1, 0, '2' },
	{ "dports", 1, 0, '2' }, /* synonym */
	{ "ports", 1, 0, '3' },
	{0}
};

static int
service_to_port(const char *name, const char *proto)
{
	struct servent *service;

	if ((service = getservbyname(name, proto)) != NULL)
		return ntohs((unsigned short) service->s_port);

		return -1;
}

static u_int16_t
parse_port(const char *port, const char *proto)
{
	unsigned int portnum;

	if ((string_to_number(port, 0, 65535, &portnum)) != -1 ||
	    (portnum = service_to_port(port, proto)) != -1)
		return (u_int16_t)portnum;

	exit_error(PARAMETER_PROBLEM,
		   "invalid port/service `%s' specified", port);
}

static unsigned int
parse_multi_ports(const char *portstring, u_int16_t *ports, const char *proto)
{
	char *buffer, *cp, *next;
	unsigned int i;

	buffer = strdup(portstring);
	if (!buffer) exit_error(OTHER_PROBLEM, "strdup failed");

	for (cp=buffer, i=0; cp && i<IP6T_MULTI_PORTS; cp=next,i++)
	{
		next=strchr(cp, ',');
		if (next) *next++='\0';
		ports[i] = parse_port(cp, proto);
	}
	if (cp) exit_error(PARAMETER_PROBLEM, "too many ports specified");
	free(buffer);
	return i;
}

/* Initialize the match. */
static void
init(struct ip6t_entry_match *m, unsigned int *nfcache)
{
}

static const char *
check_proto(const struct ip6t_entry *entry)
{
	if (entry->ipv6.proto == IPPROTO_TCP)
		return "tcp";
	else if (entry->ipv6.proto == IPPROTO_UDP)
		return "udp";
	else if (!entry->ipv6.proto)
		exit_error(PARAMETER_PROBLEM,
			   "multiport needs `-p tcp' or `-p udp'");
	else
		exit_error(PARAMETER_PROBLEM,
			   "multiport only works with TCP or UDP");
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      unsigned int *nfcache,
      struct ip6t_entry_match **match)
{
	const char *proto;
	struct ip6t_multiport *multiinfo
		= (struct ip6t_multiport *)(*match)->data;

	switch (c) {
	case '1':
		proto = check_proto(entry);
		multiinfo->count = parse_multi_ports(argv[optind-1],
						     multiinfo->ports, proto);
		multiinfo->flags = IP6T_MULTIPORT_SOURCE;
		*nfcache |= NFC_IP6_SRC_PT;
		break;

	case '2':
		proto = check_proto(entry);
		multiinfo->count = parse_multi_ports(argv[optind-1],
						     multiinfo->ports, proto);
		multiinfo->flags = IP6T_MULTIPORT_DESTINATION;
		*nfcache |= NFC_IP6_DST_PT;
		break;

	case '3':
		proto = check_proto(entry);
		multiinfo->count = parse_multi_ports(argv[optind-1],
						     multiinfo->ports, proto);
		multiinfo->flags = IP6T_MULTIPORT_EITHER;
		*nfcache |= NFC_IP6_SRC_PT | NFC_IP6_DST_PT;
		break;

	default:
		return 0;
	}

	if (*flags)
		exit_error(PARAMETER_PROBLEM,
			   "multiport can only have one option");
	*flags = 1;
	return 1;
}

/* Final check; must specify something. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM, "multiport expection an option");
}

static char *
port_to_service(int port, u_int8_t proto)
{
	struct servent *service;

	if ((service = getservbyport(htons(port),
				     proto == IPPROTO_TCP ? "tcp" : "udp")))
		return service->s_name;

	return NULL;
}

static void
print_port(u_int16_t port, u_int8_t protocol, int numeric)
{
	char *service;

	if (numeric || (service = port_to_service(port, protocol)) == NULL)
		printf("%u", port);
	else
		printf("%s", service);
}

/* Prints out the matchinfo. */
static void
print(const struct ip6t_ip6 *ip,
      const struct ip6t_entry_match *match,
      int numeric)
{
	const struct ip6t_multiport *multiinfo
		= (const struct ip6t_multiport *)match->data;
	unsigned int i;

	printf("multiport ");

	switch (multiinfo->flags) {
	case IP6T_MULTIPORT_SOURCE:
		printf("sports ");
		break;

	case IP6T_MULTIPORT_DESTINATION:
		printf("dports ");
		break;

	case IP6T_MULTIPORT_EITHER:
		printf("ports ");
		break;

	default:
		printf("ERROR ");
		break;
	}

	for (i=0; i < multiinfo->count; i++) {
		printf("%s", i ? "," : "");
		print_port(multiinfo->ports[i], ip->proto, numeric);
	}
	printf(" ");
}

/* Saves the union ip6t_matchinfo in parsable form to stdout. */
static void save(const struct ip6t_ip6 *ip, const struct ip6t_entry_match *match)
{
	const struct ip6t_multiport *multiinfo
		= (const struct ip6t_multiport *)match->data;
	unsigned int i;

	switch (multiinfo->flags) {
	case IP6T_MULTIPORT_SOURCE:
		printf("--sports ");
		break;

	case IP6T_MULTIPORT_DESTINATION:
		printf("--dports ");
		break;

	case IP6T_MULTIPORT_EITHER:
		printf("--ports ");
		break;
	}

	for (i=0; i < multiinfo->count; i++) {
		printf("%s", i ? "," : "");
		print_port(multiinfo->ports[i], ip->proto, 0);
	}
	printf(" ");
}

static
struct ip6tables_match multiport
= { NULL,
    "multiport",
    IPTABLES_VERSION,
    IP6T_ALIGN(sizeof(struct ip6t_multiport)),
    IP6T_ALIGN(sizeof(struct ip6t_multiport)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};

void
_init(void)
{
	register_match6(&multiport);
}
