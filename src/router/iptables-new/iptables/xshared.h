#ifndef IPTABLES_XSHARED_H
#define IPTABLES_XSHARED_H 1

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/time.h>
#include <linux/netfilter_arp/arp_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>

#ifdef DEBUG
#define DEBUGP(x, args...) fprintf(stderr, x, ## args)
#else
#define DEBUGP(x, args...)
#endif

enum {
	OPT_NONE        = 0,
	OPT_NUMERIC     = 1 << 0,
	OPT_SOURCE      = 1 << 1,
	OPT_DESTINATION = 1 << 2,
	OPT_PROTOCOL    = 1 << 3,
	OPT_JUMP        = 1 << 4,
	OPT_VERBOSE     = 1 << 5,
	OPT_EXPANDED    = 1 << 6,
	OPT_VIANAMEIN   = 1 << 7,
	OPT_VIANAMEOUT  = 1 << 8,
	OPT_LINENUMBERS = 1 << 9,
	OPT_COUNTERS    = 1 << 10,
	/* below are for arptables only */
	OPT_S_MAC	= 1 << 11,
	OPT_D_MAC	= 1 << 12,
	OPT_H_LENGTH	= 1 << 13,
	OPT_OPCODE	= 1 << 14,
	OPT_H_TYPE	= 1 << 15,
	OPT_P_TYPE	= 1 << 16,
};

enum {
	CMD_NONE		= 0,
	CMD_INSERT		= 1 << 0,
	CMD_DELETE		= 1 << 1,
	CMD_DELETE_NUM		= 1 << 2,
	CMD_REPLACE		= 1 << 3,
	CMD_APPEND		= 1 << 4,
	CMD_LIST		= 1 << 5,
	CMD_FLUSH		= 1 << 6,
	CMD_ZERO		= 1 << 7,
	CMD_NEW_CHAIN		= 1 << 8,
	CMD_DELETE_CHAIN	= 1 << 9,
	CMD_SET_POLICY		= 1 << 10,
	CMD_RENAME_CHAIN	= 1 << 11,
	CMD_LIST_RULES		= 1 << 12,
	CMD_ZERO_NUM		= 1 << 13,
	CMD_CHECK		= 1 << 14,
};
#define NUMBER_OF_CMD		16

struct xtables_globals;
struct xtables_rule_match;
struct xtables_target;

/**
 * xtables_afinfo - protocol family dependent information
 * @kmod:		kernel module basename (e.g. "ip_tables")
 * @proc_exists:	file which exists in procfs when module already loaded
 * @libprefix:		prefix of .so library name (e.g. "libipt_")
 * @family:		nfproto family
 * @ipproto:		used by setsockopt (e.g. IPPROTO_IP)
 * @so_rev_match:	optname to check revision support of match
 * @so_rev_target:	optname to check revision support of target
 */
struct xtables_afinfo {
	const char *kmod;
	const char *proc_exists;
	const char *libprefix;
	uint8_t family;
	uint8_t ipproto;
	int so_rev_match;
	int so_rev_target;
};

/* trick for ebtables-compat, since watchers are targets */
struct ebt_match {
	struct ebt_match			*next;
	union {
		struct xtables_match		*match;
		struct xtables_target		*watcher;
	} u;
	bool					ismatch;
};

/* Fake ebt_entry */
struct ebt_entry {
	/* this needs to be the first field */
	unsigned int bitmask;
	unsigned int invflags;
	uint16_t ethproto;
	/* the physical in-dev */
	char in[IFNAMSIZ];
	/* the logical in-dev */
	char logical_in[IFNAMSIZ];
	/* the physical out-dev */
	char out[IFNAMSIZ];
	/* the logical out-dev */
	char logical_out[IFNAMSIZ];
	unsigned char sourcemac[6];
	unsigned char sourcemsk[6];
	unsigned char destmac[6];
	unsigned char destmsk[6];
};

struct iptables_command_state {
	union {
		struct ebt_entry eb;
		struct ipt_entry fw;
		struct ip6t_entry fw6;
		struct arpt_entry arp;
	};
	int invert;
	int c;
	unsigned int options;
	struct xtables_rule_match *matches;
	struct ebt_match *match_list;
	struct xtables_target *target;
	struct xt_counters counters;
	char *protocol;
	int proto_used;
	const char *jumpto;
	char **argv;
	bool restore;
};

typedef int (*mainfunc_t)(int, char **);

struct subcommand {
	const char *name;
	mainfunc_t main;
};

enum {
	XT_OPTION_OFFSET_SCALE = 256,
};

extern void print_extension_helps(const struct xtables_target *,
	const struct xtables_rule_match *);
extern const char *proto_to_name(uint8_t, int);
extern int command_default(struct iptables_command_state *,
	struct xtables_globals *);
extern struct xtables_match *load_proto(struct iptables_command_state *);
extern int subcmd_main(int, char **, const struct subcommand *);
extern void xs_init_target(struct xtables_target *);
extern void xs_init_match(struct xtables_match *);

/**
 * Values for the iptables lock.
 *
 * A value >= 0 indicates the lock filedescriptor. Other values are:
 *
 * XT_LOCK_FAILED : The lock could not be acquired.
 *
 * XT_LOCK_BUSY : The lock was held by another process. xtables_lock only
 * returns this value when |wait| == false. If |wait| == true, xtables_lock
 * will not return unless the lock has been acquired.
 *
 * XT_LOCK_NOT_ACQUIRED : We have not yet attempted to acquire the lock.
 */
enum {
	XT_LOCK_BUSY = -1,
	XT_LOCK_FAILED = -2,
	XT_LOCK_NOT_ACQUIRED  = -3,
};
extern void xtables_unlock(int lock);
extern int xtables_lock_or_exit(int wait, struct timeval *tv);

int parse_wait_time(int argc, char *argv[]);
void parse_wait_interval(int argc, char *argv[], struct timeval *wait_interval);
int parse_counters(const char *string, struct xt_counters *ctr);
bool tokenize_rule_counters(char **bufferp, char **pcnt, char **bcnt, int line);
bool xs_has_arg(int argc, char *argv[]);

extern const struct xtables_afinfo *afinfo;

#define MAX_ARGC	255
struct argv_store {
	int argc;
	char *argv[MAX_ARGC];
	int argvattr[MAX_ARGC];
};

void add_argv(struct argv_store *store, const char *what, int quoted);
void free_argv(struct argv_store *store);
void save_argv(struct argv_store *dst, struct argv_store *src);
void add_param_to_argv(struct argv_store *store, char *parsestart, int line);
#ifdef DEBUG
void debug_print_argv(struct argv_store *store);
#else
#  define debug_print_argv(...) /* nothing */
#endif

void print_ipv4_addresses(const struct ipt_entry *fw, unsigned int format);
void print_ipv6_addresses(const struct ip6t_entry *fw6, unsigned int format);

void print_ifaces(const char *iniface, const char *outiface, uint8_t invflags,
		  unsigned int format);

void command_match(struct iptables_command_state *cs);
const char *xt_parse_target(const char *targetname);
void command_jump(struct iptables_command_state *cs, const char *jumpto);

char cmd2char(int option);
void add_command(unsigned int *cmd, const int newcmd,
		 const int othercmds, int invert);
int parse_rulenumber(const char *rule);

#endif /* IPTABLES_XSHARED_H */
