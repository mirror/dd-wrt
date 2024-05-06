#include "mod/common/atomic_config.h"

#include <linux/kref.h>
#include <linux/timer.h>
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/global.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/db/eam.h"
#include "mod/common/db/denylist4.h"
#include "mod/common/joold.h"
#include "mod/common/db/pool4/db.h"
#include "mod/common/db/bib/db.h"

/**
 * This represents the new configuration the user wants to apply to a certain
 * Jool instance.
 *
 * On account that the tables can hold any amount of entries, the configuration
 * can be quite big, so it is quite plausible it might not entirely fit in a
 * single Netlink message. So, in order to guarantee a configuration file is
 * loaded atomically, the values are stored in a separate container (a
 * "configuration candidate") as Netlink messages arrive. The running
 * configuration is then only replaced when the candidate has been completed and
 * validated.
 */
struct config_candidate {
	struct xlator xlator;

	/** Last jiffy the user made an edit. */
	unsigned long update_time;
	/** Process ID of the client that is populating this candidate. */
	pid_t pid;

	struct list_head list_hook;
};

/**
 * We'll purge candidates after they've been inactive for this long.
 * This is because otherwise we depend on userspace sending us a commit at some
 * point, and we don't trust them.
 */
#define TIMEOUT msecs_to_jiffies(2000)

static LIST_HEAD(db);
static DEFINE_MUTEX(lock);

static void candidate_destroy(struct config_candidate *candidate)
{
	LOG_DEBUG("Destroying atomic configuration candidate '%s'.",
			candidate->xlator.iname);
	xlator_put(&candidate->xlator);
	list_del(&candidate->list_hook);
	wkfree(struct config_candidate, candidate);
}

void atomconfig_teardown(void)
{
	struct config_candidate *candidate;
	struct config_candidate *tmp;

	list_for_each_entry_safe(candidate, tmp, &db, list_hook)
		candidate_destroy(candidate);
}

static void candidate_expire_maybe(struct config_candidate *candidate)
{
	/*
	 * The userspace application makes a series of requests which we pile
	 * up in @candidate. If any of them fails, the @candidate is rolled
	 * back. When the app finishes, it states so by requesting a commit.
	 * But we don't trust the app. What happens if it dies before sending
	 * the commit?
	 *
	 * Well, the candidate needs to expire.
	 *
	 * The natural solution to that would be a kernel timer, right? So why
	 * is that nowhere to be found?
	 * Because a timer would force us to synchronize access to @candidate
	 * with a spinlock. (Mutexes kill timers.) That would also be incorrect
	 * because all the handle_* functions below (which the spinlock would
	 * also need to protect) can sleep for a variety of reasons.
	 *
	 * So instead, if the userspace app dies too early to send a commit, we
	 * will hold the candidate until another atomic configuration request is
	 * made and this if realizes that the previous one expired.
	 *
	 * So what prevents a commitless sequence of requests from claiming
	 * memory pointlessly (other than another sequence of requests)?
	 * Nothing. But if they were done out of malice, then the system has
	 * much more to fear because it means the attacker has sudo. And if they
	 * were not, the user will follow shortly with another request or kill
	 * the NAT64 instance. So the @candidate will be released in the end
	 * despite the fuss. It's not a memory leak, after all.
	 *
	 * I'd like to clarify I would rather see a better solution, but I
	 * genuinely feel like making the handle_*() functions atomic is not it.
	 */
	if (time_after(jiffies, candidate->update_time + TIMEOUT))
		candidate_destroy(candidate);
}

/**
 * Returns the instance candidate whose namespace is the current one and whose
 * name is @iname.
 */
static int get_candidate(char *iname, struct config_candidate **result)
{
	struct net *ns;
	struct config_candidate *candidate;
	struct config_candidate *tmp;

	ns = get_net_ns_by_pid(task_pid_vnr(current));
	if (IS_ERR(ns)) {
		log_err("Could not retrieve the current namespace.");
		return PTR_ERR(ns);
	}

	list_for_each_entry_safe(candidate, tmp, &db, list_hook) {
		if ((candidate->xlator.ns == ns)
				&& (strcmp(candidate->xlator.iname, iname) == 0)
				&& (candidate->pid == task_pid_nr(current))) {
			*result = candidate;
			put_net(ns);
			return 0;
		}

		candidate_expire_maybe(candidate);
	}

	log_err("Instance not found.");
	return -ESRCH;
}

static int handle_init(struct config_candidate **out, struct nlattr *attr,
		char *iname, xlator_type xt)
{
	struct config_candidate *candidate;
	struct net *ns;
	int error;

	LOG_DEBUG("Handling atomic INIT attribute.");

	ns = get_net_ns_by_pid(task_pid_vnr(current));
	if (IS_ERR(ns)) {
		log_err("Could not retrieve the current namespace.");
		return PTR_ERR(ns);
	}

	candidate = wkmalloc(struct config_candidate, GFP_KERNEL);
	if (!candidate) {
		error = -ENOMEM;
		goto end;
	}

	error = xlator_init(&candidate->xlator, ns, iname, nla_get_u8(attr) | xt,
			NULL);
	if (error) {
		wkfree(struct config_candidate, candidate);
		goto end;
	}
	candidate->update_time = jiffies;
	candidate->pid = task_pid_nr(current);
	list_add(&candidate->list_hook, &db);
	*out = candidate;
	/* Fall through */

end:	put_net(ns);
	return error;
}

static int handle_global(struct config_candidate *new, struct nlattr *attr,
		joolnlhdr_flags flags)
{
	LOG_DEBUG("Handling atomic global attribute.");
	return global_update(&new->xlator.globals,
			xlator_flags2xt(new->xlator.flags),
			!!(flags & JOOLNLHDR_FLAGS_FORCE), attr);
}

static int handle_eamt(struct config_candidate *new, struct nlattr *root,
		bool force)
{
	struct nlattr *attr;
	struct eamt_entry entry;
	int rem;
	int error;

	LOG_DEBUG("Handling atomic EAMT attribute.");

	if (xlator_is_nat64(&new->xlator)) {
		log_err("Stateful NAT64 doesn't have an EAMT.");
		return -EINVAL;
	}

	nla_for_each_nested(attr, root, rem) {
		if (nla_type(attr) != JNLAL_ENTRY)
			continue; /* ? */
		error = jnla_get_eam(attr, "EAMT entry", &entry);
		if (error)
			return error;
		error = eamt_add(new->xlator.siit.eamt, &entry, force, false);
		if (error)
			return error;
	}

	return 0;
}

static int handle_denylist4(struct config_candidate *new, struct nlattr *root,
		bool force)
{
	struct nlattr *attr;
	struct ipv4_prefix entry;
	int rem;
	int error;

	LOG_DEBUG("Handling atomic denylist4 attribute.");

	if (xlator_is_nat64(&new->xlator)) {
		log_err("Stateful NAT64 doesn't have denylist4.");
		return -EINVAL;
	}

	nla_for_each_nested(attr, root, rem) {
		if (nla_type(attr) != JNLAL_ENTRY)
			continue; /* ? */
		error = jnla_get_prefix4(attr, "IPv4 denylist4 entry", &entry);
		if (error)
			return error;
		error = denylist4_add(new->xlator.siit.denylist4, &entry, force);
		if (error)
			return error;
	}

	return 0;
}

static int handle_pool4(struct config_candidate *new, struct nlattr *root)
{
	struct nlattr *attr;
	struct pool4_entry entry;
	int rem;
	int error;

	LOG_DEBUG("Handling atomic pool4 attribute.");

	if (xlator_is_siit(&new->xlator)) {
		log_err("SIIT doesn't have pool4.");
		return -EINVAL;
	}

	nla_for_each_nested(attr, root, rem) {
		if (nla_type(attr) != JNLAL_ENTRY)
			continue; /* ? */
		error = jnla_get_pool4(attr, "pool4 entry", &entry);
		if (error)
			return error;
		error = pool4db_add(new->xlator.nat64.pool4, &entry);
		if (error)
			return error;
	}

	return 0;
}

static int handle_bib(struct config_candidate *new, struct nlattr *root)
{
	struct nlattr *attr;
	struct bib_entry entry;
	int rem;
	int error;

	LOG_DEBUG("Handling atomic BIB attribute.");

	if (xlator_is_siit(&new->xlator)) {
		log_err("SIIT doesn't have BIBs.");
		return -EINVAL;
	}

	nla_for_each_nested(attr, root, rem) {
		if (nla_type(attr) != JNLAL_ENTRY)
			continue; /* ? */
		error = jnla_get_bib(attr, "BIB entry", &entry);
		if (error)
			return error;
		error = bib_add_static(&new->xlator, &entry);
		if (error)
			return error;
	}

	return 0;
}

static int commit(struct config_candidate *candidate)
{
	int error;

	LOG_DEBUG("Handling atomic END attribute.");

	error = xlator_replace(&candidate->xlator);
	if (error) {
		log_err("xlator_replace() failed. Errcode %d", error);
		return error;
	}

	candidate_destroy(candidate);
	LOG_DEBUG("The atomic configuration transaction was a success.");
	return 0;
}

int atomconfig_add(struct sk_buff *skb, struct genl_info *info)
{
	struct config_candidate *candidate = NULL;
	struct joolnlhdr *jhdr;
	int error;

	jhdr = get_jool_hdr(info);
	error = iname_validate(jhdr->iname, false);
	if (error) {
		log_err(INAME_VALIDATE_ERRMSG);
		return error;
	}

	mutex_lock(&lock);

	error = info->attrs[JNLAR_ATOMIC_INIT]
			? handle_init(&candidate, info->attrs[JNLAR_ATOMIC_INIT], jhdr->iname, jhdr->xt)
			: get_candidate(jhdr->iname, &candidate);
	if (error)
		goto end;

	if (info->attrs[JNLAR_GLOBALS]) {
		error = handle_global(candidate, info->attrs[JNLAR_GLOBALS], jhdr->flags);
		if (error)
			goto revert;
	}
	if (info->attrs[JNLAR_BL4_ENTRIES]) {
		error = handle_denylist4(candidate, info->attrs[JNLAR_BL4_ENTRIES], jhdr->flags & JOOLNLHDR_FLAGS_FORCE);
		if (error)
			goto revert;
	}
	if (info->attrs[JNLAR_EAMT_ENTRIES]) {
		error = handle_eamt(candidate, info->attrs[JNLAR_EAMT_ENTRIES], jhdr->flags & JOOLNLHDR_FLAGS_FORCE);
		if (error)
			goto revert;
	}
	if (info->attrs[JNLAR_POOL4_ENTRIES]) {
		error = handle_pool4(candidate, info->attrs[JNLAR_POOL4_ENTRIES]);
		if (error)
			goto revert;
	}
	if (info->attrs[JNLAR_BIB_ENTRIES]) {
		error = handle_bib(candidate, info->attrs[JNLAR_BIB_ENTRIES]);
		if (error)
			goto revert;
	}
	if (info->attrs[JNLAR_ATOMIC_END]) {
		error = commit(candidate);
		if (error)
			goto revert;
	}

	candidate->update_time = jiffies;
	goto end;

revert:
	candidate_destroy(candidate);
end:
	mutex_unlock(&lock);
	return error;
}
