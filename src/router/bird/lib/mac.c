/*
 *	BIRD Library -- Message Authentication Codes
 *
 *	(c) 2016 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2016 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Message authentication codes
 *
 * MAC algorithms are simple cryptographic tools for message authentication.
 * They use shared a secret key a and message text to generate authentication
 * code, which is then passed with the message to the other side, where the code
 * is verified. There are multiple families of MAC algorithms based on different
 * cryptographic primitives, BIRD implements two MAC families which use hash
 * functions.
 *
 * The first family is simply a cryptographic hash camouflaged as MAC algorithm.
 * Originally supposed to be (m|k)-hash (message is concatenated with key, and
 * that is hashed), but later it turned out that a raw hash is more practical.
 * This is used for cryptographic authentication in OSPFv2, RIP and BFD.
 *
 * The second family is the standard HMAC (RFC 2104), using inner and outer hash
 * to process key and message. HMAC (with SHA) is used in advanced OSPF and RIP
 * authentication (RFC 5709, RFC 4822).
 */

#include "lib/mac.h"
#include "lib/md5.h"
#include "lib/sha1.h"
#include "lib/sha256.h"
#include "lib/sha512.h"


/*
 *	Internal hash calls
 */

static inline void
hash_init(struct mac_context *mctx, struct hash_context *hctx)
{ mctx->type->hash_init(hctx); }

static inline void
hash_update(struct mac_context *mctx, struct hash_context *hctx, const byte *buf, uint len)
{ mctx->type->hash_update(hctx, buf, len); }

static inline byte *
hash_final(struct mac_context *mctx, struct hash_context *hctx)
{ return mctx->type->hash_final(hctx); }

static inline void
hash_buffer(struct mac_context *mctx, byte *outbuf, const byte *buffer, uint length)
{
  struct hash_context hctx;

  hash_init(mctx, &hctx);
  hash_update(mctx, &hctx, buffer, length);
  memcpy(outbuf, hash_final(mctx, &hctx), mctx->type->hash_size);
}


/*
 *	(not-really-MAC) Hash
 */

static void
nrmh_init(struct mac_context *ctx, const byte *key UNUSED, uint keylen UNUSED)
{
  struct nrmh_context *ct = (void *) ctx;
  hash_init(ctx, &ct->ictx);
}

static void
nrmh_update(struct mac_context *ctx, const byte *data, uint datalen)
{
  struct nrmh_context *ct = (void *) ctx;
  hash_update(ctx, &ct->ictx, data, datalen);
}

static byte *
nrmh_final(struct mac_context *ctx)
{
  struct nrmh_context *ct = (void *) ctx;
  return hash_final(ctx, &ct->ictx);
}


/*
 *	HMAC
 */

static void
hmac_init(struct mac_context *ctx, const byte *key, uint keylen)
{
  struct hmac_context *ct = (void *) ctx;
  uint block_size = ctx->type->block_size;
  uint hash_size = ctx->type->hash_size;

  byte *keybuf = alloca(block_size);
  byte *buf = alloca(block_size);
  uint i;

  /* Hash the key if necessary */
  if (keylen <= block_size)
  {
    memcpy(keybuf, key, keylen);
    memset(keybuf + keylen, 0, block_size - keylen);
  }
  else
  {
    hash_buffer(ctx, keybuf, key, keylen);
    memset(keybuf + hash_size, 0, block_size - hash_size);
  }

  /* Initialize the inner digest */
  hash_init(ctx, &ct->ictx);
  for (i = 0; i < block_size; i++)
    buf[i] = keybuf[i] ^ 0x36;
  hash_update(ctx, &ct->ictx, buf, block_size);

  /* Initialize the outer digest */
  hash_init(ctx, &ct->octx);
  for (i = 0; i < block_size; i++)
    buf[i] = keybuf[i] ^ 0x5c;
  hash_update(ctx, &ct->octx, buf, block_size);
}

static void
hmac_update(struct mac_context *ctx, const byte *data, uint datalen)
{
  struct hmac_context *ct = (void *) ctx;

  /* Just update the inner digest */
  hash_update(ctx, &ct->ictx, data, datalen);
}

static byte *
hmac_final(struct mac_context *ctx)
{
  struct hmac_context *ct = (void *) ctx;

  /* Finish the inner digest */
  byte *isha = hash_final(ctx, &ct->ictx);

  /* Finish the outer digest */
  hash_update(ctx, &ct->octx, isha, ctx->type->hash_size);
  return hash_final(ctx, &ct->octx);
}


/*
 *	Common code
 */

#define HASH_DESC(name, px, PX) \
  { name, PX##_SIZE, sizeof(struct nrmh_context), nrmh_init, nrmh_update, nrmh_final, \
    PX##_SIZE, PX##_BLOCK_SIZE, px##_init, px##_update, px##_final }

#define HMAC_DESC(name, px, PX)						\
  { name, PX##_SIZE, sizeof(struct hmac_context), hmac_init, hmac_update, hmac_final, \
    PX##_SIZE, PX##_BLOCK_SIZE, px##_init, px##_update, px##_final }

const struct mac_desc mac_table[ALG_MAX] = {
  [ALG_MD5] =		HASH_DESC("Keyed MD5",		md5,	MD5),
  [ALG_SHA1] =		HASH_DESC("Keyed SHA-1",	sha1,	SHA1),
  [ALG_SHA224] =	HASH_DESC("Keyed SHA-224",	sha224,	SHA224),
  [ALG_SHA256] = 	HASH_DESC("Keyed SHA-256",	sha256,	SHA256),
  [ALG_SHA384] = 	HASH_DESC("Keyed SHA-384",	sha384,	SHA384),
  [ALG_SHA512] = 	HASH_DESC("Keyed SHA-512",	sha512,	SHA512),
  [ALG_HMAC_MD5] = 	HMAC_DESC("HMAC-MD5",		md5,	MD5),
  [ALG_HMAC_SHA1] = 	HMAC_DESC("HMAC-SHA-1",		sha1,	SHA1),
  [ALG_HMAC_SHA224] = 	HMAC_DESC("HMAC-SHA-224",	sha224,	SHA224),
  [ALG_HMAC_SHA256] = 	HMAC_DESC("HMAC-SHA-256",	sha256,	SHA256),
  [ALG_HMAC_SHA384] = 	HMAC_DESC("HMAC-SHA-384",	sha384,	SHA384),
  [ALG_HMAC_SHA512] = 	HMAC_DESC("HMAC-SHA-512",	sha512,	SHA512),
};


/**
 * mac_init - initialize MAC algorithm
 * @ctx: context to initialize
 * @id: MAC algorithm ID
 * @key: MAC key
 * @keylen: MAC key length
 *
 * Initialize MAC context @ctx for algorithm @id (e.g., %ALG_HMAC_SHA1), with
 * key @key of length @keylen. After that, message data could be added using
 * mac_update() function.
 */
void
mac_init(struct mac_context *ctx, uint id, const byte *key, uint keylen)
{
  ctx->type = &mac_table[id];
  ctx->type->init(ctx, key, keylen);
}

#if 0
/**
 * mac_update - add more data to MAC algorithm
 * @ctx: MAC context
 * @data: data to add
 * @datalen: length of data
 *
 * Push another @datalen bytes of data pointed to by @data into the MAC
 * algorithm currently in @ctx. Can be called multiple times for the same MAC
 * context. It has the same effect as concatenating all the data together and
 * passing them at once.
 */
void mac_update(struct mac_context *ctx, const byte *data, uint datalen)
{ DUMMY; }

/**
 * mac_final - finalize MAC algorithm
 * @ctx: MAC context
 *
 * Finish MAC computation and return a pointer to the result. No more
 * @mac_update() calls could be done, but the context may be reinitialized
 * later.
 *
 * Note that the returned pointer points into data in the @ctx context. If it
 * ceases to exist, the pointer becomes invalid.
 */
byte *mac_final(struct mac_context *ctx)
{ DUMMY; }

/**
 * mac_cleanup - cleanup MAC context
 * @ctx: MAC context
 *
 * Cleanup MAC context after computation (by filling with zeros). Not strictly
 * necessary, just to erase sensitive data from stack. This also invalidates the
 * pointer returned by @mac_final().
 */
void mac_cleanup(struct mac_context *ctx)
{ DUMMY; }

#endif

/**
 * mac_fill - compute and fill MAC
 * @id: MAC algorithm ID
 * @key: secret key
 * @keylen: key length
 * @data: message data
 * @datalen: message length
 * @mac: place to fill MAC
 *
 * Compute MAC for specified key @key and message @data using algorithm @id and
 * copy it to buffer @mac. mac_fill() is a shortcut function doing all usual
 * steps for transmitted messages.
 */
void
mac_fill(uint id, const byte *key, uint keylen, const byte *data, uint datalen, byte *mac)
{
  struct mac_context ctx;

  mac_init(&ctx, id, key, keylen);
  mac_update(&ctx, data, datalen);
  memcpy(mac, mac_final(&ctx), mac_get_length(&ctx));
  mac_cleanup(&ctx);
}

/**
 * mac_verify - compute and verify MAC
 * @id: MAC algorithm ID
 * @key: secret key
 * @keylen: key length
 * @data: message data
 * @datalen: message length
 * @mac: received MAC
 *
 * Compute MAC for specified key @key and message @data using algorithm @id and
 * compare it with received @mac, return whether they are the same. mac_verify()
 * is a shortcut function doing all usual steps for received messages.
 */
int
mac_verify(uint id, const byte *key, uint keylen, const byte *data, uint datalen, const byte *mac)
{
  struct mac_context ctx;

  mac_init(&ctx, id, key, keylen);
  mac_update(&ctx, data, datalen);
  int res = !memcmp(mac, mac_final(&ctx), mac_get_length(&ctx));
  mac_cleanup(&ctx);

  return res;
}
