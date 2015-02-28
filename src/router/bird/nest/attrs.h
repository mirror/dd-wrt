/*
 *	BIRD Internet Routing Daemon -- Attribute Operations
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_ATTRS_H_
#define _BIRD_ATTRS_H_

#include <stdint.h>

/* a-path.c */

#define AS_PATH_SET		1	/* Types of path segments */
#define AS_PATH_SEQUENCE	2
#define AS_PATH_CONFED_SEQUENCE	3
#define AS_PATH_CONFED_SET	4

#define AS_PATH_MAXLEN		10000

#define AS_TRANS		23456
/* AS_TRANS is used when we need to store 32bit ASN larger than 0xFFFF
 * to 16bit slot (like in 16bit AS_PATH). See RFC 4893 for details
 */

struct f_tree;

struct adata *as_path_prepend(struct linpool *pool, struct adata *olda, u32 as);
int as_path_convert_to_old(struct adata *path, byte *dst, int *new_used);
int as_path_convert_to_new(struct adata *path, byte *dst, int req_as);
void as_path_format(struct adata *path, byte *buf, unsigned int size);
int as_path_getlen(struct adata *path);
int as_path_getlen_int(struct adata *path, int bs);
int as_path_get_first(struct adata *path, u32 *orig_as);
int as_path_get_last(struct adata *path, u32 *last_as);
int as_path_contains(struct adata *path, u32 as, int min);
int as_path_match_set(struct adata *path, struct f_tree *set);
struct adata *as_path_filter(struct linpool *pool, struct adata *path, struct f_tree *set, u32 key, int pos);


#define PM_ASN		0
#define PM_QUESTION	1
#define PM_ASTERISK	2
#define PM_ASN_EXPR	3

struct f_path_mask {
  struct f_path_mask *next;
  int kind;
  uintptr_t val;
};

int as_path_match(struct adata *path, struct f_path_mask *mask);

/* a-set.c */


/* Extended Community subtypes (kinds) */
#define EC_RT 0x0002
#define EC_RO 0x0003

#define EC_GENERIC 0xFFFF

/* Transitive bit (for first u32 half of EC) */
#define EC_TBIT 0x40000000


static inline int int_set_get_size(struct adata *list)
{ return list->length / 4; }

static inline int ec_set_get_size(struct adata *list)
{ return list->length / 8; }

static inline u32 *int_set_get_data(struct adata *list)
{ return (u32 *) list->data; }

static inline u32 ec_hi(u64 ec) { return ec >> 32; }
static inline u32 ec_lo(u64 ec) { return ec; }
static inline u64 ec_get(const u32 *l, int i)
{ return (((u64) l[i]) << 32) | l[i+1]; }

/* RFC 4360 3.1.  Two-Octet AS Specific Extended Community */
static inline u64 ec_as2(u64 kind, u64 key, u64 val)
{ return ((kind | 0x0000) << 48) | (key << 32) | val; }

/* RFC 5668  4-Octet AS Specific BGP Extended Community */
static inline u64 ec_as4(u64 kind, u64 key, u64 val)
{ return ((kind | 0x0200) << 48) | (key << 16) | val; }

/* RFC 4360 3.2.  IPv4 Address Specific Extended Community */
static inline u64 ec_ip4(u64 kind, u64 key, u64 val)
{ return ((kind | 0x0100) << 48) | (key << 16) | val; }

static inline u64 ec_generic(u64 key, u64 val)
{ return (key << 32) | val; }

int int_set_format(struct adata *set, int way, int from, byte *buf, unsigned int size);
int ec_format(byte *buf, u64 ec);
int ec_set_format(struct adata *set, int from, byte *buf, unsigned int size);
int int_set_contains(struct adata *list, u32 val);
int ec_set_contains(struct adata *list, u64 val);
struct adata *int_set_add(struct linpool *pool, struct adata *list, u32 val);
struct adata *ec_set_add(struct linpool *pool, struct adata *list, u64 val);
struct adata *int_set_del(struct linpool *pool, struct adata *list, u32 val);
struct adata *ec_set_del(struct linpool *pool, struct adata *list, u64 val);
struct adata *int_set_union(struct linpool *pool, struct adata *l1, struct adata *l2);
struct adata *ec_set_union(struct linpool *pool, struct adata *l1, struct adata *l2);


#endif
