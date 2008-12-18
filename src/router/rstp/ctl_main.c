/*
  Command parsing taken from brctl utility.
  Display code from stp_cli.c in rstplib.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <getopt.h>

#include <net/if.h>

/* For scanning through sysfs directories */
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ctl_socket_client.h"
#include "ctl_functions.h"

int get_index_die(const char *ifname, const char *doc, int die)
{
	int r = if_nametoindex(ifname);
	if (r == 0) {
		fprintf(stderr,
			"Can't find index for %s %s. Not a valid interface.\n",
			doc, ifname);
		if (die)
			exit(1);
		return -1;
	}
	return r;
}

int get_index(const char *ifname, const char *doc)
{
	return get_index_die(ifname, doc, 1);
}

#define BR_ID_FMT "%02x%02x.%02x%02x%02x%02x%02x%02x"
#define BR_ID_ARGS(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]

#define BOOL_STR(x) ((x) ? "yes" : "no")

static int do_showbridge(const char *br_name)
{
	STP_BridgeStatus s;

	int br_index = get_index_die(br_name, "bridge", 0);
	if (br_index < 0)
		return -1;

	int r = CTL_get_bridge_status(br_index, &s);
	if (r) {
		return -1;
	}
	printf("%s\n", br_name);
	printf(" enabled\t\t%4s\n", BOOL_STR(s.enabled));
	printf(" bridge id\t\t" BR_ID_FMT "\n", BR_ID_ARGS(s.bridge_id));
	printf(" designated root\t" BR_ID_FMT "\n",
	       BR_ID_ARGS(s.designated_root));
	//printf(" designated bridge\t\t", BR_ID_FMT "\n",
	//       BR_ID_ARGS(s.designated_bridge));
	printf(" root port\t\t%4u", s.root_port);
	printf("\t\t\tpath cost\t\t%4u\n", s.root_path_cost);
	printf(" max age\t\t%4u", s.max_age);
	printf("\t\t\tbridge max age\t\t%4u\n", s.bridge_max_age);
	printf(" hello time\t\t%4u", s.hello_time);
	printf("\t\t\tbridge hello time\t%4u\n", s.bridge_hello_time);
	printf(" forward delay\t\t%4u", s.forward_delay);
	printf("\t\t\tbridge forward delay\t%4u\n", s.bridge_forward_delay);
	printf(" tx hold count\t\t%4u\n", s.tx_hold_count);
	printf(" protocol version\t%4u\n", s.protocol_version);
	printf(" time since topology change\t%4u\n",
	       s.time_since_topology_change);
	printf(" toplogy change count\t\t%4u\n", s.topology_change_count);
	printf(" topology change\t\t%4s\n", BOOL_STR(s.topology_change));

	return 0;
}

#define SYSFS_PATH_MAX 256
#define SYSFS_CLASS_NET "/sys/class/net"

static int isbridge(const struct dirent *entry)
{
	char path[SYSFS_PATH_MAX];
	struct stat st;

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "/%s/bridge",
		 entry->d_name);
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int cmd_showbridge(int argc, char *const *argv)
{
	int i, count = 0;
	int r = 0;
	struct dirent **namelist;

	if (argc > 1) {
		count = argc - 1;
	} else {
		count =
		    scandir(SYSFS_CLASS_NET, &namelist, isbridge, alphasort);
		if (count < 0) {
			fprintf(stderr, "Error getting list of all bridges\n");
			return -1;
		}
	}

	for (i = 0; i < count; i++) {
		const char *name;
		if (argc > 1)
			name = argv[i + 1];
		else
			name = namelist[i]->d_name;

		int err = do_showbridge(name);
		if (err)
			r = err;
	}

	if (argc <= 1) {
		for (i = 0; i < count; i++)
			free(namelist[i]);
		free(namelist);
	}

	return r;
}


#define STATE_STR(_state)			\
	({					\
		int _s = _state;		\
		char *_str = "unknown";		\
		switch (_s)			\
		{					     \
		case STP_PORT_STATE_DISCARDING: _str = "discarding"; break; \
		case STP_PORT_STATE_LEARNING:   _str = "learning"; break; \
		case STP_PORT_STATE_FORWARDING: _str = "forwarding"; break; \
		}							\
		_str;							\
	})

#define SHORT_STATE_STR(_state)			\
	({					\
		int _s = _state;		\
		char *_str = "unkn";		\
		switch (_s)			\
		{				       \
		case STP_PORT_STATE_DISCARDING: _str = "disc"; break;	\
		case STP_PORT_STATE_LEARNING:   _str = "lear"; break;	\
		case STP_PORT_STATE_FORWARDING: _str = "forw"; break;	\
		}							\
		_str;							\
	})

#define ADMIN_P2P_STR(_state)			\
	({					\
		int _s = _state;		\
		char *_str = "unkn";		\
		switch (_s)			\
		{				     \
		case STP_ADMIN_P2P_FORCE_FALSE: _str = "no"; break;	\
		case STP_ADMIN_P2P_FORCE_TRUE:   _str = "yes"; break;	\
		case STP_ADMIN_P2P_AUTO: _str = "auto"; break;		\
		}							\
		_str;							\
	})


int detail = 0;

static int do_showport(int br_index, const char *port_name)
{
	STP_PortStatus s;
	int r = 0;
	int port_index = get_index_die(port_name, "port", 0);
	if (port_index < 0)
		return -1;

	r = CTL_get_port_status(br_index, port_index, &s);
	if (r) {
		fprintf(stderr, "Failed to get port state for port %d\n",
			port_index);
		return -1;
	}

	if (detail) {
		printf("%s (%u)\n", port_name, (s.id & 0xfff));
		printf(" enabled\t\t%4s\n", BOOL_STR(s.enabled));
		printf(" port id\t\t%04x\t\t\tstate\t\t%15s\n",
		       s.id, STATE_STR(s.state));
		printf(" path cost\t%12d\t\t\tadmin path cost\t%12d\n", 
		       s.path_cost, s.admin_path_cost);
		printf(" designated root\t" BR_ID_FMT,
		       BR_ID_ARGS(s.designated_root));
		printf("\tdesignated cost\t%12u\n", s.designated_cost);
		printf(" designated bridge\t" BR_ID_FMT,
		       BR_ID_ARGS(s.designated_bridge));
		printf("\tdesignated port\t\t%04x\n", s.designated_port);
		printf(" admin edge port\t%4s", BOOL_STR(s.admin_edge_port));
		printf("\t\t\tauto edge port\t\t%4s\n",
		       BOOL_STR(s.auto_edge_port));
		printf(" oper edge port\t\t%4s", BOOL_STR(s.oper_edge_port));
		printf("\t\t\ttoplogy change ack\t%4s\n", BOOL_STR(s.tc_ack));
		printf(" point to point\t\t%4s", BOOL_STR(s.oper_p2p));
		printf("\t\t\tadmin point to point\t%4s\n",
		       ADMIN_P2P_STR(s.admin_p2p));
	} else {
		printf("%c%c %4s %04x %4s " BR_ID_FMT " " BR_ID_FMT " %04x\n",
		       (s.oper_p2p) ? ' ' : '*',
		       (s.oper_edge_port) ? 'E' : ' ',
		       port_name,
		       s.id,
		       s.enabled?SHORT_STATE_STR(s.state):"down",
		       BR_ID_ARGS(s.designated_root),
		       BR_ID_ARGS(s.designated_bridge),
		       s.designated_port);
	}
	return 0;
}

static int not_dot_dotdot(const struct dirent *entry)
{
	const char *n = entry->d_name;

	return !(n[0] == '.' && (n[1] == 0 || (n[1] == '.' && n[2] == 0)));
}

static int cmd_showport(int argc, char *const *argv)
{
	int r = 0;

	int br_index = get_index(argv[1], "bridge");

	int i, count = 0;
	struct dirent **namelist;

	if (argc > 2) {
		count = argc - 2;
	} else {
		char buf[SYSFS_PATH_MAX];
		snprintf(buf, sizeof(buf), SYSFS_CLASS_NET "/%s/brif", argv[1]);
		count = scandir(buf, &namelist, not_dot_dotdot, alphasort);
		if (count < 0) {
			fprintf(stderr,
				"Error getting list of all ports of bridge %s\n",
				argv[1]);
			return -1;
		}
	}

	for (i = 0; i < count; i++) {
		const char *name;
		if (argc > 2)
			name = argv[i + 2];
		else
			name = namelist[i]->d_name;

		int err = do_showport(br_index, name);
		if (err)
			r = err;
	}

	if (argc <= 2) {
		for (i = 0; i < count; i++)
			free(namelist[i]);
		free(namelist);
	}

	return r;
}

static int cmd_showportdetail(int argc, char *const *argv)
{
	detail = 1;
	return cmd_showport(argc, argv);
}

unsigned int getuint(const char *s)
{
	char *end;
	long l;
	l = strtoul(s, &end, 0);
	if (*s == 0 || *end != 0 || l > INT_MAX) {
		fprintf(stderr, "Invalid unsigned int arg %s\n", s);
		exit(1);
	}
	return l;
}

int getenum(const char *s, const char *opt[])
{
	int i;
	for (i = 0; opt[i] != NULL; i++)
		if (strcmp(s, opt[i]) == 0)
			return i;

	fprintf(stderr, "Invalid argument %s: expecting one of ", s);
	for (i = 0; opt[i] != NULL; i++)
		fprintf(stderr, "%s%s", opt[i], (opt[i + 1] ? ", " : "\n"));

	exit(1);
}

int getyesno(const char *s, const char *yes, const char *no)
{
	/* Reverse yes and no so error message looks more normal */
	const char *opt[] = { yes, no, NULL };
	return 1 - getenum(s, opt);
}

#define set_bridge_cfg(field, value) \
({ \
	STP_BridgeConfig c; \
	memset(&c, 0, sizeof(c)); \
	c.field = value; \
	c.set_ ## field = 1; \
	int r = CTL_set_bridge_config(br_index, &c); \
	if (r) \
		printf("Couldn't change bridge " #field "\n"); \
	r; \
})
   
#define set_port_cfg(field, value) \
({ \
	STP_PortConfig c; \
	memset(&c, 0, sizeof(c)); \
	c.field = value; \
	c.set_ ## field = 1; \
	int r = CTL_set_port_config(br_index, port_index, &c); \
	if (r) \
		printf("Couldn't change port " #field "\n"); \
	r; \
})
   


static int cmd_setbridgeprio(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_priority, getuint(argv[2]));
}

static int cmd_setbridgemaxage(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_max_age, getuint(argv[2]));
}

static int cmd_setbridgehello(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_hello_time, getuint(argv[2]));
}

static int cmd_setbridgefdelay(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_forward_delay, getuint(argv[2]));
}

static int cmd_setbridgeforcevers(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_protocol_version,
			      2 * getyesno(argv[2], "normal", "slow"));
}

static int cmd_setbridgetxholdcount(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	return set_bridge_cfg(bridge_tx_hold_count, getuint(argv[2]));
}


static int cmd_setportprio(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	return set_port_cfg(port_priority, getuint(argv[3]));
}

static int cmd_setportpathcost(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	return set_port_cfg(port_pathcost, getuint(argv[3]));
}

static int cmd_setportadminedge(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	return set_port_cfg(port_admin_edge, getyesno(argv[3], "yes", "no"));
}

static int cmd_setportautoedge(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	return set_port_cfg(port_auto_edge, getyesno(argv[3], "yes", "no"));
}

static int cmd_setportp2p(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	const char *opts[] = { "no", "yes", "auto", NULL };
	int vals[] = { STP_ADMIN_P2P_FORCE_FALSE, STP_ADMIN_P2P_FORCE_TRUE,
		       STP_ADMIN_P2P_AUTO };

	return set_port_cfg(port_admin_p2p, vals[getenum(argv[3], opts)]);
}

static int cmd_portmcheck(int argc, char *const *argv)
{

	int br_index = get_index(argv[1], "bridge");
	int port_index = get_index(argv[2], "port");
	return CTL_port_mcheck(br_index, port_index);
}

static int cmd_debuglevel(int argc, char *const *argv)
{
	return CTL_set_debug_level(getuint(argv[1]));
}

struct command {
	int nargs;
	int optargs;
	const char *name;
	int (*func) (int argc, char *const *argv);
	const char *help;
};

static const struct command commands[] = {
	{0, 32, "showbridge", cmd_showbridge,
	 "[<bridge> ... ]\t\tshow bridge state"},
	{1, 32, "showport", cmd_showport,
	 "<bridge> [<port> ... ]\tshow port state"},
	{1, 32, "showportdetail", cmd_showportdetail,
	 "<bridge> [<port> ... ]\tshow port state (detail)"},
	{2, 0, "setbridgeprio", cmd_setbridgeprio,
	 "<bridge> <priority>\tset bridge priority (0-61440)"},
	{2, 0, "sethello", cmd_setbridgehello,
	 "<bridge> <hellotime>\tset bridge hello time (1-10)"},
	{2, 0, "setmaxage", cmd_setbridgemaxage,
	 "<bridge> <maxage>\tset bridge max age (6-40)"},
	{2, 0, "setfdelay", cmd_setbridgefdelay,
	 "<bridge> <fwd_delay>\tset bridge forward delay (4-30)"},
	{2, 0, "setforcevers", cmd_setbridgeforcevers,
	 "<bridge> {normal|slow}\tnormal RSTP or force to STP"},
	{2, 0, "settxholdcount", cmd_setbridgetxholdcount,
	 "<bridge> <tx_hold_count>\tset bridge transmit hold count (1-10)"},
	{3, 0, "setportprio", cmd_setportprio,
	 "<bridge> <port> <priority>\tset port priority (0-240)"},
	{3, 0, "setportpathcost", cmd_setportpathcost,
	 "<bridge> <port> <cost>\tset port path cost"},
	{3, 0, "setportadminedge", cmd_setportadminedge,
	 "<bridge> <port> {yes|no}\tconfigure if it is an admin edge port"},
	{3, 0, "setportautoedge", cmd_setportautoedge,
	 "<bridge> <port> {yes|no}\tconfigure if it is an auto edge port"},
	{3, 0, "setportp2p", cmd_setportp2p,
	 "<bridge> <port> {yes|no|auto}\tset whether p2p connection"},
	{2, 0, "portmcheck", cmd_portmcheck,
	 "<bridge> <port>\ttry to get back from STP to RSTP mode"},
	{1, 0, "debuglevel", cmd_debuglevel, "<level>\t\tLevel of verbosity"},
};

const struct command *command_lookup(const char *cmd)
{
	int i;

	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		if (!strcmp(cmd, commands[i].name))
			return &commands[i];
	}

	return NULL;
}

void command_helpall(void)
{
	int i;

	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		printf("\t%-10s\t%s\n", commands[i].name, commands[i].help);
	}
}

static void help()
{
	printf("Usage: rstpctl [commands]\n");
	printf("commands:\n");
	command_helpall();
}

#define PACKAGE_VERSION2(v, b) "rstp, " #v "-" #b
#define PACKAGE_VERSION(v, b) PACKAGE_VERSION2(v, b)

int main(int argc, char *const *argv)
{
	const struct command *cmd;
	int f;
	static const struct option options[] = {
		{.name = "help",.val = 'h'},
		{.name = "version",.val = 'V'},
		{0}
	};

	while ((f = getopt_long(argc, argv, "Vh", options, NULL)) != EOF)
		switch (f) {
		case 'h':
			help();
			return 0;
		case 'V':
			printf("%s\n", PACKAGE_VERSION(VERSION, BUILD));
			return 0;
		default:
			fprintf(stderr, "Unknown option '%c'\n", f);
			goto help;
		}

	if (argc == optind)
		goto help;

	if (ctl_client_init()) {
		fprintf(stderr, "can't setup control connection\n");
		return 1;
	}

	argc -= optind;
	argv += optind;
	if ((cmd = command_lookup(argv[0])) == NULL) {
		fprintf(stderr, "never heard of command [%s]\n", argv[0]);
		goto help;
	}

	if (argc < cmd->nargs + 1 || argc > cmd->nargs + cmd->optargs + 1) {
		printf("Incorrect number of arguments for command\n");
		printf("Usage: rstpctl %s %s\n", cmd->name, cmd->help);
		return 1;
	}

	return cmd->func(argc, argv);

      help:
	help();
	return 1;
}
