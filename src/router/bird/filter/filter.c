/*
 *	Filters: utility functions
 *
 *	Copyright 1998 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

/**
 * DOC: Filters
 *
 * You can find sources of the filter language in |filter/|
 * directory. File |filter/config.Y| contains filter grammar and basically translates
 * the source from user into a tree of &f_inst structures. These trees are
 * later interpreted using code in |filter/filter.c|.
 *
 * A filter is represented by a tree of &f_inst structures, one structure per
 * "instruction". Each &f_inst contains @code, @aux value which is
 * usually the data type this instruction operates on and two generic
 * arguments (@a1, @a2). Some instructions contain pointer(s) to other
 * instructions in their (@a1, @a2) fields.
 *
 * Filters use a &f_val structure for their data. Each &f_val
 * contains type and value (types are constants prefixed with %T_). Few
 * of the types are special; %T_RETURN can be or-ed with a type to indicate
 * that return from a function or from the whole filter should be
 * forced. Important thing about &f_val's is that they may be copied
 * with a simple |=|. That's fine for all currently defined types: strings
 * are read-only (and therefore okay), paths are copied for each
 * operation (okay too).
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/string.h"
#include "lib/unaligned.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/attrs.h"
#include "conf/conf.h"
#include "filter/filter.h"

#define P(a,b) ((a<<8) | b)

#define CMP_ERROR 999

static struct adata *
adata_empty(struct linpool *pool, int l)
{
  struct adata *res = lp_alloc(pool, sizeof(struct adata) + l);
  res->length = l;
  return res;
}

static int
pm_path_compare(struct f_path_mask *m1, struct f_path_mask *m2)
{
  while (1) {
    if ((!m1) || (!m2))
      return !((!m1) && (!m2));

    /* FIXME: buggy, should return -1, 0, 1; but it doesn't matter */
    if ((m1->kind != m2->kind) || (m1->val != m2->val)) return 1;
    m1 = m1->next;
    m2 = m2->next;
  }
}

u32 f_eval_asn(struct f_inst *expr);

static void
pm_format(struct f_path_mask *p, byte *buf, unsigned int size)
{
  byte *end = buf + size - 16;

  while (p)
    {
      if (buf > end)
	{
	  strcpy(buf, " ...");
	  return;
	}

      switch(p->kind)
	{
	case PM_ASN:
	  buf += bsprintf(buf, " %u", p->val);
	  break;

	case PM_QUESTION:
	  buf += bsprintf(buf, " ?");
	  break;

	case PM_ASTERISK:
	  buf += bsprintf(buf, " *");
	  break;

	case PM_ASN_EXPR:
	  buf += bsprintf(buf, " %u", f_eval_asn((struct f_inst *) p->val));
	  break;
	}

      p = p->next;
    }

  *buf = 0;
}

static inline int int_cmp(int i1, int i2)
{
  if (i1 == i2) return 0;
  if (i1 < i2) return -1;
  else return 1;
}

static inline int uint_cmp(unsigned int i1, unsigned int i2)
{
  if (i1 == i2) return 0;
  if (i1 < i2) return -1;
  else return 1;
}

static inline int u64_cmp(u64 i1, u64 i2)
{
  if (i1 == i2) return 0;
  if (i1 < i2) return -1;
  else return 1;
}

/**
 * val_compare - compare two values
 * @v1: first value
 * @v2: second value
 *
 * Compares two values and returns -1, 0, 1 on <, =, > or 999 on error.
 * Tree module relies on this giving consistent results so that it can
 * build balanced trees.
 */
int
val_compare(struct f_val v1, struct f_val v2)
{
  int rc;

  if ((v1.type == T_VOID) && (v2.type == T_VOID))
    return 0;
  if (v1.type == T_VOID)	/* Hack for else */
    return -1;
  if (v2.type == T_VOID)
    return 1;

  if (v1.type != v2.type) {
#ifndef IPV6
    /* IP->Quad implicit conversion */
    if ((v1.type == T_QUAD) && (v2.type == T_IP))
      return uint_cmp(v1.val.i, ipa_to_u32(v2.val.px.ip));
    if ((v1.type == T_IP) && (v2.type == T_QUAD))
      return uint_cmp(ipa_to_u32(v1.val.px.ip), v2.val.i);
#endif

    debug( "Types do not match in val_compare\n" );
    return CMP_ERROR;
  }
  switch (v1.type) {
  case T_ENUM:
  case T_INT:
  case T_BOOL:
    return int_cmp(v1.val.i, v2.val.i);
  case T_PAIR:
  case T_QUAD:
    return uint_cmp(v1.val.i, v2.val.i);
  case T_EC:
    return u64_cmp(v1.val.ec, v2.val.ec);
  case T_IP:
    return ipa_compare(v1.val.px.ip, v2.val.px.ip);
  case T_PREFIX:
    if (rc = ipa_compare(v1.val.px.ip, v2.val.px.ip))
      return rc;
    if (v1.val.px.len < v2.val.px.len)
      return -1;
    if (v1.val.px.len > v2.val.px.len)
      return 1;
    return 0;
  case T_PATH_MASK:
    return pm_path_compare(v1.val.path_mask, v2.val.path_mask);
  case T_STRING:
    return strcmp(v1.val.s, v2.val.s);
  default:
    debug( "Compare of unknown entities: %x\n", v1.type );
    return CMP_ERROR;
  }
}

int 
tree_compare(const void *p1, const void *p2)
{
  return val_compare((* (struct f_tree **) p1)->from, (* (struct f_tree **) p2)->from);
}

void
fprefix_get_bounds(struct f_prefix *px, int *l, int *h)
{
  *l = *h = px->len & LEN_MASK;

  if (px->len & LEN_MINUS)
    *l = 0;

  else if (px->len & LEN_PLUS)
    *h = MAX_PREFIX_LENGTH;

  else if (px->len & LEN_RANGE)
    {
      *l = 0xff & (px->len >> 16);
      *h = 0xff & (px->len >> 8);
    }
}

/*
 * val_simple_in_range - check if @v1 ~ @v2 for everything except sets
 */ 
static int
val_simple_in_range(struct f_val v1, struct f_val v2)
{
  if ((v1.type == T_PATH) && (v2.type == T_PATH_MASK))
    return as_path_match(v1.val.ad, v2.val.path_mask);
  if ((v1.type == T_INT) && (v2.type == T_PATH))
    return as_path_is_member(v2.val.ad, v1.val.i);

  if (((v1.type == T_PAIR) || (v1.type == T_QUAD)) && (v2.type == T_CLIST))
    return int_set_contains(v2.val.ad, v1.val.i);
#ifndef IPV6
  /* IP->Quad implicit conversion */
  if ((v1.type == T_IP) && (v2.type == T_CLIST))
    return int_set_contains(v2.val.ad, ipa_to_u32(v1.val.px.ip));
#endif
  if ((v1.type == T_EC) && (v2.type == T_ECLIST))
    return ec_set_contains(v2.val.ad, v1.val.ec);

  if ((v1.type == T_STRING) && (v2.type == T_STRING))
    return patmatch(v2.val.s, v1.val.s);

  if ((v1.type == T_IP) && (v2.type == T_PREFIX))
    return ipa_in_net(v1.val.px.ip, v2.val.px.ip, v2.val.px.len);

  if ((v1.type == T_PREFIX) && (v2.type == T_PREFIX))
    return net_in_net(v1.val.px.ip, v1.val.px.len, v2.val.px.ip, v2.val.px.len);

  return CMP_ERROR;
}

static int
clist_set_type(struct f_tree *set, struct f_val *v)
{
 switch (set->from.type) {
  case T_PAIR:
    v->type = T_PAIR;
    return 1;
  case T_QUAD:
#ifndef IPV6
  case T_IP:
#endif
    v->type = T_QUAD;
    return 1;
    break;
  default:
    v->type = T_VOID;
    return 0;
  }
}

static inline int
eclist_set_type(struct f_tree *set)
{ return set->from.type == T_EC; }

static int
clist_match_set(struct adata *clist, struct f_tree *set)
{
  if (!clist)
    return 0;

  struct f_val v;
  if (!clist_set_type(set, &v))
    return CMP_ERROR;

  u32 *l = (u32 *) clist->data;
  u32 *end = l + clist->length/4;

  while (l < end) {
    v.val.i = *l++;
    if (find_tree(set, v))
      return 1;
  }
  return 0;
}

static int
eclist_match_set(struct adata *list, struct f_tree *set)
{
  if (!list)
    return 0;

  if (!eclist_set_type(set))
    return CMP_ERROR;

  struct f_val v;
  u32 *l = int_set_get_data(list);
  int len = int_set_get_size(list);
  int i;

  v.type = T_EC;
  for (i = 0; i < len; i += 2) {
    v.val.ec = ec_get(l, i);
    if (find_tree(set, v))
      return 1;
  }

  return 0;
}

static struct adata *
clist_filter(struct linpool *pool, struct adata *list, struct f_val set, int pos)
{
  if (!list)
    return NULL;

  int tree = (set.type == T_SET);	/* 1 -> set is T_SET, 0 -> set is T_CLIST */
  struct f_val v;
  if (tree)
    clist_set_type(set.val.t, &v);
  else
    v.type = T_PAIR;

  int len = int_set_get_size(list);
  u32 *l = int_set_get_data(list);
  u32 tmp[len];
  u32 *k = tmp;
  u32 *end = l + len;

  while (l < end) {
    v.val.i = *l++;
    /* pos && member(val, set) || !pos && !member(val, set),  member() depends on tree */
    if ((tree ? !!find_tree(set.val.t, v) : int_set_contains(set.val.ad, v.val.i)) == pos)
      *k++ = v.val.i;
  }

  int nl = (k - tmp) * 4;
  if (nl == list->length)
    return list;

  struct adata *res = adata_empty(pool, nl);
  memcpy(res->data, tmp, nl);
  return res;
}

static struct adata *
eclist_filter(struct linpool *pool, struct adata *list, struct f_val set, int pos)
{
  if (!list)
    return NULL;

  int tree = (set.type == T_SET);	/* 1 -> set is T_SET, 0 -> set is T_CLIST */
  struct f_val v;

  int len = int_set_get_size(list);
  u32 *l = int_set_get_data(list);
  u32 tmp[len];
  u32 *k = tmp;
  int i;

  v.type = T_EC;
  for (i = 0; i < len; i += 2) {
    v.val.ec = ec_get(l, i);
    /* pos && member(val, set) || !pos && !member(val, set),  member() depends on tree */
    if ((tree ? !!find_tree(set.val.t, v) : ec_set_contains(set.val.ad, v.val.ec)) == pos) {
      *k++ = l[i];
      *k++ = l[i+1];
    }
  }

  int nl = (k - tmp) * 4;
  if (nl == list->length)
    return list;

  struct adata *res = adata_empty(pool, nl);
  memcpy(res->data, tmp, nl);
  return res;
}

/**
 * val_in_range - implement |~| operator
 * @v1: element
 * @v2: set
 *
 * Checks if @v1 is element (|~| operator) of @v2. Sets are internally represented as balanced trees, see
 * |tree.c| module (this is not limited to sets, but for non-set cases, val_simple_in_range() is called early).
 */
static int
val_in_range(struct f_val v1, struct f_val v2)
{
  int res;

  res = val_simple_in_range(v1, v2);

  if (res != CMP_ERROR)
    return res;
  
  if ((v1.type == T_PREFIX) && (v2.type == T_PREFIX_SET))
    return trie_match_fprefix(v2.val.ti, &v1.val.px);

  if ((v1.type == T_CLIST) && (v2.type == T_SET))
    return clist_match_set(v1.val.ad, v2.val.t);

  if ((v1.type == T_ECLIST) && (v2.type == T_SET))
    return eclist_match_set(v1.val.ad, v2.val.t);

  if (v2.type == T_SET)
    switch (v1.type) {
    case T_ENUM:
    case T_INT:
    case T_PAIR:
    case T_QUAD:
    case T_IP:
    case T_EC:
      {
	struct f_tree *n;
	n = find_tree(v2.val.t, v1);
	if (!n)
	  return 0;
	return !! (val_simple_in_range(v1, n->from));	/* We turn CMP_ERROR into compared ok, and that's fine */
      }
    }
  return CMP_ERROR;
}

static void val_print(struct f_val v);

static void
tree_node_print(struct f_tree *t, char **sep)
{
  if (t == NULL)
    return;

  tree_node_print(t->left, sep);

  logn(*sep);
  val_print(t->from);
  if (val_compare(t->from, t->to) != 0)
    {
      logn( ".." );
      val_print(t->to);
    }
  *sep = ", ";

  tree_node_print(t->right, sep);
}

static void
tree_print(struct f_tree *t)
{
  char *sep = "";
  logn( "[" );
  tree_node_print(t, &sep);
  logn( "] " );
}

/*
 * val_print - format filter value
 */
static void
val_print(struct f_val v)
{
  char buf2[1024];
  switch (v.type) {
  case T_VOID: logn("(void)"); return;
  case T_BOOL: logn(v.val.i ? "TRUE" : "FALSE"); return;
  case T_INT: logn("%d", v.val.i); return;
  case T_STRING: logn("%s", v.val.s); return;
  case T_IP: logn("%I", v.val.px.ip); return;
  case T_PREFIX: logn("%I/%d", v.val.px.ip, v.val.px.len); return;
  case T_PAIR: logn("(%d,%d)", v.val.i >> 16, v.val.i & 0xffff); return;
  case T_QUAD: logn("%R", v.val.i); return;
  case T_EC: ec_format(buf2, v.val.ec); logn("%s", buf2); return;
  case T_PREFIX_SET: trie_print(v.val.ti); return;
  case T_SET: tree_print(v.val.t); return;
  case T_ENUM: logn("(enum %x)%d", v.type, v.val.i); return;
  case T_PATH: as_path_format(v.val.ad, buf2, 1000); logn("(path %s)", buf2); return;
  case T_CLIST: int_set_format(v.val.ad, 1, -1, buf2, 1000); logn("(clist %s)", buf2); return;
  case T_ECLIST: ec_set_format(v.val.ad, -1, buf2, 1000); logn("(eclist %s)", buf2); return;
  case T_PATH_MASK: pm_format(v.val.path_mask, buf2, 1000); logn("(pathmask%s)", buf2); return;
  default: logn( "[unknown type %x]", v.type ); return;
  }
}

static struct rte **f_rte;
static struct rta *f_old_rta;
static struct ea_list **f_tmp_attrs;
static struct linpool *f_pool;
static int f_flags;

static inline void f_rte_cow(void)
{
  *f_rte = rte_cow(*f_rte); 
}

/*
 * rta_cow - prepare rta for modification by filter
 */
static void
f_rta_cow(void)
{
  if ((*f_rte)->attrs->aflags & RTAF_CACHED) {

    /* Prepare to modify rte */
    f_rte_cow();

    /* Store old rta to free it later */
    f_old_rta = (*f_rte)->attrs;

    /* 
     * Alloc new rta, do shallow copy and update rte. Fields eattrs
     * and nexthops of rta are shared with f_old_rta (they will be
     * copied when the cached rta will be obtained at the end of
     * f_run()), also the lock of hostentry is inherited (we suppose
     * hostentry is not changed by filters).
     */
    rta *ra = lp_alloc(f_pool, sizeof(rta));
    memcpy(ra, f_old_rta, sizeof(rta));
    ra->aflags = 0;
    (*f_rte)->attrs = ra;
  }
}

static struct rate_limit rl_runtime_err;

#define runtime(x) do { \
    log_rl(&rl_runtime_err, L_ERR "filters, line %d: %s", what->lineno, x); \
    res.type = T_RETURN; \
    res.val.i = F_ERROR; \
    return res; \
  } while(0)

#define ARG(x,y) \
	x = interpret(what->y); \
	if (x.type & T_RETURN) \
		return x;

#define ONEARG ARG(v1, a1.p)
#define TWOARGS ARG(v1, a1.p) \
		ARG(v2, a2.p)
#define TWOARGS_C TWOARGS \
                  if (v1.type != v2.type) \
		    runtime( "Can't operate with values of incompatible types" );

/**
 * interpret
 * @what: filter to interpret
 *
 * Interpret given tree of filter instructions. This is core function
 * of filter system and does all the hard work.
 *
 * Each instruction has 4 fields: code (which is instruction code),
 * aux (which is extension to instruction code, typically type),
 * arg1 and arg2 - arguments. Depending on instruction, arguments
 * are either integers, or pointers to instruction trees. Common 
 * instructions like +, that have two expressions as arguments use
 * TWOARGS macro to get both of them evaluated.
 *
 * &f_val structures are copied around, so there are no problems with
 * memory managment.
 */
static struct f_val
interpret(struct f_inst *what)
{
  struct symbol *sym;
  struct f_val v1, v2, res, *vp;
  unsigned u1, u2;
  int i;
  u32 as;

  res.type = T_VOID;
  if (!what)
    return res;

  switch(what->code) {
  case ',':
    TWOARGS;
    break;

/* Binary operators */
  case '+':
    TWOARGS_C;
    switch (res.type = v1.type) {
    case T_VOID: runtime( "Can't operate with values of type void" );
    case T_INT: res.val.i = v1.val.i + v2.val.i; break;
    default: runtime( "Usage of unknown type" );
    }
    break;
  case '-':
    TWOARGS_C;
    switch (res.type = v1.type) {
    case T_VOID: runtime( "Can't operate with values of type void" );
    case T_INT: res.val.i = v1.val.i - v2.val.i; break;
    default: runtime( "Usage of unknown type" );
    }
    break;
  case '*':
    TWOARGS_C;
    switch (res.type = v1.type) {
    case T_VOID: runtime( "Can't operate with values of type void" );
    case T_INT: res.val.i = v1.val.i * v2.val.i; break;
    default: runtime( "Usage of unknown type" );
    }
    break;
  case '/':
    TWOARGS_C;
    switch (res.type = v1.type) {
    case T_VOID: runtime( "Can't operate with values of type void" );
    case T_INT: if (v2.val.i == 0) runtime( "Mother told me not to divide by 0" );
      	        res.val.i = v1.val.i / v2.val.i; break;
    default: runtime( "Usage of unknown type" );
    }
    break;
    
  case '&':
  case '|':
    ARG(v1, a1.p);
    if (v1.type != T_BOOL)
      runtime( "Can't do boolean operation on non-booleans" );
    if (v1.val.i == (what->code == '|')) {
      res.type = T_BOOL;
      res.val.i = v1.val.i;
      break;
    }

    ARG(v2, a2.p);
    if (v2.type != T_BOOL)
      runtime( "Can't do boolean operation on non-booleans" );
    res.type = T_BOOL;
    res.val.i = v2.val.i;
    break;

  case P('m','p'):
    TWOARGS;
    if ((v1.type != T_INT) || (v2.type != T_INT))
      runtime( "Can't operate with value of non-integer type in pair constructor" );
    u1 = v1.val.i;
    u2 = v2.val.i;
    if ((u1 > 0xFFFF) || (u2 > 0xFFFF))
      runtime( "Can't operate with value out of bounds in pair constructor" );
    res.val.i = (u1 << 16) | u2;
    res.type = T_PAIR;
    break;

  case P('m','c'):
    {
      TWOARGS;

      int check, ipv4_used;
      u32 key, val;

      if (v1.type == T_INT) {
	ipv4_used = 0; key = v1.val.i;
      } 
      else if (v1.type == T_QUAD) {
	ipv4_used = 1; key = v1.val.i;
      }
#ifndef IPV6
      /* IP->Quad implicit conversion */
      else if (v1.type == T_IP) {
	ipv4_used = 1; key = ipa_to_u32(v1.val.px.ip);
      }
#endif
      else
	runtime("Can't operate with key of non-integer/IPv4 type in EC constructor");

      if (v2.type != T_INT)
	runtime("Can't operate with value of non-integer type in EC constructor");
      val = v2.val.i;

      res.type = T_EC;

      if (what->aux == EC_GENERIC) {
	check = 0; res.val.ec = ec_generic(key, val);
      }
      else if (ipv4_used) {
	check = 1; res.val.ec = ec_ip4(what->aux, key, val);
      }
      else if (key < 0x10000) {
	check = 0; res.val.ec = ec_as2(what->aux, key, val);
      }
      else {
	check = 1; res.val.ec = ec_as4(what->aux, key, val);
      }

      if (check && (val > 0xFFFF))
	runtime("Can't operate with value out of bounds in EC constructor");

      break;
    }

/* Relational operators */

#define COMPARE(x) \
    TWOARGS; \
    i = val_compare(v1, v2); \
    if (i==CMP_ERROR) \
      runtime( "Can't compare values of incompatible types" ); \
    res.type = T_BOOL; \
    res.val.i = (x); \
    break;

  case P('!','='): COMPARE(i!=0);
  case P('=','='): COMPARE(i==0);
  case '<': COMPARE(i==-1);
  case P('<','='): COMPARE(i!=1);

  case '!':
    ONEARG;
    if (v1.type != T_BOOL)
      runtime( "Not applied to non-boolean" );
    res = v1;
    res.val.i = !res.val.i;
    break;

  case '~':
    TWOARGS;
    res.type = T_BOOL;
    res.val.i = val_in_range(v1, v2);
    if (res.val.i == CMP_ERROR)
      runtime( "~ applied on unknown type pair" );
    res.val.i = !!res.val.i;
    break;
  case P('d','e'):
    ONEARG;
    res.type = T_BOOL;
    res.val.i = (v1.type != T_VOID);
    break;

  /* Set to indirect value, a1 = variable, a2 = value */
  case 's':
    ARG(v2, a2.p);
    sym = what->a1.p;
    vp = sym->def;
    if ((sym->class != (SYM_VARIABLE | v2.type)) && (v2.type != T_VOID)) {
#ifndef IPV6
      /* IP->Quad implicit conversion */
      if ((sym->class == (SYM_VARIABLE | T_QUAD)) && (v2.type == T_IP)) {
	vp->type = T_QUAD;
	vp->val.i = ipa_to_u32(v2.val.px.ip);
	break;
      }
#endif
      runtime( "Assigning to variable of incompatible type" );
    }
    *vp = v2; 
    break;

    /* some constants have value in a2, some in *a1.p, strange. */
  case 'c':	/* integer (or simple type) constant, string, set, or prefix_set */
    res.type = what->aux;

    if (res.type == T_PREFIX_SET)
      res.val.ti = what->a2.p;
    else if (res.type == T_SET)
      res.val.t = what->a2.p;
    else if (res.type == T_STRING)
      res.val.s = what->a2.p;
    else
      res.val.i = what->a2.i;
    break;
  case 'V':
  case 'C':
    res = * ((struct f_val *) what->a1.p);
    break;
  case 'p':
    ONEARG;
    val_print(v1);
    break;
  case '?':	/* ? has really strange error value, so we can implement if ... else nicely :-) */
    ONEARG;
    if (v1.type != T_BOOL)
      runtime( "If requires boolean expression" );
    if (v1.val.i) {
      ARG(res,a2.p);
      res.val.i = 0;
    } else res.val.i = 1;
    res.type = T_BOOL;
    break;
  case '0':
    debug( "No operation\n" );
    break;
  case P('p',','):
    ONEARG;
    if (what->a2.i == F_NOP || (what->a2.i != F_NONL && what->a1.p))
      log_commit(*L_INFO);

    switch (what->a2.i) {
    case F_QUITBIRD:
      die( "Filter asked me to die" );
    case F_ACCEPT:
      /* Should take care about turning ACCEPT into MODIFY */
    case F_ERROR:
    case F_REJECT:	/* FIXME (noncritical) Should print complete route along with reason to reject route */
      res.type = T_RETURN;
      res.val.i = what->a2.i;
      return res;	/* We have to return now, no more processing. */
    case F_NONL:
    case F_NOP:
      break;
    default:
      bug( "unknown return type: Can't happen");
    }
    break;
  case 'a':	/* rta access */
    {
      struct rta *rta = (*f_rte)->attrs;
      res.type = what->aux;
      switch(res.type) {
      case T_IP:
	res.val.px.ip = * (ip_addr *) ((char *) rta + what->a2.i);
	break;
      case T_ENUM:
	res.val.i = * ((char *) rta + what->a2.i);
	break;
      case T_STRING:	/* Warning: this is a special case for proto attribute */
	res.val.s = rta->proto->name;
	break;
      case T_PREFIX:	/* Warning: this works only for prefix of network */
	{
	  res.val.px.ip = (*f_rte)->net->n.prefix;
	  res.val.px.len = (*f_rte)->net->n.pxlen;
	  break;
	}
      default:
	bug( "Invalid type for rta access (%x)", res.type );
      }
    }
    break;
  case P('a','S'):
    ONEARG;
    if (what->aux != v1.type)
      runtime( "Attempt to set static attribute to incompatible type" );
    f_rta_cow();
    {
      struct rta *rta = (*f_rte)->attrs;
      switch (what->aux) {

      case T_IP:
	* (ip_addr *) ((char *) rta + what->a2.i) = v1.val.px.ip;
	break;

      case T_ENUM_SCOPE:
	rta->scope = v1.val.i;
	break;

      case T_ENUM_RTD:
	i = v1.val.i;
	if ((i != RTD_BLACKHOLE) && (i != RTD_UNREACHABLE) && (i != RTD_PROHIBIT))
	  runtime( "Destination can be changed only to blackhole, unreachable or prohibit" );
	rta->dest = i;
	rta->gw = IPA_NONE;
	rta->iface = NULL;
	rta->nexthops = NULL;
	break;

      default:
	bug( "Unknown type in set of static attribute" );
      }
    }
    break;
  case P('e','a'):	/* Access to extended attributes */
    {
      eattr *e = NULL;
      if (!(f_flags & FF_FORCE_TMPATTR))
	e = ea_find( (*f_rte)->attrs->eattrs, what->a2.i );
      if (!e) 
	e = ea_find( (*f_tmp_attrs), what->a2.i );
      if ((!e) && (f_flags & FF_FORCE_TMPATTR))
	e = ea_find( (*f_rte)->attrs->eattrs, what->a2.i );

      if (!e) {
	/* A special case: undefined int_set looks like empty int_set */
	if ((what->aux & EAF_TYPE_MASK) == EAF_TYPE_INT_SET) {
	  res.type = T_CLIST;
	  res.val.ad = adata_empty(f_pool, 0);
	  break;
	}
	/* The same special case for ec_set */
	else if ((what->aux & EAF_TYPE_MASK) == EAF_TYPE_EC_SET) {
	  res.type = T_ECLIST;
	  res.val.ad = adata_empty(f_pool, 0);
	  break;
	}

	/* Undefined value */
	res.type = T_VOID;
	break;
      }

      switch (what->aux & EAF_TYPE_MASK) {
      case EAF_TYPE_INT:
	res.type = T_INT;
	res.val.i = e->u.data;
	break;
      case EAF_TYPE_ROUTER_ID:
	res.type = T_QUAD;
	res.val.i = e->u.data;
	break;
      case EAF_TYPE_OPAQUE:
	res.type = T_ENUM_EMPTY;
	res.val.i = 0;
	break;
      case EAF_TYPE_IP_ADDRESS:
	res.type = T_IP;
	struct adata * ad = e->u.ptr;
	res.val.px.ip = * (ip_addr *) ad->data;
	break;
      case EAF_TYPE_AS_PATH:
        res.type = T_PATH;
	res.val.ad = e->u.ptr;
	break;
      case EAF_TYPE_INT_SET:
	res.type = T_CLIST;
	res.val.ad = e->u.ptr;
	break;
      case EAF_TYPE_EC_SET:
	res.type = T_ECLIST;
	res.val.ad = e->u.ptr;
	break;
      case EAF_TYPE_UNDEF:
	res.type = T_VOID;
	break;
      default:
	bug("Unknown type in e,a");
      }
    }
    break;
  case P('e','S'):
    ONEARG;
    {
      struct ea_list *l = lp_alloc(f_pool, sizeof(struct ea_list) + sizeof(eattr));

      l->next = NULL;
      l->flags = EALF_SORTED;
      l->count = 1;
      l->attrs[0].id = what->a2.i;
      l->attrs[0].flags = 0;
      l->attrs[0].type = what->aux | EAF_ORIGINATED;
      switch (what->aux & EAF_TYPE_MASK) {
      case EAF_TYPE_INT:
	if (v1.type != T_INT)
	  runtime( "Setting int attribute to non-int value" );
	l->attrs[0].u.data = v1.val.i;
	break;

      case EAF_TYPE_ROUTER_ID:
#ifndef IPV6
	/* IP->Quad implicit conversion */
	if (v1.type == T_IP) {
	  l->attrs[0].u.data = ipa_to_u32(v1.val.px.ip);
	  break;
	}
#endif
	/* T_INT for backward compatibility */
	if ((v1.type != T_QUAD) && (v1.type != T_INT))
	  runtime( "Setting quad attribute to non-quad value" );
	l->attrs[0].u.data = v1.val.i;
	break;

      case EAF_TYPE_OPAQUE:
	runtime( "Setting opaque attribute is not allowed" );
	break;
      case EAF_TYPE_IP_ADDRESS:
	if (v1.type != T_IP)
	  runtime( "Setting ip attribute to non-ip value" );
	int len = sizeof(ip_addr);
	struct adata *ad = lp_alloc(f_pool, sizeof(struct adata) + len);
	ad->length = len;
	(* (ip_addr *) ad->data) = v1.val.px.ip;
	l->attrs[0].u.ptr = ad;
	break;
      case EAF_TYPE_AS_PATH:
	if (v1.type != T_PATH)
	  runtime( "Setting path attribute to non-path value" );
	l->attrs[0].u.ptr = v1.val.ad;
	break;
      case EAF_TYPE_INT_SET:
	if (v1.type != T_CLIST)
	  runtime( "Setting clist attribute to non-clist value" );
	l->attrs[0].u.ptr = v1.val.ad;
	break;
      case EAF_TYPE_EC_SET:
	if (v1.type != T_ECLIST)
	  runtime( "Setting eclist attribute to non-eclist value" );
	l->attrs[0].u.ptr = v1.val.ad;
	break;
      case EAF_TYPE_UNDEF:
	if (v1.type != T_VOID)
	  runtime( "Setting void attribute to non-void value" );
	l->attrs[0].u.data = 0;
	break;
      default: bug("Unknown type in e,S");
      }

      if (!(what->aux & EAF_TEMP) && (!(f_flags & FF_FORCE_TMPATTR))) {
	f_rta_cow();
	l->next = (*f_rte)->attrs->eattrs;
	(*f_rte)->attrs->eattrs = l;
      } else {
	l->next = (*f_tmp_attrs);
	(*f_tmp_attrs) = l;
      }
    }
    break;
  case 'P':
    res.type = T_INT;
    res.val.i = (*f_rte)->pref;
    break;
  case P('P','S'):
    ONEARG;
    if (v1.type != T_INT)
      runtime( "Can't set preference to non-integer" );
    if ((v1.val.i < 0) || (v1.val.i > 0xFFFF))
      runtime( "Setting preference value out of bounds" );
    f_rte_cow();
    (*f_rte)->pref = v1.val.i;
    break;
  case 'L':	/* Get length of */
    ONEARG;
    res.type = T_INT;
    switch(v1.type) {
    case T_PREFIX: res.val.i = v1.val.px.len; break;
    case T_PATH:   res.val.i = as_path_getlen(v1.val.ad); break;
    default: runtime( "Prefix or path expected" );
    }
    break;
  case P('c','p'):	/* Convert prefix to ... */
    ONEARG;
    if (v1.type != T_PREFIX)
      runtime( "Prefix expected" );
    res.type = what->aux;
    switch(res.type) {
      /*    case T_INT:	res.val.i = v1.val.px.len; break; Not needed any more */
    case T_IP: res.val.px.ip = v1.val.px.ip; break;
    default: bug( "Unknown prefix to conversion" );
    }
    break;
  case P('a','f'):	/* Get first ASN from AS PATH */
    ONEARG;
    if (v1.type != T_PATH)
      runtime( "AS path expected" );

    as = 0;
    as_path_get_first(v1.val.ad, &as);
    res.type = T_INT;
    res.val.i = as;
    break;
  case P('a','l'):	/* Get last ASN from AS PATH */
    ONEARG;
    if (v1.type != T_PATH)
      runtime( "AS path expected" );

    as = 0;
    as_path_get_last(v1.val.ad, &as);
    res.type = T_INT;
    res.val.i = as;
    break;
  case 'r':
    ONEARG;
    res = v1;
    res.type |= T_RETURN;
    return res;
  case P('c','a'): /* CALL: this is special: if T_RETURN and returning some value, mask it out  */
    ONEARG;
    res = interpret(what->a2.p);
    if (res.type == T_RETURN)
      return res;
    res.type &= ~T_RETURN;    
    break;
  case P('c','v'):	/* Clear local variables */
    for (sym = what->a1.p; sym != NULL; sym = sym->aux2)
      ((struct f_val *) sym->def)->type = T_VOID;
    break;
  case P('S','W'):
    ONEARG;
    {
      struct f_tree *t = find_tree(what->a2.p, v1);
      if (!t) {
	v1.type = T_VOID;
	t = find_tree(what->a2.p, v1);
	if (!t) {
	  debug( "No else statement?\n");
	  break;
	}
      }	
      /* It is actually possible to have t->data NULL */

      res = interpret(t->data);
      if (res.type & T_RETURN)
	return res;
    }
    break;
  case P('i','M'): /* IP.MASK(val) */
    TWOARGS;
    if (v2.type != T_INT)
      runtime( "Integer expected");
    if (v1.type != T_IP)
      runtime( "You can mask only IP addresses" );
    {
      ip_addr mask = ipa_mkmask(v2.val.i);
      res.type = T_IP;
      res.val.px.ip = ipa_and(mask, v1.val.px.ip);
    }
    break;

  case 'E':	/* Create empty attribute */
    res.type = what->aux;
    res.val.ad = adata_empty(f_pool, 0);
    break;
  case P('A','p'):	/* Path prepend */
    TWOARGS;
    if (v1.type != T_PATH)
      runtime("Can't prepend to non-path");
    if (v2.type != T_INT)
      runtime("Can't prepend non-integer");

    res.type = T_PATH;
    res.val.ad = as_path_prepend(f_pool, v1.val.ad, v2.val.i);
    break;

  case P('C','a'):	/* (Extended) Community list add or delete */
    TWOARGS;
    if (v1.type == T_CLIST)
    {
      /* Community (or cluster) list */
      struct f_val dummy;
      int arg_set = 0;
      i = 0;

      if ((v2.type == T_PAIR) || (v2.type == T_QUAD))
	i = v2.val.i;
#ifndef IPV6
      /* IP->Quad implicit conversion */
      else if (v2.type == T_IP)
	i = ipa_to_u32(v2.val.px.ip);
#endif
      else if ((v2.type == T_SET) && clist_set_type(v2.val.t, &dummy))
	arg_set = 1;
      else if (v2.type == T_CLIST)
	arg_set = 2;
      else
	runtime("Can't add/delete non-pair");

      res.type = T_CLIST;
      switch (what->aux)
      {
      case 'a':
	if (arg_set == 1)
	  runtime("Can't add set");
	else if (!arg_set)
	  res.val.ad = int_set_add(f_pool, v1.val.ad, i);
	else 
	  res.val.ad = int_set_union(f_pool, v1.val.ad, v2.val.ad);
	break;
      
      case 'd':
	if (!arg_set)
	  res.val.ad = int_set_del(f_pool, v1.val.ad, i);
	else
	  res.val.ad = clist_filter(f_pool, v1.val.ad, v2, 0);
	break;

      case 'f':
	if (!arg_set)
	  runtime("Can't filter pair");
	res.val.ad = clist_filter(f_pool, v1.val.ad, v2, 1);
	break;

      default:
	bug("unknown Ca operation");
      }
    }
    else if (v1.type == T_ECLIST)
    {
      /* Extended community list */
      int arg_set = 0;
      
      /* v2.val is either EC or EC-set */
      if ((v2.type == T_SET) && eclist_set_type(v2.val.t))
	arg_set = 1;
      else if (v2.type == T_ECLIST)
	arg_set = 2;
      else if (v2.type != T_EC)
	runtime("Can't add/delete non-pair");

      res.type = T_ECLIST;
      switch (what->aux)
      {
      case 'a':
	if (arg_set == 1)
	  runtime("Can't add set");
	else if (!arg_set)
	  res.val.ad = ec_set_add(f_pool, v1.val.ad, v2.val.ec);
	else 
	  res.val.ad = ec_set_union(f_pool, v1.val.ad, v2.val.ad);
	break;
      
      case 'd':
	if (!arg_set)
	  res.val.ad = ec_set_del(f_pool, v1.val.ad, v2.val.ec);
	else
	  res.val.ad = eclist_filter(f_pool, v1.val.ad, v2, 0);
	break;

      case 'f':
	if (!arg_set)
	  runtime("Can't filter ec");
	res.val.ad = eclist_filter(f_pool, v1.val.ad, v2, 1);
	break;

      default:
	bug("unknown Ca operation");
      }
    }
    else
      runtime("Can't add/delete to non-(e)clist");

    break;

  case P('R','C'):	/* ROA Check */
    if (what->arg1)
    {
      TWOARGS;
      if ((v1.type != T_PREFIX) || (v2.type != T_INT))
	runtime("Invalid argument to roa_check()");

      as = v2.val.i;
    }
    else
    {
      v1.val.px.ip = (*f_rte)->net->n.prefix;
      v1.val.px.len = (*f_rte)->net->n.pxlen;

      /* We ignore temporary attributes, probably not a problem here */
      /* 0x02 is a value of BA_AS_PATH, we don't want to include BGP headers */
      eattr *e = ea_find((*f_rte)->attrs->eattrs, EA_CODE(EAP_BGP, 0x02));

      if (!e || e->type != EAF_TYPE_AS_PATH)
	runtime("Missing AS_PATH attribute");

      as_path_get_last(e->u.ptr, &as);
    }

    struct roa_table_config *rtc = ((struct f_inst_roa_check *) what)->rtc;
    if (!rtc->table)
      runtime("Missing ROA table");

    res.type = T_ENUM_ROA;
    res.val.i = roa_check(rtc->table, v1.val.px.ip, v1.val.px.len, as);
    break;

  default:
    bug( "Unknown instruction %d (%c)", what->code, what->code & 0xff);
  }
  if (what->next)
    return interpret(what->next);
  return res;
}

#undef ARG
#define ARG(x,y) \
	if (!i_same(f1->y, f2->y)) \
		return 0;

#define ONEARG ARG(v1, a1.p)
#define TWOARGS ARG(v1, a1.p) \
		ARG(v2, a2.p)

#define A2_SAME if (f1->a2.i != f2->a2.i) return 0;

/*
 * i_same - function that does real comparing of instruction trees, you should call filter_same from outside
 */
int
i_same(struct f_inst *f1, struct f_inst *f2)
{
  if ((!!f1) != (!!f2))
    return 0;
  if (!f1)
    return 1;
  if (f1->aux != f2->aux)
    return 0;
  if (f1->code != f2->code)
    return 0;
  if (f1 == f2)		/* It looks strange, but it is possible with call rewriting trickery */
    return 1;

  switch(f1->code) {
  case ',': /* fall through */
  case '+':
  case '-':
  case '*':
  case '/':
  case '|':
  case '&':
  case P('m','p'):
  case P('m','c'):
  case P('!','='):
  case P('=','='):
  case '<':
  case P('<','='): TWOARGS; break;

  case '!': ONEARG; break;
  case '~': TWOARGS; break;
  case P('d','e'): ONEARG; break;

  case 's':
    ARG(v2, a2.p);
    {
      struct symbol *s1, *s2;
      s1 = f1->a1.p;
      s2 = f2->a1.p;
      if (strcmp(s1->name, s2->name))
	return 0;
      if (s1->class != s2->class)
	return 0;
    }
    break;

  case 'c': 
    switch (f1->aux) {

    case T_PREFIX_SET:
      if (!trie_same(f1->a2.p, f2->a2.p))
	return 0;
      break;

    case T_SET:
      if (!same_tree(f1->a2.p, f2->a2.p))
	return 0;
      break;

    case T_STRING:
      if (strcmp(f1->a2.p, f2->a2.p))
	return 0;
      break;

    default:
      A2_SAME;
    }
    break;
  case 'C': 
    if (val_compare(* (struct f_val *) f1->a1.p, * (struct f_val *) f2->a1.p))
      return 0;
    break;
  case 'V': 
    if (strcmp((char *) f1->a2.p, (char *) f2->a2.p))
      return 0;
    break;
  case 'p': case 'L': ONEARG; break;
  case '?': TWOARGS; break;
  case '0': case 'E': break;
  case P('p',','): ONEARG; A2_SAME; break;
  case 'P':
  case 'a': A2_SAME; break;
  case P('e','a'): A2_SAME; break;
  case P('P','S'):
  case P('a','S'):
  case P('e','S'): ONEARG; A2_SAME; break;

  case 'r': ONEARG; break;
  case P('c','p'): ONEARG; break;
  case P('c','a'): /* Call rewriting trickery to avoid exponential behaviour */
             ONEARG; 
	     if (!i_same(f1->a2.p, f2->a2.p))
	       return 0; 
	     f2->a2.p = f1->a2.p;
	     break;
  case P('c','v'): break; /* internal instruction */ 
  case P('S','W'): ONEARG; if (!same_tree(f1->a2.p, f2->a2.p)) return 0; break;
  case P('i','M'): TWOARGS; break;
  case P('A','p'): TWOARGS; break;
  case P('C','a'): TWOARGS; break;
  case P('a','f'):
  case P('a','l'): ONEARG; break;
  case P('R','C'):
    TWOARGS;
    /* Does not really make sense - ROA check resuls may change anyway */
    if (strcmp(((struct f_inst_roa_check *) f1)->rtc->name, 
	       ((struct f_inst_roa_check *) f2)->rtc->name))
      return 0;
    break;
  default:
    bug( "Unknown instruction %d in same (%c)", f1->code, f1->code & 0xff);
  }
  return i_same(f1->next, f2->next);
}

/**
 * f_run - run a filter for a route
 * @filter: filter to run
 * @rte: route being filtered, may be modified
 * @tmp_attrs: temporary attributes, prepared by caller or generated by f_run()
 * @tmp_pool: all filter allocations go from this pool
 * @flags: flags
 *
 * If filter needs to modify the route, there are several
 * posibilities. @rte might be read-only (with REF_COW flag), in that
 * case rw copy is obtained by rte_cow() and @rte is replaced. If
 * @rte is originally rw, it may be directly modified (and it is never
 * copied).
 *
 * The returned rte may reuse the (possibly cached, cloned) rta, or
 * (if rta was modificied) contains a modified uncached rta, which
 * uses parts allocated from @tmp_pool and parts shared from original
 * rta. There is one exception - if @rte is rw but contains a cached
 * rta and that is modified, rta in returned rte is also cached.
 *
 * Ownership of cached rtas is consistent with rte, i.e.
 * if a new rte is returned, it has its own clone of cached rta
 * (and cached rta of read-only source rte is intact), if rte is
 * modified in place, old cached rta is possibly freed.
 */
int
f_run(struct filter *filter, struct rte **rte, struct ea_list **tmp_attrs, struct linpool *tmp_pool, int flags)
{
  int rte_cow = ((*rte)->flags & REF_COW);
  DBG( "Running filter `%s'...", filter->name );

  f_rte = rte;
  f_old_rta = NULL;
  f_tmp_attrs = tmp_attrs;
  f_pool = tmp_pool;
  f_flags = flags;

  log_reset();
  struct f_val res = interpret(filter->root);

  if (f_old_rta) {
    /*
     * Cached rta was modified and f_rte contains now an uncached one,
     * sharing some part with the cached one. The cached rta should
     * be freed (if rte was originally COW, f_old_rta is a clone
     * obtained during rte_cow()).
     *
     * This also implements the exception mentioned in f_run()
     * description. The reason for this is that rta reuses parts of
     * f_old_rta, and these may be freed during rta_free(f_old_rta).
     * This is not the problem if rte was COW, because original rte
     * also holds the same rta.
     */
    if (!rte_cow)
      (*f_rte)->attrs = rta_lookup((*f_rte)->attrs);

    rta_free(f_old_rta);
  }


  if (res.type != T_RETURN) {
    log( L_ERR "Filter %s did not return accept nor reject. Make up your mind", filter->name); 
    return F_ERROR;
  }
  DBG( "done (%d)\n", res.val.i );
  return res.val.i;
}

int
f_eval_int(struct f_inst *expr)
{
  /* Called independently in parse-time to eval expressions */
  struct f_val res;

  f_flags = 0;
  f_tmp_attrs = NULL;
  f_rte = NULL;
  f_pool = cfg_mem;

  log_reset();
  res = interpret(expr);

  if (res.type != T_INT)
    cf_error("Integer expression expected");
  return res.val.i;
}

u32
f_eval_asn(struct f_inst *expr)
{
  /* Called as a part of another interpret call, therefore no log_reset() */
  struct f_val res = interpret(expr);
  return (res.type == T_INT) ? res.val.i : 0;
}

/**
 * filter_same - compare two filters
 * @new: first filter to be compared
 * @old: second filter to be compared, notice that this filter is
 * damaged while comparing.
 *
 * Returns 1 in case filters are same, otherwise 0. If there are
 * underlying bugs, it will rather say 0 on same filters than say
 * 1 on different.
 */
int
filter_same(struct filter *new, struct filter *old)
{
  if (old == new)	/* Handle FILTER_ACCEPT and FILTER_REJECT */
    return 1;
  if (old == FILTER_ACCEPT || old == FILTER_REJECT ||
      new == FILTER_ACCEPT || new == FILTER_REJECT)
    return 0;
  return i_same(new->root, old->root);
}
