/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include "mdadm.h"
#include <dirent.h>
#include <fnmatch.h>
#include <ctype.h>
#include "dlink.h"
/*
 * Policy module for mdadm.
 * A policy statement about a device lists a set of values for each
 * of a set of names.  Each value can have a metadata type as context.
 *
 * names include:
 *   action - the actions that can be taken on hot-plug
 *   domain - the domain(s) that the device is part of
 *
 * Policy information is extracted from various sources, but
 * particularly from a set of policy rules in mdadm.conf
 */

static void pol_new(struct dev_policy **pol, char *name, const char *val,
		    const char *metadata)
{
	struct dev_policy *n = xmalloc(sizeof(*n));
	const char *real_metadata = NULL;
	int i;

	n->name = name;
	n->value = val;

	/* We need to normalise the metadata name */
	if (metadata) {
		for (i = 0; superlist[i] ; i++)
			if (strcmp(metadata, superlist[i]->name) == 0) {
				real_metadata = superlist[i]->name;
				break;
			}
		if (!real_metadata) {
			if (strcmp(metadata, "1") == 0 ||
			    strcmp(metadata, "1.0") == 0 ||
			    strcmp(metadata, "1.1") == 0 ||
			    strcmp(metadata, "1.2") == 0)
				real_metadata = super1.name;
		}
		if (!real_metadata) {
			static const char *prev = NULL;
			if (prev != metadata) {
				pr_err("metadata=%s unrecognised - ignoring rule\n",
					metadata);
				prev = metadata;
			}
			real_metadata = "unknown";
		}
	}

	n->metadata = real_metadata;
	n->next = *pol;
	*pol = n;
}

static int pol_lesseq(struct dev_policy *a, struct dev_policy *b)
{
	int cmp;

	if (a->name < b->name)
		return 1;
	if (a->name > b->name)
		return 0;

	cmp = strcmp(a->value, b->value);
	if (cmp < 0)
		return 1;
	if (cmp > 0)
		return 0;

	return (a->metadata <= b->metadata);
}

static void pol_sort(struct dev_policy **pol)
{
	/* sort policy list in *pol by name/metadata/value
	 * using merge sort
	 */

	struct dev_policy *pl[2];
	pl[0] = *pol;
	pl[1] = NULL;

	do {
		struct dev_policy **plp[2], *p[2];
		int curr = 0;
		struct dev_policy nul = { NULL, NULL, NULL, NULL };
		struct dev_policy *prev = &nul;
		int next = 0;

		/* p[] are the two lists that we are merging.
		 * plp[] are the ends of the two lists we create
		 * from the merge.
		 * 'curr' is which of plp[] that we are currently
		 *   adding items to.
		 * 'next' is which if p[] we will take the next
		 *   item from.
		 * 'prev' is that last value, which was placed in
		 * plp[curr].
		 */
		plp[0] = &pl[0];
		plp[1] = &pl[1];
		p[0] = pl[0];
		p[1] = pl[1];

		/* take least of p[0] and p[1]
		 * if it is larger than prev, add to
		 * plp[curr], else swap curr then add
		 */
		while (p[0] || p[1]) {
			if (p[next] == NULL ||
			    (p[1-next] != NULL &&
			     !(pol_lesseq(prev, p[1-next])
			       ^pol_lesseq(prev, p[next])
			       ^pol_lesseq(p[next], p[1-next])))
				)
				next = 1 - next;

			if (!pol_lesseq(prev, p[next]))
				curr = 1 - curr;

			*plp[curr] = prev = p[next];
			plp[curr] = &p[next]->next;
			p[next] = p[next]->next;
		}
		*plp[0] = NULL;
		*plp[1] = NULL;
	} while (pl[0] && pl[1]);
	if (pl[0])
		*pol = pl[0];
	else
		*pol = pl[1];
}

static void pol_dedup(struct dev_policy *pol)
{
	/* This is a sorted list - remove duplicates. */
	while (pol && pol->next) {
		if (pol_lesseq(pol->next, pol)) {
			struct dev_policy *tmp = pol->next;
			pol->next = tmp->next;
			free(tmp);
		} else
			pol = pol->next;
	}
}

/*
 * pol_find finds the first entry in the policy
 * list to match name.
 * If it returns non-NULL there is at least one
 * value, but how many can only be found by
 * iterating through the list.
 */
struct dev_policy *pol_find(struct dev_policy *pol, char *name)
{
	while (pol && pol->name < name)
		pol = pol->next;

	if (!pol || pol->name != name)
		return NULL;
	return pol;
}

static char *disk_path(struct mdinfo *disk)
{
	struct stat stb;
	int prefix_len;
	DIR *by_path;
	char symlink[PATH_MAX] = "/dev/disk/by-path/";
	char nm[PATH_MAX];
	struct dirent *ent;
	int rv;

	by_path = opendir(symlink);
	if (by_path) {
		prefix_len = strlen(symlink);
		while ((ent = readdir(by_path)) != NULL) {
			if (ent->d_type != DT_LNK)
				continue;
			strncpy(symlink + prefix_len,
					ent->d_name,
					sizeof(symlink) - prefix_len);
			if (stat(symlink, &stb) < 0)
				continue;
			if ((stb.st_mode & S_IFMT) != S_IFBLK)
				continue;
			if (stb.st_rdev != makedev(disk->disk.major, disk->disk.minor))
				continue;
			closedir(by_path);
			return xstrdup(ent->d_name);
		}
		closedir(by_path);
	}
	/* A NULL path isn't really acceptable - use the devname.. */
	sprintf(symlink, "/sys/dev/block/%d:%d", disk->disk.major, disk->disk.minor);
	rv = readlink(symlink, nm, sizeof(nm)-1);
	if (rv > 0) {
		char *dname;
		nm[rv] = 0;
		dname = strrchr(nm, '/');
		if (dname)
			return xstrdup(dname + 1);
	}
	return xstrdup("unknown");
}

char type_part[] = "part";
char type_disk[] = "disk";
static char *disk_type(struct mdinfo *disk)
{
	char buf[30+20+20];
	struct stat stb;
	sprintf(buf, "/sys/dev/block/%d:%d/partition",
		disk->disk.major, disk->disk.minor);
	if (stat(buf, &stb) == 0)
		return type_part;
	else
		return type_disk;
}

static int pol_match(struct rule *rule, char *path, char *type)
{
	/* check if this rule matches on path and type */
	int pathok = 0; /* 0 == no path, 1 == match, -1 == no match yet */
	int typeok = 0;

	while (rule) {
		if (rule->name == rule_path) {
			if (pathok == 0)
				pathok = -1;
			if (path && fnmatch(rule->value, path, 0) == 0)
				pathok = 1;
		}
		if (rule->name == rule_type) {
			if (typeok == 0)
				typeok = -1;
			if (type && strcmp(rule->value, type) == 0)
				typeok = 1;
		}
		rule = rule->next;
	}
	return pathok >= 0 && typeok >= 0;
}

static void pol_merge(struct dev_policy **pol, struct rule *rule)
{
	/* copy any name assignments from rule into pol */
	struct rule *r;
	char *metadata = NULL;
	for (r = rule; r ; r = r->next)
		if (r->name == pol_metadata)
			metadata = r->value;

	for (r = rule; r ; r = r->next)
		if (r->name == pol_act ||
		    r->name == pol_domain ||
		    r->name == pol_auto)
			pol_new(pol, r->name, r->value, metadata);
}

static int path_has_part(char *path, char **part)
{
	/* check if path ends with "-partNN" and
	 * if it does, place a pointer to "-pathNN"
	 * in 'part'.
	 */
	int l;
	if (!path)
		return 0;
	l = strlen(path);
	while (l > 1 && isdigit(path[l-1]))
		l--;
	if (l < 5 || strncmp(path+l-5, "-part", 5) != 0)
		return 0;
	*part = path+l-4;
	return 1;
}

static void pol_merge_part(struct dev_policy **pol, struct rule *rule, char *part)
{
	/* copy any name assignments from rule into pol, appending
	 * -part to any domain.  The string with -part appended is
	 * stored with the rule so it has a lifetime to match
	 * the rule.
	 */
	struct rule *r;
	char *metadata = NULL;
	for (r = rule; r ; r = r->next)
		if (r->name == pol_metadata)
			metadata = r->value;

	for (r = rule; r ; r = r->next) {
		if (r->name == pol_act)
			pol_new(pol, r->name, r->value, metadata);
		else if (r->name == pol_domain) {
			char *dom;
			int len;
			if (r->dups == NULL)
				r->dups = dl_head();
			len = strlen(r->value);
			for (dom = dl_next(r->dups); dom != r->dups;
			     dom = dl_next(dom))
				if (strcmp(dom+len+1, part)== 0)
					break;
			if (dom == r->dups) {
				char *newdom = dl_strndup(
					r->value, len + 1 + strlen(part));
				strcat(strcat(newdom, "-"), part);
				dl_add(r->dups, newdom);
				dom = newdom;
			}
			pol_new(pol, r->name, dom, metadata);
		}
	}
}

static struct pol_rule *config_rules = NULL;
static struct pol_rule **config_rules_end = NULL;
static int config_rules_has_path = 0;

/*
 * most policy comes from a set policy rules that are
 * read from the config file.
 * path_policy() gathers policy information for the
 * disk described in the given a 'path' and a 'type'.
 */
struct dev_policy *path_policy(char *path, char *type)
{
	struct pol_rule *rules;
	struct dev_policy *pol = NULL;
	int i;

	rules = config_rules;

	while (rules) {
		char *part;
		if (rules->type == rule_policy)
			if (pol_match(rules->rule, path, type))
				pol_merge(&pol, rules->rule);
		if (rules->type == rule_part && strcmp(type, type_part) == 0)
			if (path_has_part(path, &part)) {
				*part = 0;
				if (pol_match(rules->rule, path, type_disk))
					pol_merge_part(&pol, rules->rule, part+1);
				*part = '-';
			}
		rules = rules->next;
	}

	/* Now add any metadata-specific internal knowledge
	 * about this path
	 */
	for (i=0; path && superlist[i]; i++)
		if (superlist[i]->get_disk_controller_domain) {
			const char *d =
				superlist[i]->get_disk_controller_domain(path);
			if (d)
				pol_new(&pol, pol_domain, d, superlist[i]->name);
		}

	pol_sort(&pol);
	pol_dedup(pol);
	return pol;
}

void pol_add(struct dev_policy **pol,
		    char *name, char *val,
		    char *metadata)
{
	pol_new(pol, name, val, metadata);
	pol_sort(pol);
	pol_dedup(*pol);
}

/*
 * disk_policy() gathers policy information for the
 * disk described in the given mdinfo (disk.{major,minor}).
 */
struct dev_policy *disk_policy(struct mdinfo *disk)
{
	char *path = NULL;
	char *type = disk_type(disk);
	struct dev_policy *pol = NULL;

	if (config_rules_has_path)
		path = disk_path(disk);

	pol = path_policy(path, type);

	free(path);
	return pol;
}

struct dev_policy *devid_policy(int dev)
{
	struct mdinfo disk;
	disk.disk.major = major(dev);
	disk.disk.minor = minor(dev);
	return disk_policy(&disk);
}

/*
 * process policy rules read from config file.
 */

char rule_path[] = "path";
char rule_type[] = "type";

char rule_policy[] = "policy";
char rule_part[] = "part-policy";

char pol_metadata[] = "metadata";
char pol_act[] = "action";
char pol_domain[] = "domain";
char pol_auto[] = "auto";

static int try_rule(char *w, char *name, struct rule **rp)
{
	struct rule *r;
	int len = strlen(name);
	if (strncmp(w, name, len) != 0 ||
	    w[len] != '=')
		return 0;
	r = xmalloc(sizeof(*r));
	r->next = *rp;
	r->name = name;
	r->value = xstrdup(w+len+1);
	r->dups = NULL;
	*rp = r;
	return 1;
}

void policyline(char *line, char *type)
{
	struct pol_rule *pr;
	char *w;

	if (config_rules_end == NULL)
		config_rules_end = &config_rules;

	pr = xmalloc(sizeof(*pr));
	pr->type = type;
	pr->rule = NULL;
	for (w = dl_next(line); w != line ; w = dl_next(w)) {
		if (try_rule(w, rule_path, &pr->rule))
			config_rules_has_path = 1;
		else if (! try_rule(w, rule_type, &pr->rule) &&
			 ! try_rule(w, pol_metadata, &pr->rule) &&
			 ! try_rule(w, pol_act, &pr->rule) &&
			 ! try_rule(w, pol_domain, &pr->rule) &&
			 ! try_rule(w, pol_auto, &pr->rule))
			pr_err("policy rule %s unrecognised and ignored\n",
				w);
	}
	pr->next = config_rules;
	config_rules = pr;
}

void policy_add(char *type, ...)
{
	va_list ap;
	struct pol_rule *pr;
	char *name, *val;

	pr = xmalloc(sizeof(*pr));
	pr->type = type;
	pr->rule = NULL;

	va_start(ap, type);
	while ((name = va_arg(ap, char*)) != NULL) {
		struct rule *r;

		val = va_arg(ap, char*);
		r = xmalloc(sizeof(*r));
		r->next = pr->rule;
		r->name = name;
		r->value = xstrdup(val);
		r->dups = NULL;
		pr->rule = r;
	}
	pr->next = config_rules;
	config_rules = pr;
	va_end(ap);
}

void policy_free(void)
{
	while (config_rules) {
		struct pol_rule *pr = config_rules;
		struct rule *r;

		config_rules = config_rules->next;

		for (r = pr->rule; r; ) {
			struct rule *next = r->next;
			free(r->value);
			if (r->dups)
				free_line(r->dups);
			free(r);
			r = next;
		}
		free(pr);
	}
	config_rules_end = NULL;
	config_rules_has_path = 0;
}

void dev_policy_free(struct dev_policy *p)
{
	struct dev_policy *t;
	while (p) {
		t = p;
		p = p->next;
		free(t);
	}
}

static enum policy_action map_act(const char *act)
{
	if (strcmp(act, "include") == 0)
		return act_include;
	if (strcmp(act, "re-add") == 0)
		return act_re_add;
	if (strcmp(act, "spare") == 0)
		return act_spare;
	if (strcmp(act, "spare-same-slot") == 0)
		return act_spare_same_slot;
	if (strcmp(act, "force-spare") == 0)
		return act_force_spare;
	return act_err;
}

static enum policy_action policy_action(struct dev_policy *plist, const char *metadata)
{
	enum policy_action rv = act_default;
	struct dev_policy *p;

	plist = pol_find(plist, pol_act);
	pol_for_each(p, plist, metadata) {
		enum policy_action a = map_act(p->value);
		if (a > rv)
			rv = a;
	}
	return rv;
}

int policy_action_allows(struct dev_policy *plist, const char *metadata, enum policy_action want)
{
	enum policy_action act = policy_action(plist, metadata);

	if (act == act_err)
		return 0;
	return (act >= want);
}

int disk_action_allows(struct mdinfo *disk, const char *metadata, enum policy_action want)
{
	struct dev_policy *pol = disk_policy(disk);
	int rv = policy_action_allows(pol, metadata, want);

	dev_policy_free(pol);
	return rv;
}

/* Domain policy:
 * Any device can have a list of domains asserted by different policy
 * statements.
 * An array also has a list of domains comprising all the domains of
 * all the devices in an array.
 * Where an array has a spare-group, that becomes an addition domain for
 * every device in the array and thus for the array.
 *
 * We keep the list of domains in a sorted linked list
 * As dev policies are already sorted, this is fairly easy to manage.
 */

static struct domainlist **domain_merge_one(struct domainlist **domp,
					    const char *domain)
{
	/* merge a domain name into a sorted list and return the
	 * location of the insertion or match
	 */
	struct domainlist *dom = *domp;

	while (dom && strcmp(dom->dom, domain) < 0) {
		domp = &dom->next;
		dom = *domp;
	}
	if (dom == NULL || strcmp(dom->dom, domain) != 0) {
		dom = xmalloc(sizeof(*dom));
		dom->next = *domp;
		dom->dom = domain;
		*domp = dom;
	}
	return domp;
}

#if (DEBUG)
void dump_policy(struct dev_policy *policy)
{
	while (policy) {
		dprintf("policy: %p name: %s value: %s metadata: %s\n",
			policy,
			policy->name,
			policy->value,
			policy->metadata);
		policy = policy->next;
	}
}
#endif

void domain_merge(struct domainlist **domp, struct dev_policy *pollist,
			 const char *metadata)
{
	/* Add to 'domp' all the domains in pol that apply to 'metadata'
	 * which are not already in domp
	 */
	struct dev_policy *pol;
	pollist = pol_find(pollist, pol_domain);
	pol_for_each(pol, pollist, metadata)
		domain_merge_one(domp, pol->value);
}

int domain_test(struct domainlist *dom, struct dev_policy *pol,
		const char *metadata)
{
	/* Check that all domains in pol (for metadata) are also in
	 * dom.  Both lists are sorted.
	 * If pol has no domains, we don't really know about this device
	 * so we allow caller to choose:
	 * -1:  has no domains
	 *  0:  has domains, not all match
	 *  1:  has domains, all match
	 */
	int found_any = -1;
	int has_one_domain = 1;
	struct dev_policy *p;

	pol = pol_find(pol, pol_domain);
	pol_for_each(p, pol, metadata) {
		found_any = 1;
		while (dom && strcmp(dom->dom, p->value) < 0)
			dom = dom->next;
		if (!dom || strcmp(dom->dom, p->value) != 0)
			return 0;
		if (has_one_domain && metadata && strcmp(metadata, "imsm") == 0)
			found_any = -1;
		has_one_domain = 0;
	}
	return found_any;
}

void domainlist_add_dev(struct domainlist **dom, int devid, const char *metadata)
{
	struct dev_policy *pol = devid_policy(devid);
	domain_merge(dom, pol, metadata);
	dev_policy_free(pol);
}

struct domainlist *domain_from_array(struct mdinfo *mdi, const char *metadata)
{
	struct domainlist *domlist = NULL;

	if (!mdi)
		return NULL;
	for (mdi = mdi->devs ; mdi ; mdi = mdi->next)
		domainlist_add_dev(&domlist, makedev(mdi->disk.major,
						     mdi->disk.minor),
				   metadata);

	return domlist;
}

void domain_add(struct domainlist **domp, char *domain)
{
	domain_merge_one(domp, domain);
}

void domain_free(struct domainlist *dl)
{
	while (dl) {
		struct domainlist *head = dl;
		dl = dl->next;
		free(head);
	}
}

/*
 * same-path policy.
 * Some policy decisions are guided by knowledge of which
 * array previously owned the device at a given physical location (path).
 * When removing a device from an array we might record the array against
 * the path, and when finding a new device, we might look for which
 * array previously used that path.
 *
 * The 'array' is described by a map_ent, and the path by a the disk in an
 * mdinfo, or a string.
 */

void policy_save_path(char *id_path, struct map_ent *array)
{
	char path[PATH_MAX];
	FILE *f = NULL;

	if (mkdir(FAILED_SLOTS_DIR, S_IRWXU) < 0 && errno != EEXIST) {
		pr_err("can't create file to save path to old disk: %s\n", strerror(errno));
		return;
	}

	snprintf(path, PATH_MAX, FAILED_SLOTS_DIR "/%s", id_path);
	f = fopen(path, "w");
	if (!f) {
		pr_err("can't create file to save path to old disk: %s\n",
			strerror(errno));
		return;
	}

	if (fprintf(f, "%s %08x:%08x:%08x:%08x\n",
		    array->metadata,
		    array->uuid[0], array->uuid[1],
		    array->uuid[2], array->uuid[3]) <= 0)
		pr_err("Failed to write to <id_path> cookie\n");

	fclose(f);
}

int policy_check_path(struct mdinfo *disk, struct map_ent *array)
{
	char path[PATH_MAX];
	FILE *f = NULL;
	char *id_path = disk_path(disk);
	int rv;

	if (!id_path)
		return 0;

	snprintf(path, PATH_MAX, FAILED_SLOTS_DIR "/%s", id_path);
	f = fopen(path, "r");
	if (!f) {
		free(id_path);
		return 0;
	}

	rv = fscanf(f, " %s %x:%x:%x:%x\n",
		    array->metadata,
		    array->uuid,
		    array->uuid+1,
		    array->uuid+2,
		    array->uuid+3);
	fclose(f);
	free(id_path);
	return rv == 5;
}

/* invocation of udev rule file */
char udev_template_start[] =
"# do not edit this file, it is automatically generated by mdadm\n"
"\n";

/* find rule named rule_type and return its value */
char *find_rule(struct rule *rule, char *rule_type)
{
	while (rule) {
		if (rule->name == rule_type)
			return rule->value;

		rule = rule->next;
	}
	return NULL;
}

#define UDEV_RULE_FORMAT \
"ACTION==\"add\", SUBSYSTEM==\"block\", " \
"ENV{DEVTYPE}==\"%s\", ENV{ID_PATH}==\"%s\", " \
"RUN+=\"" BINDIR "/mdadm --incremental $env{DEVNAME}\"\n"

#define UDEV_RULE_FORMAT_NOTYPE \
"ACTION==\"add\", SUBSYSTEM==\"block\", " \
"ENV{ID_PATH}==\"%s\", " \
"RUN+=\"" BINDIR "/mdadm --incremental $env{DEVNAME}\"\n"

/* Write rule in the rule file. Use format from UDEV_RULE_FORMAT */
int write_rule(struct rule *rule, int fd, int force_part)
{
	char line[1024];
	char *pth = find_rule(rule, rule_path);
	char *typ = find_rule(rule, rule_type);
	if (!pth)
		return -1;

	if (force_part)
		typ = type_part;
	if (typ)
		snprintf(line, sizeof(line) - 1, UDEV_RULE_FORMAT, typ, pth);
	else
		snprintf(line, sizeof(line) - 1, UDEV_RULE_FORMAT_NOTYPE, pth);
	return write(fd, line, strlen(line)) == (int)strlen(line);
}

/* Generate single entry in udev rule basing on POLICY line found in config
 * file. Take only those with paths, only first occurrence if paths are equal
 * and if actions supports handling of spares (>=act_spare_same_slot)
 */
int generate_entries(int fd)
{
	struct pol_rule *loop, *dup;
	char *loop_value, *dup_value;
	int duplicate;

	for (loop = config_rules; loop; loop = loop->next) {
		if (loop->type != rule_policy && loop->type != rule_part)
			continue;
		duplicate = 0;

		/* only policies with paths and with actions supporting
		 * bare disks are considered */
		loop_value = find_rule(loop->rule, pol_act);
		if (!loop_value || map_act(loop_value) < act_spare_same_slot)
			continue;
		loop_value = find_rule(loop->rule, rule_path);
		if (!loop_value)
			continue;
		for (dup = config_rules; dup != loop; dup = dup->next) {
			if (dup->type != rule_policy && loop->type != rule_part)
				continue;
			dup_value = find_rule(dup->rule, pol_act);
			if (!dup_value || map_act(dup_value) < act_spare_same_slot)
				continue;
			dup_value = find_rule(dup->rule, rule_path);
			if (!dup_value)
				continue;
			if (strcmp(loop_value, dup_value) == 0) {
				duplicate = 1;
				break;
			}
		}

		/* not a dup or first occurrence */
		if (!duplicate)
			if (!write_rule(loop->rule, fd, loop->type == rule_part) )
				return 0;
	}
	return 1;
}

/* Write_rules routine creates dynamic udev rules used to handle
 * hot-plug events for bare devices (and making them spares)
 */
int Write_rules(char *rule_name)
{
	int fd;
	char udev_rule_file[PATH_MAX];

	if (rule_name) {
		strncpy(udev_rule_file, rule_name, sizeof(udev_rule_file) - 6);
		udev_rule_file[sizeof(udev_rule_file) - 6] = '\0';
		strcat(udev_rule_file, ".temp");
		fd = creat(udev_rule_file,
			   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (fd == -1)
			return 1;
	} else
		fd = 1;

	/* write static invocation */
	if (write(fd, udev_template_start, sizeof(udev_template_start) - 1) !=
	    (int)sizeof(udev_template_start) - 1)
		goto abort;

	/* iterate, if none created or error occurred, remove file */
	if (generate_entries(fd) < 0)
		goto abort;

	fsync(fd);
	if (rule_name) {
		close(fd);
		rename(udev_rule_file, rule_name);
	}
	return 0;
abort:
	if (rule_name) {
		close(fd);
		unlink(udev_rule_file);
	}
	return 1;
}
