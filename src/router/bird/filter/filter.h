/*
 *	BIRD Internet Routing Daemon -- Filters
 *
 *	(c) 1999 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_FILT_H_
#define _BIRD_FILT_H_

#include "lib/resource.h"
#include "lib/ip.h"
#include "nest/route.h"
#include "nest/attrs.h"

struct f_inst {		/* Instruction */
  struct f_inst *next;	/* Structure is 16 bytes, anyway */
  u16 code;
  u16 aux;
  union {
    int i;
    void *p;
  } a1;
  union {
    int i;
    void *p;
  } a2;
  int lineno;
};

#define arg1 a1.p
#define arg2 a2.p

/* Not enough fields in f_inst for three args used by roa_check() */
struct f_inst_roa_check {
  struct f_inst i;
  struct roa_table_config *rtc;	
};

struct f_prefix {
  ip_addr ip;
  int len;
#define LEN_MASK 0xff
#define LEN_PLUS  0x1000000
#define LEN_MINUS 0x2000000
#define LEN_RANGE 0x4000000
  /* If range then prefix must be in range (len >> 16 & 0xff, len >> 8 & 0xff) */
};

struct f_val {
  int type;
  union {
    uint i;
    u64 ec;
    /*    ip_addr ip; Folded into prefix */	
    struct f_prefix px;
    char *s;
    struct f_tree *t;
    struct f_trie *ti;
    struct adata *ad;
    struct f_path_mask *path_mask;
  } val;
};

struct filter {
  char *name;
  struct f_inst *root;
};

struct f_inst *f_new_inst(void);
struct f_inst *f_new_dynamic_attr(int type, int f_type, int code);	/* Type as core knows it, type as filters know it, and code of dynamic attribute */
struct f_tree *f_new_tree(void);
struct f_inst *f_generate_complex(int operation, int operation_aux, struct f_inst *dyn, struct f_inst *argument);
struct f_inst *f_generate_roa_check(struct symbol *sym, struct f_inst *prefix, struct f_inst *asn);


struct f_tree *build_tree(struct f_tree *);
struct f_tree *find_tree(struct f_tree *t, struct f_val val);
int same_tree(struct f_tree *t1, struct f_tree *t2);
void tree_format(struct f_tree *t, buffer *buf);

struct f_trie *f_new_trie(linpool *lp);
void trie_add_prefix(struct f_trie *t, ip_addr px, int plen, int l, int h);
int trie_match_prefix(struct f_trie *t, ip_addr px, int plen);
int trie_same(struct f_trie *t1, struct f_trie *t2);
void trie_format(struct f_trie *t, buffer *buf);

void fprefix_get_bounds(struct f_prefix *px, int *l, int *h);

static inline void
trie_add_fprefix(struct f_trie *t, struct f_prefix *px)
{
  int l, h;
  fprefix_get_bounds(px, &l, &h);
  trie_add_prefix(t, px->ip, px->len & LEN_MASK, l, h);
}

static inline int
trie_match_fprefix(struct f_trie *t, struct f_prefix *px)
{
  return trie_match_prefix(t, px->ip, px->len & LEN_MASK);
}


struct ea_list;
struct rte;

int f_run(struct filter *filter, struct rte **rte, struct ea_list **tmp_attrs, struct linpool *tmp_pool, int flags);
struct f_val f_eval(struct f_inst *expr, struct linpool *tmp_pool);
uint f_eval_int(struct f_inst *expr);
u32 f_eval_asn(struct f_inst *expr);

char *filter_name(struct filter *filter);
int filter_same(struct filter *new, struct filter *old);

int i_same(struct f_inst *f1, struct f_inst *f2);

int val_compare(struct f_val v1, struct f_val v2);
int val_same(struct f_val v1, struct f_val v2);

void val_format(struct f_val v, buffer *buf);


#define F_NOP 0
#define F_NONL 1
#define F_ACCEPT 2	/* Need to preserve ordering: accepts < rejects! */
#define F_REJECT 3
#define F_ERROR 4
#define F_QUITBIRD 5

#define FILTER_ACCEPT NULL
#define FILTER_REJECT ((void *) 1)

/* Type numbers must be in 0..0xff range */
#define T_MASK 0xff

/* Internal types */
/* Do not use type of zero, that way we'll see errors easier. */
#define T_VOID 1

/* User visible types, which fit in int */
#define T_INT 0x10
#define T_BOOL 0x11
#define T_PAIR 0x12  /*	Notice that pair is stored as integer: first << 16 | second */
#define T_QUAD 0x13

/* Put enumerational types in 0x30..0x3f range */
#define T_ENUM_LO 0x30
#define T_ENUM_HI 0x3f

#define T_ENUM_RTS 0x30
#define T_ENUM_BGP_ORIGIN 0x31
#define T_ENUM_SCOPE 0x32
#define T_ENUM_RTC 0x33
#define T_ENUM_RTD 0x34
#define T_ENUM_ROA 0x35
/* new enums go here */
#define T_ENUM_EMPTY 0x3f	/* Special hack for atomic_aggr */

#define T_ENUM T_ENUM_LO ... T_ENUM_HI

/* Bigger ones */
#define T_IP 0x20
#define T_PREFIX 0x21
#define T_STRING 0x22
#define T_PATH_MASK 0x23	/* mask for BGP path */
#define T_PATH 0x24		/* BGP path */
#define T_CLIST 0x25		/* Community list */
#define T_ECLIST 0x26		/* Extended community list */
#define T_EC 0x27		/* Extended community value, u64 */

#define T_RETURN 0x40
#define T_SET 0x80
#define T_PREFIX_SET 0x81


#define SA_FROM		 1    
#define SA_GW		 2      
#define SA_NET		 3     
#define SA_PROTO	 4   
#define SA_SOURCE	 5  
#define SA_SCOPE	 6   
#define SA_CAST    	 7
#define SA_DEST    	 8
#define SA_IFNAME  	 9
#define SA_IFINDEX    	10


struct f_tree {
  struct f_tree *left, *right;
  struct f_val from, to;
  void *data;
};

struct f_trie_node
{
  ip_addr addr, mask, accept;
  int plen;
  struct f_trie_node *c[2];
};

struct f_trie
{
  linpool *lp;
  int zero;
  struct f_trie_node root;
};

#define NEW_F_VAL struct f_val * val; val = cfg_alloc(sizeof(struct f_val));

#define FF_FORCE_TMPATTR 1		/* Force all attributes to be temporary */

#endif
