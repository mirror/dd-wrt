#ifndef _NFT_CACHE_H_
#define _NFT_CACHE_H_

#include <list.h>

struct handle;

enum cache_level_bits {
	NFT_CACHE_TABLE_BIT	= (1 << 0),
	NFT_CACHE_CHAIN_BIT	= (1 << 1),
	NFT_CACHE_SET_BIT	= (1 << 2),
	NFT_CACHE_FLOWTABLE_BIT	= (1 << 3),
	NFT_CACHE_OBJECT_BIT	= (1 << 4),
	NFT_CACHE_SETELEM_BIT	= (1 << 5),
	NFT_CACHE_RULE_BIT	= (1 << 6),
	__NFT_CACHE_MAX_BIT	= (1 << 7),
};

enum cache_level_flags {
	NFT_CACHE_EMPTY		= 0,
	NFT_CACHE_TABLE		= NFT_CACHE_TABLE_BIT,
	NFT_CACHE_CHAIN		= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_CHAIN_BIT,
	NFT_CACHE_SET		= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_SET_BIT,
	NFT_CACHE_FLOWTABLE	= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_FLOWTABLE_BIT,
	NFT_CACHE_OBJECT	= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_OBJECT_BIT,
	NFT_CACHE_SETELEM	= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_SET_BIT |
				  NFT_CACHE_SETELEM_BIT,
	NFT_CACHE_RULE		= NFT_CACHE_TABLE_BIT |
				  NFT_CACHE_RULE_BIT,
	NFT_CACHE_FULL		= __NFT_CACHE_MAX_BIT - 1,
	NFT_CACHE_TERSE		= (1 << 27),
	NFT_CACHE_SETELEM_MAYBE	= (1 << 28),
	NFT_CACHE_REFRESH	= (1 << 29),
	NFT_CACHE_UPDATE	= (1 << 30),
	NFT_CACHE_FLUSHED	= (1 << 31),
};

struct nft_filter_obj {
	struct list_head list;
	uint32_t	family;
	const char	*table;
	const char	*set;
};

#define NFT_CACHE_HSIZE	8192

struct nft_cache_filter {
	struct {
		uint32_t	family;
		const char	*table;
		const char	*chain;
		const char	*obj;
		const char	*set;
		const char	*ft;
		int		obj_type;
		uint64_t	rule_handle;
	} list;

	struct {
		struct list_head head;
	} obj[NFT_CACHE_HSIZE];

	struct {
		bool		obj;
		bool		rule;
		bool		elem;
	} reset;
};

struct nft_cache;
struct nft_ctx;
enum cmd_ops;

int nft_cache_evaluate(struct nft_ctx *nft, struct list_head *cmds,
		       struct list_head *msgs, struct nft_cache_filter *filter,
		       unsigned int *flags);
int nft_cache_update(struct nft_ctx *ctx, unsigned int flags,
		     struct list_head *msgs,
		     const struct nft_cache_filter *filter);
bool nft_cache_needs_update(struct nft_cache *cache);
void nft_cache_release(struct nft_cache *cache);

static inline uint32_t djb_hash(const char *key)
{
	uint32_t i, hash = 5381;

	for (i = 0; i < strlen(key); i++)
		hash = ((hash << 5) + hash) + key[i];

	return hash;
}

struct nft_cache_filter *nft_cache_filter_init(void);
void nft_cache_filter_fini(struct nft_cache_filter *filter);

struct table;
struct chain;

void chain_cache_add(struct chain *chain, struct table *table);
void chain_cache_del(struct chain *chain);
struct chain *chain_cache_find(const struct table *table, const char *name);

struct set;

void set_cache_add(struct set *set, struct table *table);
void set_cache_del(struct set *set);
struct set *set_cache_find(const struct table *table, const char *name);

struct cache {
	struct list_head	*ht;
	struct list_head	list;
};

struct cache_item {
	struct list_head	hlist;
	struct list_head	list;
};

void cache_init(struct cache *cache);
void cache_free(struct cache *cache);
void cache_add(struct cache_item *item, struct cache *cache, const char *name);
void cache_del(struct cache_item *item);

void table_cache_add(struct table *table, struct nft_cache *cache);
void table_cache_del(struct table *table);
struct table *table_cache_find(const struct cache *cache, const char *name,
			       uint32_t family);

struct obj;

void obj_cache_add(struct obj *obj, struct table *table);
void obj_cache_del(struct obj *obj);
struct obj *obj_cache_find(const struct table *table, const char *name,
			   uint32_t obj_type);

struct flowtable;
void ft_cache_add(struct flowtable *ft, struct table *table);
void ft_cache_del(struct flowtable *ft);
struct flowtable *ft_cache_find(const struct table *table, const char *name);

struct nft_cache {
	uint32_t		genid;
	struct cache		table_cache;
	uint32_t		seqnum;
	uint32_t		flags;
};

struct netlink_ctx;

void nft_chain_cache_update(struct netlink_ctx *ctx, struct table *table,
			    const char *chain);

#endif /* _NFT_CACHE_H_ */
