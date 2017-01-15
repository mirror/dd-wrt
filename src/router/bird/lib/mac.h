/*
 *	BIRD Library -- Message Authentication Codes
 *
 *	(c) 2016 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2016 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_MAC_H_
#define _BIRD_MAC_H_

#include "nest/bird.h"
#include "lib/sha512.h"


#define ALG_UNDEFINED		0
#define ALG_MD5			0x01
#define ALG_SHA1		0x02
#define ALG_SHA224		0x03
#define ALG_SHA256		0x04
#define ALG_SHA384		0x05
#define ALG_SHA512		0x06
#define ALG_HMAC		0x10
#define ALG_HMAC_MD5		0x11
#define ALG_HMAC_SHA1		0x12
#define ALG_HMAC_SHA224		0x13
#define ALG_HMAC_SHA256		0x14
#define ALG_HMAC_SHA384		0x15
#define ALG_HMAC_SHA512		0x16
#define ALG_MAX			0x17

/* These are maximums for HASH/MAC lengths and required context space */
#define MAX_HASH_SIZE		SHA512_SIZE
#define HASH_STORAGE		sizeof(struct sha512_context)
#define MAC_STORAGE		sizeof(struct hmac_context)

/* This value is used by several IETF protocols for padding */
#define HMAC_MAGIC		htonl(0x878FE1F3)

/* Generic context used by hash functions */
struct hash_context
{
  u8 data[HASH_STORAGE];
  u64 align[0];
};

/* Context for embedded hash (not-really-MAC hash) */
struct nrmh_context {
  const struct mac_desc *type;
  struct hash_context ictx;
};

/* Context for hash based HMAC */
struct hmac_context {
  const struct mac_desc *type;
  struct hash_context ictx;
  struct hash_context octx;
};

/* Generic context used by MAC functions */
struct mac_context
{
  const struct mac_desc *type;
  u8 data[MAC_STORAGE - sizeof(void *)];
  u64 align[0];
};

/* Union to satisfy C aliasing rules */
union mac_context_union {
  struct mac_context mac;
  struct nrmh_context nrmh;
  struct hmac_context hmac;
};


struct mac_desc {
  const char *name;			/* Name of MAC algorithm */
  uint mac_length;			/* Length of authentication code */
  uint ctx_length;			/* Length of algorithm context */
  void (*init)(struct mac_context *ctx, const byte *key, uint keylen);
  void (*update)(struct mac_context *ctx, const byte *data, uint datalen);
  byte *(*final)(struct mac_context *ctx);

  uint hash_size;			/* Hash length, for hash-based MACs */
  uint block_size;			/* Hash block size, for hash-based MACs */
  void (*hash_init)(struct hash_context *ctx);
  void (*hash_update)(struct hash_context *ctx, const byte *data, uint datalen);
  byte *(*hash_final)(struct hash_context *ctx);
};

extern const struct mac_desc mac_table[ALG_MAX];

static inline const char *mac_type_name(uint id)
{ return mac_table[id].name; }

static inline uint mac_type_length(uint id)
{ return mac_table[id].mac_length; }

static inline const char *mac_get_name(struct mac_context *ctx)
{ return ctx->type->name; }

static inline uint mac_get_length(struct mac_context *ctx)
{ return ctx->type->mac_length; }

void mac_init(struct mac_context *ctx, uint id, const byte *key, uint keylen);

static inline void mac_update(struct mac_context *ctx, const byte *data, uint datalen)
{ ctx->type->update(ctx, data, datalen); }

static inline byte *mac_final(struct mac_context *ctx)
{ return ctx->type->final(ctx); }

static inline void mac_cleanup(struct mac_context *ctx)
{ memset(ctx, 0, ctx->type->ctx_length); }

void mac_fill(uint id, const byte *key, uint keylen, const byte *data, uint datalen, byte *mac);
int mac_verify(uint id, const byte *key, uint keylen, const byte *data, uint datalen, const byte *mac);


#endif /* _BIRD_MAC_H_ */
