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

#define P(a,b) ((a<<8) | b)

static int make_pair(int i1, int i2)
{
  unsigned u1 = i1;
  unsigned u2 = i2;

  if ((u1 > 0xFFFF) || (u2 > 0xFFFF))
    cf_error( "Can't operate with value out of bounds in pair constructor");

  return (u1 << 16) | u2;
}

CF_DECLS

CF_KEYWORDS(FUNCTION, PRINT, PRINTN, UNSET, RETURN,
	ACCEPT, REJECT, ERROR, QUITBIRD,
	INT, BOOL, IP, PREFIX, PAIR, QUAD, SET, STRING, BGPMASK, BGPPATH, CLIST,
	IF, THEN, ELSE, CASE,
	TRUE, FALSE,
	FROM, GW, NET, MASK, PROTO, SOURCE, SCOPE, CAST, DEST, PREFERENCE,
	LEN,
	DEFINED,
	ADD, DELETE, CONTAINS, RESET,
	PREPEND, FIRST, LAST, MATCH,
	EMPTY,
	FILTER, WHERE, EVAL)

%nonassoc THEN
%nonassoc ELSE

%type <x> term block cmds cmds_int cmd function_body constant print_one print_list var_list var_listn dynamic_attr static_attr function_call symbol dpair bgp_path_expr
%type <f> filter filter_body where_filter
%type <i> type break_command cpair
%type <e> set_item set_items switch_body
%type <trie> fprefix_set
%type <v> set_atom fprefix fprefix_s fipa
%type <s> decls declsn one_decl function_params 
%type <h> bgp_path bgp_path_tail1 bgp_path_tail2

CF_GRAMMAR

CF_ADDTO(conf, filter_def)
filter_def:
   FILTER SYM { cf_push_scope( $2 ); } filter_body {
     $2 = cf_define_symbol($2, SYM_FILTER, $4);
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
 | STRING { $$ = T_STRING; }
 | BGPMASK { $$ = T_PATH_MASK; }
 | BGPPATH { $$ = T_PATH; }
 | CLIST { $$ = T_CLIST; }
 | type SET { 
	switch ($1) {
	  case T_INT:
	  case T_PAIR:
	  case T_QUAD:
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
     val->type = $1; 
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
 | declsn ';' one_decl {
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
 * Simple types, their bison value is int
 */
cpair:
   '(' NUM ',' NUM ')' { $$ = make_pair($2, $4); }
 ;

/*
 * Complex types, their bison value is struct f_val
 */
fipa:
   IPA %prec PREFIX_DUMMY { $$.type = T_IP; $$.val.px.ip = $1; }
 ;

set_atom:
   NUM   { $$.type = T_INT; $$.val.i = $1; }
 | RTRID { $$.type = T_QUAD; $$.val.i = $1; }
 | cpair { $$.type = T_PAIR; $$.val.i = $1; }
 | fipa  { $$ = $1; }
 | ENUM  {  $$.type = $1 >> 16; $$.val.i = $1 & 0xffff; }
 ; 

set_item:
   '(' NUM ',' '*' ')' { 
	$$ = f_new_tree(); 
	$$->from.type = $$->to.type = T_PAIR;
	$$->from.val.i = make_pair($2, 0); 
	$$->to.val.i = make_pair($2, 0xffff);
   }
 | set_atom { 
	$$ = f_new_tree(); 
	$$->from = $1; 
	$$->to = $1;
   }
 | set_atom '.' '.' set_atom { 
	$$ = f_new_tree(); 
	$$->from = $1; 
	$$->to = $4; 
   }
 ;

set_items:
   set_item { $$ = $1; }
 | set_items ',' set_item { $$ = $3; $$->left = $1; }
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
   fprefix { $$ = f_new_trie(); trie_add_prefix($$, &($1.val.px)); }
 | fprefix_set ',' fprefix { $$ = $1; trie_add_prefix($$, &($3.val.px)); }
 ;

switch_body: /* EMPTY */ { $$ = NULL; }
 | switch_body set_item ':' cmds  {
     $$ = $2;
     $$->data = $4;
     $$->left = $1;
   }
 | switch_body ELSE ':' cmds {
     $$ = f_new_tree(); 
     $$->from.type = T_VOID; 
     $$->to.type = T_VOID;
     $$->data = $4;
     $$->left = $1;
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
   NUM bgp_path_tail1 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN;      $$->val = $1; }
 | '*' bgp_path_tail1 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASTERISK; $$->val  = 0; }
 | '?' bgp_path_tail1 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_QUESTION; $$->val  = 0; }
 | bgp_path_expr bgp_path_tail1 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN_EXPR; $$->val = (uintptr_t) $1; }
 |  		      { $$ = NULL; }
 ;

bgp_path_tail2:
   NUM bgp_path_tail2 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASN;      $$->val = $1; }
 | '?' bgp_path_tail2 { $$ = cfg_alloc(sizeof(struct f_path_mask)); $$->next = $2; $$->kind = PM_ASTERISK; $$->val  = 0; }
 | 		      { $$ = NULL; }
 ;

dpair:
   '(' term ',' term ')' {
        if (($2->code == 'c') && ($4->code == 'c'))
          { 
            if (($2->aux != T_INT) || ($4->aux != T_INT))
              cf_error( "Can't operate with value of non-integer type in pair constructor" );
            $$ = f_new_inst(); $$->code = 'c'; $$->aux = T_PAIR;  $$->a2.i = make_pair($2->a2.i, $4->a2.i);
          }
	else
	  { $$ = f_new_inst(); $$->code = P('m', 'p'); $$->a1.p = $2; $$->a2.p = $4; }
    }
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


/*
 *  Maybe there are no dynamic attributes defined by protocols.
 *  For such cases, we force the dynamic_attr list to contain
 *  at least an invalid token, so it's syntantically correct.
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
     switch ($1->class) {
       case SYM_NUMBER:
	$$ = f_new_inst();
	$$->code = 'c'; 
	$$->aux = T_INT; 
	$$->a2.i = $1->aux;
	break;
       case SYM_IPA:
	{ NEW_F_VAL; $$ = f_new_inst(); $$->code = 'C'; $$->a1.p = val; val->type = T_IP; val->val.px.ip = * (ip_addr *) ($1->def); }
	break;
       case SYM_VARIABLE | T_BOOL:
       case SYM_VARIABLE | T_INT:
       case SYM_VARIABLE | T_PAIR:
       case SYM_VARIABLE | T_QUAD:
       case SYM_VARIABLE | T_STRING:
       case SYM_VARIABLE | T_IP:
       case SYM_VARIABLE | T_PREFIX:
       case SYM_VARIABLE | T_PREFIX_SET:
       case SYM_VARIABLE | T_SET:
       case SYM_VARIABLE | T_PATH:
       case SYM_VARIABLE | T_PATH_MASK:
       case SYM_VARIABLE | T_CLIST:
	 $$->code = 'V';
	 $$->a1.p = $1->def;
	 $$->a2.p = $1->name;
	 break;
       default:
	 cf_error("%s: variable expected.", $1->name );
     }
   }

static_attr:
   FROM    { $$ = f_new_inst(); $$->aux = T_IP;         $$->a2.i = OFFSETOF(struct rta, from);   $$->a1.i = 1; }

 | GW      { $$ = f_new_inst(); $$->aux = T_IP;         $$->a2.i = OFFSETOF(struct rta, gw);     $$->a1.i = 1; }
 | NET     { $$ = f_new_inst(); $$->aux = T_PREFIX;     $$->a2.i = 0x12345678; /* This is actually ok - T_PREFIX is special-cased. */ }
 | PROTO   { $$ = f_new_inst(); $$->aux = T_STRING;     $$->a2.i = 0x12345678; /* T_STRING is also special-cased. */ }
 | SOURCE  { $$ = f_new_inst(); $$->aux = T_ENUM_RTS;   $$->a2.i = OFFSETOF(struct rta, source); }
 | SCOPE   { $$ = f_new_inst(); $$->aux = T_ENUM_SCOPE; $$->a2.i = OFFSETOF(struct rta, scope);  $$->a1.i = 1; }
 | CAST    { $$ = f_new_inst(); $$->aux = T_ENUM_RTC;   $$->a2.i = OFFSETOF(struct rta, cast); }
 | DEST    { $$ = f_new_inst(); $$->aux = T_ENUM_RTD;   $$->a2.i = OFFSETOF(struct rta, dest); }
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
 | '!' term { $$ = f_new_inst(); $$->code = '!'; $$->a1.p = $2; }
 | DEFINED '(' term ')' { $$ = f_new_inst(); $$->code = P('d','e');  $$->a1.p = $3; }

 | symbol   { $$ = $1; }
 | constant { $$ = $1; }
 | dpair    { $$ = $1; }

 | PREFERENCE { $$ = f_new_inst(); $$->code = 'P'; }

 | rtadot static_attr { $$ = $2; $$->code = 'a'; }

 | rtadot dynamic_attr { $$ = $2; $$->code = P('e','a'); }

 | term '.' IP { $$ = f_new_inst(); $$->code = P('c','p'); $$->a1.p = $1; $$->aux = T_IP; }
 | term '.' LEN { $$ = f_new_inst(); $$->code = 'L'; $$->a1.p = $1; }
 | term '.' MASK '(' term ')' { $$ = f_new_inst(); $$->code = P('i','M'); $$->a1.p = $1; $$->a2.p = $5; }
 | term '.' FIRST { $$ = f_new_inst(); $$->code = P('a','f'); $$->a1.p = $1; }
 | term '.' LAST  { $$ = f_new_inst(); $$->code = P('a','l'); $$->a1.p = $1; }

/* Communities */
/* This causes one shift/reduce conflict
 | rtadot dynamic_attr '.' ADD '(' term ')' { }
 | rtadot dynamic_attr '.' DELETE '(' term ')' { }
 | rtadot dynamic_attr '.' CONTAINS '(' term ')' { }
 | rtadot dynamic_attr '.' RESET{ }
*/

 | '+' EMPTY '+' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_PATH; }
 | '-' EMPTY '-' { $$ = f_new_inst(); $$->code = 'E'; $$->aux = T_CLIST; }
 | PREPEND '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('A','p'); $$->a1.p = $3; $$->a2.p = $5; } 
 | ADD '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('C','a'); $$->a1.p = $3; $$->a2.p = $5; $$->aux = 'a'; } 
 | DELETE '(' term ',' term ')' { $$ = f_new_inst(); $$->code = P('C','a'); $$->a1.p = $3; $$->a2.p = $5; $$->aux = 'd'; }

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


 | rtadot dynamic_attr '.' EMPTY ';' 
  { struct f_inst *i = f_new_inst(); i->code = 'E'; i->aux = T_CLIST; $$ = $2; $$->code = P('e','S'); $$->a1.p = i; }
 | rtadot dynamic_attr '.' PREPEND '(' term ')' ';'   { $$ = f_generate_complex( P('A','p'), 'x', $2, $6 ); }
 | rtadot dynamic_attr '.' ADD '(' term ')' ';'       { $$ = f_generate_complex( P('C','a'), 'a', $2, $6 ); } 
 | rtadot dynamic_attr '.' DELETE '(' term ')' ';'    { $$ = f_generate_complex( P('C','a'), 'd', $2, $6 ); } 
 ;

CF_END
