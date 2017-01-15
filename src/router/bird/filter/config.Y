/*
 *	BIRD - filters
 *
 *	Copyright 1998--2000 Pavel Machek
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 *
	FIXME: priority of ! should be lower
 */

CF_HDR

CF_DEFINES

#define P(a,b) ((a << 8) | b)

static inline u32 pair(u32 a, u32 b) { return (a << 16) | b; }
static inline u32 pair_a(u32 p) { return p >> 16; }
static inline u32 pair_b(u32 p) { return p & 0xFFFF; }


/*
 * Sets and their items are during parsing handled as lists, linked
 * through left ptr. The first item in a list also contains a pointer
 * to the last item in a list (right ptr). For convenience, even items
 * are handled as one-item lists. Lists are merged by f_merge_items().
 */
static int
f_valid_set_type(int type)
{
  switch (type)
  {
  case T_INT:
  case T_PAIR:
  case T_QUAD:
  case T_ENUM:
  case T_IP:
  case T_EC:
  case T_LC:
    return 1;

  default:
    return 0;
  }
}

static inline struct f_tree *
f_new_item(struct f_val from, struct f_val to)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from = from;
  t->to = to;
  return t;
}

static inline struct f_tree *
f_merge_items(struct f_tree *a, struct f_tree *b)
{
  if (!a) return b;
  a->right->left = b;
  a->right = b->right;
  b->right = NULL;
  return a;
}

static inline struct f_tree *
f_new_pair_item(int fa, int ta, int fb, int tb)
{
  check_u16(fa);
  check_u16(ta);
  check_u16(fb);
  check_u16(tb);

  if ((ta < fa) || (tb < fb))
    cf_error( "From value cannot be higher that To value in pair sets");

  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_PAIR;
  t->from.val.i = pair(fa, fb);
  t->to.val.i = pair(ta, tb);
  return t;
}

static inline struct f_tree *
f_new_pair_set(int fa, int ta, int fb, int tb)
{
  check_u16(fa);
  check_u16(ta);
  check_u16(fb);
  check_u16(tb);

  if ((ta < fa) || (tb < fb))
    cf_error( "From value cannot be higher that To value in pair sets");

  struct f_tree *lst = NULL;
  int i;

  for (i = fa; i <= ta; i++)
    lst = f_merge_items(lst, f_new_pair_item(i, i, fb, tb));

  return lst;
}

#define CC_ALL 0xFFFF
#define EC_ALL 0xFFFFFFFF
#define LC_ALL 0xFFFFFFFF

static struct f_tree *
f_new_ec_item(u32 kind, u32 ipv4_used, u32 key, u32 vf, u32 vt)
{
  u64 fm, to;

  if (ipv4_used || (key >= 0x10000)) {
    check_u16(vf);
    if (vt == EC_ALL)
      vt = 0xFFFF;
    else
      check_u16(vt);
  }

  if (kind == EC_GENERIC) {
    fm = ec_generic(key, vf);
    to = ec_generic(key, vt);
  }
  else if (ipv4_used) {
    fm = ec_ip4(kind, key, vf);
    to = ec_ip4(kind, key, vt);
  }
  else if (key < 0x10000) {
    fm = ec_as2(kind, key, vf);
    to = ec_as2(kind, key, vt);
  }
  else {
    fm = ec_as4(kind, key, vf);
    to = ec_as4(kind, key, vt);
  }

  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_EC;
  t->from.val.ec = fm;
  t->to.val.ec = to;
  return t;
}

static struct f_tree *
f_new_lc_item(u32 f1, u32 t1, u32 f2, u32 t2, u32 f3, u32 t3)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_LC;
  t->from.val.lc = (lcomm) {f1, f2, f3};
  t->to.val.lc = (lcomm) {t1, t2, t3};
  return t;
}

static inline struct f_inst *
f_generate_empty(struct f_inst *dyn)
{
  struct f_inst *e = f_new_inst();
  e->code = 'E';

  switch (dyn->aux & EAF_TYPE_MASK) {
    case EAF_TYPE_AS_PATH:
      e->aux = T_PATH;
      break;
    case EAF_TYPE_INT_SET:
      e->aux = T_CLIST;
      break;
    case EAF_TYPE_EC_SET:
      e->aux = T_ECLIST;
      break;
    case EAF_TYPE_LC_SET:
      e->aux = T_LCLIST;
      break;
    default:
      cf_error("Can't empty that attribute");
  }

  dyn->code = P('e','S');
  dyn->a1.p = e;
  return dyn;
}


static inline struct f_inst *
f_generate_dpair(struct f_inst *t1, struct f_inst *t2)
{
  struct f_inst *rv;

  if ((t1->code == 'c') && (t2->code == 'c')) {
    if ((t1->aux != T_INT) || (t2->aux != T_INT))
      cf_error( "Can't operate with value of non-integer type in pair constructor");

    check_u16(t1->a2.i);
    check_u16(t2->a2.i);

    rv = f_new_inst();
    rv->code = 'c';
    rv->aux = T_PAIR;
    rv->a2.i = pair(t1->a2.i, t2->a2.i);
  }
  else {
    rv = f_new_inst();
    rv->code = P('m', 'p');
    rv->a1.p = t1;
    rv->a2.p = t2;
  }

  return rv;
}

static inline struct f_inst *
f_generate_ec(u16 kind, struct f_inst *tk, struct f_inst *tv)
{
  struct f_inst *rv;
  int c1 = 0, c2 = 0, ipv4_used = 0;
  u32 key = 0, val2 = 0;

  if (tk->code == 'c') {
    c1 = 1;

    if (tk->aux == T_INT) {
      ipv4_used = 0; key = tk->a2.i;
    }
    else if (tk->aux == T_QUAD) {
      ipv4_used = 1; key = tk->a2.i;
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }

#ifndef IPV6
  /* IP->Quad implicit conversion */
  else if (tk->code == 'C') {
    c1 = 1;
    struct f_val *val = tk->a1.p;

    if (val->type == T_INT) {
      ipv4_used = 0; key = val->val.i;
    }
    else if (val->type == T_QUAD) {
      ipv4_used = 1; key = val->val.i;
    }
    else if (val->type == T_IP) {
      ipv4_used = 1; key = ipa_to_u32(val->val.px.ip);
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }
#endif

  if (tv->code == 'c') {
    if (tv->aux != T_INT)
      cf_error("Can't operate with value of non-integer type in EC constructor");
    c2 = 1;
    val2 = tv->a2.i;
  }

  if (c1 && c2) {
    u64 ec;

    if (kind == EC_GENERIC) {
      ec = ec_generic(key, val2);
    }
    else if (ipv4_used) {
      check_u16(val2);
      ec = ec_ip4(kind, key, val2);
    }
    else if (key < 0x10000) {
      ec = ec_as2(kind, key, val2);
    }
    else {
      check_u16(val2);
      ec = ec_as4(kind, key, val2);
    }

    NEW_F_VAL;
    rv = f_new_inst();
    rv->code = 'C';
    rv->a1.p = val;
    val->type = T_EC;
    val->val.ec = ec;
  }
  else {
    rv = f_new_inst();
    rv->code = P('m','c');
    rv->aux = kind;
    rv->a1.p = tk;
    rv->a2.p = tv;
  }

  return rv;
}

static inline struct f_inst *
f_generate_lc(struct f_inst *t1, struct f_inst *t2, struct f_inst *t3)
{
  struct f_inst *rv;

  if ((t1->code == 'c') && (t2->code == 'c') && (t3->code == 'c')) {
    if ((t1->aux != T_INT) || (t2->aux != T_INT) || (t3->aux != T_INT))
      cf_error( "LC - Can't operate with value of non-integer type in tuple constructor");

    rv = f_new_inst();
    rv->code = 'C';

    NEW_F_VAL;
    rv->a1.p = val;
    val->type = T_LC;
    val->val.lc = (lcomm) { t1->a2.i, t2->a2.i, t3->a2.i };
  }
  else
  {
    rv = cfg_allocz(sizeof(struct f_inst3));
    rv->lineno = ifs->lino;
    rv->code = P('m','l');
    rv->a1.p = t1;
    rv->a2.p = t2;
    INST3(rv).p = t3;
  }

  return rv;
}



CF_DECLS

CF_KEYWORDS(FUNCTION, PRINT, PRINTN, UNSET, RETURN,
	ACCEPT, REJECT, ERROR, QUITBIRD,
	INT, BOOL, IP, PREFIX, PAIR, QUAD, EC, LC,
	SET, STRING, BGPMASK, BGPPATH, CLIST, ECLIST, LCLIST,
	IF, THEN, ELSE, CASE,
	TRUE, FALSE, RT, RO, UNKNOWN, GENERIC,
	FROM, GW, NET, MASK, PROTO, SOURCE, SCOPE, CAST, DEST, IFNAME, IFINDEX,
	PREFERENCE,
	LEN,
	DEFINED,
	ADD, DELETE, CONTAINS, RESET,
	PREPEND, FIRST, LAST, LAST_NONAGGREGATED, MATCH,
	ROA_CHECK,
	EMPTY,
	FILTER, WHERE, EVAL)

%nonassoc THEN
%nonassoc ELSE

%type <x> term block cmds cmds_int cmd function_body constant constructor print_one print_list var_list var_listn dynamic_attr static_attr function_call symbol bgp_path_expr
%type <f> filter filter_body where_filter
%type <i> type break_command ec_kind
%type <i32> cnum
%type <e> pair_item ec_item lc_item set_item switch_item set_items switch_items switch_body
%type <trie> fprefix_set
%type <v> set_atom switch_atom fprefix fprefix_s fipa
%type <s> decls declsn one_decl function_params
%type <h> bgp_path bgp_path_tail1 bgp_path_tail2

CF_GRAMMAR

CF_ADDTO(conf, filter_def)
filter_def:
   FILTER SYM { $2 = cf_define_symbol($2, SYM_FILTER, NULL); cf_push_scope( $2 ); }
     filter_body {
     $2->def = $4;
     $4->name = $2->name;
     DBG( "We have new filter defined (%s)\n", $2->name );
     cf_pop_scope();
   }
 ;

CF_ADDTO(conf, filter_eval)
filter_eval:
   EVAL term { f_eval_int($2); }
 ;

type:
   INT { $$ = T_INT; }
 | BOOL { $$ = T_BOOL; }
 | IP { $$ = T_IP; }
 | PREFIX { $$ = T_PREFIX; }
 | PAIR { $$ = T_PAIR; }
 | QUAD { $$ = T_QUAD; }
 | EC { $$ = T_EC; }
 | LC { $$ = T_LC; }
 | STRING { $$ = T_STRING; }
 | BGPMASK { $$ = T_PATH_MASK; }
 | BGPPATH { $$ = T_PATH; }
 | CLIST { $$ = T_CLIST; }
 | ECLIST { $$ = T_ECLIST; }
 | LCLIST { $$ = T_LCLIST; }
 | type SET {
	switch ($1) {
	  case T_INT:
	  case T_PAIR:
	  case T_QUAD:
	  case T_EC:
	  case T_LC:
	  case T_IP:
	       $$ = T_SET;
	       break;

	  case T_PREFIX:
	       $$ = T_PREFIX_SET;
	    break;

	  default:
		cf_error( "You can't create sets of this type." );
	}
   }
 ;

one_decl:
   type SYM {
     struct f_val * val = cfg_alloc(sizeof(struct f_val));
     val->type = T_VOID;
     $2 = cf_define_symbol($2, SYM_VARIABLE | $1, val);
     DBG( "New variable %s type %x\n", $2->name, $1 );
     $2->aux2 = NULL;
     $$=$2;
   }
 ;

/* Decls with ';' at the end */
decls: /* EMPTY */ { $$ = NULL; }
 | one_decl ';' decls {
     $$ = $1;
     $$->aux2 = $3;
   }
 ;

/* Declarations that have no ';' at the end. */
declsn: one_decl { $$ = $1; }
 | one_decl ';' declsn {
     $$ = $1;
     $$->aux2 = $3;
   }
 ;

filter_body:
   function_body {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = $1;
     $$ = f;
   }
 ;

filter:
   SYM {
     if ($1->class != SYM_FILTER) cf_error("No such filter.");
     $$ = $1->def;
   }
 | filter_body
 ;

where_filter:
   WHERE term {
     /* Construct 'IF term THEN ACCEPT; REJECT;' */
     struct filter *f = cfg_alloc(sizeof(struct filter));
     struct f_inst *i, *acc, *rej;
     acc = f_new_inst();		/* ACCEPT */
     acc->code = P('p',',');
     acc->a1.p = NULL;
     acc->a2.i = F_ACCEPT;
     rej = f_new_inst();		/* REJECT */
     rej->code = P('p',',');
     rej->a1.p = NULL;
     rej->a2.i = F_REJECT;
     i = f_new_inst();			/* IF */
     i->code = '?';
     i->a1.p = $2;
     i->a2.p = acc;
     i->next = rej;
     f->name = NULL;
     f->root = i;
     $$ = f;
  }
 ;

function_params:
   '(' declsn ')' { DBG( "Have function parameters\n" ); $$=$2; }
 | '(' ')' { $$=NULL; }
 ;

function_body:
   decls '{' cmds '}' {
     if ($1) {
       /* Prepend instruction to clear local variables */
       $$ = f_new_inst();
       $$->code = P('c','v');
       $$->a1.p = $1;
       $$->next = $3;
     } else
       $$ = $3;
   }
 ;

CF_ADDTO(conf, function_def)
function_def:
   FUNCTION SYM { DBG( "Beginning of function %s\n", $2->name );
     $2 = cf_define_symbol($2, SYM_FUNCTION, NULL);
     cf_push_scope($2);
   } function_params function_body {
     $2->def = $5;
     $2->aux2 = $4;
     DBG("Hmm, we've got one function here - %s\n", $2->name);
     cf_pop_scope();
   }
 ;

/* Programs */

/* Hack: $$ of cmds_int is the last node.
   $$->next of cmds_int is temporary used for the first node */

cmds: /* EMPTY */ { $$ = NULL; }
 | cmds_int { $$ = $1->next; $1->next = NULL; }
 ;

cmds_int: cmd { $$ = $1; $1->next = $1; }
 | cmds_int cmd { $$ = $2; $2->next = $1->next ; $1->next = $2; }
 ;

block:
   cmd {
     $$=$1;
   }
 | '{' cmds '}' {
     $$=$2;
   }
 ;

/*
 * Complex types, their bison value is struct f_val
 */
fipa:
   IPA %prec PREFIX_DUMMY { $$.type = T_IP; $$.val.px.ip = $1; }
 ;



/*
 * Set constants. They are also used in switch cases. We use separate
 * nonterminals for switch (set_atom/switch_atom, set_item/switch_item ...)
 * to elude a collision between symbol (in expr) in set_atom and symbol
 * as a function call in switch case cmds.
 */

set_atom:
   NUM   { $$.type = T_INT; $$.val.i = $1; }
 | RTRID { $$.type = T_QUAD; $$.val.i = $1; }
 | fipa  { $$ = $1; }
 | ENUM  { $$.type = pair_a($1); $$.val.i = pair_b($1); }
 | '(' term ')' {
     $$ = f_eval($2, cfg_mem);
     if (!f_valid_set_type($$.type)) cf_error("Set-incompatible type");
   }
 | SYM {
     if (!cf_symbol_is_constant($1)) cf_error("%s: constant expected", $1->name);
     if (!f_valid_set_type(SYM_TYPE($1))) cf_error("%s: set-incompatible type", $1->name);
     $$ = *(struct f_val *)($1->def);
   }
 ;

switch_atom:
   NUM   { $$.type = T_INT; $$.val.i = $1; }
 | '(' term ')' { $$.type = T_INT; $$.val.i = f_eval_int($2); }
 | RTRID { $$.type = T_QUAD; $$.val.i = $1; }
 | fipa  { $$ = $1; }
 | ENUM  { $$.type = pair_a($1); $$.val.i = pair_b($1); }
 ;

cnum:
   term { $$ = f_eval_int($1); }

pair_item:
   '(' cnum ',' cnum ')'		{ $$ = f_new_pair_item($2, $2, $4, $4); }
 | '(' cnum ',' cnum DDOT cnum ')'	{ $$ = f_new_pair_item($2, $2, $4, $6); }
 | '(' cnum ',' '*' ')'			{ $$ = f_new_pair_item($2, $2, 0, CC_ALL); }
 | '(' cnum DDOT cnum ',' cnum ')'	{ $$ = f_new_pair_set($2, $4, $6, $6); }
 | '(' cnum DDOT cnum ',' cnum DDOT cnum ')' { $$ = f_new_pair_set($2, $4, $6, $8); }
 | '(' cnum DDOT cnum ',' '*' ')'	{ $$ = f_new_pair_item($2, $4, 0, CC_ALL); }
 | '(' '*' ',' cnum ')'			{ $$ = f_new_pair_set(0, CC_ALL, $4, $4); }
 | '(' '*' ',' cnum DDOT cnum ')'	{ $$ = f_new_pair_set(0, CC_ALL, $4, $6); }
 | '(' '*' ',' '*' ')'			{ $$ = f_new_pair_item(0, CC_ALL, 0, CC_ALL); }
 | '(' cnum ',' cnum ')' DDOT '(' cnum ',' cnum ')'
   { $$ = f_new_pair_item($2, $8, $4, $10); }
 ;

ec_kind:
   RT { $$ = EC_RT; }
 | RO { $$ = EC_RO; }
 | UNKNOWN NUM { $$ = $2; }
 | GENERIC { $$ = EC_GENERIC; }
 ;

ec_item:
   '(' ec_kind ',' cnum ',' cnum ')'		{ $$ = f_new_ec_item($2, 0, $4, $6, $6); }
 | '(' ec_kind ',' cnum ',' cnum DDOT cnum ')'	{ $$ = f_new_ec_item($2, 0, $4, $6, $8); }
 | '(' ec_kind ',' cnum ',' '*' ')'		{ $$ = f_new_ec_item($2, 0, $4, 0, EC_ALL); }
 ;

lc_item:
   '(' cnum ',' cnum ',' cnum ')'	    { $$ = f_new_lc_item($2, $2, $4, $4, $6, $6); }
 | '(' cnum ',' cnum ',' cnum DDOT cnum ')' { $$ = f_new_lc_item($2, $2, $4, $4, $6, $8); }
 | '(' cnum ',' cnum ',' '*' ')'	    { $$ = f_new_lc_item($2, $2, $4, $4, 0, LC_ALL); }
 | '(' cnum ',' cnum DDOT cnum ',' '*' ')'  { $$ = f_new_lc_item($2, $2, $4, $6, 0, LC_ALL); }
 | '(' cnum ',' '*' ',' '*' ')'		    { $$ = f_new_lc_item($2, $2, 0, LC_ALL, 0, LC_ALL); }
 | '(' cnum DDOT cnum ',' '*' ',' '*' ')'   { $$ = f_new_lc_item($2, $4, 0, LC_ALL, 0, LC_ALL); }
 | '(' '*' ',' '*' ',' '*' ')'		    { $$ = f_new_lc_item(0, LC_ALL, 0, LC_ALL, 0, LC_ALL); }
 | '(' cnum ',' cnum ',' cnum ')' DDOT '(' cnum ',' cnum ',' cnum ')'
   { $$ = f_new_lc_item($2, $10, $4, $12, $6, $14); }
;

set_item:
   pair_item
 | ec_item
 | lc_item
 | set_atom { $$ = f_new_item($1, $1); }
 | set_atom DDOT set_atom { $$ = f_new_item($1, $3); }
 ;

switch_item:
   pair_item
 | ec_item
 | lc_item
 | switch_atom { $$ = f_new_item($1, $1); }
 | switch_atom DDOT switch_atom { $$ = f_new_item($1, $3); }
 ;

set_items:
   set_item
 | set_items ',' set_item { $$ = f_merge_items($1, $3); }
 ;

switch_items:
   switch_item
 | switch_items ',' switch_item { $$ = f_merge_items($1, $3); }
 ;

fprefix_s:
   IPA '/' NUM %prec '/' {
     if (($3 < 0) || ($3 > MAX_PREFIX_LENGTH) || !ip_is_prefix($1, $3)) cf_error("Invalid network prefix: %I/%d.", $1, $3);
     $$.type = T_PREFIX; $$.val.px.ip = $1; $$.val.px.len = $3;
   }
 ;

fprefix:
   fprefix_s { $$ = $1; }
 | fprefix_s '+' { $$ = $1; $$.val.px.len |= LEN_PLUS; }
 | fprefix_s '-' { $$ = $1; $$.val.px.len |= LEN_MINUS; }
 | fprefix_s '{' NUM ',' NUM '}' {
     if (! ((0 <= $3) && ($3 <= $5) && ($5 <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", $3, $5);
     $$ = $1; $$.val.px.len |= LEN_RANGE | ($3 << 16) | ($5 << 8);
   }
 ;

fprefix_set:
   fprefix { $$ = f_new_trie(cfg_mem, sizeof(struct f_trie_node)); trie_add_fprefix($$, &($1.val.px)); }
 | fprefix_set ',' fprefix { $$ = $1; trie_add_fprefix($$, &($3.val.px)); }
 ;

switch_body: /* EMPTY */ { $$ = NULL; }
 | switch_body switch_items ':' cmds  {
     /* Fill data fields */
     struct f_tree *t;
     for (t = $2; t; t = t->left)
       t->data = $4;
     $$ = f_merge_items($1, $2);
   }
 | switch_body ELSECOL cmds {
     struct f_tree *t = f_new_tree();
     t->from.type = t->to.type = T_VOID;
     t->right = t;
     t->data = $3;
     $$ = f_merge_items($1, t);
 }
 ;

/* CONST '(' expr ')' { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_INT; $$->a2.i = $3; } */

bgp_path_expr:
   symbol       { $$ = $1; }
 | '(' term ')' { $$ = $2; }
 ;

bgp_path:
   PO  bgp_path_tail1 PC  { $$ = $2; }
 | '/' bgp_path_tail2 '/' { $$ = $2; }
 ;

bgp_path_tail1:
   NUM bgp_path_tail1		{ $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN; $$->val = $1; }
 | NUM DDOT NUM bgp_path_tail1	{ $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $4; $$->kind = PM_ASN_RANGE; $$->val = $1; $$->val2 = $3; }
 | '*' bgp_path_tail1		{ $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASTERISK; }
 | '?' bgp_path_tail1		{ $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_QUESTION; }
 | bgp_path_expr bgp_path_tail1	{ $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN_EXPR; $$->val = (uintptr_t) $1; }
 | 				{ $$ = NULL; }
 ;

bgp_path_tail2:
   NUM bgp_path_tail2 { $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN; $$->val = $1; }
 | '?' bgp_path_tail2 { $$ = cfg_allocz(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASTERISK; }
 | 		      { $$ = NULL; }
 ;

constant:
   NUM    { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_INT;  $$->a2.i = $1; }
 | TRUE   { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_BOOL; $$->a2.i = 1;  }
 | FALSE  { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_BOOL; $$->a2.i = 0;  }
 | TEXT   { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_STRING; $$->a2.p = $1; }
 | fipa	   { NEW_F_VAL; $$ = f_new_inst(); $$->code = 'C'; $$->a1.p = val; *val = $1; }
 | fprefix_s {NEW_F_VAL; $$ = f_new_inst(); $$->code = 'C'; $$->a1.p = val; *val = $1; }
 | RTRID  { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_QUAD;  $$->a2.i = $1; }
 | '[' set_items ']' { DBG( "We've got a set here..." ); $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_SET; $$->a2.p = build_tree($2); DBG( "ook\n" ); }
 | '[' fprefix_set ']' { $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_PREFIX_SET;  $$->a2.p = $2; }
 | ENUM	  { $$ = f_new_inst(); $$->code = 'c'; $$->aux = $1 >> 16; $$->a2.i = $1 & 0xffff; }
 | bgp_path { NEW_F_VAL; $$ = f_new_inst(); $$->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = $1; $$->a1.p = val; }
 ;

constructor:
   '(' term ',' term ')' { $$ = f_generate_dpair($2, $4); }
 | '(' ec_kind ',' term ',' term ')' { $$ = f_generate_ec($2, $4, $6); }
 | '(' term ',' term ',' term ')' { $$ = f_generate_lc($2, $4, $6); }
 ;


/*
 *  Maybe there are no dynamic attributes defined by protocols.
 *  For such cases, we force the dynamic_attr list to contain
 *  at least an invalid token, so it is syntantically correct.
 */
CF_ADDTO(dynamic_attr, INVALID_TOKEN { $$ = NULL; })

rtadot: /* EMPTY, we are not permitted RTA. prefix */
 ;

function_call:
   SYM '(' var_list ')' {
     struct symbol *sym;
     struct f_inst *inst = $3;
     if ($1->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", $1->name);
     $$ = f_new_inst();
     $$->code = P('c','a');
     $$->a1.p = inst;
     $$->a2.p = $1->def;
     sym = $1->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", $1->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
 ;

symbol:
   SYM {
     $$ = f_new_inst();

     switch ($1->class & 0xff00) {
       case SYM_CONSTANT: $$->code = 'C'; break;
       case SYM_VARIABLE: $$->code = 'V'; break;
       default: cf_error("%s: variable expected.", $1->name);
     }

     $$->a1.p = $1->def;
     $$->a2.p = $1->name;
   }

static_attr:
   FROM    { $$ = f_new_inst(); $$->aux = T_IP;         $$->a2.i = SA_FROM;	$$->a1.i = 1; }
 | GW      { $$ = f_new_inst(); $$->aux = T_IP;         $$->a2.i = SA_GW;	$$->a1.i = 1; }
 | NET     { $$ = f_new_inst(); $$->aux = T_PREFIX;     $$->a2.i = SA_NET; }
 | PROTO   { $$ = f_new_inst(); $$->aux = T_STRING;     $$->a2.i = SA_PROTO; }
 | SOURCE  { $$ = f_new_inst(); $$->aux = T_ENUM_RTS;   $$->a2.i = SA_SOURCE; }
 | SCOPE   { $$ = f_new_inst(); $$->aux = T_ENUM_SCOPE; $$->a2.i = SA_SCOPE;	$$->a1.i = 1; }
 | CAST    { $$ = f_new_inst(); $$->aux = T_ENUM_RTC;   $$->a2.i = SA_CAST; }
 | DEST    { $$ = f_new_inst(); $$->aux = T_ENUM_RTD;   $$->a2.i = SA_DEST;	$$->a1.i = 1; }
 | IFNAME  { $$ = f_new_inst(); $$->aux = T_STRING;     $$->a2.i = SA_IFNAME; }
 | IFINDEX { $$ = f_new_inst(); $$->aux = T_INT;        $$->a2.i = SA_IFINDEX; }
 ;

term:
   '(' term ')'      { $$ = $2; }
 | term '+' term     { $$ = f_new_inst(); $$->code = '+';        $$->a1.p = $1; $$->a2.p = $3; }
 | term '-' term     { $$ = f_new_inst(); $$->code = '-';        $$->a1.p = $1; $$->a2.p = $3; }
 | term '*' term     { $$ = f_new_inst(); $$->code = '*';        $$->a1.p = $1; $$->a2.p = $3; }
 | term '/' term     { $$ = f_new_inst(); $$->code = '/';        $$->a1.p = $1; $$->a2.p = $3; }
 | term AND term     { $$ = f_new_inst(); $$->code = '&';        $$->a1.p = $1; $$->a2.p = $3; }
 | term OR  term     { $$ = f_new_inst(); $$->code = '|';        $$->a1.p = $1; $$->a2.p = $3; }
 | term '=' term     { $$ = f_new_inst(); $$->code = P('=','='); $$->a1.p = $1; $$->a2.p = $3; }
 | term NEQ term { $$ = f_new_inst(); $$->code = P('!','=');     $$->a1.p = $1; $$->a2.p = $3; }
 | term '<' term     { $$ = f_new_inst(); $$->code = '<';        $$->a1.p = $1; $$->a2.p = $3; }
 | term LEQ term { $$ = f_new_inst(); $$->code = P('<','=');     $$->a1.p = $1; $$->a2.p = $3; }
 | term '>' term     { $$ = f_new_inst(); $$->code = '<';        $$->a1.p = $3; $$->a2.p = $1; }
 | term GEQ term { $$ = f_new_inst(); $$->code = P('<','=');     $$->a1.p = $3; $$->a2.p = $1; }
 | term '~' term     { $$ = f_new_inst(); $$->code = '~';        $$->a1.p = $1; $$->a2.p = $3; }
 | term NMA term { $$ = f_new_inst(); $$->code = P('!','~');     $$->a1.p = $1; $$->a2.p = $3; }
 | '!' term { $$ = f_new_inst(); $$->code = '!'; $$->a1.p = $2; }
 | DEFINED '(' term ')' { $$ = f_new_inst(); $$->code = P('d','e');  $$->a1.p = $3; }

 | symbol   { $$ = $1; }
 | constant { $$ = $1; }
 | constructor { $$ = $1; }

 | PREFERENCE { $$ = f_new_inst(); $$->code = 'P'; }

 | rtadot static_attr { $$ = $2; $$->code = 'a'; }

 | rtadot dynamic_attr { $$ = $2; $$->code = P('e','a'); }

 | term '.' IP { $$ = f_new_inst(); $$->code = P('c','p'); $$->a1.p = $1; $$->aux = T_IP; }
 | term '.' LEN { $$ = f_new_inst(); $$->code = 'L'; $$->a1.p = $1; }
 | term '.' MASK '(' term ')' { $$ = f_new_inst(); $$->code = P('i','M'); $$->a1.p = $1; $$->a2.p = $5; }
 | term '.' FIRST { $$ = f_new_inst(); $$->code = P('a','f'); $$->a1.p = $1; }
 | term '.' LAST  { $$ = f_new_inst(); $$->code = P('a','l'); $$->a1.p = $1; }
 | term '.' LAST_NONAGGREGATED  { $$ = f_new_inst(); $$->code = P('a','L'); $$->a1.p = $1; }

/* Communities */
/* This causes one shift/reduce conflict
 | rtadot dynamic_attr '.' ADD '(' term ')' { }
 | rtadot dynamic_attr '.' DELETE '(' term ')' { }
 | rtadot dynamic_attr '.' CONTAINS '(' term ')' { }
 | rtadot dynamic_attr '.' RESET{ }
*/

 | '+' EMPTY '+' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_PATH; }
 | '-' EMPTY '-' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_CLIST; }
 | '-' '-' EMPTY '-' '-' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_ECLIST; }
 | '-' '-' '-' EMPTY '-' '-' '-' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_LCLIST; }
 | PREPEND '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('A','p'); $$->a1.p = $3; $$->a2.p = $5; }
 | ADD '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('C','a'); $$->a1.p = $3; $$->a2.p = $5; $$->aux = 'a'; }
 | DELETE '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('C','a'); $$->a1.p = $3; $$->a2.p = $5; $$->aux = 'd'; }
 | FILTER '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('C','a'); $$->a1.p = $3; $$->a2.p = $5; $$->aux = 'f'; }

 | ROA_CHECK '(' SYM ')' { $$ = f_generate_roa_check($3, NULL, NULL); }
 | ROA_CHECK '(' SYM ',' term ',' term ')' { $$ = f_generate_roa_check($3, $5, $7); }

/* | term '.' LEN { $$->code = P('P','l'); } */

/* function_call is inlined here */
 | SYM '(' var_list ')' {
     struct symbol *sym;
     struct f_inst *inst = $3;
     if ($1->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", $1->name);
     $$ = f_new_inst();
     $$->code = P('c','a');
     $$->a1.p = inst;
     $$->a2.p = $1->def;
     sym = $1->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", $1->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
 ;

break_command:
   QUITBIRD { $$ = F_QUITBIRD; }
 | ACCEPT { $$ = F_ACCEPT; }
 | REJECT { $$ = F_REJECT; }
 | ERROR { $$ = F_ERROR; }
 | PRINT { $$ = F_NOP; }
 | PRINTN { $$ = F_NONL; }
 ;

print_one:
   term { $$ = f_new_inst(); $$->code = 'p'; $$->a1.p = $1; $$->a2.p = NULL; }
 ;

print_list: /* EMPTY */ { $$ = NULL; }
 | print_one { $$ = $1; }
 | print_one ',' print_list {
     if ($1) {
       $1->next = $3;
       $$ = $1;
     } else $$ = $3;
   }
 ;

var_listn: term {
     $$ = f_new_inst();
     $$->code = 's';
     $$->a1.p = NULL;
     $$->a2.p = $1;
     $$->next = NULL;
   }
 | term ',' var_listn {
     $$ = f_new_inst();
     $$->code = 's';
     $$->a1.p = NULL;
     $$->a2.p = $1;
     $$->next = $3;
   }
 ;

var_list: /* EMPTY */ { $$ = NULL; }
 | var_listn { $$ = $1; }
 ;

cmd:
   IF term THEN block {
     $$ = f_new_inst();
     $$->code = '?';
     $$->a1.p = $2;
     $$->a2.p = $4;
   }
 | IF term THEN block ELSE block {
     struct f_inst *i = f_new_inst();
     i->code = '?';
     i->a1.p = $2;
     i->a2.p = $4;
     $$ = f_new_inst();
     $$->code = '?';
     $$->a1.p = i;
     $$->a2.p = $6;
   }
 | SYM '=' term ';' {
     $$ = f_new_inst();
     DBG( "Ook, we'll set value\n" );
     if (($1->class & ~T_MASK) != SYM_VARIABLE)
       cf_error( "You may set only variables." );
     $$->code = 's';
     $$->a1.p = $1;
     $$->a2.p = $3;
   }
 | RETURN term ';' {
     $$ = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     $$->code = 'r';
     $$->a1.p = $2;
   }
 | rtadot dynamic_attr '=' term ';' {
     $$ = $2;
     $$->code = P('e','S');
     $$->a1.p = $4;
   }
 | rtadot static_attr '=' term ';' {
     $$ = $2;
     if (!$$->a1.i)
       cf_error( "This static attribute is read-only.");
     $$->code = P('a','S');
     $$->a1.p = $4;
   }
 | PREFERENCE '=' term ';' {
     $$ = f_new_inst();
     $$->code = P('P','S');
     $$->a1.p = $3;
   }
 | UNSET '(' rtadot dynamic_attr ')' ';' {
     $$ = $4;
     $$->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     $$->code = P('e','S');
     $$->a1.p = NULL;
   }
 | break_command print_list ';' { $$ = f_new_inst(); $$->code = P('p',','); $$->a1.p = $2; $$->a2.i = $1; }
 | function_call ';' { $$ = $1; }
 | CASE term '{' switch_body '}' {
      $$ = f_new_inst();
      $$->code = P('S','W');
      $$->a1.p = $2;
      $$->a2.p = build_tree( $4 );
   }


 | rtadot dynamic_attr '.' EMPTY ';' { $$ = f_generate_empty($2); }
 | rtadot dynamic_attr '.' PREPEND '(' term ')' ';'   { $$ = f_generate_complex( P('A','p'), 'x', $2, $6 ); }
 | rtadot dynamic_attr '.' ADD '(' term ')' ';'       { $$ = f_generate_complex( P('C','a'), 'a', $2, $6 ); }
 | rtadot dynamic_attr '.' DELETE '(' term ')' ';'    { $$ = f_generate_complex( P('C','a'), 'd', $2, $6 ); }
 | rtadot dynamic_attr '.' FILTER '(' term ')' ';'    { $$ = f_generate_complex( P('C','a'), 'f', $2, $6 ); }
 ;

CF_END
