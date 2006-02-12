/*
 * ebtables.c, v2.0 July 2002
 *
 * Author: Bart De Schuymer
 *
 *  This code is stongly inspired on the iptables code which is
 *  Copyright (C) 1999 Paul `Rusty' Russell & Michael J. Neuling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netinet/ether.h>
#include "include/ebtables_u.h"
#include "include/ethernetdb.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/*
 * Don't use this function, use print_bug()
 */
void __print_bug(char *file, int line, char *format, ...)
{
	va_list l;

	va_start(l, format);
	printf(PROGNAME" v"PROGVERSION":%s:%d:--BUG--: \n", file, line);
	vprintf(format, l);
	printf("\n");
	va_end(l);
	exit (-1);
}

#ifndef PROC_SYS_MODPROBE
#define PROC_SYS_MODPROBE "/proc/sys/kernel/modprobe"
#endif
#define ATOMIC_ENV_VARIABLE "EBTABLES_ATOMIC_FILE"
#define PRINT_VERSION printf(PROGNAME" v"PROGVERSION" ("PROGDATE")\n")


char *hooknames[NF_BR_NUMHOOKS] =
{
	[NF_BR_PRE_ROUTING]"PREROUTING",
	[NF_BR_LOCAL_IN]"INPUT",
	[NF_BR_FORWARD]"FORWARD",
	[NF_BR_LOCAL_OUT]"OUTPUT",
	[NF_BR_POST_ROUTING]"POSTROUTING",
	[NF_BR_BROUTING]"BROUTING"
};

/*
 * default command line options
 * do not mess around with the already assigned numbers unless
 * you know what you are doing
 */
static struct option ebt_original_options[] =
{
	{ "append"        , required_argument, 0, 'A' },
	{ "insert"        , required_argument, 0, 'I' },
	{ "delete"        , required_argument, 0, 'D' },
	{ "list"          , optional_argument, 0, 'L' },
	{ "Lc"            , no_argument      , 0, 4   },
	{ "Ln"            , no_argument      , 0, 5   },
	{ "Lx"            , no_argument      , 0, 6   },
	{ "Lmac2"         , no_argument      , 0, 12  },
	{ "zero"          , optional_argument, 0, 'Z' },
	{ "flush"         , optional_argument, 0, 'F' },
	{ "policy"        , required_argument, 0, 'P' },
	{ "in-interface"  , required_argument, 0, 'i' },
	{ "in-if"         , required_argument, 0, 'i' },
	{ "logical-in"    , required_argument, 0, 2   },
	{ "logical-out"   , required_argument, 0, 3   },
	{ "out-interface" , required_argument, 0, 'o' },
	{ "out-if"        , required_argument, 0, 'o' },
	{ "version"       , no_argument      , 0, 'V' },
	{ "help"          , no_argument      , 0, 'h' },
	{ "jump"          , required_argument, 0, 'j' },
	{ "proto"         , required_argument, 0, 'p' },
	{ "protocol"      , required_argument, 0, 'p' },
	{ "db"            , required_argument, 0, 'b' },
	{ "source"        , required_argument, 0, 's' },
	{ "src"           , required_argument, 0, 's' },
	{ "destination"   , required_argument, 0, 'd' },
	{ "dst"           , required_argument, 0, 'd' },
	{ "table"         , required_argument, 0, 't' },
	{ "modprobe"      , required_argument, 0, 'M' },
	{ "new-chain"     , required_argument, 0, 'N' },
	{ "rename-chain"  , required_argument, 0, 'E' },
	{ "delete-chain"  , optional_argument, 0, 'X' },
	{ "atomic-init"   , no_argument      , 0, 7   },
	{ "atomic-commit" , no_argument      , 0, 8   },
	{ "atomic-file"   , required_argument, 0, 9   },
	{ "atomic-save"   , no_argument      , 0, 10  },
	{ "init-table"    , no_argument      , 0, 11  },
	{ 0 }
};

static struct option *ebt_options = ebt_original_options;

char* standard_targets[NUM_STANDARD_TARGETS] =
{
	"ACCEPT",
	"DROP",
	"CONTINUE",
	"RETURN",
};

unsigned char mac_type_unicast[ETH_ALEN] =   {0,0,0,0,0,0};
unsigned char msk_type_unicast[ETH_ALEN] =   {1,0,0,0,0,0};
unsigned char mac_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
unsigned char msk_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
unsigned char mac_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
unsigned char msk_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
unsigned char mac_type_bridge_group[ETH_ALEN] = {0x01,0x80,0xc2,0,0,0};
unsigned char msk_type_bridge_group[ETH_ALEN] = {255,255,255,255,255,255};

/*
 * holds all the data
 */
static struct ebt_u_replace replace;

/*
 * the chosen table
 */
static struct ebt_u_table *table = NULL;
/*
 * the lists of supported tables, matches, watchers and targets
 */
static struct ebt_u_table *tables = NULL;
static struct ebt_u_match *matches = NULL;
static struct ebt_u_watcher *watchers = NULL;
static struct ebt_u_target *targets = NULL;

struct ebt_u_target *find_target(const char *name)
{
	struct ebt_u_target *t = targets;

	while(t && strcmp(t->name, name))
		t = t->next;
	return t;
}

struct ebt_u_match *find_match(const char *name)
{
	struct ebt_u_match *m = matches;

	while(m && strcmp(m->name, name))
		m = m->next;
	return m;
}

struct ebt_u_watcher *find_watcher(const char *name)
{
	struct ebt_u_watcher *w = watchers;

	while(w && strcmp(w->name, name))
		w = w->next;
	return w;
}

struct ebt_u_table *find_table(char *name)
{
	struct ebt_u_table *t = tables;

	while (t && strcmp(t->name, name))
		t = t->next;
	return t;
}

/*
 * The pointers in here are special:
 * The struct ebt_target * pointer is actually a struct ebt_u_target * pointer.
 * instead of making yet a few other structs, we just do a cast.
 * We need a struct ebt_u_target pointer because we know the address of the data
 * they point to won't change. We want to allow that the struct ebt_u_target.t
 * member can change.
 * Same holds for the struct ebt_match and struct ebt_watcher pointers
 */
struct ebt_u_entry *new_entry;

static void initialize_entry(struct ebt_u_entry *e)
{
	e->bitmask = EBT_NOPROTO;
	e->invflags = 0;
	e->ethproto = 0;
	strcpy(e->in, "");
	strcpy(e->out, "");
	strcpy(e->logical_in, "");
	strcpy(e->logical_out, "");
	e->m_list = NULL;
	e->w_list = NULL;
	/*
	 * the init function of the standard target should have put the verdict
	 * on CONTINUE
	 */
	e->t = (struct ebt_entry_target *)find_target(EBT_STANDARD_TARGET);
	if (!e->t)
		print_bug("Couldn't load standard target");
}

/*
 * this doesn't free e, becoz the calling function might need e->next
 */
static void free_u_entry(struct ebt_u_entry *e)
{
	struct ebt_u_match_list *m_l, *m_l2;
	struct ebt_u_watcher_list *w_l, *w_l2;

	m_l = e->m_list;
	while (m_l) {
		m_l2 = m_l->next;
		free(m_l->m);
		free(m_l);
		m_l = m_l2;
	}
	w_l = e->w_list;
	while (w_l) {
		w_l2 = w_l->next;
		free(w_l->w);
		free(w_l);
		w_l = w_l2;
	}
	free(e->t);
}

/*
 * the user will use the match, so put it in new_entry
 */
static void add_match(struct ebt_u_match *m)
{
	struct ebt_u_match_list **m_list, *new;

	m->used = 1;
	for (m_list = &new_entry->m_list; *m_list; m_list = &(*m_list)->next);
	new = (struct ebt_u_match_list *)
	   malloc(sizeof(struct ebt_u_match_list));
	if (!new)
		print_memory();
	*m_list = new;
	new->next = NULL;
	new->m = (struct ebt_entry_match *)m;
}

static void add_watcher(struct ebt_u_watcher *w)
{
	struct ebt_u_watcher_list **w_list;
	struct ebt_u_watcher_list *new;

	w->used = 1;
	for (w_list = &new_entry->w_list; *w_list; w_list = &(*w_list)->next);
	new = (struct ebt_u_watcher_list *)
	   malloc(sizeof(struct ebt_u_watcher_list));
	if (!new)
		print_memory();
	*w_list = new;
	new->next = NULL;
	new->w = (struct ebt_entry_watcher *)w;
}

static int global_option_offset = 0;
#define OPTION_OFFSET 256
static struct option *
merge_options(struct option *oldopts, const struct option *newopts,
	    unsigned int *options_offset)
{
	unsigned int num_old, num_new, i;
	struct option *merge;

	if (!newopts || !oldopts || !options_offset)
		print_bug("merge wrong");
	for (num_old = 0; oldopts[num_old].name; num_old++);
	for (num_new = 0; newopts[num_new].name; num_new++);

	global_option_offset += OPTION_OFFSET;
	*options_offset = global_option_offset;

	merge = malloc(sizeof(struct option) * (num_new + num_old + 1));
	if (!merge)
		print_memory();
	memcpy(merge, oldopts, num_old * sizeof(struct option));
	for (i = 0; i < num_new; i++) {
		merge[num_old + i] = newopts[i];
		merge[num_old + i].val += *options_offset;
	}
	memset(merge + num_old + num_new, 0, sizeof(struct option));
	/* only free dynamically allocated stuff */
	if (oldopts != ebt_original_options)
		free(oldopts);

	return merge;
}

void register_match(struct ebt_u_match *m)
{
	int size = EBT_ALIGN(m->size) + sizeof(struct ebt_entry_match);
	struct ebt_u_match **i;

	m->m = (struct ebt_entry_match *)malloc(size);
	if (!m->m)
		print_memory();
	strcpy(m->m->u.name, m->name);
	m->m->match_size = EBT_ALIGN(m->size);
	ebt_options = merge_options
	   (ebt_options, m->extra_ops, &(m->option_offset));
	m->init(m->m);

	for (i = &matches; *i; i = &((*i)->next));
	m->next = NULL;
	*i = m;
}

void register_watcher(struct ebt_u_watcher *w)
{
	int size = EBT_ALIGN(w->size) + sizeof(struct ebt_entry_watcher);
	struct ebt_u_watcher **i;

	w->w = (struct ebt_entry_watcher *)malloc(size);
	if (!w->w)
		print_memory();
	strcpy(w->w->u.name, w->name);
	w->w->watcher_size = EBT_ALIGN(w->size);
	ebt_options = merge_options
	   (ebt_options, w->extra_ops, &(w->option_offset));
	w->init(w->w);

	for (i = &watchers; *i; i = &((*i)->next));
	w->next = NULL;
	*i = w;
}

void register_target(struct ebt_u_target *t)
{
	int size = EBT_ALIGN(t->size) + sizeof(struct ebt_entry_target);
	struct ebt_u_target **i;

	t->t = (struct ebt_entry_target *)malloc(size);
	if (!t->t)
		print_memory();
	strcpy(t->t->u.name, t->name);
	t->t->target_size = EBT_ALIGN(t->size);
	ebt_options = merge_options
	   (ebt_options, t->extra_ops, &(t->option_offset));
	t->init(t->t);

	for (i = &targets; *i; i = &((*i)->next));
	t->next = NULL;
	*i = t;
}

void register_table(struct ebt_u_table *t)
{
	t->next = tables;
	tables = t;
}

const char *modprobe = NULL;
/*
 * blatently stolen (again) from iptables.c userspace program
 * find out where the modprobe utility is located
 */
static char *get_modprobe(void)
{
	int procfile;
	char *ret;

	procfile = open(PROC_SYS_MODPROBE, O_RDONLY);
	if (procfile < 0)
		return NULL;

	ret = malloc(1024);
	if (ret) {
		switch (read(procfile, ret, 1024)) {
		case -1: goto fail;
		case 1024: goto fail; /* Partial read.  Wierd */
		}
		if (ret[strlen(ret)-1]=='\n')
			ret[strlen(ret)-1]=0;
		close(procfile);
		return ret;
	}
 fail:
	free(ret);
	close(procfile);
	return NULL;
}

int ebtables_insmod(const char *modname)
{
	char *buf = NULL;
	char *argv[3];

	/* If they don't explicitly set it, read out of kernel */
	if (!modprobe) {
		buf = get_modprobe();
		if (!buf)
			return -1;
		modprobe = buf;
	}

	switch (fork()) {
	case 0:
		argv[0] = (char *)modprobe;
		argv[1] = (char *)modname;
		argv[2] = NULL;
		execv(argv[0], argv);

		/* not usually reached */
		exit(0);
	case -1:
		return -1;

	default: /* parent */
		wait(NULL);
	}

	free(buf);
	return 0;
}

static void list_extensions()
{
	struct ebt_u_table *tbl = tables;
        struct ebt_u_target *t = targets;
        struct ebt_u_match *m = matches;
        struct ebt_u_watcher *w = watchers;

	PRINT_VERSION;
	printf("Supported userspace extensions:\n\nSupported tables:\n");
        while(tbl) {
		printf("%s\n", tbl->name);
                tbl = tbl->next;
	}
	printf("\nSupported targets:\n");
        while(t) {
		printf("%s\n", t->name);
                t = t->next;
	}
	printf("\nSupported matches:\n");
        while(m) {
		printf("%s\n", m->name);
                m = m->next;
	}
	printf("\nSupported watchers:\n");
        while(w) {
		printf("%s\n", w->name);
                w = w->next;
	}
	exit(0);
}

/*
 * we use replace.flags, so we can't use the following values:
 * 0x01 == OPT_COMMAND, 0x02 == OPT_TABLE, 0x100 == OPT_ZERO
 */
#define LIST_N    0x04
#define LIST_C    0x08
#define LIST_X    0x10
#define LIST_MAC2 0x20

void print_mac(const char *mac)
{
	if (replace.flags & LIST_MAC2) {
		int j;
		for (j = 0; j < ETH_ALEN; j++)
			printf("%02x%s", (unsigned char)mac[j],
				(j==ETH_ALEN-1) ? "" : ":");
	} else
		printf("%s", ether_ntoa((struct ether_addr *) mac));
}

void print_mac_and_mask(const char *mac, const char *mask)
{
	char hlpmsk[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (!memcmp(mac, mac_type_unicast, 6) &&
	    !memcmp(mask, msk_type_unicast, 6))
		printf("Unicast");
	else if (!memcmp(mac, mac_type_multicast, 6) &&
	         !memcmp(mask, msk_type_multicast, 6))
		printf("Multicast");
	else if (!memcmp(mac, mac_type_broadcast, 6) &&
	         !memcmp(mask, msk_type_broadcast, 6))
		printf("Broadcast");
	else if (!memcmp(mac, mac_type_bridge_group, 6) &&
	         !memcmp(mask, msk_type_bridge_group, 6))
		printf("BGA");
	else {
		print_mac(mac);
		if (memcmp(mask, hlpmsk, 6)) {
			printf("/");
			print_mac(mask);
		}
	}
}

/*
 * helper function for list_rules()
 */
static void list_em(struct ebt_u_entries *entries)
{
	int i, j, space = 0, digits;
	struct ebt_u_entry *hlp;
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;
	struct ebt_u_match *m;
	struct ebt_u_watcher *w;
	struct ebt_u_target *t;

	hlp = entries->entries;
	if (replace.flags & LIST_X && entries->policy != EBT_ACCEPT) {
		printf("ebtables -t %s -P %s %s\n", replace.name,
		   entries->name, standard_targets[-entries->policy - 1]);
	} else if (!(replace.flags & LIST_X)) {
		printf("\nBridge chain: %s, entries: %d, policy: %s\n",
		   entries->name, entries->nentries,
		   standard_targets[-entries->policy - 1]);
	}

	i = entries->nentries;
	while (i > 9) {
		space++;
		i /= 10;
	}

	for (i = 0; i < entries->nentries; i++) {
		if (replace.flags & LIST_N) {
			digits = 0;
			/* A little work to get nice rule numbers. */
			j = i + 1;
			while (j > 9) {
				digits++;
				j /= 10;
			}
			for (j = 0; j < space - digits; j++)
				printf(" ");
			printf("%d. ", i + 1);
		}
		if (replace.flags & LIST_X)
			printf("ebtables -t %s -A %s ",
			   replace.name, entries->name);

		/*
		 * Don't print anything about the protocol if no protocol was
		 * specified, obviously this means any protocol will do.
		 */
		if (!(hlp->bitmask & EBT_NOPROTO)) {
			printf("-p ");
			if (hlp->invflags & EBT_IPROTO)
				printf("! ");
			if (hlp->bitmask & EBT_802_3)
				printf("Length ");
			else {
				struct ethertypeent *ent;

				ent = getethertypebynumber(ntohs(hlp->ethproto));
				if (!ent)
					printf("0x%x ", ntohs(hlp->ethproto));
				else
					printf("%s ", ent->e_name);
			}
		}
		if (hlp->bitmask & EBT_SOURCEMAC) {
			printf("-s ");
			if (hlp->invflags & EBT_ISOURCE)
				printf("! ");
			print_mac_and_mask(hlp->sourcemac, hlp->sourcemsk);
			printf(" ");
		}
		if (hlp->bitmask & EBT_DESTMAC) {
			printf("-d ");
			if (hlp->invflags & EBT_IDEST)
				printf("! ");
			print_mac_and_mask(hlp->destmac, hlp->destmsk);
			printf(" ");
		}
		if (hlp->in[0] != '\0') {
			printf("-i ");
			if (hlp->invflags & EBT_IIN)
				printf("! ");
			printf("%s ", hlp->in);
		}
		if (hlp->logical_in[0] != '\0') {
			printf("--logical-in ");
			if (hlp->invflags & EBT_ILOGICALIN)
				printf("! ");
			printf("%s ", hlp->logical_in);
		}
		if (hlp->logical_out[0] != '\0') {
			printf("--logical-out ");
			if (hlp->invflags & EBT_ILOGICALOUT)
				printf("! ");
			printf("%s ", hlp->logical_out);
		}
		if (hlp->out[0] != '\0') {
			printf("-o ");
			if (hlp->invflags & EBT_IOUT)
				printf("! ");
			printf("%s ", hlp->out);
		}

		m_l = hlp->m_list;
		while (m_l) {
			m = find_match(m_l->m->u.name);
			if (!m)
				print_bug("Match not found");
			m->print(hlp, m_l->m);
			m_l = m_l->next;
		}
		w_l = hlp->w_list;
		while (w_l) {
			w = find_watcher(w_l->w->u.name);
			if (!w)
				print_bug("Watcher not found");
			w->print(hlp, w_l->w);
			w_l = w_l->next;
		}

		printf("-j ");
		if (strcmp(hlp->t->u.name, EBT_STANDARD_TARGET))
			printf("%s ", hlp->t->u.name);
		t = find_target(hlp->t->u.name);
		if (!t)
			print_bug("Target not found");
		t->print(hlp, hlp->t);
		if (replace.flags & LIST_C)
			printf(", pcnt = %llu -- bcnt = %llu",
			   replace.counters[entries->counter_offset + i].pcnt,
			   replace.counters[entries->counter_offset + i].bcnt);
		printf("\n");
		hlp = hlp->next;
	}
}

struct ebt_u_entries *nr_to_chain(int nr)
{
	if (nr == -1)
		return NULL;
	if (nr < NF_BR_NUMHOOKS)
		return replace.hook_entry[nr];
	else {
		int i;
		struct ebt_u_chain_list *cl = replace.udc;

		i = nr - NF_BR_NUMHOOKS;
		while (i > 0 && cl) {
			cl = cl->next;
			i--;
		}
		if (cl)
			return cl->udc;
		else
			return NULL;
	}
}

static inline struct ebt_u_entries *to_chain()
{
	return nr_to_chain(replace.selected_hook);
}

struct ebt_u_stack
{
	int chain_nr;
	int n;
	struct ebt_u_entry *e;
	struct ebt_u_entries *entries;
};

static void check_for_loops()
{
	int chain_nr , i, j , k, sp = 0, verdict;
	struct ebt_u_entries *entries, *entries2;
	struct ebt_u_stack *stack = NULL;
	struct ebt_u_entry *e;

	i = -1;
	/*
	 * initialize hook_mask to 0
	 */
	while (1) {
		i++;
		if (i < NF_BR_NUMHOOKS && !(replace.valid_hooks & (1 << i)))
			continue;
		entries = nr_to_chain(i);
		if (!entries)
			break;
		entries->hook_mask = 0;
	}
	if (i > NF_BR_NUMHOOKS) {
		stack = (struct ebt_u_stack *)malloc((i - NF_BR_NUMHOOKS) *
		   sizeof(struct ebt_u_stack));
		if (!stack)
			print_memory();
	}

	/*
	 * check for loops, starting from every base chain
	 */
	for (i = 0; i < NF_BR_NUMHOOKS; i++) {
		if (!(replace.valid_hooks & (1 << i)))
			continue;
		entries = nr_to_chain(i);
		/*
		 * (1 << NF_BR_NUMHOOKS) implies it's a standard chain
		 * (usefull in the final_check() funtions)
		 */
		entries->hook_mask = (1 << i) | (1 << NF_BR_NUMHOOKS);
		chain_nr = i;

		e = entries->entries;
		for (j = 0; j < entries->nentries; j++) {
			if (strcmp(e->t->u.name, EBT_STANDARD_TARGET))
				goto letscontinue;
			verdict = ((struct ebt_standard_target *)(e->t))->verdict;
			if (verdict < 0)
				goto letscontinue;
			entries2 = nr_to_chain(verdict + NF_BR_NUMHOOKS);
			entries2->hook_mask |= entries->hook_mask;
			/*
			 * now see if we've been here before
			 */
			for (k = 0; k < sp; k++)
				if (stack[k].chain_nr == verdict + NF_BR_NUMHOOKS)
					print_error("Loop from chain %s to chain %s",
					   nr_to_chain(chain_nr)->name,
					   nr_to_chain(stack[k].chain_nr)->name);
			/*
			 * jump to the chain, make sure we know how to get back
			 */
			stack[sp].chain_nr = chain_nr;
			stack[sp].n = j;
			stack[sp].entries = entries;
			stack[sp].e = e;
			sp++;
			j = -1;
			e = entries2->entries;
			chain_nr = verdict + NF_BR_NUMHOOKS;
			entries = entries2;
			continue;
letscontinue:
			e = e->next;
		}
		/*
		 * we are at the end of a standard chain
		 */
		if (sp == 0)
			continue;
		/*
		 * go back to the chain one level higher
		 */
		sp--;
		j = stack[sp].n;
		chain_nr = stack[sp].chain_nr;
		e = stack[sp].e;
		entries = stack[sp].entries;
		goto letscontinue;
	}
	free(stack);
	return;
}

/*
 * parse the chain name and return the corresponding nr
 * returns -1 on failure
 */
int get_hooknr(char* arg)
{
	int i;
	struct ebt_u_chain_list *cl = replace.udc;

	for (i = 0; i < NF_BR_NUMHOOKS; i++) {
		if (!(replace.valid_hooks & (1 << i)))
			continue;
		if (!strcmp(arg, replace.hook_entry[i]->name))
			return i;
	}
	while(cl) {
		if (!strcmp(arg, cl->udc->name))
			return i;
		i++;
		cl = cl->next;
	}
	return -1;
}

static void print_help()
{
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;

	PRINT_VERSION;
	printf(
"Usage:\n"
"ebtables -[ADI] chain rule-specification [options]\n"
"ebtables -P chain target\n"
"ebtables -[LFZ] [chain]\n"
"ebtables -[NX] [chain]\n"
"ebtables -E old-chain-name new-chain-name\n\n"
"Commands:\n"
"--append -A chain             : append to chain\n"
"--delete -D chain             : delete matching rule from chain\n"
"--delete -D chain rulenum     : delete rule at position rulenum from chain\n"
"--insert -I chain rulenum     : insert rule at position rulenum in chain\n"
"--list   -L [chain]           : list the rules in a chain or in all chains\n"
"--flush  -F [chain]           : delete all rules in chain or in all chains\n"
"--init-table                  : replace the kernel table with the initial table\n"
"--zero   -Z [chain]           : put counters on zero in chain or in all chains\n"
"--policy -P chain target      : change policy on chain to target\n"
"--new-chain -N chain          : create a user defined chain\n"
"--rename-chain -E old new     : rename a chain\n"
"--delete-chain -X [chain]     : delete a user defined chain\n"
"--atomic-commit               : update the kernel w/t table contained in <FILE>\n"
"--atomic-init                 : put the initial kernel table into <FILE>\n"
"--atomic-save                 : put the current kernel table into <FILE>\n"
"--atomic-file file            : set <FILE> to file\n\n"
"Options:\n"
"--proto  -p [!] proto         : protocol hexadecimal, by name or LENGTH\n"
"--src    -s [!] address[/mask]: source mac address\n"
"--dst    -d [!] address[/mask]: destination mac address\n"
"--in-if  -i [!] name          : network input interface name\n"
"--out-if -o [!] name          : network output interface name\n"
"--logical-in  [!] name        : logical bridge input interface name\n"
"--logical-out [!] name        : logical bridge output interface name\n"
"--modprobe -M program         : try to insert modules using this program\n"
"--version -V                  : print package version\n\n"
"Environment variable:\n"
ATOMIC_ENV_VARIABLE "          : if set <FILE> (see above) will equal its value"
"\n\n");

	m_l = new_entry->m_list;
	while (m_l) {
		((struct ebt_u_match *)m_l->m)->help();
		printf("\n");
		m_l = m_l->next;
	}
	w_l = new_entry->w_list;
	while (w_l) {
		((struct ebt_u_watcher *)w_l->w)->help();
		printf("\n");
		w_l = w_l->next;
	}
	((struct ebt_u_target *)new_entry->t)->help();
	printf("\n");
	if (table->help)
		table->help(hooknames);
	exit(0);
}

/*
 * execute command L
 */
static void list_rules()
{
	int i;

	if (!(replace.flags & LIST_X))
		printf("Bridge table: %s\n", table->name);
	if (replace.selected_hook != -1) {
		list_em(to_chain());
	} else {
		struct ebt_u_chain_list *cl = replace.udc;

		/*
		 * create new chains and rename standard chains when necessary
		 */
		if (replace.flags & LIST_X) {
			while (cl) {
				printf("ebtables -t %s -N %s\n", replace.name,
				   cl->udc->name);
				cl = cl->next;
			}
			cl = replace.udc;
			for (i = 0; i < NF_BR_NUMHOOKS; i++)
				if (replace.valid_hooks & (1 << i) &&
				   strcmp(replace.hook_entry[i]->name, hooknames[i]))
					printf("ebtables -t %s -E %s %s\n",
					   replace.name, hooknames[i],
					   replace.hook_entry[i]->name);
		}
		i = 0;
		while (1) {
			if (i < NF_BR_NUMHOOKS) {
				if (replace.valid_hooks & (1 << i))
					list_em(replace.hook_entry[i]);
				i++;
				continue;
			} else {
				if (!cl)
					break;
				list_em(cl->udc);
				cl = cl->next;
			}
		}
	}
}

static void counters_nochange()
{
	int i;

	replace.num_counters = replace.nentries;
	if (replace.nentries) {
		/*
		 * '+ 1' for the CNT_END
		 */
		if (!(replace.counterchanges = (unsigned short *) malloc(
		   (replace.nentries + 1) * sizeof(unsigned short))))
			print_memory();
		/*
		 * done nothing special to the rules
		 */
		for (i = 0; i < replace.nentries; i++)
			replace.counterchanges[i] = CNT_NORM;
		replace.counterchanges[replace.nentries] = CNT_END;
	}
	else
		replace.counterchanges = NULL;
}

/*
 * execute command P
 */
static void change_policy(int policy)
{
	struct ebt_u_entries *entries = to_chain();

	/*
	 * don't do anything if the policy is the same
	 */
	if (entries->policy != policy) {
		entries->policy = policy;
		counters_nochange();
	} else
		exit(0);
}

/*
 * flush one chain or the complete table
 * -1 == nothing to do
 * 0 == give back to kernel
 */
static int flush_chains()
{
	int i, j, oldnentries, numdel;
	unsigned short *cnt;
	struct ebt_u_entry *u_e, *tmp;
	struct ebt_u_entries *entries = to_chain();

	/*
	 * flush whole table
	 */
	if (!entries) {
		if (replace.nentries == 0)
			return -1;
		replace.nentries = 0;
		/*
		 * no need for the kernel to give us counters back
		 */
		replace.num_counters = 0;

		/*
		 * free everything and zero (n)entries
		 */
		i = -1;
		while (1) {
			i++;
			entries = nr_to_chain(i);
			if (!entries) {
				if (i < NF_BR_NUMHOOKS)
					continue;
				else
					break;
			}
			entries->nentries = 0;
			entries->counter_offset = 0;
			u_e = entries->entries;
			entries->entries = NULL;
			while (u_e) {
				free_u_entry(u_e);
				tmp = u_e->next;
				free(u_e);
				u_e = tmp;
			}
		}
		return 0;
	}

	if (entries->nentries == 0)
		return -1;
	oldnentries = replace.nentries;
	replace.nentries -= entries->nentries;
	numdel = entries->nentries;

	if (replace.nentries) {
		/*
		 * +1 for CNT_END
		 */
		if ( !(replace.counterchanges = (unsigned short *)
		   malloc((oldnentries + 1) * sizeof(unsigned short))) )
			print_memory();
	}
	/*
	 * delete the counters belonging to the specified chain,
	 * update counter_offset
	 */
	i = -1;
	cnt = replace.counterchanges;
	while (1) {
		i++;
		entries = nr_to_chain(i);
		if (!entries) {
			if (i < NF_BR_NUMHOOKS)
				continue;
			else
				break;
		}
		if (i > replace.selected_hook)
			entries->counter_offset -= numdel;
		if (replace.nentries) {
			for (j = 0; j < entries->nentries; j++) {
				if (i == replace.selected_hook)
					*cnt = CNT_DEL;
				else
					*cnt = CNT_NORM;
				cnt++;
			}
		}
	}

	if (replace.nentries) {
		*cnt = CNT_END;
		replace.num_counters = oldnentries;
	} else
		replace.num_counters = 0;

	entries = to_chain();
	entries->nentries = 0;
	u_e = entries->entries;
	while (u_e) {
		free_u_entry(u_e);
		tmp = u_e->next;
		free(u_e);
		u_e = tmp;
	}
	entries->entries = NULL;
	return 0;
}

/*
 * -1 == no match
 */
static int check_rule_exists(int rule_nr)
{
	struct ebt_u_entry *u_e;
	struct ebt_u_match_list *m_l, *m_l2;
	struct ebt_u_match *m;
	struct ebt_u_watcher_list *w_l, *w_l2;
	struct ebt_u_watcher *w;
	struct ebt_u_target *t = (struct ebt_u_target *)new_entry->t;
	struct ebt_u_entries *entries = to_chain();
	int i, j, k;

	/*
	 * handle '-D chain rulenr' command
	 */
	if (rule_nr != 0) {
		if (rule_nr > entries->nentries)
			return -1;
		/*
		 * user starts counting from 1
		 */
		return rule_nr - 1;
	}
	u_e = entries->entries;
	/*
	 * check for an existing rule (if there are duplicate rules,
	 * take the first occurance)
	 */
	for (i = 0; i < entries->nentries; i++, u_e = u_e->next) {
		if (!u_e)
			print_bug("Hmm, trouble");
		if (u_e->ethproto != new_entry->ethproto)
			continue;
		if (strcmp(u_e->in, new_entry->in))
			continue;
		if (strcmp(u_e->out, new_entry->out))
			continue;
		if (strcmp(u_e->logical_in, new_entry->logical_in))
			continue;
		if (strcmp(u_e->logical_out, new_entry->logical_out))
			continue;
		if (new_entry->bitmask & EBT_SOURCEMAC &&
		   memcmp(u_e->sourcemac, new_entry->sourcemac, ETH_ALEN))
			continue;
		if (new_entry->bitmask & EBT_DESTMAC &&
		   memcmp(u_e->destmac, new_entry->destmac, ETH_ALEN))
			continue;
		if (new_entry->bitmask != u_e->bitmask ||
		   new_entry->invflags != u_e->invflags)
			continue;
		/*
		 * compare all matches
		 */
		m_l = new_entry->m_list;
		j = 0;
		while (m_l) {
			m = (struct ebt_u_match *)(m_l->m);
			m_l2 = u_e->m_list;
			while (m_l2 && strcmp(m_l2->m->u.name, m->m->u.name))
				m_l2 = m_l2->next;
			if (!m_l2 || !m->compare(m->m, m_l2->m))
				goto letscontinue;
			j++;
			m_l = m_l->next;
		}
		/*
		 * now be sure they have the same nr of matches
		 */
		k = 0;
		m_l = u_e->m_list;
		while (m_l) {
			k++;
			m_l = m_l->next;
		}
		if (j != k)
			continue;

		/*
		 * compare all watchers
		 */
		w_l = new_entry->w_list;
		j = 0;
		while (w_l) {
			w = (struct ebt_u_watcher *)(w_l->w);
			w_l2 = u_e->w_list;
			while (w_l2 && strcmp(w_l2->w->u.name, w->w->u.name))
				w_l2 = w_l2->next;
			if (!w_l2 || !w->compare(w->w, w_l2->w))
				goto letscontinue;
			j++;
			w_l = w_l->next;
		}
		k = 0;
		w_l = u_e->w_list;
		while (w_l) {
			k++;
			w_l = w_l->next;
		}
		if (j != k)
			continue;
		if (strcmp(t->t->u.name, u_e->t->u.name))
			continue;
		if (!t->compare(t->t, u_e->t))
			continue;
		return i;
letscontinue:;
	}
	return -1;
}

/* execute command A or I */
static void add_rule(int rule_nr)
{
	int i, j;
	struct ebt_u_entry **u_e;
	unsigned short *cnt;
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;
	struct ebt_u_entries *entries = to_chain(), *entries2;

	if (rule_nr <= 0)
		rule_nr += entries->nentries;
	else
		rule_nr--;
	if (rule_nr > entries->nentries || rule_nr < 0)
		print_error("The specified rule number is incorrect");
	/*
	 * we're adding one rule
	 */
	replace.num_counters = replace.nentries;
	replace.nentries++;
	entries->nentries++;

	/*
	 * handle counter stuff
	 * +1 for CNT_END
	 */
	if ( !(replace.counterchanges = (unsigned short *)
	   malloc((replace.nentries + 1) * sizeof(unsigned short))) )
		print_memory();
	cnt = replace.counterchanges;
	for (i = 0; i < replace.selected_hook; i++) {
		if (i < NF_BR_NUMHOOKS && !(replace.valid_hooks & (1 << i)))
			continue;
		entries2 = nr_to_chain(i);
		for (j = 0; j < entries2->nentries; j++) {
			*cnt = CNT_NORM;
			cnt++;
		}
	}
	for (i = 0; i < rule_nr; i++) {
		*cnt = CNT_NORM;
		cnt++;
	}
	*cnt = CNT_ADD;
	cnt++;
	while (cnt != replace.counterchanges + replace.nentries) {
		*cnt = CNT_NORM;
		cnt++;
	}
	*cnt = CNT_END;

	/*
	 * go to the right position in the chain
	 */
	u_e = &entries->entries;
	for (i = 0; i < rule_nr; i++)
		u_e = &(*u_e)->next;
	/*
	 * insert the rule
	 */
	new_entry->next = *u_e;
	*u_e = new_entry;

	/*
	 * put the ebt_[match, watcher, target] pointers in place
	 */
	m_l = new_entry->m_list;
	while (m_l) {
		m_l->m = ((struct ebt_u_match *)m_l->m)->m;
		m_l = m_l->next;
	}
	w_l = new_entry->w_list;
	while (w_l) {
		w_l->w = ((struct ebt_u_watcher *)w_l->w)->w;
		w_l = w_l->next;
	}
	new_entry->t = ((struct ebt_u_target *)new_entry->t)->t;

	/*
	 * update the counter_offset of chains behind this one
	 */
	i = replace.selected_hook;
	while (1) {
		i++;
		entries = nr_to_chain(i);
		if (!entries) {
			if (i < NF_BR_NUMHOOKS)
				continue;
			else
				break;
		} else
			entries->counter_offset++;
	}
}

/*
 * execute command D
 */
static void delete_rule(int begin, int end)
{
	int j, lentmp = 0, nr_deletes;
	unsigned short *cnt;
	struct ebt_u_entry **u_e, *u_e2;
	struct ebt_u_entries *entries = to_chain(), *entries2;

	if (begin < 0)
		begin += entries->nentries + 1;
	if (end < 0)
		end += entries->nentries + 1;

	if (begin < 0 || begin > end || end > entries->nentries)
		print_error("Sorry, wrong rule numbers");

	if ((begin = check_rule_exists(begin)) == -1 ||
	    (end = check_rule_exists(end)) == -1)
		print_error("Sorry, rule does not exist");

	/*
	 * we're deleting rules
	 */
	replace.num_counters = replace.nentries;
	nr_deletes = end - begin + 1;
	replace.nentries -= nr_deletes;
	entries->nentries -= nr_deletes;

	if (replace.nentries) {
		for (j = 0; j < replace.selected_hook; j++) {
			if (j < NF_BR_NUMHOOKS &&
			   !(replace.valid_hooks & (1 << j)))
				continue;
			entries2 = nr_to_chain(j);
			lentmp += entries2->nentries;
		}
		lentmp += begin;
		/*
		 * +1 for CNT_END
		 */
		if ( !(replace.counterchanges = (unsigned short *)malloc(
		   (replace.num_counters + 1) * sizeof(unsigned short))) )
			print_memory();
		cnt = replace.counterchanges;
		for (j = 0; j < lentmp; j++, cnt++)
			*cnt = CNT_NORM;
		for (j = 0; j < nr_deletes; j++, cnt++)
			*cnt = CNT_DEL;
  
		for (j = 0; j < replace.num_counters - lentmp - nr_deletes;
		     j++, cnt++)
			*cnt = CNT_NORM;
  
		*cnt = CNT_END;
	}
	else
		replace.num_counters = 0;

	/*
	 * go to the right position in the chain
	 */
	u_e = &entries->entries;
	for (j = 0; j < begin; j++)
		u_e = &(*u_e)->next;
	/*
	 * remove the rules
	 */
	j = nr_deletes;
	while(j--) {
		u_e2 = *u_e;
		*u_e = (*u_e)->next;
		/* free everything */
		free_u_entry(u_e2);
		free(u_e2);
	}

	/*
	 * update the counter_offset of chains behind this one
	 */
	j = replace.selected_hook;
	while (1) {
		j++;
		entries = nr_to_chain(j);
		if (!entries) {
			if (j < NF_BR_NUMHOOKS)
				continue;
			else
				break;
		} else 
			entries->counter_offset -= nr_deletes;
	}
}

/*
 * execute command Z
 */
static void zero_counters(int zerochain)
{

	if (zerochain == -1) {
		/*
		 * tell main() we don't update the counters
		 * this results in tricking the kernel to zero its counters,
		 * naively expecting userspace to update its counters. Muahahaha
		 */
		replace.counterchanges = NULL;
		replace.num_counters = 0;
	} else {
		int i, j;
		unsigned short *cnt;
		struct ebt_u_entries *entries = nr_to_chain(zerochain), *e2;

		if (entries->nentries == 0)
			exit(0);
		replace.counterchanges = (unsigned short *)
		   malloc((replace.nentries + 1) * sizeof(unsigned short));
		if (!replace.counterchanges)
			print_memory();
		cnt = replace.counterchanges;
		for (i = 0; i < zerochain; i++) {
			if (i < NF_BR_NUMHOOKS &&
			   !(replace.valid_hooks & (1 << i)))
				continue;
			e2 = nr_to_chain(i);
			for (j = 0; j < e2->nentries; j++) {
				*cnt = CNT_NORM;
				cnt++;
			}
		}
		for (i = 0; i < entries->nentries; i++) {
			*cnt = CNT_ZERO;
			cnt++;
		}
		while (cnt != replace.counterchanges + replace.nentries) {
			*cnt = CNT_NORM;
			cnt++;
		}
		*cnt = CNT_END;
	}
}

/*
 * Checks the type for validity and calls getethertypebynumber()
 */
struct ethertypeent *parseethertypebynumber(int type)
{
	if (type < 1536)
		print_error("Ethernet protocols have values >= 0x0600");
	if (type > 0xffff)
		print_error("Ethernet protocols have values <= 0xffff");
	return getethertypebynumber(type);
}

/*
 * put the mac address into 6 (ETH_ALEN) bytes
 * returns 0 on success
 */
int get_mac_and_mask(char *from, char *to, char *mask)
{
	char *p;
	int i;
	struct ether_addr *addr;

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, mac_type_unicast, ETH_ALEN);
		memcpy(mask, msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, mac_type_multicast, ETH_ALEN);
		memcpy(mask, msk_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, mac_type_broadcast, ETH_ALEN);
		memcpy(mask, msk_type_broadcast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "BGA") == 0) {
		memcpy(to, mac_type_bridge_group, ETH_ALEN);
		memcpy(mask, msk_type_bridge_group, ETH_ALEN);
		return 0;
	}
	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		if (!(addr = ether_aton(p + 1)))
			return -1;
		memcpy(mask, addr, ETH_ALEN);
	} else
		memset(mask, 0xff, ETH_ALEN);
	if (!(addr = ether_aton(from)))
		return -1;
	memcpy(to, addr, ETH_ALEN);
	for (i = 0; i < ETH_ALEN; i++)
		to[i] &= mask[i];
	return 0;
}

/*
 * executes the final_check() function for all extensions used by the rule
 */
static void do_final_checks(struct ebt_u_entry *e, struct ebt_u_entries *entries)
{
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;
	struct ebt_u_target *t;
	struct ebt_u_match *m;
	struct ebt_u_watcher *w;

	m_l = e->m_list;
	w_l = e->w_list;
	while (m_l) {
		m = find_match(m_l->m->u.name);
		m->final_check(e, m_l->m, replace.name,
		   entries->hook_mask, 1);
		m_l = m_l->next;
	}
	while (w_l) {
		w = find_watcher(w_l->w->u.name);
		w->final_check(e, w_l->w, replace.name,
		   entries->hook_mask, 1);
		w_l = w_l->next;
	}
	t = find_target(e->t->u.name);
	t->final_check(e, e->t, replace.name,
	   entries->hook_mask, 1);
}

/*
 * used for the -X command
 * type = 0 => update chain jumps
 * type = 1 => check for reference
 */
static int iterate_entries(int chain_nr, int silent, int type)
{
	int i = -1, j;
	struct ebt_u_entries *entries;
	struct ebt_u_entry *e;

	while (1) {
		i++;
		entries = nr_to_chain(i);
		if (!entries) {
			if (i < NF_BR_NUMHOOKS)
				continue;
			else
				break;
		}
		e = entries->entries;
		j = 0;
		while (e) {
			int chain_jmp;

			j++;
			if (strcmp(e->t->u.name, EBT_STANDARD_TARGET)) {
				e = e->next;
				continue;
			}
			chain_jmp = ((struct ebt_standard_target *)e->t)->verdict;
			switch (type) {
			case 1:
			if (chain_jmp == chain_nr) {
				if (silent)
					return 1;
				print_error("Can't delete the chain, it's referenced "
				   "in chain %s, rule %d", entries->name, j);
			}
			break;
			case 0:
			/* adjust the chain jumps when necessary */
			if (chain_jmp > chain_nr)
				((struct ebt_standard_target *)e->t)->verdict--;
			break;
			} /* end switch */
			e = e->next;
		}
	}
	return 0;
}

static void decrease_chain_jumps(int chain_nr)
{
	iterate_entries(chain_nr, 1, 0);
}

static int check_for_references(int chain_nr, int silent)
{
	return iterate_entries(chain_nr, silent, 1);
}

static int *determine_referenced_chains(int *n)
{
	int *nrs, i = 0, j = 0;

	*n = 0;
	while (nr_to_chain(i + NF_BR_NUMHOOKS)) {
		if (check_for_references(i, 1))
			(*n)++;
		i++;
	}
	if (*n == 0)
		return NULL;
	nrs = malloc(*n * sizeof(int));
	i = 0;
	while (nr_to_chain(i + NF_BR_NUMHOOKS)) {
		if (check_for_references(i, 1)) {
			nrs[j] = i;
			j++;
		}
		i++;
	}
	return nrs;
}

static void remove_udc(int udc_nr)
{
	struct ebt_u_chain_list *cl, **cl2;
	struct ebt_u_entries *entries;
	struct ebt_u_entry *u_e, *tmp;

	/* first free the rules */
	entries = nr_to_chain(udc_nr + NF_BR_NUMHOOKS);
	u_e = entries->entries;
	while (u_e) {
		free_u_entry(u_e);
		tmp = u_e->next;
		free(u_e);
		u_e = tmp;
	}

	/* next, remove the chain */
	cl2 = &(replace.udc);
	while ((*cl2)->udc != entries)
		cl2 = &((*cl2)->next);
	cl = (*cl2);
	(*cl2) = (*cl2)->next;
	free(cl->udc);
	free(cl);
}

/* Removes all udc that aren't referenced at the time of execution */
static void delete_all_user_chains()
{
	struct ebt_u_chain_list *chain;
	int *ref, nr_ref, chain_nr = 0, counter_offset, i;
	struct ebt_u_entries *entries;

	/* initialize counterchanges */
	counters_nochange();

	ref = determine_referenced_chains(&nr_ref);

	chain = replace.udc;
	counter_offset = 0;
	/* skip the standard chains */
	for (i = 0; i < NF_BR_NUMHOOKS; i++)
		if ((entries = nr_to_chain(i)) != NULL)
			counter_offset += entries->nentries;

	/* first update chain jumps and counterchanges */
	while (chain) {
		int nentries;

		nentries = chain->udc->nentries;
		for (i = 0; i < nr_ref; i++)
			if (ref[i] == chain_nr)
				goto letscontinue;
		decrease_chain_jumps(chain_nr);
		for (i = counter_offset; i < counter_offset + nentries; i++)
			replace.counterchanges[i] = CNT_DEL;
		replace.nentries -= nentries;
letscontinue:
		counter_offset += nentries;
		chain = chain->next;
		chain_nr++;
	}		
	chain = replace.udc;
	chain_nr = -1;
	/* next, remove the chains, update the counter offset of
	 * non-removed chains */
	counter_offset = 0;
	while (chain) {
		int real_cn = 0;

		chain_nr++;
		for (i = 0; i < nr_ref; i++)
			if (ref[i] == chain_nr)
				break;
		if (i != nr_ref) {
			real_cn++;
			chain->udc->counter_offset -= counter_offset;
			chain = chain->next;
			continue;
		}
		counter_offset += chain->udc->nentries;
		chain = chain->next;
		remove_udc(real_cn);
	}
}

static int parse_delete_rule(const char *argv, int *rule_nr, int *rule_nr_end)
{
	char *colon = strchr(argv, ':'), *buffer;

	if (colon) {
		*colon = '\0';
		if (*(colon + 1) == '\0')
			*rule_nr_end = -1; /* until the last rule */
		else {
			*rule_nr_end = strtol(colon + 1, &buffer, 10);
			if (*buffer != '\0' || *rule_nr_end == 0)
				return -1;
		}
	}
	if (colon == argv)
		*rule_nr = 1; /* beginning with the first rule */
	else {
		*rule_nr = strtol(argv, &buffer, 10);
		if (*buffer != '\0' || *rule_nr == 0)
			return -1;
	}
	if (!colon)
		*rule_nr_end = *rule_nr;
	return 0;
}

static int invert = 0;
int check_inverse(const char option[])
{
	if (strcmp(option, "!") == 0) {
		if (invert == 1)
			print_error("double use of '!' not allowed");
		optind++;
		invert = 1;
		return 1;
	}
	return invert;
}

void check_option(unsigned int *flags, unsigned int mask)
{
	if (*flags & mask)
		print_error("Multiple use of same option not allowed");
	*flags |= mask;
}

static void get_kernel_table()
{
	if ( !(table = find_table(replace.name)) )
		print_error("Bad table name");
	/*
	 * get the kernel's information
	 */
	if (get_table(&replace)) {
		ebtables_insmod("ebtables");
		if (get_table(&replace))
			print_error("The kernel doesn't support the ebtables "
			"%s table", replace.name);
	}
	/*
	 * when listing a table contained in a file, we don't demand that
	 * the user knows the table's name
	 */
	if ( !(table = find_table(replace.name)) )
		print_error("Bad table name");
}

#define print_if_l_error print_error("Interface name length must be less " \
   "than %d", IFNAMSIZ)
#define OPT_COMMAND    0x01
#define OPT_TABLE      0x02
#define OPT_IN         0x04
#define OPT_OUT        0x08
#define OPT_JUMP       0x10
#define OPT_PROTOCOL   0x20
#define OPT_SOURCE     0x40
#define OPT_DEST       0x80
#define OPT_ZERO       0x100
#define OPT_LOGICALIN  0x200
#define OPT_LOGICALOUT 0x400
/* the main thing */
int main(int argc, char *argv[])
{
	char *buffer;
	int c, i;
	/*
	 * this special one for the -Z option (we can have -Z <this> -L <that>)
	 */
	int zerochain = -1;
	int policy = 0;
	int rule_nr = 0; /* used for -[D,I] */
	int rule_nr_end = 0; /* used for -I */
	struct ebt_u_target *t;
	struct ebt_u_match *m;
	struct ebt_u_watcher *w;
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;
	struct ebt_u_entries *entries;

	opterr = 0;

	replace.filename = getenv(ATOMIC_ENV_VARIABLE);
	/*
	 * initialize the table name, OPT_ flags, selected hook and command
	 */
	strcpy(replace.name, "filter");
	replace.flags = 0;
	replace.selected_hook = -1;
	replace.command = 'h';
	replace.counterchanges = NULL;

	new_entry = (struct ebt_u_entry *)malloc(sizeof(struct ebt_u_entry));
	if (!new_entry)
		print_memory();
	/*
	 * put some sane values in our new entry
	 */
	initialize_entry(new_entry);

	/*
	 * The scenario induced by this loop makes that:
	 * '-t'  ,'-M' and --atomic (if specified) have to come
	 * before '-A' and the like
	 */

	/*
	 * getopt saves the day
	 */
	while ((c = getopt_long(argc, argv,
	   "-A:D:I:N:E:X::L::Z::F::P:Vhi:o:j:p:s:d:t:M:", ebt_options, NULL)) != -1) {
		switch (c) {

		case 'A': /* add a rule */
		case 'D': /* delete a rule */
		case 'P': /* define policy */
		case 'I': /* insert a rule */
		case 'N': /* make a user defined chain */
		case 'E': /* rename chain */
		case 'X': /* delete chain */
			replace.command = c;
			if (replace.flags & OPT_COMMAND)
				print_error("Multiple commands not allowed");
			replace.flags |= OPT_COMMAND;
			get_kernel_table();
			if (optarg && (optarg[0] == '-' ||
			    !strcmp(optarg, "!")))
				print_error("No chain name specified");
			if (c == 'N') {
				struct ebt_u_chain_list *cl, **cl2;

				if (get_hooknr(optarg) != -1)
					print_error("Chain %s already exists",
					   optarg);
				if (find_target(optarg))
					print_error("Target with name %s exists"
					   , optarg);
				if (strlen(optarg) >= EBT_CHAIN_MAXNAMELEN)
					print_error("Chain name length can't exceed %d",
					   EBT_CHAIN_MAXNAMELEN - 1);
				cl = (struct ebt_u_chain_list *)
				   malloc(sizeof(struct ebt_u_chain_list));
				if (!cl)
					print_memory();
				cl->next = NULL;
				cl->udc = (struct ebt_u_entries *)
				   malloc(sizeof(struct ebt_u_entries));
				if (!cl->udc)
					print_memory();
				cl->udc->nentries = 0;
				cl->udc->policy = EBT_ACCEPT;
				cl->udc->counter_offset = replace.nentries;
				cl->udc->hook_mask = 0;
				strcpy(cl->udc->name, optarg);
				cl->udc->entries = NULL;
				cl->kernel_start = NULL;
				/*
				 * put the new chain at the end
				 */
				cl2 = &replace.udc;
				while (*cl2)
					cl2 = &((*cl2)->next);
				*cl2 = cl;
				counters_nochange();
				break;
			}
			if (c == 'X') {
				char *opt;
				int udc_nr;

				if (!optarg && (optind >= argc ||
				   (argv[optind][0] == '-'
				    && strcmp(argv[optind], "!")))) {
					delete_all_user_chains();
					break;
				}
				if (optarg)
					opt = optarg;
				else {
					opt = argv[optind];
					optind++;
				}
				if ((replace.selected_hook = get_hooknr(opt)) == -1)
					print_error("Chain %s doesn't exist", optarg);
				if (replace.selected_hook < NF_BR_NUMHOOKS)
					print_error("You can't remove a standard chain");
				/*
				 * if the chain is referenced, don't delete it,
				 * also decrement jumps to a chain behind the
				 * one we're deleting
				 */
				udc_nr=replace.selected_hook-NF_BR_NUMHOOKS;
				check_for_references(udc_nr, 0);
				decrease_chain_jumps(udc_nr);
				if (flush_chains() == -1)
					counters_nochange();
				remove_udc(udc_nr);
				break;
			}

			if ((replace.selected_hook = get_hooknr(optarg)) == -1)
				print_error("Chain %s doesn't exist", optarg);
			if (c == 'E') {
				if (optind >= argc || argv[optind][0] == '-' ||
				   !strcmp(argv[optind], "!"))
					print_error("No new chain name specified");
				if (strlen(argv[optind]) >= EBT_CHAIN_MAXNAMELEN)
					print_error("Chain name len can't exceed %d",
					   EBT_CHAIN_MAXNAMELEN - 1);
				if (get_hooknr(argv[optind]) != -1)
					print_error("Chain %s already exists",
					   argv[optind]);
				if (find_target(argv[optind]))
					print_error("Target with name %s exists"
					   , argv[optind]);
				entries = to_chain();
				strcpy(entries->name, argv[optind]);
				counters_nochange();
				optind++;
				break;
			}

			if (c == 'D' && optind < argc &&
			    (argv[optind][0] != '-' ||
			    (argv[optind][1] >= '0' && argv[optind][1] <= '9'))) {
				if (parse_delete_rule(argv[optind],
				    &rule_nr, &rule_nr_end))
					print_error("Problem with the "
					            "specified rule number(s)");
				optind++;
			}
			if (c == 'I') {
				if (optind >= argc || (argv[optind][0] == '-' &&
				    (argv[optind][1] < '0' || argv[optind][1] > '9')))
					print_error("No rulenr for -I"
					            " specified");
				rule_nr = strtol(argv[optind], &buffer, 10);
				if (*buffer != '\0')
					print_error("Problem with the "
					            "specified rule number");
				optind++;
			}
			if (c == 'P') {
				if (optind >= argc)
					print_error("No policy specified");
				policy = 0;
				for (i = 0; i < NUM_STANDARD_TARGETS; i++)
					if (!strcmp(argv[optind],
					   standard_targets[i])) {
						policy = -i -1;
						if (policy == EBT_CONTINUE)
							policy = 0;
						break;
					}
				if (policy == 0)
					print_error("Wrong policy");
				optind++;
			}
			break;

		case 'L': /* list */
		case 'F': /* flush */
		case 'Z': /* zero counters */
			if (c == 'Z') {
				if (replace.flags & OPT_ZERO)
					print_error("Multiple commands"
					            " not allowed");
				if ( (replace.flags & OPT_COMMAND &&
				   replace.command != 'L'))
					print_error("command -Z only allowed "
					            "together with command -L");
				replace.flags |= OPT_ZERO;
			} else {
				replace.command = c;
				if (replace.flags & OPT_COMMAND)
					print_error("Multiple commands"
					            " not allowed");
				replace.flags |= OPT_COMMAND;
			}
			get_kernel_table();
			i = -1;
			if (optarg) {
				if ( (i = get_hooknr(optarg)) == -1 )
					print_error("Bad chain");
			} else
				if (optind < argc && argv[optind][0] != '-') {
					if ((i = get_hooknr(argv[optind])) == -1)
						print_error("Bad chain");
					optind++;
				}
			if (i != -1) {
				if (c == 'Z')
					zerochain = i;
				else
					replace.selected_hook = i;
			}
			break;

		case 'V': /* version */
			replace.command = 'V';
			if (replace.flags & OPT_COMMAND)
				print_error("Multiple commands not allowed");
			PRINT_VERSION;
			exit(0);

		case 'M': /* modprobe */
			if (replace.command != 'h')
				print_error("Please put the -M option earlier");
			modprobe = optarg;
			break;

		case 'h': /* help */
			if (replace.flags & OPT_COMMAND)
				print_error("Multiple commands not allowed");
			replace.command = 'h';
			/*
			 * All other arguments should be extension names
			 */
			while (optind < argc) {
				struct ebt_u_match *m;
				struct ebt_u_watcher *w;

				if (!strcasecmp("list_extensions",
				   argv[optind]))
					list_extensions();
					
				if ((m = find_match(argv[optind])))
					add_match(m);
				else if ((w = find_watcher(argv[optind])))
					add_watcher(w);
				else {
					if (!(t = find_target(argv[optind])))
						print_error("Extension %s "
						   "not found", argv[optind]);
					if (replace.flags & OPT_JUMP)
						print_error("Sorry, you can "
						 "only see help for one "
						 "target extension each time");
					replace.flags |= OPT_JUMP;
					new_entry->t =
					   (struct ebt_entry_target *)t;
				}
				optind++;
			}
			break;

		case 't': /* table */
			if (replace.command != 'h')
				print_error("Please put the -t option first");
			check_option(&replace.flags, OPT_TABLE);
			if (strlen(optarg) > EBT_TABLE_MAXNAMELEN - 1)
				print_error("Table name too long");
			strcpy(replace.name, optarg);
			break;

		case 'i': /* input interface */
		case 2  : /* logical input interface */
		case 'o': /* output interface */
		case 3  : /* logical output interface */
		case 'j': /* target */
		case 'p': /* net family protocol */
		case 's': /* source mac */
		case 'd': /* destination mac */
			if ((replace.flags & OPT_COMMAND) == 0)
				print_error("No command specified");
			if ( replace.command != 'A' &&
			   replace.command != 'D' && replace.command != 'I')
				print_error("Command and option do not match");
			if (c == 'i') {
				check_option(&replace.flags, OPT_IN);
				if (replace.selected_hook > 2 &&
				   replace.selected_hook < NF_BR_BROUTING)
					print_error("Use in-interface only in "
					   "INPUT, FORWARD, PREROUTING and"
					   "BROUTING chains");
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_IIN;

				if (optind > argc)
					print_error("No in-interface "
					            "specified");
				if (strlen(argv[optind - 1]) >= IFNAMSIZ)
					print_if_l_error;
				strcpy(new_entry->in, argv[optind - 1]);
				break;
			}
			if (c == 2) {
				check_option(&replace.flags, OPT_LOGICALIN);
				if (replace.selected_hook > 2 &&
				   replace.selected_hook < NF_BR_BROUTING)
					print_error("Use logical in-interface "
					   "only in INPUT, FORWARD, "
					   "PREROUTING and BROUTING chains");
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_ILOGICALIN;

				if (optind > argc)
					print_error("No logical in-interface "
					            "specified");
				if (strlen(argv[optind - 1]) >= IFNAMSIZ)
					print_if_l_error;
				strcpy(new_entry->logical_in, argv[optind - 1]);
				break;
			}
			if (c == 'o') {
				check_option(&replace.flags, OPT_OUT);
				if (replace.selected_hook < 2)
					print_error("Use out-interface only"
					   " in OUTPUT, FORWARD and "
					   "POSTROUTING chains");
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_IOUT;

				if (optind > argc)
					print_error("No out-interface "
					            "specified");

				if (strlen(argv[optind - 1]) >= IFNAMSIZ)
					print_if_l_error;
				strcpy(new_entry->out, argv[optind - 1]);
				break;
			}
			if (c == 3) {
				check_option(&replace.flags, OPT_LOGICALOUT);
				if (replace.selected_hook < 2)
					print_error("Use logical out-interface "
					   "only in OUTPUT, FORWARD and "
					   "POSTROUTING chains");
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_ILOGICALOUT;

				if (optind > argc)
					print_error("No logical out-interface "
					            "specified");

				if (strlen(argv[optind - 1]) >= IFNAMSIZ)
					print_if_l_error;
				strcpy(new_entry->logical_out,
				   argv[optind - 1]);
				break;
			}
			if (c == 'j') {
				check_option(&replace.flags, OPT_JUMP);
				for (i = 0; i < NUM_STANDARD_TARGETS; i++)
					if (!strcmp(optarg,
					   standard_targets[i])) {
						t = find_target(
						   EBT_STANDARD_TARGET);
						((struct ebt_standard_target *)
						   t->t)->verdict = -i - 1;
						break;
					}
				if (-i - 1 == EBT_RETURN) {
					if (replace.selected_hook < NF_BR_NUMHOOKS)
						print_error("Return target"
						" only for user defined chains");
				}
				if (i != NUM_STANDARD_TARGETS)
					break;
				if ((i = get_hooknr(optarg)) != -1) {
						if (i < NF_BR_NUMHOOKS)
							print_error("don't jump"
							  " to a standard chain");
						t = find_target(
						   EBT_STANDARD_TARGET);
						((struct ebt_standard_target *)
						   t->t)->verdict = i - NF_BR_NUMHOOKS;
						break;
					}
				else {
					/*
					 * must be an extension then
					 */
					struct ebt_u_target *t;

					t = find_target(optarg);
					/*
					 * -j standard not allowed either
					 */
					if (!t || t ==
					   (struct ebt_u_target *)new_entry->t)
						print_error("Illegal target "
						            "name");
					new_entry->t =
					   (struct ebt_entry_target *)t;
				}
				break;
			}
			if (c == 's') {
				check_option(&replace.flags, OPT_SOURCE);
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_ISOURCE;

				if (optind > argc)
					print_error("No source mac "
					            "specified");
				if (get_mac_and_mask(argv[optind - 1],
				   new_entry->sourcemac, new_entry->sourcemsk))
					print_error("Problem with specified "
					            "source mac");
				new_entry->bitmask |= EBT_SOURCEMAC;
				break;
			}
			if (c == 'd') {
				check_option(&replace.flags, OPT_DEST);
				if (check_inverse(optarg))
					new_entry->invflags |= EBT_IDEST;

				if (optind > argc)
					print_error("No destination mac "
					            "specified");
				if (get_mac_and_mask(argv[optind - 1],
				   new_entry->destmac, new_entry->destmsk))
					print_error("Problem with specified "
					            "destination mac");
				new_entry->bitmask |= EBT_DESTMAC;
				break;
			}
			check_option(&replace.flags, OPT_PROTOCOL);
			if (check_inverse(optarg))
				new_entry->invflags |= EBT_IPROTO;

			if (optind > argc)
				print_error("No protocol specified");
			new_entry->bitmask &= ~((unsigned int)EBT_NOPROTO);
			i = strtol(argv[optind - 1], &buffer, 16);
			if (*buffer == '\0' && (i < 0 || i > 0xFFFF))
				print_error("Problem with the specified "
				            "protocol");
			new_entry->ethproto = i;
			if (*buffer != '\0') {
				struct ethertypeent *ent;

				if (!strcasecmp(argv[optind - 1], "LENGTH")) {
					new_entry->bitmask |= EBT_802_3;
					break;
				}
				ent = getethertypebyname(argv[optind - 1]);
				if (!ent)
					print_error("Problem with the specified"
					            " protocol");
				new_entry->ethproto = ent->e_ethertype;
			}
			if (new_entry->ethproto < 1536 &&
			   !(new_entry->bitmask & EBT_802_3))
				print_error("Sorry, protocols have values above"
				            " or equal to 0x0600");
			break;

		case 4  : /* Lc */
			check_option(&replace.flags, LIST_C);
			if (replace.command != 'L')
				print_error("Use --Lc with -L");
			if (replace.flags & LIST_X)
				print_error("--Lx not compatible with --Lc");
			replace.flags |= LIST_C;
			break;
		case 5  : /* Ln */
			check_option(&replace.flags, LIST_N);
			if (replace.command != 'L')
				print_error("Use --Ln with -L");
			if (replace.flags & LIST_X)
				print_error("--Lx not compatible with --Ln");
			replace.flags |= LIST_N;
			break;
		case 6  : /* Lx */
			check_option(&replace.flags, LIST_X);
			if (replace.command != 'L')
				print_error("Use --Lx with -L");
			if (replace.flags & LIST_C)
				print_error("--Lx not compatible with --Lc");
			if (replace.flags & LIST_N)
				print_error("--Lx not compatible with --Ln");
			replace.flags |= LIST_X;
			break;
		case 12 : /* Lmac2 */
			check_option(&replace.flags, LIST_MAC2);
			if (replace.command != 'L')
				print_error("Use --Lmac2 with -L");
			replace.flags |= LIST_MAC2;
			break;
		case 8 : /* atomic-commit */
			replace.command = c;
			if (replace.flags & OPT_COMMAND)
				print_error("Multiple commands not allowed");
			replace.flags |= OPT_COMMAND;
			if (!replace.filename)
				print_error("No atomic file specified");
			/*
			 * get the information from the file
			 */
			get_table(&replace);
                        if (replace.nentries) {
                                replace.counterchanges = (unsigned short *)
                                   malloc(sizeof(unsigned short) * (replace.nentries + 1));
				for (i = 0; i < replace.nentries; i++)
					replace.counterchanges[i] = CNT_NORM;
					replace.counterchanges[i] = CNT_END;
                        }
			/*
			 * we don't want the kernel giving us its counters, they would
			 * overwrite the counters extracted from the file
			 */
			replace.num_counters = 0;
			/*
			 * make sure the table will be written to the kernel
			 * possible memory leak here
			 */
			replace.filename = NULL;
			break;
		case 7 : /* atomic-init */
		case 10: /* atomic-save */
		case 11: /* init-table */
			replace.command = c;
			if (replace.flags & OPT_COMMAND)
				print_error("Multiple commands not allowed");
			if (c != 11 && !replace.filename)
				print_error("No atomic file specified");
			replace.flags |= OPT_COMMAND;
			{
				char *tmp = replace.filename;

				tmp = replace.filename;
				/* get the kernel table */
				replace.filename = NULL;
				get_kernel_table();
				replace.filename = tmp;
			}
			if (replace.nentries) {
				replace.counterchanges = (unsigned short *)
				   malloc(sizeof(unsigned short) * (replace.nentries + 1));
				for (i = 0; i < replace.nentries; i++)
					replace.counterchanges[i] = CNT_NORM;
				replace.counterchanges[i] = CNT_END;
			}
			break;
		case 9 : /* atomic */
			if (replace.flags & OPT_COMMAND)
				print_error("--atomic has to come before"
				" the command");
			/* another possible memory leak here */
			replace.filename = (char *)malloc(strlen(optarg) + 1);
			strcpy(replace.filename, optarg);
			break;
		case 1 :
			if (!strcmp(optarg, "!"))
				check_inverse(optarg);
			else
				print_error("Bad argument : %s", optarg);
			/*
			 * check_inverse() did optind++
			 */
			optind--;
			continue;
		default:
			/*
			 * is it a target option?
			 */
			t = (struct ebt_u_target *)new_entry->t;
			if ((t->parse(c - t->option_offset, argv, argc,
			   new_entry, &t->flags, &t->t)))
				goto check_extension;

			/*
			 * is it a match_option?
			 */
			for (m = matches; m; m = m->next)
				if (m->parse(c - m->option_offset, argv,
				   argc, new_entry, &m->flags, &m->m))
					break;

			if (m != NULL) {
				if (m->used == 0)
					add_match(m);
				goto check_extension;
			}

			/*
			 * is it a watcher option?
			 */
			for (w = watchers; w; w = w->next)
				if (w->parse(c-w->option_offset, argv,
				   argc, new_entry, &w->flags, &w->w))
					break;

			if (w == NULL)
				print_error("Unknown argument");
			if (w->used == 0)
				add_watcher(w);
check_extension:
			if (replace.command != 'A' && replace.command != 'I' &&
			   replace.command != 'D')
				print_error("Extensions only for -A, -I and -D");
		}
		invert = 0;
	}

	if ( !table && !(table = find_table(replace.name)) )
		print_error("Bad table name");

	if ( (replace.flags & OPT_COMMAND) && replace.command != 'L' &&
	   replace.flags & OPT_ZERO )
		print_error("Command -Z only allowed together with command -L");

	/*
	 * do this after parsing everything, so we can print specific info
	 */
	if (replace.command == 'h' && !(replace.flags & OPT_ZERO))
		print_help();

	/*
	 * do the final checks
	 */
	if (replace.command == 'A' || replace.command == 'I' ||
	   replace.command == 'D') {
		/*
		 * this will put the hook_mask right for the chains
		 */
		check_for_loops();
		entries = to_chain();
		m_l = new_entry->m_list;
		w_l = new_entry->w_list;
		t = (struct ebt_u_target *)new_entry->t;
		while (m_l) {
			m = (struct ebt_u_match *)(m_l->m);
			m->final_check(new_entry, m->m, replace.name,
			   entries->hook_mask, 0);
			m_l = m_l->next;
		}
		while (w_l) {
			w = (struct ebt_u_watcher *)(w_l->w);
			w->final_check(new_entry, w->w, replace.name,
			   entries->hook_mask, 0);
			w_l = w_l->next;
		}
		t->final_check(new_entry, t->t, replace.name,
		   entries->hook_mask, 0);
	}
	/*
	 * so, the extensions can work with the host endian
	 * the kernel does not have to do this ofcourse
	 */
	new_entry->ethproto = htons(new_entry->ethproto);

	if (replace.command == 'P') {
		if (replace.selected_hook < NF_BR_NUMHOOKS &&
		   policy == EBT_RETURN)
			print_error("Policy RETURN only allowed for user "
			            "defined chains");
		change_policy(policy);
	} else if (replace.command == 'L') {
		list_rules();
		if (replace.flags & OPT_ZERO)
			zero_counters(zerochain);
		else
			exit(0);
	}
	if (replace.flags & OPT_ZERO)
		zero_counters(zerochain);
	else if (replace.command == 'F') {
		if (flush_chains() == -1)
			exit(0);
	} else if (replace.command == 'A' || replace.command == 'I') {
		add_rule(rule_nr);
		check_for_loops();
		/*
		 * do the final_check(), for all entries
		 * needed when adding a rule that has a chain target
		 */
		i = -1;
		while (1) {
			struct ebt_u_entry *e;

			i++;
			entries = nr_to_chain(i);
			if (!entries) {
				if (i < NF_BR_NUMHOOKS)
					continue;
				else
					break;
			}
			e = entries->entries;
			while (e) {
				/*
				 * userspace extensions use host endian
				 */
				e->ethproto = ntohs(e->ethproto);
				do_final_checks(e, entries);
				e->ethproto = htons(e->ethproto);
				e = e->next;
			}
		}
	} else if (replace.command == 'D')
		delete_rule(rule_nr, rule_nr_end);
	/*
	 * commands -N, -E, -X, --atomic-commit, --atomic-commit, --atomic-save,
	 * --init-table fall through
	 */

	if (table->check)
		table->check(&replace);

	deliver_table(&replace);

	if (replace.counterchanges)
		deliver_counters(&replace);
	return 0;
}
