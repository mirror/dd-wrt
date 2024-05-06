#include "mod/common/joold.h"

#include <linux/inet.h>

#include "common/constants.h"
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/nl/nl_handler.h"
#include "mod/common/db/bib/db.h"
#include "mod/common/steps/send_packet.h"

#define GLOBALS(xlator) (xlator->globals.nat64.joold)

struct counted_list {
	struct list_head list;
	unsigned int count;
};

/**
 * Can we send our packet yet?
 * (We need to wait for ACKs because the kernel can't handle too many
 * Netlink messages at once.)
 */
#define JQF_ACK_RECEIVED (1 << 0)
#define JQF_AD_ONGOING (1 << 1) /** Advertisement requested by user? */

struct joold_queue {
	unsigned int flags; /** JQF */

	struct counted_list deferred; /** Queued sessions */

	/**
	 * Jiffy at which the last batch of sessions was sent.
	 * If the ACK was lost for some reason, this should get us back on
	 * track.
	 */
	unsigned long last_flush_time;

	spinlock_t lock;
	struct kref refs;
};

struct ad_arg {
	struct joold_queue *queue;
	struct list_head *ready;
};

/**
 * A session or group of sessions that need to be transmitted to other Jool
 * instances in the near future.
 */
struct deferred_session {
	struct session_entry session;
	/** List hook to joold_queue.deferred.  */
	struct list_head lh;
};

static struct kmem_cache *deferred_cache;

#define ALLOC_DEFERRED \
	wkmem_cache_alloc("joold session", deferred_cache, GFP_ATOMIC)
#define FREE_DEFERRED(deferred) \
	wkmem_cache_free("joold session", deferred_cache, deferred)

static struct deferred_session *first_deferred(struct list_head *list)
{
	return list_first_entry(list, struct deferred_session, lh);
}

static int joold_setup(void)
{
	deferred_cache = kmem_cache_create("joold_sessions",
			sizeof(struct deferred_session), 0, 0, NULL);
	return deferred_cache ? 0 : -EINVAL;
}

void joold_teardown(void)
{
	if (deferred_cache) {
		kmem_cache_destroy(deferred_cache);
		deferred_cache = NULL;
	}
}

static void delete_sessions(struct list_head *sessions)
{
	struct list_head *cursor, *tmp;

	list_for_each_safe(cursor, tmp, sessions) {
		list_del(cursor);
		FREE_DEFERRED(list_entry(cursor, struct deferred_session, lh));
	}
}

/* "advertise session," not "add session." Although we're adding it too. */
static int ad_session(struct session_entry const *_session, void *arg)
{
	struct counted_list *list;
	struct deferred_session *session;

	session = ALLOC_DEFERRED;
	if (!session)
		return -ENOMEM;
	session->session = *_session;

	list = arg;
	list_add_tail(&session->lh, &list->list);
	list->count++;
	return 0;
}

static bool should_send(struct xlator *jool)
{
	struct joold_queue *queue;
	unsigned long deadline;

	queue = jool->nat64.joold;

	if (queue->deferred.count == 0)
		return false;

	deadline = msecs_to_jiffies(GLOBALS(jool).flush_deadline);
	if (time_before(queue->last_flush_time + deadline, jiffies))
		return true;

	if (!(queue->flags & JQF_ACK_RECEIVED))
		return false;

	if (queue->flags & JQF_AD_ONGOING)
		return true;

	if (GLOBALS(jool).flush_asap)
		return true;

	return queue->deferred.count >= GLOBALS(jool).max_sessions_per_pkt;
}

static bool too_many_sessions(struct xlator *jool)
{
	struct joold_queue *queue = jool->nat64.joold;

	if (queue->flags & JQF_AD_ONGOING)
		return false;

	return queue->deferred.count >= GLOBALS(jool).capacity;
}

/**
 * Always swallows @session, can be NULL.
 * Assumes the lock is held.
 * You have to send_to_userspace(@jool, @prepared) after releasing the spinlock.
 */
static void send_to_userspace_prepare(struct xlator *jool,
		struct deferred_session *session,
		struct list_head *prepared)
{
	struct joold_queue *queue;
	struct list_head *cut;
	unsigned int d;

	queue = jool->nat64.joold;

	if (session) {
		if (too_many_sessions(jool)) {
			log_warn_once("joold: Too many sessions deferred! I need to drop some; sorry.");
		} else {
			list_add_tail(&session->lh, &queue->deferred.list);
			queue->deferred.count++;
		}
	}

	if (!should_send(jool))
		return;

	if (queue->deferred.count <= GLOBALS(jool).max_sessions_per_pkt) {
		cut = queue->deferred.list.prev;
		d = queue->deferred.count;
	} else {
		cut = &queue->deferred.list;
		for (d = 0; d < GLOBALS(jool).max_sessions_per_pkt; d++)
			cut = cut->next;
	}

	list_cut_position(prepared, &queue->deferred.list, cut);
	queue->deferred.count -= d;

	/*
	 * BTW: This sucks.
	 * We're assuming that the nlcore_send_multicast_message() during
	 * send_to_userspace() is going to succeed.
	 * But the alternative is to do the nlcore_send_multicast_message()
	 * with the lock held, and I don't have the stomach for that.
	 */
	queue->flags &= ~JQF_ACK_RECEIVED;
	if (queue->deferred.count == 0)
		queue->flags &= ~JQF_AD_ONGOING;
	queue->last_flush_time = jiffies;
}

/*
 * Swallows ownership of the sessions.
 */
static void send_to_userspace(struct xlator *jool, struct list_head *sessions)
{
	struct sk_buff *skb;
	struct joolnlhdr *jhdr;
	struct nlattr *root;
	struct deferred_session *session;
	unsigned int count;
	int error;

	if (list_empty(sessions))
		return;

	skb = genlmsg_new(1500, GFP_ATOMIC);
	if (!skb)
		goto revert_list;

	jhdr = genlmsg_put(skb, 0, 0, jnl_family(), 0, 0);
	if (WARN(!jhdr, "genlmsg_put() returned NULL"))
		goto revert_skb;

	memset(jhdr, 0, sizeof(*jhdr));
	memcpy(jhdr->magic, JOOLNL_HDR_MAGIC, JOOLNL_HDR_MAGIC_LEN);
	jhdr->version = cpu_to_be32(xlat_version());
	jhdr->xt = XT_NAT64;
	memcpy(jhdr->iname, jool->iname, INAME_MAX_SIZE);

	root = nla_nest_start(skb, JNLAR_SESSION_ENTRIES);
	if (WARN(!root, "nla_nest_start() returned NULL"))
		goto revert_skb;

	count = 0;
	while (!list_empty(sessions)) {
		session = first_deferred(sessions);
		error = jnla_put_session(skb, JNLAL_ENTRY, &session->session);
		if (WARN(error, "jnla_put_session() returned %d", error))
			goto revert_skb;
		list_del(&session->lh);
		FREE_DEFERRED(session);
		count++;
	}

	nla_nest_end(skb, root);
	genlmsg_end(skb, jhdr);
	sendpkt_multicast(jool, skb);
	return;

revert_skb:
	kfree_skb(skb);
revert_list:
	delete_sessions(sessions);
}

/**
 * joold_create - Constructor for joold_queue structs.
 */
struct joold_queue *joold_alloc(void)
{
	struct joold_queue *queue;
	bool cache_created;

	cache_created = false;
	if (!deferred_cache) {
		if (joold_setup())
			return NULL;
		cache_created = true;
	}

	queue = wkmalloc(struct joold_queue, GFP_KERNEL);
	if (!queue) {
		if (cache_created)
			joold_teardown();
		return NULL;
	}

	queue->flags = JQF_ACK_RECEIVED;
	INIT_LIST_HEAD(&queue->deferred.list);
	queue->deferred.count = 0;
	queue->last_flush_time = jiffies;
	spin_lock_init(&queue->lock);
	kref_init(&queue->refs);

	return queue;
}

void joold_get(struct joold_queue *queue)
{
	kref_get(&queue->refs);
}

static void joold_release(struct kref *refs)
{
	struct joold_queue *queue;
	queue = container_of(refs, struct joold_queue, refs);
	delete_sessions(&queue->deferred.list);
	wkfree(struct joold_queue, queue);
}

void joold_put(struct joold_queue *queue)
{
	kref_put(&queue->refs, joold_release);
}

/**
 * joold_add - Add @session to @jool->nat64.joold.
 *
 * This is the function that gets called whenever a packet translation
 * successfully triggers the creation of a session entry. @session will be sent
 * to the joold daemon.
 */
void joold_add(struct xlator *jool, struct session_entry *_session)
{
	struct joold_queue *queue;
	struct deferred_session *session;
	struct list_head prepared;

	if (!GLOBALS(jool).enabled)
		return;

	session = ALLOC_DEFERRED;
	if (!session)
		return;
	session->session = *_session;
	queue = jool->nat64.joold;
	INIT_LIST_HEAD(&prepared);

	spin_lock_bh(&queue->lock);
	send_to_userspace_prepare(jool, session, &prepared);
	spin_unlock_bh(&queue->lock);

	send_to_userspace(jool, &prepared);
}

struct add_params {
	struct session_entry new;
	bool success;
};

static enum session_fate collision_cb(struct session_entry *old, void *arg)
{
	struct add_params *params = arg;
	struct session_entry *new = &params->new;

	if (session_equals(old, new)) { /* It's the same session; update it. */
		old->state = new->state;
		old->timer_type = new->timer_type;
		old->update_time = new->update_time;
		params->success = true;
		return FATE_TIMER_SLOW;
	}

	log_err("We're out of sync: Incoming session entry " SEPP
			" collides with DB entry " SEPP ".",
			SEPA(new), SEPA(old));
	params->success = false;
	return FATE_PRESERVE;
}

static bool add_new_session(struct xlator *jool, struct nlattr *attr)
{
	struct add_params params;
	struct collision_cb cb;
	int error;

	__log_debug(jool, "Adding session!");

	error = jnla_get_session(attr, "joold session",
			&jool->globals.nat64.bib, &params.new);
	if (error)
		return false;

	params.success = true;
	cb.cb = collision_cb;
	cb.arg = &params;

	error = bib_add_session(jool, &params.new, &cb);
	if (error == -EEXIST)
		return params.success;
	if (error) {
		log_err("sessiondb_add() threw unknown error code %d.", error);
		return false;
	}

	return true;
}

static bool joold_disabled(struct xlator *jool)
{
	if (!GLOBALS(jool).enabled) {
		log_err("Session sync is disabled on this instance.");
		return true;
	}

	return false;
}

/**
 * joold_sync - Parses a bunch of sessions out of @data and adds them to @jool's
 * session database.
 *
 * This is the function that gets called whenever the jool daemon sends data to
 * the @jool Jool instance.
 */
int joold_sync(struct xlator *jool, struct nlattr *root)
{
	struct nlattr *attr;
	int rem;
	bool success;

	if (joold_disabled(jool))
		return -EINVAL;

	success = true;
	nla_for_each_nested(attr, root, rem)
		success &= add_new_session(jool, attr);

	__log_debug(jool, "Done.");
	return success ? 0 : -EINVAL;
}

int joold_advertise(struct xlator *jool)
{
	l4_protocol proto;
	struct joold_queue *queue;
	struct counted_list sessions;
	struct list_head prepared;
	int error;

	if (joold_disabled(jool))
		return -EINVAL;

	/* Collect the current sessions */
	INIT_LIST_HEAD(&sessions.list);
	sessions.count = 0;
	for (proto = L4PROTO_TCP; proto <= L4PROTO_ICMP; proto++) {
		error = bib_foreach_session(jool, proto, ad_session, &sessions,
				NULL);
		if (error) {
			log_err("joold advertisement interrupted.");
			delete_sessions(&sessions.list);
			return error;
		}
	}

	if (sessions.count == 0)
		return 0;

	queue = jool->nat64.joold;
	INIT_LIST_HEAD(&prepared);

	spin_lock_bh(&queue->lock);

	if (queue->flags & JQF_AD_ONGOING) {
		spin_unlock_bh(&queue->lock);
		delete_sessions(&sessions.list);
		log_err("joold advertisement already in progress.");
		return -EINVAL;
	}
	queue->flags |= JQF_AD_ONGOING;

	list_move_all(&sessions.list, &queue->deferred.list);
	queue->deferred.count += sessions.count;

	send_to_userspace_prepare(jool, NULL, &prepared);

	spin_unlock_bh(&queue->lock);

	send_to_userspace(jool, &prepared);
	return 0;
}

void joold_ack(struct xlator *jool)
{
	struct joold_queue *queue;
	struct list_head prepared;

	if (joold_disabled(jool))
		return;

	queue = jool->nat64.joold;
	INIT_LIST_HEAD(&prepared);

	spin_lock_bh(&queue->lock);
	queue->flags |= JQF_ACK_RECEIVED;
	send_to_userspace_prepare(jool, NULL, &prepared);
	spin_unlock_bh(&queue->lock);

	send_to_userspace(jool, &prepared);
}

/**
 * Called every now and then to flush the queue in case nodes have been queued,
 * the deadline is in the past and no new packets have triggered a flush.
 * It's just a last-resort attempt to prevent nodes from lingering here for too
 * long that's generally only useful in non-flush-asap mode.
 */
void joold_clean(struct xlator *jool)
{
	spinlock_t *lock;
	struct list_head prepared;

	if (!GLOBALS(jool).enabled)
		return;

	lock = &jool->nat64.joold->lock;
	INIT_LIST_HEAD(&prepared);

	spin_lock_bh(lock);
	send_to_userspace_prepare(jool, NULL, &prepared);
	spin_unlock_bh(lock);

	send_to_userspace(jool, &prepared);
}
