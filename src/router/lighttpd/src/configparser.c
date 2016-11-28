/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include "first.h"
#include <stdio.h>
#line 5 "../../src/configparser.y"

#include "first.h"
#include "configfile.h"
#include "buffer.h"
#include "array.h"
#include "request.h" /* http_request_host_normalize() */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void configparser_push(config_t *ctx, data_config *dc, int isnew) {
  if (isnew) {
    dc->context_ndx = ctx->all_configs->used;
    force_assert(dc->context_ndx > ctx->current->context_ndx);
    array_insert_unique(ctx->all_configs, (data_unset *)dc);
    dc->parent = ctx->current;
    vector_config_weak_push(&dc->parent->children, dc);
  }
  if (ctx->configs_stack.used > 0 && ctx->current->context_ndx == 0) {
    fprintf(stderr, "Cannot use conditionals inside a global { ... } block\n");
    exit(-1);
  }
  vector_config_weak_push(&ctx->configs_stack, ctx->current);
  ctx->current = dc;
}

static data_config *configparser_pop(config_t *ctx) {
  data_config *old = ctx->current;
  ctx->current = vector_config_weak_pop(&ctx->configs_stack);
  return old;
}

/* return a copied variable */
static data_unset *configparser_get_variable(config_t *ctx, const buffer *key) {
  data_unset *du;
  data_config *dc;

#if 0
  fprintf(stderr, "get var %s\n", key->ptr);
#endif
  for (dc = ctx->current; dc; dc = dc->parent) {
#if 0
    fprintf(stderr, "get var on block: %s\n", dc->key->ptr);
    array_print(dc->value, 0);
#endif
    if (NULL != (du = array_get_element(dc->value, key->ptr))) {
      return du->copy(du);
    }
  }
  return NULL;
}

/* op1 is to be eat/return by this function if success, op1->key is not cared
   op2 is left untouch, unreferenced
 */
data_unset *configparser_merge_data(data_unset *op1, const data_unset *op2) {
  /* type mismatch */
  if (op1->type != op2->type) {
    if (op1->type == TYPE_STRING && op2->type == TYPE_INTEGER) {
      data_string *ds = (data_string *)op1;
      buffer_append_int(ds->value, ((data_integer*)op2)->value);
      return op1;
    } else if (op1->type == TYPE_INTEGER && op2->type == TYPE_STRING) {
      data_string *ds = data_string_init();
      buffer_append_int(ds->value, ((data_integer*)op1)->value);
      buffer_append_string_buffer(ds->value, ((data_string*)op2)->value);
      op1->free(op1);
      return (data_unset *)ds;
    } else {
      fprintf(stderr, "data type mismatch, cannot merge\n");
      op1->free(op1);
      return NULL;
    }
  }

  switch (op1->type) {
    case TYPE_STRING:
      buffer_append_string_buffer(((data_string *)op1)->value, ((data_string *)op2)->value);
      break;
    case TYPE_INTEGER:
      ((data_integer *)op1)->value += ((data_integer *)op2)->value;
      break;
    case TYPE_ARRAY: {
      array *dst = ((data_array *)op1)->value;
      array *src = ((data_array *)op2)->value;
      data_unset *du;
      size_t i;

      for (i = 0; i < src->used; i ++) {
        du = (data_unset *)src->data[i];
        if (du) {
          if (du->is_index_key || buffer_is_empty(du->key) || !array_get_element(dst, du->key->ptr)) {
            array_insert_unique(dst, du->copy(du));
          } else {
            fprintf(stderr, "Duplicate array-key '%s'\n", du->key->ptr);
            op1->free(op1);
            return NULL;
          }
        }
      }
      break;
    default:
      force_assert(0);
      break;
    }
  }
  return op1;
}

static int configparser_remoteip_normalize_compat(buffer *rvalue) {
  /* $HTTP["remoteip"] IPv6 accepted with or without '[]' for config compat
   * http_request_host_normalize() expects IPv6 with '[]',
   * and config processing at runtime expects COMP_HTTP_REMOTE_IP
   * compared without '[]', so strip '[]' after normalization */
  buffer *b = buffer_init();
  int rc;

  if (rvalue->ptr[0] != '[') {
      buffer_append_string_len(b, CONST_STR_LEN("["));
      buffer_append_string_buffer(b, rvalue);
      buffer_append_string_len(b, CONST_STR_LEN("]"));
  } else {
      buffer_append_string_buffer(b, rvalue);
  }

  rc = http_request_host_normalize(b);

  if (0 == rc) {
    /* remove surrounding '[]' */
    size_t blen = buffer_string_length(b);
    if (blen > 1) buffer_copy_string_len(rvalue, b->ptr+1, blen-2);
  }

  buffer_free(b);
  return rc;
}


#line 151 "configparser.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/*
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands.
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    configparserTOKENTYPE     is the data type used for minor tokens given
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is configparserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    configparserARG_SDECL     A static variable declaration for the %extra_argument
**    configparserARG_PDECL     A parameter declaration for the %extra_argument
**    configparserARG_STORE     Code to store %extra_argument into yypParser
**    configparserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
/*  */
#define YYCODETYPE unsigned char
#define YYNOCODE 50
#define YYACTIONTYPE unsigned char
#define configparserTOKENTYPE buffer *
typedef union {
  configparserTOKENTYPE yy0;
  data_config * yy2;
  array * yy22;
  data_unset * yy59;
  config_cond_t yy75;
  buffer * yy87;
  int yy99;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define configparserARG_SDECL config_t *ctx;
#define configparserARG_PDECL ,config_t *ctx
#define configparserARG_FETCH config_t *ctx = yypParser->ctx
#define configparserARG_STORE yypParser->ctx = ctx
#define YYNSTATE 68
#define YYNRULE 43
#define YYERRORSYMBOL 26
#define YYERRSYMDT yy99
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static YYACTIONTYPE yy_action[] = {
 /*     0 */     2,    3,    4,    5,   14,   15,  112,    1,    7,   46,
 /*    10 */    68,   16,  103,   17,   21,   47,   16,   42,   11,   21,
 /*    20 */    18,   39,   41,    9,   10,   12,   47,   41,   93,   63,
 /*    30 */    13,   47,   11,   64,   59,   61,   29,   28,   38,   59,
 /*    40 */    61,   31,   26,   23,   35,   16,   20,    8,   21,   36,
 /*    50 */    16,   20,  108,   21,   32,   33,   41,   20,   45,  102,
 /*    60 */    47,   41,   44,   67,   48,   47,   95,   52,   59,   61,
 /*    70 */    29,   28,   34,   59,   61,   43,   26,   23,   35,   21,
 /*    80 */    53,   24,   25,   27,   30,   49,   29,   50,    6,   29,
 /*    90 */    50,   65,   26,   23,   51,   26,   23,   60,   66,   29,
 /*   100 */    50,   94,   94,   94,   94,   26,   23,   62,   94,   94,
 /*   110 */    21,   94,   24,   25,   27,   29,   19,   29,   37,   29,
 /*   120 */    40,   26,   23,   26,   23,   26,   23,   55,   56,   57,
 /*   130 */    58,   29,   54,   22,   94,   94,   94,   26,   23,   26,
 /*   140 */    23,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    29,   30,   31,   32,   33,   34,   27,   28,   45,   38,
 /*    10 */     0,    1,   13,   42,    4,   16,    1,   46,   47,    4,
 /*    20 */     2,    3,   12,   38,   39,   13,   16,   12,   15,   14,
 /*    30 */    28,   16,   47,   48,   24,   25,   35,   36,   37,   24,
 /*    40 */    25,   40,   41,   42,   43,    1,    5,   15,    4,   11,
 /*    50 */     1,    5,   11,    4,    9,   10,   12,    5,   14,   13,
 /*    60 */    16,   12,   28,   14,   17,   16,   13,   19,   24,   25,
 /*    70 */    35,   36,   37,   24,   25,   13,   41,   42,   43,    4,
 /*    80 */    44,    6,    7,    8,    9,   18,   35,   36,    1,   35,
 /*    90 */    36,   13,   41,   42,   43,   41,   42,   43,   28,   35,
 /*   100 */    36,   49,   15,   49,   49,   41,   42,   43,   49,   49,
 /*   110 */     4,   49,    6,    7,    8,   35,   36,   35,   36,   35,
 /*   120 */    36,   41,   42,   41,   42,   41,   42,   20,   21,   22,
 /*   130 */    23,   35,   36,   35,   49,   49,   49,   41,   42,   41,
 /*   140 */    42,
};
#define YY_SHIFT_USE_DFLT (-2)
static signed char yy_shift_ofst[] = {
 /*     0 */    -2,   10,   -2,   -2,   -2,   87,   13,   32,   -1,   -2,
 /*    10 */    -2,   12,   -2,   15,   -2,   -2,   -2,   18,  106,   52,
 /*    20 */   106,   -2,   -2,   -2,   -2,   -2,   -2,   75,   41,   -2,
 /*    30 */    -2,   45,   -2,  106,   -2,   38,  106,   52,   -2,  106,
 /*    40 */    52,   53,   62,   -2,   44,   -2,   -2,   47,   67,  106,
 /*    50 */    52,   48,  107,  106,   46,   -2,   -2,   -2,   -2,  106,
 /*    60 */    -2,  106,   -2,   -2,   78,   -2,   49,   -2,
};
#define YY_REDUCE_USE_DFLT (-38)
static signed char yy_reduce_ofst[] = {
 /*     0 */   -21,  -29,  -38,  -38,  -38,  -37,  -38,  -38,  -15,  -38,
 /*    10 */   -38,  -38,    2,  -29,  -38,  -38,  -38,  -38,   80,  -38,
 /*    20 */    98,  -38,  -38,  -38,  -38,  -38,  -38,    1,  -38,  -38,
 /*    30 */   -38,  -38,  -38,   35,  -38,  -38,   82,  -38,  -38,   84,
 /*    40 */   -38,  -38,  -38,   34,  -29,  -38,  -38,  -38,  -38,   51,
 /*    50 */   -38,  -38,   36,   96,  -38,  -38,  -38,  -38,  -38,   54,
 /*    60 */   -38,   64,  -38,  -38,  -38,   70,  -29,  -38,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */    70,  111,   69,   71,   72,  111,   73,  111,  111,   97,
 /*    10 */    98,  111,   70,  111,   74,   75,   76,  111,  111,   77,
 /*    20 */   111,   79,   80,   82,   83,   84,   85,  111,   91,   81,
 /*    30 */    86,  111,   87,   89,   88,  111,  111,   92,   90,  111,
 /*    40 */    78,  111,  111,   70,  111,   96,   99,  111,  111,  111,
 /*    50 */   108,  111,  111,  111,  111,  104,  105,  106,  107,  111,
 /*    60 */   109,  111,  110,  100,  111,   70,  111,  101,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
**
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  configparserARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = NULL;
static char *yyTracePrompt = NULL;
#endif /* NDEBUG */

#ifndef NDEBUG
/*
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
#if 0
void configparserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *yyTokenName[] = {
  "$",             "EOL",           "ASSIGN",        "APPEND",      
  "LKEY",          "PLUS",          "STRING",        "INTEGER",     
  "LPARAN",        "RPARAN",        "COMMA",         "ARRAY_ASSIGN",
  "GLOBAL",        "LCURLY",        "RCURLY",        "ELSE",        
  "DOLLAR",        "SRVVARNAME",    "LBRACKET",      "RBRACKET",    
  "EQ",            "MATCH",         "NE",            "NOMATCH",     
  "INCLUDE",       "INCLUDE_SHELL",  "error",         "input",       
  "metalines",     "metaline",      "varline",       "global",      
  "condlines",     "include",       "include_shell",  "value",       
  "expression",    "aelement",      "condline",      "cond_else",   
  "aelements",     "array",         "key",           "stringop",    
  "cond",          "eols",          "globalstart",   "context",     
  "context_else",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *yyRuleName[] = {
 /*   0 */ "input ::= metalines",
 /*   1 */ "metalines ::= metalines metaline",
 /*   2 */ "metalines ::=",
 /*   3 */ "metaline ::= varline",
 /*   4 */ "metaline ::= global",
 /*   5 */ "metaline ::= condlines EOL",
 /*   6 */ "metaline ::= include",
 /*   7 */ "metaline ::= include_shell",
 /*   8 */ "metaline ::= EOL",
 /*   9 */ "varline ::= key ASSIGN expression",
 /*  10 */ "varline ::= key APPEND expression",
 /*  11 */ "key ::= LKEY",
 /*  12 */ "expression ::= expression PLUS value",
 /*  13 */ "expression ::= value",
 /*  14 */ "value ::= key",
 /*  15 */ "value ::= STRING",
 /*  16 */ "value ::= INTEGER",
 /*  17 */ "value ::= array",
 /*  18 */ "array ::= LPARAN RPARAN",
 /*  19 */ "array ::= LPARAN aelements RPARAN",
 /*  20 */ "aelements ::= aelements COMMA aelement",
 /*  21 */ "aelements ::= aelements COMMA",
 /*  22 */ "aelements ::= aelement",
 /*  23 */ "aelement ::= expression",
 /*  24 */ "aelement ::= stringop ARRAY_ASSIGN expression",
 /*  25 */ "eols ::= EOL",
 /*  26 */ "eols ::=",
 /*  27 */ "globalstart ::= GLOBAL",
 /*  28 */ "global ::= globalstart LCURLY metalines RCURLY",
 /*  29 */ "condlines ::= condlines eols ELSE condline",
 /*  30 */ "condlines ::= condlines eols ELSE cond_else",
 /*  31 */ "condlines ::= condline",
 /*  32 */ "condline ::= context LCURLY metalines RCURLY",
 /*  33 */ "cond_else ::= context_else LCURLY metalines RCURLY",
 /*  34 */ "context ::= DOLLAR SRVVARNAME LBRACKET stringop RBRACKET cond expression",
 /*  35 */ "context_else ::=",
 /*  36 */ "cond ::= EQ",
 /*  37 */ "cond ::= MATCH",
 /*  38 */ "cond ::= NE",
 /*  39 */ "cond ::= NOMATCH",
 /*  40 */ "stringop ::= expression",
 /*  41 */ "include ::= INCLUDE stringop",
 /*  42 */ "include_shell ::= INCLUDE_SHELL stringop",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
#if 0
const char *configparserTokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && (size_t)tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}
#endif

/*
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to configparser and configparserFree.
*/
void *configparserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
#line 183 "../../src/configparser.y"
{ buffer_free((yypminor->yy0)); }
#line 571 "configparser.c"
      break;
    case 35:
#line 174 "../../src/configparser.y"
{ if ((yypminor->yy59)) (yypminor->yy59)->free((yypminor->yy59)); }
#line 576 "configparser.c"
      break;
    case 36:
#line 175 "../../src/configparser.y"
{ if ((yypminor->yy59)) (yypminor->yy59)->free((yypminor->yy59)); }
#line 581 "configparser.c"
      break;
    case 37:
#line 176 "../../src/configparser.y"
{ if ((yypminor->yy59)) (yypminor->yy59)->free((yypminor->yy59)); }
#line 586 "configparser.c"
      break;
    case 40:
#line 177 "../../src/configparser.y"
{ array_free((yypminor->yy22)); }
#line 591 "configparser.c"
      break;
    case 41:
#line 178 "../../src/configparser.y"
{ array_free((yypminor->yy22)); }
#line 596 "configparser.c"
      break;
    case 42:
#line 179 "../../src/configparser.y"
{ buffer_free((yypminor->yy87)); }
#line 601 "configparser.c"
      break;
    case 43:
#line 180 "../../src/configparser.y"
{ buffer_free((yypminor->yy87)); }
#line 606 "configparser.c"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos;

  if( pParser->yyidx<0 ) return 0;
  yytos = &pParser->yystack[pParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/*
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from configparserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void configparserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==NULL ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;

  /* if( pParser->yyidx<0 ) return YY_NO_ACTION;  */
  i = yy_shift_ofst[stateno];
  if( i==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || (size_t)i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;

  i = yy_reduce_ofst[stateno];
  if( i==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || (size_t)i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
     configparserARG_FETCH;
     yypParser->yyidx--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     configparserARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 27, 1 },
  { 28, 2 },
  { 28, 0 },
  { 29, 1 },
  { 29, 1 },
  { 29, 2 },
  { 29, 1 },
  { 29, 1 },
  { 29, 1 },
  { 30, 3 },
  { 30, 3 },
  { 42, 1 },
  { 36, 3 },
  { 36, 1 },
  { 35, 1 },
  { 35, 1 },
  { 35, 1 },
  { 35, 1 },
  { 41, 2 },
  { 41, 3 },
  { 40, 3 },
  { 40, 2 },
  { 40, 1 },
  { 37, 1 },
  { 37, 3 },
  { 45, 1 },
  { 45, 0 },
  { 46, 1 },
  { 31, 4 },
  { 32, 4 },
  { 32, 4 },
  { 32, 1 },
  { 38, 4 },
  { 39, 4 },
  { 47, 7 },
  { 48, 0 },
  { 44, 1 },
  { 44, 1 },
  { 44, 1 },
  { 44, 1 },
  { 43, 1 },
  { 33, 2 },
  { 34, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  configparserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE ) {
    if (yyruleno>=0
        && (size_t)yyruleno<sizeof(yyRuleName)/sizeof(yyRuleName[0]) ){
      fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
        yyRuleName[yyruleno]);
    } else {
      return; /*(should not happen)*/
    }
  }
#endif /* NDEBUG */

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
        /* No destructor defined for metalines */
        break;
      case 1:
        /* No destructor defined for metalines */
        /* No destructor defined for metaline */
        break;
      case 2:
        break;
      case 3:
        /* No destructor defined for varline */
        break;
      case 4:
        /* No destructor defined for global */
        break;
      case 5:
#line 156 "../../src/configparser.y"
{ yymsp[-1].minor.yy2 = NULL; }
#line 884 "configparser.c"
  yy_destructor(1,&yymsp[0].minor);
        break;
      case 6:
        /* No destructor defined for include */
        break;
      case 7:
        /* No destructor defined for include_shell */
        break;
      case 8:
  yy_destructor(1,&yymsp[0].minor);
        break;
      case 9:
#line 185 "../../src/configparser.y"
{
  if (ctx->ok) {
    buffer_copy_buffer(yymsp[0].minor.yy59->key, yymsp[-2].minor.yy87);
    if (strncmp(yymsp[-2].minor.yy87->ptr, "env.", sizeof("env.") - 1) == 0) {
      fprintf(stderr, "Setting env variable is not supported in conditional %d %s: %s\n",
          ctx->current->context_ndx,
          ctx->current->key->ptr, yymsp[-2].minor.yy87->ptr);
      ctx->ok = 0;
    } else if (NULL == array_get_element(ctx->current->value, yymsp[0].minor.yy59->key->ptr)) {
      array_insert_unique(ctx->current->value, yymsp[0].minor.yy59);
      yymsp[0].minor.yy59 = NULL;
    } else {
      fprintf(stderr, "Duplicate config variable in conditional %d %s: %s\n",
              ctx->current->context_ndx,
              ctx->current->key->ptr, yymsp[0].minor.yy59->key->ptr);
      ctx->ok = 0;
      yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
      yymsp[0].minor.yy59 = NULL;
    }
  }
  buffer_free(yymsp[-2].minor.yy87);
  yymsp[-2].minor.yy87 = NULL;
}
#line 921 "configparser.c"
  yy_destructor(2,&yymsp[-1].minor);
        break;
      case 10:
#line 209 "../../src/configparser.y"
{
  if (ctx->ok) {
    array *vars = ctx->current->value;
    data_unset *du;

    if (strncmp(yymsp[-2].minor.yy87->ptr, "env.", sizeof("env.") - 1) == 0) {
      fprintf(stderr, "Appending env variable is not supported in conditional %d %s: %s\n",
          ctx->current->context_ndx,
          ctx->current->key->ptr, yymsp[-2].minor.yy87->ptr);
      ctx->ok = 0;
    } else if (NULL != (du = array_extract_element(vars, yymsp[-2].minor.yy87->ptr)) || NULL != (du = configparser_get_variable(ctx, yymsp[-2].minor.yy87))) {
      du = configparser_merge_data(du, yymsp[0].minor.yy59);
      if (NULL == du) {
        ctx->ok = 0;
      }
      else {
        buffer_copy_buffer(du->key, yymsp[-2].minor.yy87);
        array_insert_unique(ctx->current->value, du);
      }
      yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
    } else {
      buffer_copy_buffer(yymsp[0].minor.yy59->key, yymsp[-2].minor.yy87);
      array_insert_unique(ctx->current->value, yymsp[0].minor.yy59);
    }
    buffer_free(yymsp[-2].minor.yy87);
    yymsp[-2].minor.yy87 = NULL;
    yymsp[0].minor.yy59 = NULL;
  }
}
#line 955 "configparser.c"
  yy_destructor(3,&yymsp[-1].minor);
        break;
      case 11:
#line 239 "../../src/configparser.y"
{
  if (strchr(yymsp[0].minor.yy0->ptr, '.') == NULL) {
    yygotominor.yy87 = buffer_init_string("var.");
    buffer_append_string_buffer(yygotominor.yy87, yymsp[0].minor.yy0);
    buffer_free(yymsp[0].minor.yy0);
    yymsp[0].minor.yy0 = NULL;
  } else {
    yygotominor.yy87 = yymsp[0].minor.yy0;
    yymsp[0].minor.yy0 = NULL;
  }
}
#line 971 "configparser.c"
        break;
      case 12:
#line 251 "../../src/configparser.y"
{
  yygotominor.yy59 = NULL;
  if (ctx->ok) {
    yygotominor.yy59 = configparser_merge_data(yymsp[-2].minor.yy59, yymsp[0].minor.yy59);
    if (NULL == yygotominor.yy59) {
      ctx->ok = 0;
    }
    yymsp[-2].minor.yy59 = NULL;
    yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
    yymsp[0].minor.yy59 = NULL;
  }
}
#line 987 "configparser.c"
  yy_destructor(5,&yymsp[-1].minor);
        break;
      case 13:
#line 264 "../../src/configparser.y"
{
  yygotominor.yy59 = yymsp[0].minor.yy59;
  yymsp[0].minor.yy59 = NULL;
}
#line 996 "configparser.c"
        break;
      case 14:
#line 269 "../../src/configparser.y"
{
  yygotominor.yy59 = NULL;
  if (ctx->ok) {
    if (strncmp(yymsp[0].minor.yy87->ptr, "env.", sizeof("env.") - 1) == 0) {
      char *env;

      if (NULL != (env = getenv(yymsp[0].minor.yy87->ptr + 4))) {
        data_string *ds;
        ds = data_string_init();
        buffer_append_string(ds->value, env);
        yygotominor.yy59 = (data_unset *)ds;
      }
      else {
        fprintf(stderr, "Undefined env variable: %s\n", yymsp[0].minor.yy87->ptr + 4);
        ctx->ok = 0;
      }
    } else if (NULL == (yygotominor.yy59 = configparser_get_variable(ctx, yymsp[0].minor.yy87))) {
      fprintf(stderr, "Undefined config variable: %s\n", yymsp[0].minor.yy87->ptr);
      ctx->ok = 0;
    }
    buffer_free(yymsp[0].minor.yy87);
    yymsp[0].minor.yy87 = NULL;
  }
}
#line 1024 "configparser.c"
        break;
      case 15:
#line 294 "../../src/configparser.y"
{
  yygotominor.yy59 = (data_unset *)data_string_init();
  buffer_copy_buffer(((data_string *)(yygotominor.yy59))->value, yymsp[0].minor.yy0);
  buffer_free(yymsp[0].minor.yy0);
  yymsp[0].minor.yy0 = NULL;
}
#line 1034 "configparser.c"
        break;
      case 16:
#line 301 "../../src/configparser.y"
{
  char *endptr;
  yygotominor.yy59 = (data_unset *)data_integer_init();
  errno = 0;
  ((data_integer *)(yygotominor.yy59))->value = strtol(yymsp[0].minor.yy0->ptr, &endptr, 10);
  /* skip trailing whitespace */
  if (endptr != yymsp[0].minor.yy0->ptr) while (isspace(*endptr)) endptr++;
  if (0 != errno || *endptr != '\0') {
    fprintf(stderr, "error parsing number: '%s'\n", yymsp[0].minor.yy0->ptr);
    ctx->ok = 0;
  }
  buffer_free(yymsp[0].minor.yy0);
  yymsp[0].minor.yy0 = NULL;
}
#line 1052 "configparser.c"
        break;
      case 17:
#line 315 "../../src/configparser.y"
{
  yygotominor.yy59 = (data_unset *)data_array_init();
  array_free(((data_array *)(yygotominor.yy59))->value);
  ((data_array *)(yygotominor.yy59))->value = yymsp[0].minor.yy22;
  yymsp[0].minor.yy22 = NULL;
}
#line 1062 "configparser.c"
        break;
      case 18:
#line 321 "../../src/configparser.y"
{
  yygotominor.yy22 = array_init();
}
#line 1069 "configparser.c"
  yy_destructor(8,&yymsp[-1].minor);
  yy_destructor(9,&yymsp[0].minor);
        break;
      case 19:
#line 324 "../../src/configparser.y"
{
  yygotominor.yy22 = yymsp[-1].minor.yy22;
  yymsp[-1].minor.yy22 = NULL;
}
#line 1079 "configparser.c"
  yy_destructor(8,&yymsp[-2].minor);
  yy_destructor(9,&yymsp[0].minor);
        break;
      case 20:
#line 329 "../../src/configparser.y"
{
  yygotominor.yy22 = NULL;
  if (ctx->ok) {
    if (buffer_is_empty(yymsp[0].minor.yy59->key) ||
        NULL == array_get_element(yymsp[-2].minor.yy22, yymsp[0].minor.yy59->key->ptr)) {
      array_insert_unique(yymsp[-2].minor.yy22, yymsp[0].minor.yy59);
      yymsp[0].minor.yy59 = NULL;
    } else {
      fprintf(stderr, "Error: duplicate array-key: %s. Please get rid of the duplicate entry.\n",
              yymsp[0].minor.yy59->key->ptr);
      ctx->ok = 0;
      yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
      yymsp[0].minor.yy59 = NULL;
    }

    yygotominor.yy22 = yymsp[-2].minor.yy22;
    yymsp[-2].minor.yy22 = NULL;
  }
}
#line 1104 "configparser.c"
  yy_destructor(10,&yymsp[-1].minor);
        break;
      case 21:
#line 349 "../../src/configparser.y"
{
  yygotominor.yy22 = yymsp[-1].minor.yy22;
  yymsp[-1].minor.yy22 = NULL;
}
#line 1113 "configparser.c"
  yy_destructor(10,&yymsp[0].minor);
        break;
      case 22:
#line 354 "../../src/configparser.y"
{
  yygotominor.yy22 = NULL;
  if (ctx->ok) {
    yygotominor.yy22 = array_init();
    array_insert_unique(yygotominor.yy22, yymsp[0].minor.yy59);
    yymsp[0].minor.yy59 = NULL;
  }
}
#line 1126 "configparser.c"
        break;
      case 23:
#line 363 "../../src/configparser.y"
{
  yygotominor.yy59 = yymsp[0].minor.yy59;
  yymsp[0].minor.yy59 = NULL;
}
#line 1134 "configparser.c"
        break;
      case 24:
#line 367 "../../src/configparser.y"
{
  yygotominor.yy59 = NULL;
  if (ctx->ok) {
    buffer_copy_buffer(yymsp[0].minor.yy59->key, yymsp[-2].minor.yy87);
    buffer_free(yymsp[-2].minor.yy87);
    yymsp[-2].minor.yy87 = NULL;

    yygotominor.yy59 = yymsp[0].minor.yy59;
    yymsp[0].minor.yy59 = NULL;
  }
}
#line 1149 "configparser.c"
  yy_destructor(11,&yymsp[-1].minor);
        break;
      case 25:
  yy_destructor(1,&yymsp[0].minor);
        break;
      case 26:
        break;
      case 27:
#line 382 "../../src/configparser.y"
{
  data_config *dc;
  dc = (data_config *)array_get_element(ctx->srv->config_context, "global");
  force_assert(dc);
  configparser_push(ctx, dc, 0);
}
#line 1165 "configparser.c"
  yy_destructor(12,&yymsp[0].minor);
        break;
      case 28:
#line 389 "../../src/configparser.y"
{
  force_assert(ctx->current);
  configparser_pop(ctx);
  force_assert(ctx->current);
}
#line 1175 "configparser.c"
        /* No destructor defined for globalstart */
  yy_destructor(13,&yymsp[-2].minor);
        /* No destructor defined for metalines */
  yy_destructor(14,&yymsp[0].minor);
        break;
      case 29:
#line 395 "../../src/configparser.y"
{
  yygotominor.yy2 = NULL;
  if (ctx->ok) {
    if (yymsp[-3].minor.yy2->context_ndx >= yymsp[0].minor.yy2->context_ndx) {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
    }
    if (yymsp[-3].minor.yy2->cond == CONFIG_COND_ELSE) {
      fprintf(stderr, "unreachable condition following else catch-all\n");
      ctx->ok = 0;
    }
    yymsp[0].minor.yy2->prev = yymsp[-3].minor.yy2;
    yymsp[-3].minor.yy2->next = yymsp[0].minor.yy2;
    yygotominor.yy2 = yymsp[0].minor.yy2;
    yymsp[-3].minor.yy2 = NULL;
    yymsp[0].minor.yy2 = NULL;
  }
}
#line 1201 "configparser.c"
        /* No destructor defined for eols */
  yy_destructor(15,&yymsp[-1].minor);
        break;
      case 30:
#line 414 "../../src/configparser.y"
{
  yygotominor.yy2 = NULL;
  if (ctx->ok) {
    if (yymsp[-3].minor.yy2->context_ndx >= yymsp[0].minor.yy2->context_ndx) {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
    }
    if (yymsp[-3].minor.yy2->cond == CONFIG_COND_ELSE) {
      fprintf(stderr, "unreachable condition following else catch-all\n");
      ctx->ok = 0;
    }
  }
  if (ctx->ok) {
    size_t pos;
    data_config *dc;
    dc = (data_config *)array_extract_element(ctx->all_configs, yymsp[0].minor.yy2->key->ptr);
    force_assert(yymsp[0].minor.yy2 == dc);
    buffer_copy_buffer(yymsp[0].minor.yy2->key, yymsp[-3].minor.yy2->key);
    /*buffer_copy_buffer(yymsp[0].minor.yy2->comp_key, yymsp[-3].minor.yy2->comp_key);*/
    /*yymsp[0].minor.yy2->string = buffer_init_buffer(yymsp[-3].minor.yy2->string);*/
    pos = buffer_string_length(yymsp[0].minor.yy2->key)-buffer_string_length(yymsp[-3].minor.yy2->string)-2;
    switch(yymsp[-3].minor.yy2->cond) {
    case CONFIG_COND_NE:
      yymsp[0].minor.yy2->key->ptr[pos] = '='; /* opposite cond */
      /*buffer_copy_string_len(yymsp[0].minor.yy2->op, CONST_STR_LEN("=="));*/
      break;
    case CONFIG_COND_EQ:
      yymsp[0].minor.yy2->key->ptr[pos] = '!'; /* opposite cond */
      /*buffer_copy_string_len(yymsp[0].minor.yy2->op, CONST_STR_LEN("!="));*/
      break;
    case CONFIG_COND_NOMATCH:
      yymsp[0].minor.yy2->key->ptr[pos] = '='; /* opposite cond */
      /*buffer_copy_string_len(yymsp[0].minor.yy2->op, CONST_STR_LEN("=~"));*/
      break;
    case CONFIG_COND_MATCH:
      yymsp[0].minor.yy2->key->ptr[pos] = '!'; /* opposite cond */
      /*buffer_copy_string_len(yymsp[0].minor.yy2->op, CONST_STR_LEN("!~"));*/
      break;
    default: /* should not happen; CONFIG_COND_ELSE checked further above */
      force_assert(0);
    }

    if (NULL == (dc = (data_config *)array_get_element(ctx->all_configs, yymsp[0].minor.yy2->key->ptr))) {
      /* re-insert into ctx->all_configs with new yymsp[0].minor.yy2->key */
      array_insert_unique(ctx->all_configs, (data_unset *)yymsp[0].minor.yy2);
      yymsp[0].minor.yy2->prev = yymsp[-3].minor.yy2;
      yymsp[-3].minor.yy2->next = yymsp[0].minor.yy2;
    } else {
      fprintf(stderr, "unreachable else condition\n");
      ctx->ok = 0;
      yymsp[0].minor.yy2->free((data_unset *)yymsp[0].minor.yy2);
      yymsp[0].minor.yy2 = dc;
    }

    yygotominor.yy2 = yymsp[0].minor.yy2;
    yymsp[-3].minor.yy2 = NULL;
    yymsp[0].minor.yy2 = NULL;
  }
}
#line 1266 "configparser.c"
        /* No destructor defined for eols */
  yy_destructor(15,&yymsp[-1].minor);
        break;
      case 31:
#line 474 "../../src/configparser.y"
{
  yygotominor.yy2 = yymsp[0].minor.yy2;
  yymsp[0].minor.yy2 = NULL;
}
#line 1276 "configparser.c"
        break;
      case 32:
#line 479 "../../src/configparser.y"
{
  yygotominor.yy2 = NULL;
  if (ctx->ok) {
    data_config *cur;

    cur = ctx->current;
    configparser_pop(ctx);

    force_assert(cur && ctx->current);

    yygotominor.yy2 = cur;
  }
}
#line 1293 "configparser.c"
        /* No destructor defined for context */
  yy_destructor(13,&yymsp[-2].minor);
        /* No destructor defined for metalines */
  yy_destructor(14,&yymsp[0].minor);
        break;
      case 33:
#line 493 "../../src/configparser.y"
{
  yygotominor.yy2 = NULL;
  if (ctx->ok) {
    data_config *cur;

    cur = ctx->current;
    configparser_pop(ctx);

    force_assert(cur && ctx->current);

    yygotominor.yy2 = cur;
  }
}
#line 1314 "configparser.c"
        /* No destructor defined for context_else */
  yy_destructor(13,&yymsp[-2].minor);
        /* No destructor defined for metalines */
  yy_destructor(14,&yymsp[0].minor);
        break;
      case 34:
#line 507 "../../src/configparser.y"
{
  data_config *dc;
  buffer *b, *rvalue, *op;

  if (ctx->ok && yymsp[0].minor.yy59->type != TYPE_STRING) {
    fprintf(stderr, "rvalue must be string");
    ctx->ok = 0;
  }

  if (ctx->ok) {
    switch(yymsp[-1].minor.yy75) {
    case CONFIG_COND_NE:
      op = buffer_init_string("!=");
      break;
    case CONFIG_COND_EQ:
      op = buffer_init_string("==");
      break;
    case CONFIG_COND_NOMATCH:
      op = buffer_init_string("!~");
      break;
    case CONFIG_COND_MATCH:
      op = buffer_init_string("=~");
      break;
    default:
      force_assert(0);
      return; /* unreachable */
    }

    b = buffer_init();
    buffer_copy_buffer(b, ctx->current->key);
    buffer_append_string(b, "/");
    buffer_append_string_buffer(b, yymsp[-5].minor.yy0);
    buffer_append_string_buffer(b, yymsp[-3].minor.yy87);
    buffer_append_string_buffer(b, op);
    rvalue = ((data_string*)yymsp[0].minor.yy59)->value;
    buffer_append_string_buffer(b, rvalue);

    if (NULL != (dc = (data_config *)array_get_element(ctx->all_configs, b->ptr))) {
      configparser_push(ctx, dc, 0);
    } else {
      static const struct {
        comp_key_t comp;
        char *comp_key;
        size_t len;
      } comps[] = {
        { COMP_SERVER_SOCKET,      CONST_STR_LEN("SERVER[\"socket\"]"   ) },
        { COMP_HTTP_URL,           CONST_STR_LEN("HTTP[\"url\"]"        ) },
        { COMP_HTTP_HOST,          CONST_STR_LEN("HTTP[\"host\"]"       ) },
        { COMP_HTTP_REFERER,       CONST_STR_LEN("HTTP[\"referer\"]"    ) },
        { COMP_HTTP_USER_AGENT,    CONST_STR_LEN("HTTP[\"useragent\"]"  ) },
        { COMP_HTTP_USER_AGENT,    CONST_STR_LEN("HTTP[\"user-agent\"]"  ) },
        { COMP_HTTP_LANGUAGE,      CONST_STR_LEN("HTTP[\"language\"]"   ) },
        { COMP_HTTP_COOKIE,        CONST_STR_LEN("HTTP[\"cookie\"]"     ) },
        { COMP_HTTP_REMOTE_IP,     CONST_STR_LEN("HTTP[\"remoteip\"]"   ) },
        { COMP_HTTP_REMOTE_IP,     CONST_STR_LEN("HTTP[\"remote-ip\"]"   ) },
        { COMP_HTTP_QUERY_STRING,  CONST_STR_LEN("HTTP[\"querystring\"]") },
        { COMP_HTTP_QUERY_STRING,  CONST_STR_LEN("HTTP[\"query-string\"]") },
        { COMP_HTTP_REQUEST_METHOD, CONST_STR_LEN("HTTP[\"request-method\"]") },
        { COMP_HTTP_SCHEME,        CONST_STR_LEN("HTTP[\"scheme\"]"     ) },
        { COMP_UNSET, NULL, 0 },
      };
      size_t i;

      dc = data_config_init();

      buffer_copy_buffer(dc->key, b);
      buffer_copy_buffer(dc->op, op);
      buffer_copy_buffer(dc->comp_key, yymsp[-5].minor.yy0);
      buffer_append_string_len(dc->comp_key, CONST_STR_LEN("[\""));
      buffer_append_string_buffer(dc->comp_key, yymsp[-3].minor.yy87);
      buffer_append_string_len(dc->comp_key, CONST_STR_LEN("\"]"));
      dc->cond = yymsp[-1].minor.yy75;

      for (i = 0; comps[i].comp_key; i ++) {
        if (buffer_is_equal_string(
              dc->comp_key, comps[i].comp_key, comps[i].len)) {
          dc->comp = comps[i].comp;
          break;
        }
      }
      if (COMP_UNSET == dc->comp) {
        fprintf(stderr, "error comp_key %s", dc->comp_key->ptr);
        ctx->ok = 0;
      }
      else if (COMP_HTTP_REMOTE_IP == dc->comp
               && (dc->cond == CONFIG_COND_EQ || dc->cond == CONFIG_COND_NE)) {
        char * const slash = strchr(rvalue->ptr, '/'); /* CIDR mask */
        char * const colon = strchr(rvalue->ptr, ':'); /* IPv6 */
        if (NULL != slash && slash == rvalue->ptr){/*(skip AF_UNIX /path/file)*/
        }
        else if (NULL != slash) {
          char *nptr;
          const unsigned long nm_bits = strtoul(slash + 1, &nptr, 10);
          if (*nptr || 0 == nm_bits || nm_bits > (NULL != colon ? 128 : 32)) {
            /*(also rejects (slash+1 == nptr) which results in nm_bits = 0)*/
            fprintf(stderr, "invalid or missing netmask: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
          else {
            int rc;
            buffer_string_set_length(rvalue, (size_t)(slash - rvalue->ptr)); /*(truncate)*/
            rc = (NULL == colon)
              ? http_request_host_normalize(rvalue)
              : configparser_remoteip_normalize_compat(rvalue);
            buffer_append_string_len(rvalue, CONST_STR_LEN("/"));
            buffer_append_int(rvalue, (int)nm_bits);
            if (0 != rc) {
              fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
              ctx->ok = 0;
            }
          }
        }
        else {
          int rc = (NULL == colon)
            ? http_request_host_normalize(rvalue)
            : configparser_remoteip_normalize_compat(rvalue);
          if (0 != rc) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }
      else if (COMP_SERVER_SOCKET == dc->comp) {
        /*(redundant with parsing in network.c; not actually required here)*/
        if (rvalue->ptr[0] != ':' /*(network.c special-cases ":" and "[]")*/
            && !(rvalue->ptr[0] == '[' && rvalue->ptr[1] == ']')) {
          if (http_request_host_normalize(rvalue)) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }
      else if (COMP_HTTP_HOST == dc->comp) {
        if (dc->cond == CONFIG_COND_EQ || dc->cond == CONFIG_COND_NE) {
          if (http_request_host_normalize(rvalue)) {
            fprintf(stderr, "invalid IP addr: %s\n", rvalue->ptr);
            ctx->ok = 0;
          }
        }
      }

      if (ctx->ok) switch(yymsp[-1].minor.yy75) {
      case CONFIG_COND_NE:
      case CONFIG_COND_EQ:
        dc->string = buffer_init_buffer(rvalue);
        break;
      case CONFIG_COND_NOMATCH:
      case CONFIG_COND_MATCH: {
#ifdef HAVE_PCRE_H
        const char *errptr;
        int erroff, captures;

        if (NULL == (dc->regex =
            pcre_compile(rvalue->ptr, 0, &errptr, &erroff, NULL))) {
          dc->string = buffer_init_string(errptr);
          dc->cond = CONFIG_COND_UNSET;

          fprintf(stderr, "parsing regex failed: %s -> %s at offset %d\n",
              rvalue->ptr, errptr, erroff);

          ctx->ok = 0;
        } else if (NULL == (dc->regex_study =
            pcre_study(dc->regex, 0, &errptr)) &&
                   errptr != NULL) {
          fprintf(stderr, "studying regex failed: %s -> %s\n",
              rvalue->ptr, errptr);
          ctx->ok = 0;
        } else if (0 != (pcre_fullinfo(dc->regex, dc->regex_study, PCRE_INFO_CAPTURECOUNT, &captures))) {
          fprintf(stderr, "getting capture count for regex failed: %s\n",
              rvalue->ptr);
          ctx->ok = 0;
        } else if (captures > 9) {
          fprintf(stderr, "Too many captures in regex, use (?:...) instead of (...): %s\n",
              rvalue->ptr);
          ctx->ok = 0;
        } else {
          dc->string = buffer_init_buffer(rvalue);
        }
#else
        fprintf(stderr, "can't handle '$%s[%s] =~ ...' as you compiled without pcre support. \n"
                        "(perhaps just a missing pcre-devel package ?) \n",
                        yymsp[-5].minor.yy0->ptr, yymsp[-3].minor.yy87->ptr);
        ctx->ok = 0;
 #endif
        break;
      }

      default:
        fprintf(stderr, "unknown condition for $%s[%s]\n",
                        yymsp[-5].minor.yy0->ptr, yymsp[-3].minor.yy87->ptr);
        ctx->ok = 0;
        break;
      }

      if (ctx->ok) {
        configparser_push(ctx, dc, 1);
      } else {
        dc->free((data_unset*) dc);
      }
    }

    buffer_free(b);
    buffer_free(op);
    buffer_free(yymsp[-5].minor.yy0);
    yymsp[-5].minor.yy0 = NULL;
    buffer_free(yymsp[-3].minor.yy87);
    yymsp[-3].minor.yy87 = NULL;
    yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
    yymsp[0].minor.yy59 = NULL;
  }
}
#line 1533 "configparser.c"
  yy_destructor(16,&yymsp[-6].minor);
  yy_destructor(18,&yymsp[-4].minor);
  yy_destructor(19,&yymsp[-2].minor);
        break;
      case 35:
#line 719 "../../src/configparser.y"
{
  if (ctx->ok) {
    data_config *dc = data_config_init();
    buffer_copy_buffer(dc->key, ctx->current->key);
    buffer_append_string_len(dc->key, CONST_STR_LEN("/"));
    buffer_append_string_len(dc->key, CONST_STR_LEN("else_tmp_token"));
    dc->cond = CONFIG_COND_ELSE;
    configparser_push(ctx, dc, 1);
  }
}
#line 1550 "configparser.c"
        break;
      case 36:
#line 730 "../../src/configparser.y"
{
  yygotominor.yy75 = CONFIG_COND_EQ;
}
#line 1557 "configparser.c"
  yy_destructor(20,&yymsp[0].minor);
        break;
      case 37:
#line 733 "../../src/configparser.y"
{
  yygotominor.yy75 = CONFIG_COND_MATCH;
}
#line 1565 "configparser.c"
  yy_destructor(21,&yymsp[0].minor);
        break;
      case 38:
#line 736 "../../src/configparser.y"
{
  yygotominor.yy75 = CONFIG_COND_NE;
}
#line 1573 "configparser.c"
  yy_destructor(22,&yymsp[0].minor);
        break;
      case 39:
#line 739 "../../src/configparser.y"
{
  yygotominor.yy75 = CONFIG_COND_NOMATCH;
}
#line 1581 "configparser.c"
  yy_destructor(23,&yymsp[0].minor);
        break;
      case 40:
#line 743 "../../src/configparser.y"
{
  yygotominor.yy87 = NULL;
  if (ctx->ok) {
    if (yymsp[0].minor.yy59->type == TYPE_STRING) {
      yygotominor.yy87 = buffer_init_buffer(((data_string*)yymsp[0].minor.yy59)->value);
    } else if (yymsp[0].minor.yy59->type == TYPE_INTEGER) {
      yygotominor.yy87 = buffer_init();
      buffer_copy_int(yygotominor.yy87, ((data_integer *)yymsp[0].minor.yy59)->value);
    } else {
      fprintf(stderr, "operand must be string");
      ctx->ok = 0;
    }
  }
  yymsp[0].minor.yy59->free(yymsp[0].minor.yy59);
  yymsp[0].minor.yy59 = NULL;
}
#line 1602 "configparser.c"
        break;
      case 41:
#line 760 "../../src/configparser.y"
{
  if (ctx->ok) {
    if (0 != config_parse_file(ctx->srv, ctx, yymsp[0].minor.yy87->ptr)) {
      ctx->ok = 0;
    }
    buffer_free(yymsp[0].minor.yy87);
    yymsp[0].minor.yy87 = NULL;
  }
}
#line 1615 "configparser.c"
  yy_destructor(24,&yymsp[-1].minor);
        break;
      case 42:
#line 770 "../../src/configparser.y"
{
  if (ctx->ok) {
    if (0 != config_parse_cmd(ctx->srv, ctx, yymsp[0].minor.yy87->ptr)) {
      ctx->ok = 0;
    }
    buffer_free(yymsp[0].minor.yy87);
    yymsp[0].minor.yy87 = NULL;
  }
}
#line 1629 "configparser.c"
  yy_destructor(25,&yymsp[-1].minor);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  configparserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 147 "../../src/configparser.y"

  ctx->ok = 0;

#line 1663 "configparser.c"
  configparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  configparserARG_FETCH;
  UNUSED(yymajor);
  UNUSED(yyminor);
#define TOKEN (yyminor.yy0)
  configparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  configparserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  configparserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "configparserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void configparser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  configparserTOKENTYPE yyminor       /* The value for the token */
  configparserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    if( yymajor==0 ) return;
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  configparserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
