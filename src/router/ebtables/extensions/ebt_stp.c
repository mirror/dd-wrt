#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_stp.h>

#define STP_TYPE	'a'
#define STP_FLAGS	'b'
#define STP_ROOTPRIO	'c'
#define STP_ROOTADDR	'd'
#define STP_ROOTCOST	'e'
#define STP_SENDERPRIO	'f'
#define STP_SENDERADDR	'g'
#define STP_PORT	'h'
#define STP_MSGAGE	'i'
#define STP_MAXAGE	'j'
#define STP_HELLOTIME	'k'
#define STP_FWDD	'l'
#define STP_NUMOPS 12

static struct option opts[] =
{
	{ "stp-type"         , required_argument, 0, STP_TYPE},
	{ "stp-flags"        , required_argument, 0, STP_FLAGS},
	{ "stp-root-prio"    , required_argument, 0, STP_ROOTPRIO},
	{ "stp-root-addr"    , required_argument, 0, STP_ROOTADDR},
	{ "stp-root-cost"    , required_argument, 0, STP_ROOTCOST},
	{ "stp-sender-prio"  , required_argument, 0, STP_SENDERPRIO},
	{ "stp-sender-addr"  , required_argument, 0, STP_SENDERADDR},
	{ "stp-port"         , required_argument, 0, STP_PORT},
	{ "stp-msg-age"      , required_argument, 0, STP_MSGAGE},
	{ "stp-max-age"      , required_argument, 0, STP_MAXAGE},
	{ "stp-hello-time"   , required_argument, 0, STP_HELLOTIME},
	{ "stp-forward-delay", required_argument, 0, STP_FWDD},
	{ 0 }
};

#define BPDU_TYPE_CONFIG 0
#define BPDU_TYPE_TCN 0x80
#define BPDU_TYPE_CONFIG_STRING "config"
#define BPDU_TYPE_TCN_STRING "tcn"

#define FLAG_TC 0x01
#define FLAG_TC_ACK 0x80
#define FLAG_TC_STRING "topology-change"
#define FLAG_TC_ACK_STRING "topology-change-ack"

static void print_help()
{
	printf(
"stp options:\n"
"--stp-type type                  : BPDU type\n"
"--stp-flags flag                 : control flag\n"
"--stp-root-prio prio[:prio]      : root priority (16-bit) range\n"
"--stp-root-addr address[/mask]   : MAC address of root\n"
"--stp-root-cost cost[:cost]      : root cost (32-bit) range\n"
"--stp-sender-prio prio[:prio]    : sender priority (16-bit) range\n"
"--stp-sender-addr address[/mask] : MAC address of sender\n"
"--stp-port port[:port]           : port id (16-bit) range\n"
"--stp-msg-age age[:age]          : message age timer (16-bit) range\n"
"--stp-max-age age[:age]          : maximum age timer (16-bit) range\n"
"--stp-hello-time time[:time]     : hello time timer (16-bit) range\n"
"--stp-forward-delay delay[:delay]: forward delay timer (16-bit) range\n"
" Recognized BPDU type strings:\n"
"   \"config\": configuration BPDU (=0)\n"
"   \"tcn\"   : topology change notification BPDU (=0x80)\n"
" Recognized control flag strings:\n"
"   \"topology-change\"    : topology change flag (0x01)\n"
"   \"topology-change-ack\": topology change acknowledgement flag (0x80)");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)match->data;

	stpinfo->invflags = 0;
	stpinfo->bitmask = 0;
}

/* defined in ebtables.c */
int get_mac_and_mask(char *from, char *to, char *mask);
void print_mac_and_mask(const char *mac, const char *mask);

#define determine_value(p,s)			 \
{						 \
	uint32_t _tmp2;				 \
	char *_tmp;				 \
	_tmp2 = strtoul(s, &_tmp, 0);		 \
	if (*_tmp != '\0')			 \
		return -1;			 \
	if (size == 2) {			 \
		if (_tmp2 >= (1 << 16))		 \
			return -1;		 \
		*(uint16_t *)p = (uint16_t)_tmp2;\
	} else					 \
		*(uint32_t *)p = _tmp2;		 \
}

static int parse_range(char *rangestring, void *lower, void *upper,
	int size)
{
	char *buffer, *cp;

	buffer = strdup(rangestring);
	if ((cp = strchr(buffer, ':')) == NULL) {
		determine_value(lower, buffer);
		determine_value(upper, buffer);
		return 0;
	}
	*cp = '\0';
	determine_value(lower, buffer);
	determine_value(upper, cp + 1);
	if (lower > upper)
		return -1;
	return 0;
}

static void print_range(uint32_t l, uint32_t u)
{
	if (l == u)
		printf("%d ", l);
	else
		printf("%d:%d ", l, u);
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)(*match)->data;
	unsigned int flag;
	long int i;
	char *end = NULL;

	if (c < 'a' || c > ('a' + STP_NUMOPS - 1))
		return 0;
	flag = 1 << (c - 'a');
	check_option(flags, flag);
	if (check_inverse(optarg))
		stpinfo->invflags |= flag;
	if (optind > argc)
		print_error("Missing argument for --%s", opts[c-'a'].name);
	stpinfo->bitmask |= flag;
	switch (flag) {
	case EBT_STP_TYPE:
		i = strtol(argv[optind - 1], &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(argv[optind - 1],
			    BPDU_TYPE_CONFIG_STRING))
				stpinfo->type = BPDU_TYPE_CONFIG;
			else if (!strcasecmp(argv[optind - 1],
			           BPDU_TYPE_TCN_STRING))
				stpinfo->type = BPDU_TYPE_TCN;
			else
				print_error("Bad STP type argument");
		} else
			stpinfo->type = i;
		break;
	case EBT_STP_FLAGS:
		i = strtol(argv[optind - 1], &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(argv[optind - 1],
			    FLAG_TC_STRING))
				stpinfo->config.flags = FLAG_TC;
			else if (!strcasecmp(argv[optind - 1],
			           FLAG_TC_ACK_STRING))
				stpinfo->config.flags = FLAG_TC_ACK;
			else
				print_error("Bad STP config flags argument");
		} else
			stpinfo->config.flags = i;
		break;
	case EBT_STP_ROOTPRIO:
		if (parse_range(argv[optind-1], &(stpinfo->config.root_priol),
		    &(stpinfo->config.root_priou), 2))
			print_error("Bad STP config root priority range");
		break;
	case EBT_STP_ROOTCOST:
		if (parse_range(argv[optind-1], &(stpinfo->config.root_costl),
		    &(stpinfo->config.root_costu), 4))
			print_error("Bad STP config root cost range");
		break;
	case EBT_STP_SENDERPRIO:
		if (parse_range(argv[optind-1], &(stpinfo->config.sender_priol),
		    &(stpinfo->config.sender_priou), 2))
			print_error("Bad STP config sender priority range");
		break;
	case EBT_STP_PORT:
		if (parse_range(argv[optind-1], &(stpinfo->config.portl),
		    &(stpinfo->config.portu), 2))
			print_error("Bad STP config port range");
		break;
	case EBT_STP_MSGAGE:
		if (parse_range(argv[optind-1], &(stpinfo->config.msg_agel),
		    &(stpinfo->config.msg_ageu), 2))
			print_error("Bad STP config message age range");
		break;
	case EBT_STP_MAXAGE:
		if (parse_range(argv[optind-1], &(stpinfo->config.max_agel),
		    &(stpinfo->config.max_ageu), 2))
			print_error("Bad STP config maximum age range");
		break;
	case EBT_STP_HELLOTIME:
		if (parse_range(argv[optind-1], &(stpinfo->config.hello_timel),
		    &(stpinfo->config.hello_timeu), 2))
			print_error("Bad STP config hello time range");
		break;
	case EBT_STP_FWDD:
		if (parse_range(argv[optind-1], &(stpinfo->config.forward_delayl),
		    &(stpinfo->config.forward_delayu), 2))
			print_error("Bad STP config forward delay range");
		break;
	case EBT_STP_ROOTADDR:
		if (get_mac_and_mask(argv[optind-1],
		    stpinfo->config.root_addr, stpinfo->config.root_addrmsk))
			print_error("Bad STP config root address");
		break;
	case EBT_STP_SENDERADDR:
		if (get_mac_and_mask(argv[optind-1], stpinfo->config.sender_addr,
		    stpinfo->config.sender_addrmsk))
			print_error("Bad STP config sender address");
		break;
	default:
		print_error("stp match: this shouldn't happen");
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
	uint8_t bridge_ula[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
	uint8_t msk[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (memcmp(entry->destmac, bridge_ula, 6) ||
	    memcmp(entry->destmsk, msk, 6))
		print_error("STP matching is only valid when the destination"
		            " MAC address is the bridge group address (BGA)"
		            " 01:80:c2:00:00:00");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)match->data;
	struct ebt_stp_config_info *c = &(stpinfo->config);
	int i;

	for (i = 0; i < STP_NUMOPS; i++) {
		if (!(stpinfo->bitmask & (1 << i)))
			continue;
		printf("--%s %s", opts[i].name,
		       (stpinfo->invflags & (1 << i)) ? "! " : "");
		if (EBT_STP_TYPE == (1 << i)) {
			if (stpinfo->type == BPDU_TYPE_CONFIG)
				printf("%s", BPDU_TYPE_CONFIG_STRING);
			else if (stpinfo->type == BPDU_TYPE_TCN)
				printf("%s", BPDU_TYPE_TCN_STRING);
			else
				printf("%d", stpinfo->type);
		} else if (EBT_STP_FLAGS == (1 << i)) {
			if (c->flags == FLAG_TC)
				printf("%s", FLAG_TC_STRING);
			else if (c->flags == FLAG_TC_ACK)
				printf("%s", FLAG_TC_ACK_STRING);
			else
				printf("%d", c->flags);
		} else if (EBT_STP_ROOTPRIO == (1 << i))
			print_range(c->root_priol, c->root_priou);
		else if (EBT_STP_ROOTADDR == (1 << i))
			print_mac_and_mask(c->root_addr, c->root_addrmsk);
		else if (EBT_STP_ROOTCOST == (1 << i))
			print_range(c->root_costl, c->root_costu);
		else if (EBT_STP_SENDERPRIO == (1 << i))
			print_range(c->sender_priol, c->sender_priou);
		else if (EBT_STP_SENDERADDR == (1 << i))
			print_mac_and_mask(c->sender_addr, c->sender_addrmsk);
		else if (EBT_STP_PORT == (1 << i))
			print_range(c->portl, c->portu);
		else if (EBT_STP_MSGAGE == (1 << i))
			print_range(c->msg_agel, c->msg_ageu);
		else if (EBT_STP_MAXAGE == (1 << i))
			print_range(c->max_agel, c->max_ageu);
		else if (EBT_STP_HELLOTIME == (1 << i))
			print_range(c->hello_timel, c->hello_timeu);
		else if (EBT_STP_FWDD == (1 << i))
			print_range(c->forward_delayl, c->forward_delayu);
		printf(" ");
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	return (!memcmp(m1->data, m2->data, sizeof(struct ebt_stp_info)));
}

static struct ebt_u_match stp_match =
{
	.name		= EBT_STP_MATCH,
	.size		= sizeof(struct ebt_stp_info),
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
	register_match(&stp_match);
}
