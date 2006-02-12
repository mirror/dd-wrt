/*
 * communication.c, v2.0 July 2002
 *
 * Author: Bart De Schuymer
 *
 */

/*
 * All the userspace/kernel communication is in this file.
 * The other code should not have to know anything about the way the
 * kernel likes the structure of the table data.
 * The other code works with linked lists, lots of linked lists.
 * So, the translation is done here.
 */

#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "include/ebtables_u.h"

extern char* hooknames[NF_BR_NUMHOOKS];

#ifdef KERNEL_64_USERSPACE_32
#define sparc_cast (uint64_t)
#else
#define sparc_cast
#endif

int sockfd = -1;

static void get_sockfd()
{
	if (sockfd == -1) {
		sockfd = socket(AF_INET, SOCK_RAW, PF_INET);
		if (sockfd < 0)
			print_error("Problem getting a socket, "
			   "you probably don't have the right "
			   "permissions");
	}
}

static struct ebt_replace * translate_user2kernel(struct ebt_u_replace *u_repl)
{
	struct ebt_replace *new;
	struct ebt_u_entry *e;
	struct ebt_u_match_list *m_l;
	struct ebt_u_watcher_list *w_l;
	struct ebt_u_chain_list *cl;
	struct ebt_u_entries *entries;
	char *p, *base;
	int i, j;
	unsigned int entries_size = 0, *chain_offsets;

	new = (struct ebt_replace *)malloc(sizeof(struct ebt_replace));
	if (!new)
		print_memory();
	new->valid_hooks = u_repl->valid_hooks;
	strcpy(new->name, u_repl->name);
	new->nentries = u_repl->nentries;
	new->num_counters = u_repl->num_counters;
	new->counters = sparc_cast u_repl->counters;
	/* determine nr of udc */
	i = 0;
	cl = u_repl->udc;
	while (cl) {
		i++;
		cl = cl->next;
	}
	i += NF_BR_NUMHOOKS;
	chain_offsets = (unsigned int *)malloc(i * sizeof(unsigned int));
	/* determine size */
	i = 0;
	cl = u_repl->udc;
	while (1) {
		if (i < NF_BR_NUMHOOKS) {
			if (!(new->valid_hooks & (1 << i))) {
				i++;
				continue;
			}
			entries = u_repl->hook_entry[i];
		} else {
			if (!cl)
				break;
			entries = cl->udc;
		}
		chain_offsets[i] = entries_size;
		entries_size += sizeof(struct ebt_entries);
		j = 0;
		e = entries->entries;
		while (e) {
			j++;
			entries_size += sizeof(struct ebt_entry);
			m_l = e->m_list;
			while (m_l) {
				entries_size += m_l->m->match_size +
				   sizeof(struct ebt_entry_match);
				m_l = m_l->next;
			}
			w_l = e->w_list;
			while (w_l) {
				entries_size += w_l->w->watcher_size +
				   sizeof(struct ebt_entry_watcher);
				w_l = w_l->next;
			}
			entries_size += e->t->target_size +
			   sizeof(struct ebt_entry_target);
			e = e->next;
		}
		/* a little sanity check */
		if (j != entries->nentries)
			print_bug("Wrong nentries: %d != %d, hook = %s", j,
			   entries->nentries, entries->name);
		if (i >= NF_BR_NUMHOOKS)
			cl = cl->next;
		i++;
	}

	new->entries_size = entries_size;
	p = (char *)malloc(entries_size);
	if (!p)
		print_memory();

	/* put everything in one block */
	new->entries = sparc_cast p;
	i = 0;
	cl = u_repl->udc;
	while (1) {
		struct ebt_entries *hlp;

		hlp = (struct ebt_entries *)p;
		if (i < NF_BR_NUMHOOKS) {
			if (!(new->valid_hooks & (1 << i))) {
				i++;
				continue;
			}
			entries = u_repl->hook_entry[i];
			new->hook_entry[i] = sparc_cast hlp;
		} else {
			if (!cl)
				break;
			entries = cl->udc;
		}
		hlp->nentries = entries->nentries;
		hlp->policy = entries->policy;
		strcpy(hlp->name, entries->name);
		hlp->counter_offset = entries->counter_offset;
		hlp->distinguisher = 0; /* make the kernel see the light */
		p += sizeof(struct ebt_entries);
		e = entries->entries;
		while (e) {
			struct ebt_entry *tmp = (struct ebt_entry *)p;

			tmp->bitmask = e->bitmask | EBT_ENTRY_OR_ENTRIES;
			tmp->invflags = e->invflags;
			tmp->ethproto = e->ethproto;
			strcpy(tmp->in, e->in);
			strcpy(tmp->out, e->out);
			strcpy(tmp->logical_in, e->logical_in);
			strcpy(tmp->logical_out, e->logical_out);
			memcpy(tmp->sourcemac, e->sourcemac,
			   sizeof(tmp->sourcemac));
			memcpy(tmp->sourcemsk, e->sourcemsk,
			   sizeof(tmp->sourcemsk));
			memcpy(tmp->destmac, e->destmac, sizeof(tmp->destmac));
			memcpy(tmp->destmsk, e->destmsk, sizeof(tmp->destmsk));

			base = p;
			p += sizeof(struct ebt_entry);
			m_l = e->m_list;
			while (m_l) {
				memcpy(p, m_l->m, m_l->m->match_size +
				   sizeof(struct ebt_entry_match));
				p += m_l->m->match_size +
				   sizeof(struct ebt_entry_match);
				m_l = m_l->next;
			}
			tmp->watchers_offset = p - base;
			w_l = e->w_list;
			while (w_l) {
				memcpy(p, w_l->w, w_l->w->watcher_size +
				   sizeof(struct ebt_entry_watcher));
				p += w_l->w->watcher_size +
				   sizeof(struct ebt_entry_watcher);
				w_l = w_l->next;
			}
			tmp->target_offset = p - base;
			memcpy(p, e->t, e->t->target_size +
			   sizeof(struct ebt_entry_target));
			if (!strcmp(e->t->u.name, EBT_STANDARD_TARGET)) {
				struct ebt_standard_target *st =
				   (struct ebt_standard_target *)p;
				/* translate the jump to a udc */
				if (st->verdict >= 0)
					st->verdict = chain_offsets
					   [st->verdict + NF_BR_NUMHOOKS];
			}
			p += e->t->target_size +
			   sizeof(struct ebt_entry_target);
			tmp->next_offset = p - base;
			e = e->next;
		}
		if (i >= NF_BR_NUMHOOKS)
			cl = cl->next;
		i++;
	}

	/* sanity check */
	if (p - (char *)new->entries != new->entries_size)
		print_bug("Entries_size bug");
	free(chain_offsets);
	return new;
}

static void store_table_in_file(char *filename, struct ebt_replace *repl)
{
	char *command, *data;
	int size;
	FILE *file;

	/* start from an empty file with right priviliges */
	command = (char *)malloc(strlen(filename) + 15);
	if (!command)
		print_memory();
	strcpy(command, "cat /dev/null>");
	strcpy(command + 14, filename);
	if (system(command))
		print_error("Couldn't create file %s", filename);
	strcpy(command, "chmod 600 ");
	strcpy(command + 10, filename);
	if (system(command))
		print_error("Couldn't chmod file %s", filename);
	free(command);

	size = sizeof(struct ebt_replace) + repl->entries_size +
	   repl->nentries * sizeof(struct ebt_counter);
	data = (char *)malloc(size);
	if (!data)
		print_memory();
	memcpy(data, repl, sizeof(struct ebt_replace));
	memcpy(data + sizeof(struct ebt_replace), (char *)repl->entries,
	   repl->entries_size);
	/* initialize counters to zero, deliver_counters() can update them */
	memset(data + sizeof(struct ebt_replace) + repl->entries_size,
	   0, repl->nentries * sizeof(struct ebt_counter));
	if (!(file = fopen(filename, "wb")))
		print_error("Couldn't open file %s", filename);
	if (fwrite(data, sizeof(char), size, file) != size) {
		fclose(file);
		print_error("Couldn't write everything to file %s", filename);
	}
	fclose(file);
	free(data);
}

void deliver_table(struct ebt_u_replace *u_repl)
{
	socklen_t optlen;
	struct ebt_replace *repl;

	/* translate the struct ebt_u_replace to a struct ebt_replace */
	repl = translate_user2kernel(u_repl);
	if (u_repl->filename != NULL) {
		store_table_in_file(u_repl->filename, repl);
		return;
	}
	/* give the data to the kernel */
	optlen = sizeof(struct ebt_replace) + repl->entries_size;
	get_sockfd();
	if (!setsockopt(sockfd, IPPROTO_IP, EBT_SO_SET_ENTRIES, repl, optlen))
		return;
	if (u_repl->command == 8) { /* the ebtables module may not
	                            * yet be loaded with --atomic-commit */
		ebtables_insmod("ebtables");
		if (!setsockopt(sockfd, IPPROTO_IP, EBT_SO_SET_ENTRIES,
		    repl, optlen))
			return;
	}

	print_error("The kernel doesn't support a certain ebtables"
		    " extension, consider recompiling your kernel or insmod"
		    " the extension");
}

static void store_counters_in_file(char *filename, struct ebt_u_replace *repl)
{
	int size = repl->nentries * sizeof(struct ebt_counter);
	unsigned int entries_size;
	struct ebt_replace hlp;
	FILE *file;

	if (!(file = fopen(filename, "r+b")))
		print_error("Could not open file %s", filename);
	/* 
	 * find out entries_size and then set the file pointer to the
	 * counters
	 */
	if (fseek(file, (char *)(&hlp.entries_size) - (char *)(&hlp), SEEK_SET)
	   || fread(&entries_size, sizeof(char), sizeof(unsigned int), file) !=
	   sizeof(unsigned int) ||
	   fseek(file, entries_size + sizeof(struct ebt_replace), SEEK_SET)) {
		fclose(file);
		print_error("File %s is corrupt", filename);
	}
	if (fwrite(repl->counters, sizeof(char), size, file) != size) {
		fclose(file);
		print_error("Could not write everything to file %s", filename);
	}
	fclose(file);
}

/* gets executed after deliver_table */
void deliver_counters(struct ebt_u_replace *u_repl)
{
	unsigned short *point;
	struct ebt_counter *old, *new, *newcounters;
	socklen_t optlen;
	struct ebt_replace repl;
	unsigned short *counterchanges = u_repl->counterchanges;

	if (u_repl->nentries == 0)
		return;

	newcounters = (struct ebt_counter *)
	   malloc(u_repl->nentries * sizeof(struct ebt_counter));
	if (!newcounters)
		print_memory();
	memset(newcounters, 0, u_repl->nentries * sizeof(struct ebt_counter));
	old = u_repl->counters;
	new = newcounters;
	point = counterchanges;
	while (*point != CNT_END) {
		if (*point == CNT_NORM) {
			/*
			 *'normal' rule, meaning we didn't do anything to it
			 * So, we just copy
			 */
			new->pcnt = old->pcnt;
			new->bcnt = old->bcnt;
			/* we've used an old counter */
			old++;
			/* we've set a new counter */
			new++;
		} else if (*point == CNT_DEL) {
			/* don't use this old counter */
			old++;
		} else if (*point == CNT_ADD) {
			/* new counter, let it stay 0 */
			new++;
		} else {
			/* zero it (let it stay 0) */
			old++;
			new++;
		}
		point++;
	}

	free(u_repl->counters);
	u_repl->counters = newcounters;
	u_repl->num_counters = u_repl->nentries;
	if (u_repl->filename != NULL) {
		store_counters_in_file(u_repl->filename, u_repl);
		return;
	}
	optlen = u_repl->nentries * sizeof(struct ebt_counter) +
	   sizeof(struct ebt_replace);
	/* now put the stuff in the kernel's struct ebt_replace */
	repl.counters = sparc_cast u_repl->counters;
	repl.num_counters = u_repl->num_counters;
	memcpy(repl.name, u_repl->name, sizeof(repl.name));

	get_sockfd();
	if (setsockopt(sockfd, IPPROTO_IP, EBT_SO_SET_COUNTERS, &repl, optlen))
		print_bug("Couldn't update kernel counters");
}

static int
ebt_translate_match(struct ebt_entry_match *m, struct ebt_u_match_list ***l)
{
	struct ebt_u_match_list *new;

	new = (struct ebt_u_match_list *)
	   malloc(sizeof(struct ebt_u_match_list));
	if (!new)
		print_memory();
	new->m = (struct ebt_entry_match *)
	   malloc(m->match_size + sizeof(struct ebt_entry_match));
	if (!new->m)
		print_memory();
	memcpy(new->m, m, m->match_size + sizeof(struct ebt_entry_match));
	new->next = NULL;
	**l = new;
	*l = &new->next;
	if (find_match(new->m->u.name) == NULL)
		print_error("Kernel match %s unsupported by userspace tool",
		   new->m->u.name);
	return 0;
}

static int
ebt_translate_watcher(struct ebt_entry_watcher *w,
   struct ebt_u_watcher_list ***l)
{
	struct ebt_u_watcher_list *new;

	new = (struct ebt_u_watcher_list *)
	   malloc(sizeof(struct ebt_u_watcher_list));
	if (!new)
		print_memory();
	new->w = (struct ebt_entry_watcher *)
	   malloc(w->watcher_size + sizeof(struct ebt_entry_watcher));
	if (!new->w)
		print_memory();
	memcpy(new->w, w, w->watcher_size + sizeof(struct ebt_entry_watcher));
	new->next = NULL;
	**l = new;
	*l = &new->next;
	if (find_watcher(new->w->u.name) == NULL)
		print_error("Kernel watcher %s unsupported by userspace tool",
		   new->w->u.name);
	return 0;
}

static int
ebt_translate_entry(struct ebt_entry *e, unsigned int *hook, int *n, int *cnt,
   int *totalcnt, struct ebt_u_entry ***u_e, struct ebt_u_replace *u_repl,
   unsigned int valid_hooks, char *base)
{
	/* an entry */
	if (e->bitmask & EBT_ENTRY_OR_ENTRIES) {
		struct ebt_u_entry *new;
		struct ebt_u_match_list **m_l;
		struct ebt_u_watcher_list **w_l;
		struct ebt_entry_target *t;

		new = (struct ebt_u_entry *)malloc(sizeof(struct ebt_u_entry));
		if (!new)
			print_memory();
		new->bitmask = e->bitmask;
		/*
		 * plain userspace code doesn't know about
		 * EBT_ENTRY_OR_ENTRIES
		 */
		new->bitmask &= ~EBT_ENTRY_OR_ENTRIES;
		new->invflags = e->invflags;
		new->ethproto = e->ethproto;
		strcpy(new->in, e->in);
		strcpy(new->out, e->out);
		strcpy(new->logical_in, e->logical_in);
		strcpy(new->logical_out, e->logical_out);
		memcpy(new->sourcemac, e->sourcemac, sizeof(new->sourcemac));
		memcpy(new->sourcemsk, e->sourcemsk, sizeof(new->sourcemsk));
		memcpy(new->destmac, e->destmac, sizeof(new->destmac));
		memcpy(new->destmsk, e->destmsk, sizeof(new->destmsk));
		new->m_list = NULL;
		new->w_list = NULL;
		new->next = NULL;
		m_l = &new->m_list;
		EBT_MATCH_ITERATE(e, ebt_translate_match, &m_l);
		w_l = &new->w_list;
		EBT_WATCHER_ITERATE(e, ebt_translate_watcher, &w_l);

		t = (struct ebt_entry_target *)(((char *)e) + e->target_offset);
		new->t = (struct ebt_entry_target *)
		   malloc(t->target_size + sizeof(struct ebt_entry_target));
		if (!new->t)
			print_memory();
		if (find_target(t->u.name) == NULL)
			print_error("Kernel target %s unsupported by "
			            "userspace tool", t->u.name);
		memcpy(new->t, t, t->target_size +
		   sizeof(struct ebt_entry_target));
		/* deal with jumps to udc */
		if (!strcmp(t->u.name, EBT_STANDARD_TARGET)) {
			char *tmp = base;
			int verdict = ((struct ebt_standard_target *)t)->verdict;
			int i;
			struct ebt_u_chain_list *cl;

			if (verdict >= 0) {
				tmp += verdict;
				cl = u_repl->udc;
				i = 0;
				while (cl && cl->kernel_start != tmp) {
					i++;
					cl = cl->next;
				}
				if (!cl)
					print_bug("can't find udc for jump");
				((struct ebt_standard_target *)new->t)->verdict = i;
			}
		}

		/* I love pointers */
		**u_e = new;
		*u_e = &new->next;
		(*cnt)++;
		(*totalcnt)++;
		return 0;
	} else { /* a new chain */
		int i;
		struct ebt_entries *entries = (struct ebt_entries *)e;
		struct ebt_u_chain_list *cl;

		if (*n != *cnt)
			print_bug("Nr of entries in the chain is wrong");
		*n = entries->nentries;
		*cnt = 0;
		for (i = *hook + 1; i < NF_BR_NUMHOOKS; i++)
			if (valid_hooks & (1 << i))
				break;
		*hook = i;
		/* makes use of fact that standard chains come before udc */
		if (i >= NF_BR_NUMHOOKS) { /* udc */
			i -= NF_BR_NUMHOOKS;
			cl = u_repl->udc;
			while (i-- > 0)
				cl = cl->next;
			*u_e = &(cl->udc->entries);
		} else
			*u_e = &(u_repl->hook_entry[*hook]->entries);
		return 0;
	}
}

/* initialize all chain headers */
static int
ebt_translate_chains(struct ebt_entry *e, unsigned int *hook,
   struct ebt_u_replace *u_repl, unsigned int valid_hooks)
{
	int i;
	struct ebt_entries *entries = (struct ebt_entries *)e;
	struct ebt_u_entries *new;
	struct ebt_u_chain_list **chain_list;

	if (!(e->bitmask & EBT_ENTRY_OR_ENTRIES)) {
		for (i = *hook + 1; i < NF_BR_NUMHOOKS; i++)
			if (valid_hooks & (1 << i))
				break;
		/* makes use of fact that standard chains come before udc */
		if (i >= NF_BR_NUMHOOKS) { /* udc */
			chain_list = &u_repl->udc;
			/* add in the back */
			while (*chain_list)
				chain_list = &((*chain_list)->next);
			*chain_list = (struct ebt_u_chain_list *)
			   malloc(sizeof(struct ebt_u_chain_list));
			if (!(*chain_list))
				print_memory();
			(*chain_list)->next = NULL;
			(*chain_list)->udc = (struct ebt_u_entries *)
			   malloc(sizeof(struct ebt_u_entries));
			if (!((*chain_list)->udc))
				print_memory();
			new = (*chain_list)->udc;
			/*
			 * ebt_translate_entry depends on this for knowing
			 * to which chain is being jumped
			 */
			(*chain_list)->kernel_start = (char *)e;
		} else {
			*hook = i;
			new = (struct ebt_u_entries *)
			   malloc(sizeof(struct ebt_u_entries));
			if (!new)
				print_memory();
			u_repl->hook_entry[*hook] = new;
		}
		new->nentries = entries->nentries;
		new->policy = entries->policy;
		new->entries = NULL;
		new->counter_offset = entries->counter_offset;
		strcpy(new->name, entries->name);
	}
	return 0;
}

static void retrieve_from_file(char *filename, struct ebt_replace *repl,
   char command)
{
	FILE *file;
	char *hlp = NULL, *entries;
	struct ebt_counter *counters;
	int size;

	if (!(file = fopen(filename, "r+b")))
		print_error("Could not open file %s", filename);
	/*
	 * make sure table name is right if command isn't -L or --atomic-commit
	 */
	if (command != 'L' && command != 8) {
		hlp = (char *)malloc(strlen(repl->name) + 1);
		if (!hlp)
			print_memory();
		strcpy(hlp, repl->name);
	}
	if (fread(repl, sizeof(char), sizeof(struct ebt_replace), file)
	   != sizeof(struct ebt_replace))
		print_error("File %s is corrupt", filename);
	if (command != 'L' && command != 8 && strcmp(hlp, repl->name)) {
		fclose(file);
		print_error("File %s contains wrong table name or is corrupt",
		   filename);
		free(hlp);
	} else if (!find_table(repl->name)) {
		fclose(file);
		print_error("File %s contains invalid table name", filename);
	}

	size = sizeof(struct ebt_replace) +
	   repl->nentries * sizeof(struct ebt_counter) + repl->entries_size;
	fseek(file, 0, SEEK_END);
	if (size != ftell(file)) {
		fclose(file);
		print_error("File %s has wrong size", filename);
	}
	entries = (char *)malloc(repl->entries_size);
	if (!entries)
		print_memory();
	repl->entries = sparc_cast entries;
	if (repl->nentries) {
		counters = (struct ebt_counter *)
		   malloc(repl->nentries * sizeof(struct ebt_counter));
		repl->counters = sparc_cast counters;
		if (!repl->counters)
			print_memory();
	} else
		repl->counters = sparc_cast NULL;
	/* copy entries and counters */
	if (fseek(file, sizeof(struct ebt_replace), SEEK_SET) ||
	   fread((char *)repl->entries, sizeof(char), repl->entries_size, file)
	   != repl->entries_size ||
	   fseek(file, sizeof(struct ebt_replace) + repl->entries_size, SEEK_SET)
	   || fread((char *)repl->counters, sizeof(char),
	   repl->nentries * sizeof(struct ebt_counter), file)
	   != repl->nentries * sizeof(struct ebt_counter)) {
		fclose(file);
		print_error("File %s is corrupt", filename);
	}
	fclose(file);
}

static int retrieve_from_kernel(struct ebt_replace *repl, char command)
{
	socklen_t optlen;
	int optname;
	char *entries;

	optlen = sizeof(struct ebt_replace);
	get_sockfd();
	/* --atomic-init || --init-table */
	if (command == 7 || command == 11)
		optname = EBT_SO_GET_INIT_INFO;
	else
		optname = EBT_SO_GET_INFO;
	if (getsockopt(sockfd, IPPROTO_IP, optname, repl, &optlen))
		return -1;

	if ( !(entries = (char *)malloc(repl->entries_size)) )
		print_memory();
	repl->entries = sparc_cast entries;
	if (repl->nentries) {
		struct ebt_counter *counters;

		if (!(counters = (struct ebt_counter *)
		   malloc(repl->nentries * sizeof(struct ebt_counter))) )
			print_memory();
		repl->counters = sparc_cast counters;
	}
	else
		repl->counters = sparc_cast NULL;

	/* we want to receive the counters */
	repl->num_counters = repl->nentries;
	optlen += repl->entries_size + repl->num_counters *
	   sizeof(struct ebt_counter);
	if (command == 7 || command == 11)
		optname = EBT_SO_GET_INIT_ENTRIES;
	else
		optname = EBT_SO_GET_ENTRIES;
	if (getsockopt(sockfd, IPPROTO_IP, optname, repl, &optlen))
		print_bug("hmm, what is wrong??? bug#1");

	return 0;
}

int get_table(struct ebt_u_replace *u_repl)
{
	int i, j, k, hook;
	struct ebt_replace repl;
	struct ebt_u_entry **u_e;

	strcpy(repl.name, u_repl->name);
	if (u_repl->filename != NULL) {
		retrieve_from_file(u_repl->filename, &repl, u_repl->command);
		/*
		 * -L with a wrong table name should be dealt with silently
		 */
		strcpy(u_repl->name, repl.name);
	} else if (retrieve_from_kernel(&repl, u_repl->command) == -1)
		return -1;

	/* translate the struct ebt_replace to a struct ebt_u_replace */
	u_repl->valid_hooks = repl.valid_hooks;
	u_repl->nentries = repl.nentries;
	u_repl->num_counters = repl.num_counters;
	u_repl->counters = (struct ebt_counter *)repl.counters;
	u_repl->udc = NULL;
	hook = -1;
	EBT_ENTRY_ITERATE(repl.entries, repl.entries_size, ebt_translate_chains,
	   &hook, u_repl, u_repl->valid_hooks);
	i = 0; /* holds the expected nr. of entries for the chain */
	j = 0; /* holds the up to now counted entries for the chain */
	/*
	 * holds the total nr. of entries,
	 * should equal u_repl->nentries afterwards
	 */
	k = 0;
	hook = -1;
	EBT_ENTRY_ITERATE((char *)repl.entries, repl.entries_size, ebt_translate_entry,
	   &hook, &i, &j, &k, &u_e, u_repl, u_repl->valid_hooks, (char *)repl.entries);
	if (k != u_repl->nentries)
		print_bug("Wrong total nentries");
	return 0;
}
