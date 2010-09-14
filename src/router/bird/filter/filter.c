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
adata_empty(struct linpool *pool)
{
  struct adata *res = lp_alloc(pool, sizeof(struct adata));
  res->length = 0;
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
      return int_cmp(v1.val.i, ipa_to_u32(v2.val.px.ip));
    if ((v1.type == T_IP) && (v2.type == T_QUAD))
      return int_cmp(ipa_to_u32(v1.val.px.ip), v2.val.i);
#endif

    debug( "Types do not match in val_compare\n" );
    return CMP_ERROR;
  }
  switch (v1.type) {
  case T_ENUM:
  case T_INT:
  case T_BOOL:
  case T_PAIR:
  case T_QUAD:
    return int_cmp(v1.val.i, v2.val.i);
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
f_prefix_get_bounds(struct f_prefix *px, int *l, int *h)
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
  if (((v1.type == T_PAIR) || (v1.type == T_QUAD)) && (v2.type == T_CLIST))
    return int_set_contains(v2.val.ad, v1.val.i);
#ifndef IPV6
  /* IP->Quad implicit conversion */
  if ((v1.type == T_IP) && (v2.type == T_CLIST))
    return int_set_contains(v2.val.ad, ipa_to_u32(v1.val.px.ip));
#endif
  if ((v1.type == T_STRING) && (v2.type == T_STRING))
    return patmatch(v2.val.s, v1.val.s);

  if ((v1.type == T_IP) && (v2.type == T_PREFIX))
    return ipa_in_net(v1.val.px.ip, v2.val.px.ip, v2.val.px.len);

  if ((v1.type == T_PREFIX) && (v2.type == T_PREFIX))
    return ipa_in_net(v1.val.px.ip, v2.val.px.ip, v2.val.px.len) && (v1.val.px.len >= v2.val.px.len);

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

static struct adata *
clist_del_matching(struct linpool *pool, struct adata *clist, struct f_tree *set)
{
  if (!clist)
    return NULL;

  struct f_val v;
  clist_set_type(set, &v);

  u32 tmp[clist->length/4];
  u32 *l = (u32 *) clist->data;
  u32 *k = tmp;
  u32 *end = l + clist->length/4;

  while (l < end) {
    v.val.i = *l++;
    if (!find_tree(set, v))
      *k++ = v.val.i;
  }

  int nl = (k - tmp) * 4;
  if (nl == clist->length)
    return clist;

  struct adata *res = lp_alloc(pool, sizeof(struct adata) + nl);
  res->length = nl;
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
    return trie_match_prefix(v2.val.ti, &v1.val.px);

  if ((v1.type == T_CLIST) && (v2.type == T_SET))
    return clist_match_set(v1.val.ad, v2.val.t);

  if (v2.type == T_SET)
    switch (v1.type) {
    case T_ENUM:
    case T_INT:
    case T_PAIR:
    case T_QUAD:
    case T_IP:
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

static void
tree_print(struct f_tree *t)
{
  if (!t) {
    debug( "() " );
    return;
  }
  debug( "[ " );
  tree_print( t->left );
  debug( ", " ); val_print( t->from ); debug( ".." ); val_print( t->to ); debug( ", " );
  tree_print( t->right );
  debug( "] " );
}

/*
 * val_print - format filter value
 */
void
val_print(struct f_val v)
{
  char buf[2048];
  char buf2[1024];
#define PRINTF(a...) bsnprintf( buf, 2040, a )
  buf[0] = 0;
  switch (v.type) {
  case T_VOID: PRINTF( "(void)" ); break;
  case T_BOOL: PRINTF( v.val.i ? "TRUE" : "FALSE" ); break;
  case T_INT: PRINTF( "%d ", v.val.i ); break;
  case T_STRING: PRINTF( "%s", v.val.s ); break;
  case T_IP: PRINTF( "%I", v.val.px.ip ); break;
  case T_PREFIX: PRINTF( "%I/%d", v.val.px.ip, v.val.px.len ); break;
  case T_PAIR: PRINTF( "(%d,%d)", v.val.i >> 16, v.val.i & 0xffff ); break;
  case T_QUAD: PRINTF( "%R", v.val.i ); break;
  case T_PREFIX_SET: trie_print(v.val.ti, buf, 2040); break;
  case T_SET: tree_print( v.val.t ); PRINTF( "\n" ); break;
  case T_ENUM: PRINTF( "(enum %x)%d", v.type, v.val.i ); break;
  case T_PATH: as_path_format(v.val.ad, buf2, 1020); PRINTF( "(path %s)", buf2 ); break;
  case T_CLIST: int_set_format(v.val.ad, 1, buf2, 1020); PRINTF( "(clist %s)", buf2 ); break;
  case T_PATH_MASK: pm_format(v.val.path_mask, buf2, 1020); PRINTF( "(pathmask%s)", buf2 ); break;
  default: PRINTF( "[unknown type %x]", v.type );
#undef PRINTF
  }
  debug( buf );
}

static struct rte **f_rte, *f_rte_old;
static struct linpool *f_pool;
static struct ea_list **f_tmp_attrs;
static int f_flags;

/*
 * rta_cow - prepare rta for modification by filter
 */
static void
rta_cow(void)
{
  if ((*f_rte)->attrs->aflags & RTAF_CACHED) {
    rta *f_rta_copy = lp_alloc(f_pool, sizeof(rta));
    memcpy(f_rta_copy, (*f_rte)->attrs, sizeof(rta));
    f_rta_copy->aflags = 0;
    *f_rte = rte_cow(*f_rte);
    rta_free((*f_rte)->attrs);
    (*f_rte)->attrs = f_rta_copy;
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
    case T_IP: if (v2.type != T_INT)
                 runtime( "Incompatible types in / operator" );
               break;
    default: runtime( "Usage of unknown type" );
    }
    break;
    
  case '&':
    TWOARGS_C;
    res.type = v1.type;
    if (res.type != T_BOOL) runtime( "Can't do boolean operation on non-booleans" );
    res.val.i = v1.val.i && v2.val.i;
    break;
  case '|':
    TWOARGS_C;
    res.type = v1.type;
    if (res.type != T_BOOL) runtime( "Can't do boolean operation on non-booleans" );
    res.val.i = v1.val.i || v2.val.i;
    break;

  case P('m','p'):
    TWOARGS_C;
    if ((v1.type != T_INT) || (v2.type != T_INT))
      runtime( "Can't operate with value of non-integer type in pair constructor" );
    u1 = v1.val.i;
    u2 = v2.val.i;
    if ((u1 > 0xFFFF) || (u2 > 0xFFFF))
      runtime( "Can't operate with value out of bounds in pair constructor" );
    res.val.i = (u1 << 16) | u2;
    res.type = T_PAIR;
    break;

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
      debug( "\n" );

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
    rta_cow();
    {
      struct rta *rta = (*f_rte)->attrs;
      switch (what->aux) {
      case T_ENUM:
	* ((char *) rta + what->a2.i) = v1.val.i;
	break;
      case T_IP:
	* (ip_addr *) ((char *) rta + what->a2.i) = v1.val.px.ip;
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
	  res.val.ad = adata_empty(f_pool);
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
      case EAF_TYPE_ROUTER_ID:
	if (v1.type != T_INT)
	  runtime( "Setting int attribute to non-int value" );
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
	  runtime( "Setting int set attribute to non-clist value" );
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
	rta_cow();
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
    *f_rte = rte_cow(*f_rte);
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
    res.val.ad = adata_empty(f_pool);
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

  case P('C','a'):	/* Community list add or delete */
    TWOARGS;
    if (v1.type != T_CLIST)
      runtime("Can't add/delete to non-clist");

    struct f_val dummy;
    if ((v2.type == T_PAIR) || (v2.type == T_QUAD))
      i = v2.val.i;
#ifndef IPV6
    /* IP->Quad implicit conversion */
    else if (v2.type == T_IP)
      i = ipa_to_u32(v2.val.px.ip);
#endif
    else if ((v2.type == T_SET) && (what->aux == 'd') && clist_set_type(v2.val.t, &dummy))
      what->aux = 'D';
    else
      runtime("Can't add/delete non-pair");

    res.type = T_CLIST;
    switch (what->aux) {
    case 'a': res.val.ad = int_set_add(f_pool, v1.val.ad, i); break;
    case 'd': res.val.ad = int_set_del(f_pool, v1.val.ad, i); break;
    case 'D': res.val.ad = clist_del_matching(f_pool, v1.val.ad, v2.val.t); break;
    default: bug("unknown Ca operation");
    }
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
  default:
    bug( "Unknown instruction %d in same (%c)", f1->code, f1->code & 0xff);
  }
  return i_same(f1->next, f2->next);
}

/**
 * f_run - external entry point to filters
 * @filter: pointer to filter to run
 * @tmp_attrs: where to store newly generated temporary attributes
 * @rte: pointer to pointer to &rte being filtered. When route is modified, this is changed with rte_cow().
 * @tmp_pool: all filter allocations go from this pool
 * @flags: flags
 */
int
f_run(struct filter *filter, struct rte **rte, struct ea_list **tmp_attrs, struct linpool *tmp_pool, int flags)
{
  struct f_inst *inst;
  struct f_val res;
  DBG( "Running filter `%s'...", filter->name );

  f_flags = flags;
  f_tmp_attrs = tmp_attrs;
  f_rte = rte;
  f_rte_old = *rte;
  f_pool = tmp_pool;
  inst = filter->root;
  res = interpret(inst);
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
  struct f_val res;

  f_flags = 0;
  f_tmp_attrs = NULL;
  f_rte = NULL;
  f_rte_old = NULL;
  f_pool = cfg_mem;
  res = interpret(expr);
  if (res.type != T_INT)
    cf_error("Integer expression expected");
  return res.val.i;
}

u32
f_eval_asn(struct f_inst *expr)
{
  struct f_val res = interpret(expr);
  if (res.type != T_INT)
    cf_error("Can't operate with value of non-integer type in AS path mask constructor");
 
  return res.val.i;
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
