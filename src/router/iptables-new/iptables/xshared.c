#include <config.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <xtables.h>
#include <math.h>
#include "xshared.h"

/*
 * Print out any special helps. A user might like to be able to add a --help
 * to the commandline, and see expected results. So we call help for all
 * specified matches and targets.
 */
void print_extension_helps(const struct xtables_target *t,
    const struct xtables_rule_match *m)
{
	for (; t != NULL; t = t->next) {
		if (t->used) {
			printf("\n");
			if (t->help == NULL)
				printf("%s does not take any options\n",
				       t->name);
			else
				t->help();
		}
	}
	for (; m != NULL; m = m->next) {
		printf("\n");
		if (m->match->help == NULL)
			printf("%s does not take any options\n",
			       m->match->name);
		else
			m->match->help();
	}
}

const char *
proto_to_name(uint8_t proto, int nolookup)
{
	unsigned int i;

	if (proto && !nolookup) {
		struct protoent *pent = getprotobynumber(proto);
		if (pent)
			return pent->p_name;
	}

	for (i = 0; xtables_chain_protos[i].name != NULL; ++i)
		if (xtables_chain_protos[i].num == proto)
			return xtables_chain_protos[i].name;

	return NULL;
}

static struct xtables_match *
find_proto(const char *pname, enum xtables_tryload tryload,
	   int nolookup, struct xtables_rule_match **matches)
{
	unsigned int proto;

	if (xtables_strtoui(pname, NULL, &proto, 0, UINT8_MAX)) {
		const char *protoname = proto_to_name(proto, nolookup);

		if (protoname)
			return xtables_find_match(protoname, tryload, matches);
	} else
		return xtables_find_match(pname, tryload, matches);

	return NULL;
}

/*
 * Some explanations (after four different bugs in 3 different releases): If
 * we encounter a parameter, that has not been parsed yet, it's not an option
 * of an explicitly loaded match or a target. However, we support implicit
 * loading of the protocol match extension. '-p tcp' means 'l4 proto 6' and at
 * the same time 'load tcp protocol match on demand if we specify --dport'.
 *
 * To make this work, we need to make sure:
 * - the parameter has not been parsed by a match (m above)
 * - a protocol has been specified
 * - the protocol extension has not been loaded yet, or is loaded and unused
 *   [think of ip6tables-restore!]
 * - the protocol extension can be successively loaded
 */
static bool should_load_proto(struct iptables_command_state *cs)
{
	if (cs->protocol == NULL)
		return false;
	if (find_proto(cs->protocol, XTF_DONT_LOAD,
	    cs->options & OPT_NUMERIC, NULL) == NULL)
		return true;
	return !cs->proto_used;
}

struct xtables_match *load_proto(struct iptables_command_state *cs)
{
	if (!should_load_proto(cs))
		return NULL;
	return find_proto(cs->protocol, XTF_TRY_LOAD,
			  cs->options & OPT_NUMERIC, &cs->matches);
}

int command_default(struct iptables_command_state *cs,
		    struct xtables_globals *gl)
{
	struct xtables_rule_match *matchp;
	struct xtables_match *m;

	if (cs->target != NULL &&
	    (cs->target->parse != NULL || cs->target->x6_parse != NULL) &&
	    cs->c >= cs->target->option_offset &&
	    cs->c < cs->target->option_offset + XT_OPTION_OFFSET_SCALE) {
		xtables_option_tpcall(cs->c, cs->argv, cs->invert,
				      cs->target, &cs->fw);
		return 0;
	}

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		m = matchp->match;

		if (matchp->completed ||
		    (m->x6_parse == NULL && m->parse == NULL))
			continue;
		if (cs->c < matchp->match->option_offset ||
		    cs->c >= matchp->match->option_offset + XT_OPTION_OFFSET_SCALE)
			continue;
		xtables_option_mpcall(cs->c, cs->argv, cs->invert, m, &cs->fw);
		return 0;
	}

	/* Try loading protocol */
	m = load_proto(cs);
	if (m != NULL) {
		size_t size;

		cs->proto_used = 1;

		size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;

		m->m = xtables_calloc(1, size);
		m->m->u.match_size = size;
		strcpy(m->m->u.user.name, m->name);
		m->m->u.user.revision = m->revision;
		xs_init_match(m);

		if (m->x6_options != NULL)
			gl->opts = xtables_options_xfrm(gl->orig_opts,
							gl->opts,
							m->x6_options,
							&m->option_offset);
		else
			gl->opts = xtables_merge_options(gl->orig_opts,
							 gl->opts,
							 m->extra_opts,
							 &m->option_offset);
		if (gl->opts == NULL)
			xtables_error(OTHER_PROBLEM, "can't alloc memory!");
		optind--;
		/* Indicate to rerun getopt *immediately* */
 		return 1;
	}

	if (cs->c == ':')
		xtables_error(PARAMETER_PROBLEM, "option \"%s\" "
		              "requires an argument", cs->argv[optind-1]);
	if (cs->c == '?')
		xtables_error(PARAMETER_PROBLEM, "unknown option "
			      "\"%s\"", cs->argv[optind-1]);
	xtables_error(PARAMETER_PROBLEM, "Unknown arg \"%s\"", optarg);
}

static mainfunc_t subcmd_get(const char *cmd, const struct subcommand *cb)
{
	for (; cb->name != NULL; ++cb)
		if (strcmp(cb->name, cmd) == 0)
			return cb->main;
	return NULL;
}

int subcmd_main(int argc, char **argv, const struct subcommand *cb)
{
	const char *cmd = basename(*argv);
	mainfunc_t f = subcmd_get(cmd, cb);

	if (f == NULL && argc > 1) {
		/*
		 * Unable to find a main method for our command name?
		 * Let's try again with the first argument!
		 */
		++argv;
		--argc;
		f = subcmd_get(*argv, cb);
	}

	/* now we should have a valid function pointer */
	if (f != NULL)
		return f(argc, argv);

	fprintf(stderr, "ERROR: No valid subcommand given.\nValid subcommands:\n");
	for (; cb->name != NULL; ++cb)
		fprintf(stderr, " * %s\n", cb->name);
	exit(EXIT_FAILURE);
}

void xs_init_target(struct xtables_target *target)
{
	if (target->udata_size != 0) {
		free(target->udata);
		target->udata = calloc(1, target->udata_size);
		if (target->udata == NULL)
			xtables_error(RESOURCE_PROBLEM, "malloc");
	}
	if (target->init != NULL)
		target->init(target->t);
}

void xs_init_match(struct xtables_match *match)
{
	if (match->udata_size != 0) {
		/*
		 * As soon as a subsequent instance of the same match
		 * is used, e.g. "-m time -m time", the first instance
		 * is no longer reachable anyway, so we can free udata.
		 * Same goes for target.
		 */
		free(match->udata);
		match->udata = calloc(1, match->udata_size);
		if (match->udata == NULL)
			xtables_error(RESOURCE_PROBLEM, "malloc");
	}
	if (match->init != NULL)
		match->init(match->m);
}

static int xtables_lock(int wait, struct timeval *wait_interval)
{
	struct timeval time_left, wait_time;
	int fd, i = 0;

	time_left.tv_sec = wait;
	time_left.tv_usec = 0;

	fd = open(XT_LOCK_NAME, O_CREAT, 0600);
	if (fd < 0) {
		fprintf(stderr, "Fatal: can't open lock file %s: %s\n",
			XT_LOCK_NAME, strerror(errno));
		return XT_LOCK_FAILED;
	}

	if (wait == -1) {
		if (flock(fd, LOCK_EX) == 0)
			return fd;

		fprintf(stderr, "Can't lock %s: %s\n", XT_LOCK_NAME,
			strerror(errno));
		return XT_LOCK_BUSY;
	}

	while (1) {
		if (flock(fd, LOCK_EX | LOCK_NB) == 0)
			return fd;
		else if (timercmp(&time_left, wait_interval, <))
			return XT_LOCK_BUSY;

		if (++i % 10 == 0) {
			fprintf(stderr, "Another app is currently holding the xtables lock; "
				"still %lds %ldus time ahead to have a chance to grab the lock...\n",
				time_left.tv_sec, time_left.tv_usec);
		}

		wait_time = *wait_interval;
		select(0, NULL, NULL, NULL, &wait_time);
		timersub(&time_left, wait_interval, &time_left);
	}
}

void xtables_unlock(int lock)
{
	if (lock >= 0)
		close(lock);
}

int xtables_lock_or_exit(int wait, struct timeval *wait_interval)
{
	int lock = xtables_lock(wait, wait_interval);

	if (lock == XT_LOCK_FAILED) {
		xtables_free_opts(1);
		exit(RESOURCE_PROBLEM);
	}

	if (lock == XT_LOCK_BUSY) {
		fprintf(stderr, "Another app is currently holding the xtables lock. ");
		if (wait == 0)
			fprintf(stderr, "Perhaps you want to use the -w option?\n");
		else
			fprintf(stderr, "Stopped waiting after %ds.\n", wait);
		xtables_free_opts(1);
		exit(RESOURCE_PROBLEM);
	}

	return lock;
}

int parse_wait_time(int argc, char *argv[])
{
	int wait = -1;

	if (optarg) {
		if (sscanf(optarg, "%i", &wait) != 1)
			xtables_error(PARAMETER_PROBLEM,
				"wait seconds not numeric");
	} else if (xs_has_arg(argc, argv))
		if (sscanf(argv[optind++], "%i", &wait) != 1)
			xtables_error(PARAMETER_PROBLEM,
				"wait seconds not numeric");

	return wait;
}

void parse_wait_interval(int argc, char *argv[], struct timeval *wait_interval)
{
	const char *arg;
	unsigned int usec;
	int ret;

	if (optarg)
		arg = optarg;
	else if (xs_has_arg(argc, argv))
		arg = argv[optind++];
	else
		xtables_error(PARAMETER_PROBLEM, "wait interval value required");

	ret = sscanf(arg, "%u", &usec);
	if (ret == 1) {
		if (usec > 999999)
			xtables_error(PARAMETER_PROBLEM,
				      "too long usec wait %u > 999999 usec",
				      usec);

		wait_interval->tv_sec = 0;
		wait_interval->tv_usec = usec;
		return;
	}
	xtables_error(PARAMETER_PROBLEM, "wait interval not numeric");
}

int parse_counters(const char *string, struct xt_counters *ctr)
{
	int ret;

	if (!string)
		return 0;

	ret = sscanf(string, "[%llu:%llu]",
		     (unsigned long long *)&ctr->pcnt,
		     (unsigned long long *)&ctr->bcnt);

	return ret == 2;
}

/* Tokenize counters argument of typical iptables-restore format rule.
 *
 * If *bufferp contains counters, update *pcntp and *bcntp to point at them,
 * change bytes after counters in *bufferp to nul-bytes, update *bufferp to
 * point to after the counters and return true.
 * If *bufferp does not contain counters, return false.
 * If syntax is wrong in *bufferp, call xtables_error() and hence exit().
 * */
bool tokenize_rule_counters(char **bufferp, char **pcntp, char **bcntp, int line)
{
	char *ptr, *buffer = *bufferp, *pcnt, *bcnt;

	if (buffer[0] != '[')
		return false;

	/* we have counters in our input */

	ptr = strchr(buffer, ']');
	if (!ptr)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need ]\n", line);

	pcnt = strtok(buffer+1, ":");
	if (!pcnt)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need :\n", line);

	bcnt = strtok(NULL, "]");
	if (!bcnt)
		xtables_error(PARAMETER_PROBLEM, "Bad line %u: need ]\n", line);

	*pcntp = pcnt;
	*bcntp = bcnt;
	/* start command parsing after counter */
	*bufferp = ptr + 1;

	return true;
}

inline bool xs_has_arg(int argc, char *argv[])
{
	return optind < argc &&
	       argv[optind][0] != '-' &&
	       argv[optind][0] != '!';
}

/* function adding one argument to store, updating argc
 * returns if argument added, does not return otherwise */
void add_argv(struct argv_store *store, const char *what, int quoted)
{
	DEBUGP("add_argv: %s\n", what);

	if (store->argc + 1 >= MAX_ARGC)
		xtables_error(PARAMETER_PROBLEM,
			      "Parser cannot handle more arguments\n");
	if (!what)
		xtables_error(PARAMETER_PROBLEM,
			      "Trying to store NULL argument\n");

	store->argv[store->argc] = strdup(what);
	store->argvattr[store->argc] = quoted;
	store->argv[++store->argc] = NULL;
}

void free_argv(struct argv_store *store)
{
	while (store->argc) {
		store->argc--;
		free(store->argv[store->argc]);
		store->argvattr[store->argc] = 0;
	}
}

/* Save parsed rule for comparison with next rule to perform action aggregation
 * on duplicate conditions.
 */
void save_argv(struct argv_store *dst, struct argv_store *src)
{
	int i;

	free_argv(dst);
	for (i = 0; i < src->argc; i++) {
		dst->argvattr[i] = src->argvattr[i];
		dst->argv[i] = src->argv[i];
		src->argv[i] = NULL;
	}
	dst->argc = src->argc;
	src->argc = 0;
}

struct xt_param_buf {
	char	buffer[1024];
	int 	len;
};

static void add_param(struct xt_param_buf *param, const char *curchar)
{
	param->buffer[param->len++] = *curchar;
	if (param->len >= sizeof(param->buffer))
		xtables_error(PARAMETER_PROBLEM,
			      "Parameter too long!");
}

void add_param_to_argv(struct argv_store *store, char *parsestart, int line)
{
	int quote_open = 0, escaped = 0, quoted = 0;
	struct xt_param_buf param = {};
	char *curchar;

	/* After fighting with strtok enough, here's now
	 * a 'real' parser. According to Rusty I'm now no
	 * longer a real hacker, but I can live with that */

	for (curchar = parsestart; *curchar; curchar++) {
		if (quote_open) {
			if (escaped) {
				add_param(&param, curchar);
				escaped = 0;
				continue;
			} else if (*curchar == '\\') {
				escaped = 1;
				continue;
			} else if (*curchar == '"') {
				quote_open = 0;
			} else {
				add_param(&param, curchar);
				continue;
			}
		} else {
			if (*curchar == '"') {
				quote_open = 1;
				quoted = 1;
				continue;
			}
		}

		switch (*curchar) {
		case '"':
			break;
		case ' ':
		case '\t':
		case '\n':
			if (!param.len) {
				/* two spaces? */
				continue;
			}
			break;
		default:
			/* regular character, copy to buffer */
			add_param(&param, curchar);
			continue;
		}

		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, quoted);
		param.len = 0;
		quoted = 0;
	}
	if (param.len) {
		param.buffer[param.len] = '\0';
		add_argv(store, param.buffer, 0);
	}
}

#ifdef DEBUG
void debug_print_argv(struct argv_store *store)
{
	int i;

	for (i = 0; i < store->argc; i++)
		fprintf(stderr, "argv[%d]: %s\n", i, store->argv[i]);
}
#endif

static const char *ipv4_addr_to_string(const struct in_addr *addr,
				       const struct in_addr *mask,
				       unsigned int format)
{
	static char buf[BUFSIZ];

	if (!mask->s_addr && !(format & FMT_NUMERIC))
		return "anywhere";

	if (format & FMT_NUMERIC)
		strncpy(buf, xtables_ipaddr_to_numeric(addr), BUFSIZ - 1);
	else
		strncpy(buf, xtables_ipaddr_to_anyname(addr), BUFSIZ - 1);
	buf[BUFSIZ - 1] = '\0';

	strncat(buf, xtables_ipmask_to_numeric(mask),
		BUFSIZ - strlen(buf) - 1);

	return buf;
}

void print_ipv4_addresses(const struct ipt_entry *fw, unsigned int format)
{
	fputc(fw->ip.invflags & IPT_INV_SRCIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "%s "),
	       ipv4_addr_to_string(&fw->ip.src, &fw->ip.smsk, format));

	fputc(fw->ip.invflags & IPT_INV_DSTIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "-> %s"),
	       ipv4_addr_to_string(&fw->ip.dst, &fw->ip.dmsk, format));
}

static const char *ipv6_addr_to_string(const struct in6_addr *addr,
				       const struct in6_addr *mask,
				       unsigned int format)
{
	static char buf[BUFSIZ];

	if (IN6_IS_ADDR_UNSPECIFIED(addr) && !(format & FMT_NUMERIC))
		return "anywhere";

	if (format & FMT_NUMERIC)
		strncpy(buf, xtables_ip6addr_to_numeric(addr), BUFSIZ - 1);
	else
		strncpy(buf, xtables_ip6addr_to_anyname(addr), BUFSIZ - 1);
	buf[BUFSIZ - 1] = '\0';

	strncat(buf, xtables_ip6mask_to_numeric(mask),
		BUFSIZ - strlen(buf) - 1);

	return buf;
}

void print_ipv6_addresses(const struct ip6t_entry *fw6, unsigned int format)
{
	fputc(fw6->ipv6.invflags & IP6T_INV_SRCIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "%s "),
	       ipv6_addr_to_string(&fw6->ipv6.src,
				   &fw6->ipv6.smsk, format));

	fputc(fw6->ipv6.invflags & IP6T_INV_DSTIP ? '!' : ' ', stdout);
	printf(FMT("%-19s ", "-> %s"),
	       ipv6_addr_to_string(&fw6->ipv6.dst,
				   &fw6->ipv6.dmsk, format));
}

/* Luckily, IPT_INV_VIA_IN and IPT_INV_VIA_OUT
 * have the same values as IP6T_INV_VIA_IN and IP6T_INV_VIA_OUT
 * so this function serves for both iptables and ip6tables */
void print_ifaces(const char *iniface, const char *outiface, uint8_t invflags,
		  unsigned int format)
{
	const char *anyname = format & FMT_NUMERIC ? "*" : "any";
	char iface[IFNAMSIZ + 2];

	if (!(format & FMT_VIA))
		return;

	snprintf(iface, IFNAMSIZ + 2, "%s%s",
		 invflags & IPT_INV_VIA_IN ? "!" : "",
		 iniface[0] != '\0' ? iniface : anyname);

	printf(FMT(" %-6s ", "in %s "), iface);

	snprintf(iface, IFNAMSIZ + 2, "%s%s",
		 invflags & IPT_INV_VIA_OUT ? "!" : "",
		 outiface[0] != '\0' ? outiface : anyname);

	printf(FMT("%-6s ", "out %s "), iface);
}

void command_match(struct iptables_command_state *cs)
{
	struct option *opts = xt_params->opts;
	struct xtables_match *m;
	size_t size;

	if (cs->invert)
		xtables_error(PARAMETER_PROBLEM,
			   "unexpected ! flag before --match");

	m = xtables_find_match(optarg, XTF_LOAD_MUST_SUCCEED, &cs->matches);
	size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;
	m->m = xtables_calloc(1, size);
	m->m->u.match_size = size;
	if (m->real_name == NULL) {
		strcpy(m->m->u.user.name, m->name);
	} else {
		strcpy(m->m->u.user.name, m->real_name);
		if (!(m->ext_flags & XTABLES_EXT_ALIAS))
			fprintf(stderr, "Notice: the %s match is converted into %s match "
				"in rule listing and saving.\n", m->name, m->real_name);
	}
	m->m->u.user.revision = m->revision;
	xs_init_match(m);
	if (m == m->next)
		return;
	/* Merge options for non-cloned matches */
	if (m->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    m->x6_options, &m->option_offset);
	else if (m->extra_opts != NULL)
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     m->extra_opts, &m->option_offset);
	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "can't alloc memory!");
	xt_params->opts = opts;
}

const char *xt_parse_target(const char *targetname)
{
	const char *ptr;

	if (strlen(targetname) < 1)
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid target name (too short)");

	if (strlen(targetname) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid target name `%s' (%u chars max)",
			   targetname, XT_EXTENSION_MAXNAMELEN - 1);

	for (ptr = targetname; *ptr; ptr++)
		if (isspace(*ptr))
			xtables_error(PARAMETER_PROBLEM,
				   "Invalid target name `%s'", targetname);
	return targetname;
}

void command_jump(struct iptables_command_state *cs, const char *jumpto)
{
	struct option *opts = xt_params->opts;
	size_t size;

	cs->jumpto = xt_parse_target(jumpto);
	/* TRY_LOAD (may be chain name) */
	cs->target = xtables_find_target(cs->jumpto, XTF_TRY_LOAD);

	if (cs->target == NULL)
		return;

	size = XT_ALIGN(sizeof(struct xt_entry_target)) + cs->target->size;

	cs->target->t = xtables_calloc(1, size);
	cs->target->t->u.target_size = size;
	if (cs->target->real_name == NULL) {
		strcpy(cs->target->t->u.user.name, cs->jumpto);
	} else {
		/* Alias support for userspace side */
		strcpy(cs->target->t->u.user.name, cs->target->real_name);
		if (!(cs->target->ext_flags & XTABLES_EXT_ALIAS))
			fprintf(stderr, "Notice: The %s target is converted into %s target "
				"in rule listing and saving.\n",
				cs->jumpto, cs->target->real_name);
	}
	cs->target->t->u.user.revision = cs->target->revision;
	xs_init_target(cs->target);

	if (cs->target->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    cs->target->x6_options,
					    &cs->target->option_offset);
	else
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     cs->target->extra_opts,
					     &cs->target->option_offset);
	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "can't alloc memory!");
	xt_params->opts = opts;
}

char cmd2char(int option)
{
	/* cmdflags index corresponds with position of bit in CMD_* values */
	static const char cmdflags[] = { 'I', 'D', 'D', 'R', 'A', 'L', 'F', 'Z',
					 'N', 'X', 'P', 'E', 'S', 'Z', 'C' };
	int i;

	for (i = 0; option > 1; option >>= 1, i++)
		;
	if (i >= ARRAY_SIZE(cmdflags))
		xtables_error(OTHER_PROBLEM,
			      "cmd2char(): Invalid command number %u.\n",
			      1 << i);
	return cmdflags[i];
}

void add_command(unsigned int *cmd, const int newcmd,
		 const int othercmds, int invert)
{
	if (invert)
		xtables_error(PARAMETER_PROBLEM, "unexpected '!' flag");
	if (*cmd & (~othercmds))
		xtables_error(PARAMETER_PROBLEM, "Cannot use -%c with -%c\n",
			   cmd2char(newcmd), cmd2char(*cmd & (~othercmds)));
	*cmd |= newcmd;
}

/* Can't be zero. */
int parse_rulenumber(const char *rule)
{
	unsigned int rulenum;

	if (!xtables_strtoui(rule, NULL, &rulenum, 1, INT_MAX))
		xtables_error(PARAMETER_PROBLEM,
			   "Invalid rule number `%s'", rule);

	return rulenum;
}
