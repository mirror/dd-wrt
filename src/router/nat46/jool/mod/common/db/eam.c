#include "mod/common/db/eam.h"

#include "common/types.h"
#include "mod/common/address.h"
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"

#define ADDR6_BITS		128
#define ADDR4_BITS		32

#define INIT_KEY(ptr, length)	{ .bytes = (__u8 *)(ptr), .len = length }
#define ADDR_TO_KEY(addr)	INIT_KEY(addr, 8 * sizeof(*addr))
#define PREFIX_TO_KEY(prefix)	INIT_KEY(&(prefix)->addr, (prefix)->len)

/**
 * Well, it really goes without saying, but I'll say it anyway:
 *
 * The fact that @trie6 and @trie4 are protected by RCU means that there will be
 * small time windows during EAMT updates where an entry will appear in one of
 * the tries but not in the other.
 *
 * This should be fine. It simply means, for example, that when the admin adds
 * an entry, for a few milliseconds or less, the entry will serve a packet
 * traveling in the 6-to-4 direction, but not one traveling in the 4-to-6
 * direction. Since the user just added the entry, this inconsistency should go
 * unnoticed or chalked up to timing noise.
 *
 * Notice that this only applies to updates to running EAMTs. Atomic
 * configuration does not fall in this category because the full table is
 * set up before it is actually committed to serve packets.
 */
struct eam_table {
	struct rtrie trie6;
	struct rtrie trie4;
	/**
	 * This one is not RCU-friendly. Touch only while you're holding the
	 * mutex.
	 */
	u64 count;
	struct kref refcount;
};

static DEFINE_MUTEX(lock);

static bool eamt_entry_equals(const struct eamt_entry *eam1,
		const struct eamt_entry *eam2)
{
	return prefix6_equals(&eam1->prefix6, &eam2->prefix6)
			&& prefix4_equals(&eam1->prefix4, &eam2->prefix4);
}

/**
 * validate_prefixes - check @prefix6 and @prefix4 can be joined together to
 * form a (standalone) legal EAM entry.
 */
static int validate_prefixes(struct eamt_entry *new)
{
	int error;

	error = prefix6_validate(&new->prefix6);
	if (error)
		return error;
	error = prefix4_validate(&new->prefix4);
	if (error)
		return error;

	if ((ADDR4_BITS - new->prefix4.len) > (ADDR6_BITS - new->prefix6.len)) {
		log_err("The IPv4 suffix length must be smaller or equal than the IPv6 suffix length.");
		return -EINVAL;
	}

	return 0;
}

static void msg_programming_error(void)
{
	log_err("(Note: This error should have been caught earlier.");
	log_err("This looks like a bug; please report.)");
}

static int collision6(struct eamt_entry *new, struct eamt_entry *old,
		bool force)
{
	if (new->prefix6.len == old->prefix6.len) {
		if (prefix4_equals(&new->prefix4, &old->prefix4)) {
			log_err("The requested entry already exists.");
			return -EEXIST;
		}
		log_err("Existing EAM [%pI6c/%u|%pI4/%u] already contains prefix %pI6c/%u.",
				&old->prefix6.addr, old->prefix6.len,
				&old->prefix4.addr, old->prefix4.len,
				&new->prefix6.addr, new->prefix6.len);
		return -EEXIST;
	}

	if (force)
		return 0;

	log_err("Prefix %pI6c/%u overlaps with EAM [%pI6c/%u|%pI4/%u].\n"
			"(Use --force to override this validation.)",
			&new->prefix6.addr, new->prefix6.len,
			&old->prefix6.addr, old->prefix6.len,
			&old->prefix4.addr, old->prefix4.len);
	return -EEXIST;
}

static int collision4(struct eamt_entry *new, struct eamt_entry *old,
		bool force)
{
	if (new->prefix4.len == old->prefix4.len) {
		if (prefix6_equals(&new->prefix6, &old->prefix6)) {
			log_err("The requested entry already exists.");
			msg_programming_error();
			return -EEXIST;
		}
		log_err("Existing EAM [%pI6c/%u|%pI4/%u] already contains prefix %pI4/%u.",
				&old->prefix6.addr, old->prefix6.len,
				&old->prefix4.addr, old->prefix4.len,
				&new->prefix4.addr, new->prefix4.len);
		return -EEXIST;
	}

	if (force)
		return 0;

	log_err("Prefix %pI4/%u overlaps with EAM [%pI6c/%u|%pI4/%u].\n"
			"(Use --force to override this validation.)",
			&new->prefix4.addr, new->prefix4.len,
			&old->prefix6.addr, old->prefix6.len,
			&old->prefix4.addr, old->prefix4.len);
	return -EEXIST;
}

static int validate_overlapping(struct eam_table *eamt, struct eamt_entry *new,
		bool force)
{
	struct eamt_entry old;
	struct rtrie_key key6 = PREFIX_TO_KEY(&new->prefix6);
	struct rtrie_key key4 = PREFIX_TO_KEY(&new->prefix4);
	int error;

	key6.len = 128;
	key4.len = 32;

	error = rtrie_find(&eamt->trie6, &key6, &old);
	if (!error) {
		error = collision6(new, &old, force);
		if (error)
			return error;
	}

	error = rtrie_find(&eamt->trie4, &key4, &old);
	if (!error) {
		error = collision4(new, &old, force);
		if (error)
			return error;
	}

	return 0;
}

static void __revert_add6(struct eam_table *eamt, struct ipv6_prefix *prefix6,
		bool synchronize)
{
	struct rtrie_key key = PREFIX_TO_KEY(prefix6);
	int error;

	error = rtrie_rm(&eamt->trie6, &key, synchronize);
	WARN(error, "Got error %d while trying to remove an EAM I just added.",
			error);
}

static int eamt_add6(struct eam_table *eamt, struct eamt_entry *eam,
		bool synchronize)
{
	size_t addr_offset;
	int error;

	addr_offset = offsetof(typeof(*eam), prefix6.addr);
	error = rtrie_add(&eamt->trie6, eam, addr_offset, eam->prefix6.len,
			synchronize);
	if (error == -EEXIST) {
		log_err("Prefix %pI6c/%u already exists.",
				&eam->prefix6.addr, eam->prefix6.len);
		msg_programming_error();
	}
	/* rtrie_print("IPv6 trie after add", &eamt->trie6); */

	return error;
}

static int eamt_add4(struct eam_table *eamt, struct eamt_entry *eam,
		bool synchronize)
{
	size_t addr_offset;
	int error;

	addr_offset = offsetof(typeof(*eam), prefix4.addr);
	error = rtrie_add(&eamt->trie4, eam, addr_offset, eam->prefix4.len,
			synchronize);
	if (error == -EEXIST) {
		log_err("Prefix %pI4/%u already exists.",
				&eam->prefix4.addr, eam->prefix4.len);
		msg_programming_error();
	}
	/* rtrie_print("IPv4 trie after add", &eamt->trie4); */

	return error;
}

int eamt_add(struct eam_table *eamt, struct eamt_entry *new, bool force,
		bool synchronize)
{
	int error;

	error = validate_prefixes(new);
	if (error)
		return error;

	mutex_lock(&lock);

	error = validate_overlapping(eamt, new, force);
	if (error)
		goto end;

	error = eamt_add6(eamt, new, synchronize);
	if (error)
		goto end;
	error = eamt_add4(eamt, new, synchronize);
	if (error) {
		__revert_add6(eamt, &new->prefix6, synchronize);
		goto end;
	}

	eamt->count++;
end:
	mutex_unlock(&lock);
	return error;
}

static int get_exact6(struct eam_table *eamt, struct ipv6_prefix *prefix,
		struct eamt_entry *eam)
{
	struct rtrie_key key = PREFIX_TO_KEY(prefix);
	int error;

	error = rtrie_find(&eamt->trie6, &key, eam);
	if (error)
		return error;

	return (eam->prefix6.len == prefix->len) ? 0 : -ESRCH;
}

static int get_exact4(struct eam_table *eamt, struct ipv4_prefix *prefix,
		struct eamt_entry *eam)
{
	struct rtrie_key key = PREFIX_TO_KEY(prefix);
	int error;

	error = rtrie_find(&eamt->trie4, &key, eam);
	if (error)
		return error;

	return (eam->prefix4.len == prefix->len) ? 0 : -ESRCH;
}

static int __rm(struct eam_table *eamt,
		struct ipv6_prefix *prefix6,
		struct ipv4_prefix *prefix4)
{
	struct rtrie_key key6 = PREFIX_TO_KEY(prefix6);
	struct rtrie_key key4 = PREFIX_TO_KEY(prefix4);
	int error;

	error = rtrie_rm(&eamt->trie6, &key6, true);
	if (error)
		goto corrupted;
	error = rtrie_rm(&eamt->trie4, &key4, true);
	if (error)
		goto corrupted;
	eamt->count--;

	/* rtrie_print("IPv6 trie after remove", &eamt.trie6); */
	/* rtrie_print("IPv4 trie after remove", &eamt.trie4); */
	return 0;

corrupted:
	WARN(true, "EAMT entry was extracted from the table, but it no longer seems to be there.\n"
			"Errcode: %d", error);
	return error;
}

static int eamt_rm_lockless(struct eam_table *eamt,
		struct ipv6_prefix *prefix6,
		struct ipv4_prefix *prefix4)
{
	struct eamt_entry eam6;
	struct eamt_entry eam4;
	int error;

	if (!prefix4) {
		error = get_exact6(eamt, prefix6, &eam6);
		return error ? error : __rm(eamt, prefix6, &eam6.prefix4);
	}

	if (!prefix6) {
		error = get_exact4(eamt, prefix4, &eam4);
		return error ? error : __rm(eamt, &eam4.prefix6, prefix4);
	}

	error = get_exact6(eamt, prefix6, &eam6);
	if (error)
		return error;
	error = get_exact4(eamt, prefix4, &eam4);
	if (error)
		return error;

	return eamt_entry_equals(&eam6, &eam4)
			? __rm(eamt, prefix6, prefix4)
			: -ESRCH;
}

int eamt_rm(struct eam_table *eamt,
		struct ipv6_prefix *prefix6,
		struct ipv4_prefix *prefix4)
{
	int error;

	if (WARN(!prefix6 && !prefix4, "Prefixes can't both be NULL"))
		return -EINVAL;

	mutex_lock(&lock);
	error = eamt_rm_lockless(eamt, prefix6, prefix4);
	mutex_unlock(&lock);

	return error;
}

bool eamt_contains6(struct eam_table *eamt, struct in6_addr *addr)
{
	struct rtrie_key key = ADDR_TO_KEY(addr);
	return rtrie_contains(&eamt->trie6, &key);
}

bool eamt_contains4(struct eam_table *eamt, __be32 addr)
{
	struct in_addr tmp = { .s_addr = addr };
	struct rtrie_key key = ADDR_TO_KEY(&tmp);
	return rtrie_contains(&eamt->trie4, &key);
}

/** Contract: Returns 0 or -ESRCH. No other outcomes. */
int eamt_xlat_6to4(struct eam_table *eamt, struct in6_addr *addr6,
		struct result_addrxlat64 *result)
{
	struct rtrie_key key = ADDR_TO_KEY(addr6);
	struct eamt_entry *eam;
	struct in_addr *addr4;
	unsigned int i;
	int error;

	eam = &result->entry.eam;
	addr4 = &result->addr;

	/* Find the entry. */
	error = rtrie_find(&eamt->trie6, &key, eam);
	if (error)
		return error;

	/* Translate the address. */
	*addr4 = eam->prefix4.addr;

	for (i = 0; i < ADDR4_BITS - eam->prefix4.len; i++) {
		unsigned int offset4 = eam->prefix4.len + i;
		unsigned int offset6 = eam->prefix6.len + i;
		addr4_set_bit(addr4, offset4, addr6_get_bit(addr6, offset6));
	}

	/* I'm assuming the prefix address is already zero-trimmed. */
	result->entry.method = AXM_EAMT;
	return 0;
}

/** Contract: Returns 0 or -ESRCH. No other outcomes. */
int eamt_xlat_4to6(struct eam_table *eamt, struct in_addr *addr4,
		struct result_addrxlat46 *result)
{
	struct rtrie_key key = ADDR_TO_KEY(addr4);
	struct eamt_entry *eam;
	struct in6_addr *addr6;
	unsigned int i;
	int error;

	eam = &result->entry.eam;
	addr6 = &result->addr;

	/* Find the entry. */
	error = rtrie_find(&eamt->trie4, &key, eam);
	if (error)
		return error;

	/* Translate the address. */
	*addr6 = eam->prefix6.addr;

	for (i = 0; i < ADDR4_BITS - eam->prefix4.len; i++) {
		unsigned int offset4 = eam->prefix4.len + i;
		unsigned int offset6 = eam->prefix6.len + i;
		addr6_set_bit(addr6, offset6, addr4_get_bit(addr4, offset4));
	}

	/* I'm assuming the prefix address is already zero-trimmed. */
	result->entry.method = AXM_EAMT;
	return 0;
}

bool eamt_is_empty(struct eam_table *eamt)
{
	return rtrie_is_empty(&eamt->trie6);
}

struct foreach_args {
	int (*cb)(struct eamt_entry const *, void *);
	void *arg;
};

static int foreach_cb(void const *eam, void *arg)
{
	struct foreach_args *args = arg;
	return args->cb(eam, args->arg);
}

int eamt_foreach(struct eam_table *eamt,
		eamt_foreach_cb cb, void *arg,
		struct ipv4_prefix *offset)
{
	struct foreach_args args = { .cb = cb, .arg = arg };
	struct rtrie_key offset_key;
	struct rtrie_key *offset_key_ptr = NULL;
	int error;

	if (offset) {
		offset_key.bytes = (__u8 *) &offset->addr;
		offset_key.len = offset->len;
		offset_key_ptr = &offset_key;
	}

	mutex_lock(&lock);
	error = rtrie_foreach(&eamt->trie4, foreach_cb, &args, offset_key_ptr);
	mutex_unlock(&lock);
	return error;
}

void eamt_flush(struct eam_table *eamt)
{
	mutex_lock(&lock);
	rtrie_flush(&eamt->trie6);
	rtrie_flush(&eamt->trie4);
	eamt->count = 0;
	mutex_unlock(&lock);
}

struct eam_table *eamt_alloc(void)
{
	struct eam_table *result;

	result = wkmalloc(struct eam_table, GFP_KERNEL);
	if (!result)
		return NULL;

	rtrie_init(&result->trie6, sizeof(struct eamt_entry), &lock);
	rtrie_init(&result->trie4, sizeof(struct eamt_entry), &lock);
	result->count = 0;
	kref_init(&result->refcount);

	return result;
}

void eamt_get(struct eam_table *eamt)
{
	kref_get(&eamt->refcount);
}

/**
 * Please note: this function can sleep.
 */
static void eamt_release(struct kref *refcount)
{
	struct eam_table *eamt;
	eamt = container_of(refcount, struct eam_table, refcount);
	rtrie_clean(&eamt->trie6);
	rtrie_clean(&eamt->trie4);
	wkfree(struct eam_table, eamt);
}

void eamt_put(struct eam_table *eamt)
{
	kref_put(&eamt->refcount, eamt_release);
}
