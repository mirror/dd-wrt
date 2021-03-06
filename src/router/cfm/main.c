// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <stdio.h>
#include <stdbool.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <ev.h>
#include <fcntl.h>
#include <getopt.h>
#include <net/if.h>

#include "cfm_netlink.h"
#include "libnetlink.h"
#include <linux/cfm_bridge.h>

static void incomplete_command(void)
{
	fprintf(stderr, "Command line is not complete. Try option \"help\"\n");
	exit(-1);
}

#define NEXT_ARG() do { argv++; if (--argc <= 0) incomplete_command(); } while(0)
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static enum br_cfm_domain domain_int(char *arg)
{
	if (strcmp(arg, "port") == 0)
		return BR_CFM_PORT;
	if (strcmp(arg, "vlan") == 0)
		return BR_CFM_VLAN;
	return -1;
}

static enum br_cfm_mep_direction direction_int(char *arg)
{
	if (strcmp(arg, "down") == 0)
		return BR_CFM_MEP_DIRECTION_DOWN;
	if (strcmp(arg, "up") == 0)
		return BR_CFM_MEP_DIRECTION_UP;
	return -1;
}

static enum br_cfm_ccm_interval interval_int(char *arg)
{
	if (strcmp(arg, "3ms3") == 0)
		return BR_CFM_CCM_INTERVAL_3_3_MS;
	if (strcmp(arg, "10ms") == 0)
		return BR_CFM_CCM_INTERVAL_10_MS;
	if (strcmp(arg, "100ms") == 0)
		return BR_CFM_CCM_INTERVAL_100_MS;
	if (strcmp(arg, "1s") == 0)
		return BR_CFM_CCM_INTERVAL_1_SEC;
	if (strcmp(arg, "10s") == 0)
		return BR_CFM_CCM_INTERVAL_10_SEC;
	if (strcmp(arg, "1m") == 0)
		return BR_CFM_CCM_INTERVAL_1_MIN;
	if (strcmp(arg, "10m") == 0)
		return BR_CFM_CCM_INTERVAL_10_MIN;
	return -1;
}

static struct mac_addr mac_array(char *arg)
{
	struct mac_addr mac;
	int i, idx;

	/* Expected format XX-XX-XX-XX-XX-XX string where XX is hexadecimal */
	for (i = 0, idx = 0; i < 17; ++i) {
		if ((i % 3) == 0) {
			arg[i + 2] = 0;	/* '-' is changed to '0' to create XX string */
			mac.addr[idx] = (unsigned char)strtol(&arg[i], NULL, 16);
			idx ++;
		}
	}

	return mac;
}

static struct maid_data maid_array(char *domain_name, char *short_name)
{
	struct maid_data maid;
	int domain_name_len = 0;
	int short_name_len = 0;
	int maid_idx = 0;

	memset(maid.data, 0, sizeof(maid));

	if (domain_name != NULL) 
		domain_name_len = strlen(domain_name);
	if (short_name != NULL) 
		short_name_len = strlen(short_name);

	if (domain_name_len) {
		maid.data[0] = 4; /* Maintenance Domain Name Format field - Character string */
		maid.data[1] = domain_name_len;
		memcpy(&maid.data[2], domain_name, domain_name_len);
		maid_idx = 2 + domain_name_len;
	} else {
		maid.data[0] = 1; /* Maintenance Domain Name Format field - No Maintenance Domain Name present */
		maid_idx = 1;
	}

	maid.data[maid_idx + 0] = 2; /* Short MA Name Format - Character string */
	maid.data[maid_idx + 1] = short_name_len;
	if (short_name_len) {
		memcpy(&maid.data[maid_idx + 2], short_name, short_name_len);
	}

	return maid;
}

static int cmd_mep_create(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, port_ifindex = 0, instance = 0, domain = 0, direction = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "domain") == 0) {
			NEXT_ARG();
			domain = domain_int(*argv);
		} else if (strcmp(*argv, "direction") == 0) {
			NEXT_ARG();
			direction = direction_int(*argv);
		} else if (strcmp(*argv, "port") == 0) {
			NEXT_ARG();
			port_ifindex = if_nametoindex(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0 || instance == 0 || port_ifindex == 0)
		return -1;

	if (domain == -1 || direction == -1)
		return -1;

	return cfm_offload_mep_create(br_ifindex, instance, domain, direction, port_ifindex);
}

static int cmd_mep_delete(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, instance = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0 || instance == 0)
		return -1;

	return cfm_offload_mep_delete(br_ifindex, instance);
}

static int cmd_mep_config(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, level = 0, mepid = 0, instance = 0;
	struct mac_addr mac;

	memset(&mac, 0, sizeof(mac));

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "mac") == 0) {
			NEXT_ARG();
			if (strlen(*argv) != 17)	/* Must be 17 characters to be XX-XX-XX-XX-XX-XX */
				return -1;
			mac = mac_array(*argv);
		} else if (strcmp(*argv, "level") == 0) {
			NEXT_ARG();
			level = atoi(*argv);
		} else if (strcmp(*argv, "mepid") == 0) {
			NEXT_ARG();
			mepid = atoi(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0 || instance == 0)
		return -1;

	return cfm_offload_mep_config(br_ifindex, instance, &mac, level, mepid);
}

static int cmd_cc_config(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, enable = 0, interval = 0, instance = 0;
	struct maid_data maid;
	char *domain_name = NULL;
	char *short_name = NULL;

	memset(&maid, 0, sizeof(maid));

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "enable") == 0) {
			NEXT_ARG();
			enable = atoi(*argv);
		} else if (strcmp(*argv, "interval") == 0) {
			NEXT_ARG();
			interval = interval_int(*argv);
		} else if (strcmp(*argv, "domain-name") == 0) {
			NEXT_ARG();
			domain_name = *argv;
		} else if (strcmp(*argv, "short-name") == 0) {
			NEXT_ARG();
			short_name = *argv;
		} else
			return -1;

		argc--; argv++;
	}

	maid = maid_array(domain_name, short_name);

	if (br_ifindex == 0 || instance == 0)
		return -1;

	if (interval == -1)
		return -1;

	return cfm_offload_cc_config(br_ifindex, instance, enable, interval, &maid);
}

static int cmd_cc_rdi(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, rdi = 0, instance = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "rdi") == 0) {
			NEXT_ARG();
			rdi = atoi(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0 || instance == 0)
		return -1;

	return cfm_offload_cc_rdi(br_ifindex, instance, rdi);
}

static int cmd_cc_peer(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, remove = 0, mepid = 0, instance = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "remove") == 0) {
			NEXT_ARG();
			remove = atoi(*argv);
		} else if (strcmp(*argv, "mepid") == 0) {
			NEXT_ARG();
			mepid = atoi(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0 || instance == 0)
		return -1;

	return cfm_offload_cc_peer(br_ifindex, instance, remove, mepid);
}

static int cmd_cc_ccm_tx(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0, instance = 0, sequence = 0,
		 period = 0, iftlv = 0, iftlv_value = 0, porttlv = 0, porttlv_value = 0;
	struct mac_addr dmac;

	memset(&dmac, 0, sizeof(dmac));

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		} else if (strcmp(*argv, "instance") == 0) {
			NEXT_ARG();
			instance = atoi(*argv);
		} else if (strcmp(*argv, "dmac") == 0) {
			NEXT_ARG();
			if (strlen(*argv) != 17)	/* Must be 17 characters to be XX-XX-XX-XX-XX-XX format */
				return -1;
			dmac = mac_array(*argv);
		} else if (strcmp(*argv, "sequence") == 0) {
			NEXT_ARG();
			sequence = atoi(*argv);
		} else if (strcmp(*argv, "period") == 0) {
			NEXT_ARG();
			period = atoi(*argv);
		} else if (strcmp(*argv, "iftlv") == 0) {
			NEXT_ARG();
			iftlv = atoi(*argv);
		} else if (strcmp(*argv, "iftlv-value") == 0) {
			NEXT_ARG();
			iftlv_value = atoi(*argv);
		} else if (strcmp(*argv, "porttlv") == 0) {
			NEXT_ARG();
			porttlv = atoi(*argv);
		} else if (strcmp(*argv, "porttlv-value") == 0) {
			NEXT_ARG();
			porttlv_value = atoi(*argv);
		} else
			return -1;

		argc--; argv++;
	}
	if (br_ifindex == 0 || instance == 0)
		return -1;

	return cfm_offload_cc_ccm_tx(br_ifindex, instance, &dmac, sequence,
				     period, iftlv, iftlv_value, porttlv, porttlv_value);
}

static int cmd_mep_status_show(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0)
		return -1;

	return cfm_offload_mep_status_show(br_ifindex);
}

static int cmd_mep_config_show(int argc, char *const *argv)
{
	uint32_t br_ifindex = 0;

	/* skip the command */
	argv++;
	argc -= 1;

	while (argc > 0) {
		if (strcmp(*argv, "bridge") == 0) {
			NEXT_ARG();
			br_ifindex = if_nametoindex(*argv);
		}

		argc--; argv++;
	}

	if (br_ifindex == 0)
		return -1;

	return cfm_offload_mep_config_show(br_ifindex);
}

struct command
{
	const char *name;
	int (*func) (int argc, char *const *argv);
	const char *format;
	const char *help;
};

static const struct command commands[] =
{
	{"mep-create", cmd_mep_create,
	 "bridge <bridge> instance <instance> domain <domain> direction <direction> "
	 "port <port>", "Create MEP instance"},
	{"mep-delete", cmd_mep_delete,
	 "bridge <bridge> instance <instance>", "Delete MEP instance"},
	{"mep-config", cmd_mep_config,
	 "bridge <bridge> instance <instance> mac <mac> level <level> mepid <mepid> ",
	 "Configure MEP instance"},
	{"cc-config", cmd_cc_config,
	 "bridge <bridge> instance <instance> enable <enable> interval <interval> "
	 "domain-name <name> short-name <name>", "Configure CC function"},
	{"cc-peer", cmd_cc_peer,
	 "bridge <bridge> instance <instance> remove <remove> mepid <mepid> ",
	 "Configure CC Peer-MEP ID function"},
	{"cc-rdi", cmd_cc_rdi,
	 "bridge <bridge> instance <instance> rdi <rdi>", "Configure CC RDI insertion"},
	{"cc-ccm-tx", cmd_cc_ccm_tx,
	 "bridge <bridge> instance <instance> dmac <dmac> "
	 "sequence <sequence> period <period> iftlv <iftlv> iftlv-value "
	 "<iftlv-value> porttlv <porttlv> porttlv-value <porttlv-value>",
	 "Configure CC CCM TX"},
	{"mep-status-show", cmd_mep_status_show,
	 "bridge <bridge>", "Show MEP instances status"},
	{"mep-config-show", cmd_mep_config_show,
	 "bridge <bridge>", "Show MEP instances configuration"},
};

static void command_helpall(void)
{
	int i;

	for (i = 0; i < COUNT_OF(commands); ++i) {
		if(strcmp("setportdonttxmt", commands[i].name))
			printf("-%s:\n   %-16s %s\n", commands[i].help,
			       commands[i].name, commands[i].format);
	}
}

static void help(void)
{
	printf("Usage: cfm [options] [commands]\n");
	printf("options:\n");
	printf("  -h | --help              Show this help text\n");
	printf("commands:\n");
	command_helpall();
}

static const struct command *command_lookup(const char *cmd)
{
	int i;

	for(i = 0; i < COUNT_OF(commands); ++i) {
		if(!strcmp(cmd, commands[i].name))
			return &commands[i];
	}

	return NULL;
}

static const struct command *command_lookup_and_validate(int argc,
							 char *const *argv,
							 int line_num)
{
	const struct command *cmd;

	cmd = command_lookup(argv[0]);
	if (!cmd) {
		if (line_num > 0)
			fprintf(stderr, "Error on line %d:\n", line_num);
		fprintf(stderr, "Unknown command [%s]\n", argv[0]);
		if (line_num == 0) {
			help();
			return NULL;
		}
	}

	return cmd;
}

int main (int argc, char *const *argv)
{
	const struct command *cmd;
	int f;
	int ret;

	static const struct option options[] =
	{
		{.name = "help",	.val = 'h'},
		{0}
	};

	cfm_offload_init();

	while (EOF != (f = getopt_long(argc, argv, "h", options, NULL))) {
		switch (f) {
			case 'h':
			help();
			return 0;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		help();
		return 1;
	}

	cmd = command_lookup_and_validate(argc, argv, 0);
	if (!cmd)
		return 1;

	ret = cmd->func(argc, argv);

	return ret;
}

