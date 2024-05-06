#include "mod/common/db/pool4/rfc6056.h"

#include <crypto/hash.h>
#include "mod/common/linux_version.h"
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"

/*
 * My own notes on RFC 6056's algorithms:
 *
 * Algorithms 1 and 2: Currently, the main drawback is that they would require
 * calls to `get_random_bytes()`, which is a big no-no according to issue #282.
 * But IIRC, the original reason why I chose not to use them was because they
 * break games. These algorithms are very anti-RFC6146.
 *
 * (When you see the word "gaming" or "games", assume "applications that open
 * lots of connections, and which the server probably expects them all to share
 * the same IP address.")
 *
 * Algorithm 3: I no longer like this algorithm as much as I used to, TBH. The
 * fact that `next_ephemeral` is shared between all connections means that high
 * traffic will increase the probability of games breaking.
 * However, it seems that `next_ephemeral` was removed from Jool during some
 * refactor. Whether this was on purpose or not, it is both good (because it
 * breaks games less) and bad (because it creates unnecesary collisions during
 * port selection for games). Because our pool4 has the max_iterations feature,
 * the bad might not be so troublesome after all.
 *
 * The reason why I chose it is because I find it (cleverly) offers an
 * interesting tradeoff between randomization (thanks to `F`) and source
 * preservation (by way of checking adjacent ports during the loop).
 *
 * Algorithm 4: I rejected this one for two reasons. The second one is probably
 * good:
 *
 * 1. I feel like computing two separate hashes is too much overhead for such a
 *    minor operation.
 *    (But I'm making assumptions here. MD5 is probably very fast actually, and
 *    if `shash_desc` can be cloned, most of the operation will be obviated away
 *    because the hashes differ in one field only.)
 * 2. Storing the `table` array sounds like a pain.
 *    (But not that much more of a pain than maintaining a global ephemeral. My
 *    main gripe is that the array size would have to be configurable, and I
 *    really don't want to bother users with more stupefyingly-specific global
 *    fields that nobody asked for.)
 *
 * Algorithm 5: This algorithm jumps a lot (particularly with a default `N` of
 * 500), so it's also 6146-unfriendly. It also has the drawback that `N` needs
 * to be configurable, so please no.
 *
 * In all reality, the ideal solution would be to just fuck it and implement all
 * 5 algorithms. But since probably nobody cares, this would be a lot of
 * unnecesary work. 3 remains the winner for me.
 *
 * Also, I wonder if this whole gaming gimmic is that much of a factor. Do
 * servers really expect clients to maintain consistent IP addresses when NAT44
 * is so pervasive in today's Internet? Also, it's perfectly legal for
 * client to own more than one IP address. Also, I understand smartphones are a
 * huge market for online gaming, and I don't imagine it's rare for them to keep
 * switching networks on the go.
 */

/*
 * TODO (issue175) RFC 6056 wants us to change this from time to time.
 *
 * For now they are only modified during module initialization and destruction,
 * which means they don't need synchronization.
 */
static unsigned char *secret_key;
static size_t secret_key_len;

/*
 * It looks like this does not require a spinlock either:
 *
 * "The shash interface (...)
 * improves over hash in two ways.  Firstly shash is reentrant,
 * meaning that the same tfm may be used by two threads simultaneously
 * as all hashing state is stored in a local descriptor."
 * (Linux commit 7b5a080b3c46f0cac71c0d0262634c6517d4ee4f)
 */
static struct crypto_shash *shash;

int rfc6056_setup(void)
{
	int error;

	/* Secret key stuff */
	secret_key_len = (PAGE_SIZE < 128) ? PAGE_SIZE : 128;
	secret_key = __wkmalloc("Secret key", secret_key_len, GFP_KERNEL);
	if (!secret_key)
		return -ENOMEM;
	get_random_bytes(secret_key, secret_key_len);

	/* TFC stuff */
	shash = crypto_alloc_shash("md5", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(shash)) {
		error = PTR_ERR(shash);
		log_warn_once("Failed to load transform for MD5; errcode %d",
				error);
		__wkfree("Secret key", secret_key);
		return error;
	}

	return 0;
}

void rfc6056_teardown(void)
{
	crypto_free_shash(shash);
	__wkfree("Secret key", secret_key);
}

static int hash_tuple(struct shash_desc *desc, __u8 fields,
		const struct tuple *tuple6)
{
	int error;

	if (fields & F_ARGS_SRC_ADDR) {
		error = crypto_shash_update(desc, (u8 *)&tuple6->src.addr6.l3,
				sizeof(tuple6->src.addr6.l3));
		if (error)
			return error;
	}
	if (fields & F_ARGS_SRC_PORT) {
		error = crypto_shash_update(desc, (u8 *)&tuple6->src.addr6.l4,
				sizeof(tuple6->src.addr6.l4));
		if (error)
			return error;
	}
	if (fields & F_ARGS_DST_ADDR) {
		error = crypto_shash_update(desc, (u8 *)&tuple6->dst.addr6.l3,
				sizeof(tuple6->dst.addr6.l3));
		if (error)
			return error;
	}
	if (fields & F_ARGS_DST_PORT) {
		error = crypto_shash_update(desc, (u8 *)&tuple6->dst.addr6.l4,
				sizeof(tuple6->dst.addr6.l4));
		if (error)
			return error;
	}

	return crypto_shash_update(desc, secret_key, secret_key_len);
}

/**
 * RFC 6056, Algorithm 3. Returns a hash out of some of @tuple's fields.
 *
 * Just to clarify: Because our port pool is a somewhat complex data structure
 * (rather than a simple range), ephemerals are now handled by pool4. This
 * function has been stripped now to only consist of F(). (Hence the name.)
 */
int rfc6056_f(struct xlation *state, unsigned int *result)
{
	union {
		__be32 as32[4];
		__u8 as8[16];
	} md5_result;
	struct shash_desc *desc;
	int error = 0;

	desc = __wkmalloc("shash desc", sizeof(struct shash_desc)
			+ crypto_shash_descsize(shash), GFP_ATOMIC);
	if (!desc)
		return -ENOMEM;

	desc->tfm = shash;
/* Linux commit: 877b5691f27a1aec0d9b53095a323e45c30069e2 */
#if LINUX_VERSION_LOWER_THAN(5, 2, 0, 9, 0)
	desc->flags = 0;
#endif

	error = crypto_shash_init(desc);
	if (error) {
		log_debug(state, "crypto_hash_init() error: %d", error);
		goto end;
	}

	error = hash_tuple(desc, state->jool.globals.nat64.f_args,
			&state->in.tuple);
	if (error) {
		log_debug(state, "crypto_hash_update() error: %d", error);
		goto end;
	}

	error = crypto_shash_final(desc, md5_result.as8);
	if (error) {
		log_debug(state, "crypto_hash_digest() error: %d", error);
		goto end;
	}

	*result = (__force __u32)md5_result.as32[3];
	/* Fall through. */

end:
	__wkfree("shash desc", desc);
	return error;
}
