/*
 *	BIRD Internet Routing Daemon -- Attribute Operations
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_ATTRS_H_
#define _BIRD_ATTRS_H_

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

struct adata *as_path_prepend(struct linpool *pool, struct adata *olda, u32 as);
int as_path_convert_to_old(struct adata *path, byte *dst, int *new_used);
int as_path_convert_to_new(struct adata *path, byte *dst, int req_as);
void as_path_format(struct adata *path, byte *buf, unsigned int size);
int as_path_getlen(struct adata *path);
int as_path_getlen_int(struct adata *path, int bs);
int as_path_get_first(struct adata *path, u32 *orig_as);
int as_path_get_last(struct adata *path, u32 *last_as);
int as_path_is_member(struct adata *path, u32 as);

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

void int_set_format(struct adata *set, int way, byte *buf, unsigned int size);
struct adata *int_set_add(struct linpool *pool, struct adata *list, u32 val);
int int_set_contains(struct adata *list, u32 val);
struct adata *int_set_del(struct linpool *pool, struct adata *list, u32 val);

static inline int int_set_get_size(struct adata *list)
{ return list->length / 4; }

#endif
