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
    m1 = m1->next;
    m2 = m2->next;
  }
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
    bdebug( "Types do not match in val_compare\n" );
    return CMP_ERROR;
  }
  switch (v1.type) {
  case T_ENUM:
  case T_INT: 
  case T_PAIR:
    if (v1.val.i == v2.val.i) return 0;
    if (v1.val.i < v2.val.i) return -1;
    return 1;
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
  default:
    bdebug( "Compare of unkown entities: %x\n", v1.type );
    return CMP_ERROR;
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
  if ((v1.type == T_PAIR) && (v2.type == T_CLIST))
    return int_set_contains(v2.val.ad, v1.val.i);

  if ((v1.type == T_IP) && (v2.type == T_PREFIX))
    return !(ipa_compare(ipa_and(v2.val.px.ip, ipa_mkmask(v2.val.px.len)), ipa_and(v1.val.px.ip, ipa_mkmask(v2.val.px.len))));

  if ((v1.type == T_PREFIX) && (v2.type == T_PREFIX)) {
    ip_addr mask;
    if (v1.val.px.len & (LEN_PLUS | LEN_MINUS | LEN_RANGE))
      return CMP_ERROR;
    mask = ipa_mkmask( v2.val.px.len & LEN_MASK );
    if (ipa_compare(ipa_and(v2.val.px.ip, mask), ipa_and(v1.val.px.ip, mask)))
      return 0;

    if ((v2.val.px.len & LEN_MINUS) && (v1.val.px.len <= (v2.val.px.len & LEN_MASK)))
      return 0;
    if ((v2.val.px.len & LEN_PLUS) && (v1.val.px.len < (v2.val.px.len & LEN_MASK)))
      return 0;
    if ((v2.val.px.len & LEN_RANGE) && ((v1.val.px.len < (0xff & (v2.val.px.len >> 16)))
					|| (v1.val.px.len > (0xff & (v2.val.px.len >> 8)))))
      return 0;
    return 1;    
  }
  return CMP_ERROR;
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
  
  if (v2.type == T_SET)
    switch (v1.type) {
    case T_ENUM:
    case T_INT:
    case T_IP:
    case T_PREFIX:
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
    bdebug( "() " );
    return;
  }
  bdebug( "[ " );
  tree_print( t->left );
  bdebug( ", " ); val_print( t->from ); bdebug( ".." ); val_print( t->to ); bdebug( ", " );
  tree_print( t->right );
  bdebug( "] " );
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
  case T_SET: tree_print( v.val.t ); PRINTF( "\n" ); break;
  case T_ENUM: PRINTF( "(enum %x)%d", v.type, v.val.i ); break;
  case T_PATH: as_path_format(v.val.ad, buf2, 1020); PRINTF( "(path %s)", buf2 ); break;
  case T_CLIST: int_set_format(v.val.ad, buf2, 1020); PRINTF( "(clist %s)", buf2 ); break;
  case T_PATH_MASK: bdebug( "(pathmask " ); { struct f_path_mask *p = v.val.path_mask; while (p) { bdebug("%d ", p->val); p=p->next; } bdebug(")" ); } break;
  default: PRINTF( "[unknown type %x]", v.type );
#undef PRINTF
  }
  bdebug( buf );
}

static struct rte **f_rte, *f_rte_old;
static struct linpool *f_pool;
static struct ea_list **f_tmp_attrs;
static int f_flags;
static rta *f_rta_copy;

/*
 * rta_cow - prepare rta for modification by filter
 */
static void
rta_cow(void)
{
  if (!f_rta_copy) {
    f_rta_copy = lp_alloc(f_pool, sizeof(rta));
    memcpy(f_rta_copy, (*f_rte)->attrs, sizeof(rta));
    f_rta_copy->aflags = 0;
    *f_rte = rte_cow(*f_rte);
    rta_free((*f_rte)->attrs);
    (*f_rte)->attrs = f_rta_copy;
  }
}

#define runtime(x) do { \
    log( L_ERR "filters, line %d: %s", what->lineno, x); \
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
  struct f_val v1, v2, res;
  int i;

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

/* Relational operators */

#define COMPARE(x) \
    TWOARGS_C; \
    res.type = T_BOOL; \
    i = val_compare(v1, v2); \
    if (i==CMP_ERROR) \
      runtime( "Error in comparison" ); \
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
    switch (res.type = v2.type) {
    case T_VOID: runtime( "Can't assign void values" );
    case T_ENUM:
    case T_INT: 
    case T_IP: 
    case T_PREFIX: 
    case T_PAIR: 
    case T_PATH:
    case T_CLIST:
    case T_PATH_MASK:
      if (sym->class != (SYM_VARIABLE | v2.type))
	runtime( "Assigning to variable of incompatible type" );
      * (struct f_val *) sym->aux2 = v2; 
      break;
    default:
      bug( "Set to invalid type" );
    }
    break;

  case 'c':	/* integer (or simple type) constant */
    res.type = what->aux;
    res.val.i = what->a2.i;
    break;
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
    bdebug( "No operation\n" );
    break;
  case P('p',','):
    ONEARG;
    if (what->a2.i == F_NOP || (what->a2.i != F_NONL && what->a1.p))
      bdebug( "\n" );

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
      
      switch (what->aux & EAF_TYPE_MASK) {
      case EAF_TYPE_INT:
	if (!e) {
	  res.type = T_VOID;
	  break;
	}
	res.type = T_INT;
	res.val.i = e->u.data;
	break;
      case EAF_TYPE_AS_PATH:
	if (!e) {
	  res.type = T_VOID;
	  break;
	}
        res.type = T_PATH;
	res.val.ad = e->u.ptr;
	break;
      case EAF_TYPE_INT_SET:
	if (!e) {
	  res.type = T_CLIST;
	  res.val.ad = adata_empty(f_pool);
	  break;
	}
	res.type = T_CLIST;
	res.val.ad = e->u.ptr;
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
	l->next = f_rta_copy->eattrs;
	f_rta_copy->eattrs = l;
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
    *f_rte = rte_cow(*f_rte);
    (*f_rte)->pref = v1.val.i;
    break;
  case 'L':	/* Get length of */
    ONEARG;
    res.type = T_INT;
    switch(v1.type) {
    case T_PREFIX: res.val.i = v1.val.px.len; break;
    case T_PATH:   res.val.i = as_path_getlen(v1.val.ad); break;
    default: bug( "Length of what?" );
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
  case 'r':
    ONEARG;
    res = v1;
    res.type |= T_RETURN;
    break;
  case P('c','a'): /* CALL: this is special: if T_RETURN and returning some value, mask it out  */
    ONEARG;
    res = interpret(what->a2.p);
    if (res.type == T_RETURN)
      return res;
    res.type &= ~T_RETURN;    
    break;
  case P('S','W'):
    ONEARG;
    {
      struct f_tree *t = find_tree(what->a2.p, v1);
      if (!t) {
	v1.type = T_VOID;
	t = find_tree(what->a2.p, v1);
	if (!t) {
	  bdebug( "No else statement?\n");
	  break;
	}
      }	
      /* It is actually possible to have t->data NULL */
      return interpret(t->data);
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
    if (v2.type != T_PAIR)
      runtime("Can't add/delete non-pair");

    res.type = T_CLIST;
    switch (what->aux) {
    case 'a': res.val.ad = int_set_add(f_pool, v1.val.ad, v2.val.i); break;
    case 'd': res.val.ad = int_set_del(f_pool, v1.val.ad, v2.val.i); break;
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
    if (f1->aux & T_SET) {
      if (!same_tree(f1->a2.p, f2->a2.p))
	return 0;
      break;
    } 
    switch (f1->aux) {
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
  case P('S','W'): ONEARG; if (!same_tree(f1->a2.p, f2->a2.p)) return 0; break;
  case P('i','M'): TWOARGS; break;
  case P('A','p'): TWOARGS; break;
  case P('C','a'): TWOARGS; break;
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
  f_rta_copy = NULL;
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
  f_rta_copy = NULL;
  f_pool = cfg_mem;
  res = interpret(expr);
  if (res.type != T_INT)
    cf_error("Integer expression expected");
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
