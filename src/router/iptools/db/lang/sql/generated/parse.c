/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
**
** This version of "lempar.c" is modified, slightly, for use by SQLite.
** The only modifications are the addition of a couple of NEVER()
** macros to disable tests that are needed in the case of a general
** LALR(1) grammar but which are always false in the
** specific grammar used by SQLite.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 49 "parse.y"

#include "sqliteInt.h"

/*
** Disable all error recovery processing in the parser push-down
** automaton.
*/
#define YYNOERRORRECOVERY 1

/*
** Make yytestcase() the same as testcase()
*/
#define yytestcase(X) testcase(X)

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int bNot;         /* True if the NOT keyword is present */
};

/*
** An instance of the following structure describes the event of a
** TRIGGER.  "a" is the event type, one of TK_UPDATE, TK_INSERT,
** TK_DELETE, or TK_INSTEAD.  If the event is of the form
**
**      UPDATE ON (a,b,c)
**
** Then the "b" IdList records the list "a,b,c".
*/
struct TrigEvent { int a; IdList * b; };

/*
** An instance of this structure holds the ATTACH key and the key type.
*/
struct AttachKey { int type;  Token key; };

#line 412 "parse.y"

  /*
  ** For a compound SELECT statement, make sure p->pPrior->pNext==p for
  ** all elements in the list.  And make sure list length does not exceed
  ** SQLITE_LIMIT_COMPOUND_SELECT.
  */
  static void parserDoubleLinkSelect(Parse *pParse, Select *p){
    if( p->pPrior ){
      Select *pNext = 0, *pLoop;
      int mxSelect, cnt = 0;
      for(pLoop=p; pLoop; pNext=pLoop, pLoop=pLoop->pPrior, cnt++){
        pLoop->pNext = pNext;
        pLoop->selFlags |= SF_Compound;
      }
      if( (p->selFlags & SF_MultiValue)==0 && 
        (mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT])>0 &&
        cnt>mxSelect
      ){
        sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
      }
    }
  }
#line 814 "parse.y"

  /* This is a utility routine used to set the ExprSpan.zStart and
  ** ExprSpan.zEnd values of pOut so that the span covers the complete
  ** range of text beginning with pStart and going to the end of pEnd.
  */
  static void spanSet(ExprSpan *pOut, Token *pStart, Token *pEnd){
    pOut->zStart = pStart->z;
    pOut->zEnd = &pEnd->z[pEnd->n];
  }

  /* Construct a new Expr object from a single identifier.  Use the
  ** new Expr to populate pOut.  Set the span of pOut to be the identifier
  ** that created the expression.
  */
  static void spanExpr(ExprSpan *pOut, Parse *pParse, int op, Token *pValue){
    pOut->pExpr = sqlite3PExpr(pParse, op, 0, 0, pValue);
    pOut->zStart = pValue->z;
    pOut->zEnd = &pValue->z[pValue->n];
  }
#line 904 "parse.y"

  /* This routine constructs a binary expression node out of two ExprSpan
  ** objects and uses the result to populate a new ExprSpan object.
  */
  static void spanBinaryExpr(
    ExprSpan *pOut,     /* Write the result here */
    Parse *pParse,      /* The parsing context.  Errors accumulate here */
    int op,             /* The binary operation */
    ExprSpan *pLeft,    /* The left operand */
    ExprSpan *pRight    /* The right operand */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pLeft->pExpr, pRight->pExpr, 0);
    pOut->zStart = pLeft->zStart;
    pOut->zEnd = pRight->zEnd;
  }
#line 958 "parse.y"

  /* Construct an expression node for a unary postfix operator
  */
  static void spanUnaryPostfix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPostOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pOperand->zStart;
    pOut->zEnd = &pPostOp->z[pPostOp->n];
  }                           
#line 977 "parse.y"

  /* A routine to convert a binary TK_IS or TK_ISNOT expression into a
  ** unary TK_ISNULL or TK_NOTNULL expression. */
  static void binaryToUnaryIfNull(Parse *pParse, Expr *pY, Expr *pA, int op){
    sqlite3 *db = pParse->db;
    if( pY && pA && pY->op==TK_NULL ){
      pA->op = (u8)op;
      sqlite3ExprDelete(db, pA->pRight);
      pA->pRight = 0;
    }
  }
#line 1005 "parse.y"

  /* Construct an expression node for a unary prefix operator
  */
  static void spanUnaryPrefix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPreOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pPreOp->z;
    pOut->zEnd = pOperand->zEnd;
  }
#line 164 "parse.c"
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
**    sqlite3ParserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is sqlite3ParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    sqlite3ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3ParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 254
#define YYACTIONTYPE unsigned short int
#define YYWILDCARD 70
#define sqlite3ParserTOKENTYPE Token
typedef union {
  int yyinit;
  sqlite3ParserTOKENTYPE yy0;
  Select* yy3;
  ExprList* yy14;
  With* yy59;
  SrcList* yy65;
  struct LikeOp yy96;
  Expr* yy132;
  u8 yy186;
  int yy328;
  ExprSpan yy346;
  struct TrigEvent yy378;
  u16 yy381;
  IdList* yy408;
  struct {int value; int mask;} yy429;
  TriggerStep* yy473;
  struct LimitVal yy476;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define sqlite3ParserARG_SDECL Parse *pParse;
#define sqlite3ParserARG_PDECL ,Parse *pParse
#define sqlite3ParserARG_FETCH Parse *pParse = yypParser->pParse
#define sqlite3ParserARG_STORE yypParser->pParse = pParse
#define YYNSTATE 642
#define YYNRULE 327
#define YYFALLBACK 1
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
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
#define YY_ACTTAB_COUNT (1497)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   306,  212,  432,  955,  639,  191,  955,  295,  559,   88,
 /*    10 */    88,   88,   88,   81,   86,   86,   86,   86,   85,   85,
 /*    20 */    84,   84,   84,   83,  330,  185,  184,  183,  635,  635,
 /*    30 */   292,  606,  606,   88,   88,   88,   88,  683,   86,   86,
 /*    40 */    86,   86,   85,   85,   84,   84,   84,   83,  330,   16,
 /*    50 */   436,  597,   89,   90,   80,  600,  599,  601,  601,   87,
 /*    60 */    87,   88,   88,   88,   88,  684,   86,   86,   86,   86,
 /*    70 */    85,   85,   84,   84,   84,   83,  330,  306,  559,   84,
 /*    80 */    84,   84,   83,  330,   65,   86,   86,   86,   86,   85,
 /*    90 */    85,   84,   84,   84,   83,  330,  635,  635,  634,  633,
 /*   100 */   182,  682,  550,  379,  376,  375,   17,  322,  606,  606,
 /*   110 */   371,  198,  479,   91,  374,   82,   79,  165,   85,   85,
 /*   120 */    84,   84,   84,   83,  330,  598,  635,  635,  107,   89,
 /*   130 */    90,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   140 */    88,   88,  186,   86,   86,   86,   86,   85,   85,   84,
 /*   150 */    84,   84,   83,  330,  306,  594,  594,  142,  328,  327,
 /*   160 */   484,  249,  344,  238,  635,  635,  634,  633,  585,  448,
 /*   170 */   526,  525,  229,  388,    1,  394,  450,  584,  449,  635,
 /*   180 */   635,  635,  635,  319,  395,  606,  606,  199,  157,  273,
 /*   190 */   382,  268,  381,  187,  635,  635,  634,  633,  311,  555,
 /*   200 */   266,  593,  593,  266,  347,  588,   89,   90,   80,  600,
 /*   210 */   599,  601,  601,   87,   87,   88,   88,   88,   88,  478,
 /*   220 */    86,   86,   86,   86,   85,   85,   84,   84,   84,   83,
 /*   230 */   330,  306,  272,  536,  634,  633,  146,  610,  197,  310,
 /*   240 */   575,  182,  482,  271,  379,  376,  375,  506,   21,  634,
 /*   250 */   633,  634,  633,  635,  635,  374,  611,  574,  548,  440,
 /*   260 */   111,  563,  606,  606,  634,  633,  324,  479,  608,  608,
 /*   270 */   608,  300,  435,  573,  119,  407,  210,  162,  562,  883,
 /*   280 */   592,  592,  306,   89,   90,   80,  600,  599,  601,  601,
 /*   290 */    87,   87,   88,   88,   88,   88,  506,   86,   86,   86,
 /*   300 */    86,   85,   85,   84,   84,   84,   83,  330,  620,  111,
 /*   310 */   635,  635,  361,  606,  606,  358,  249,  349,  248,  433,
 /*   320 */   243,  479,  586,  634,  633,  195,  611,   93,  119,  221,
 /*   330 */   575,  497,  534,  534,   89,   90,   80,  600,  599,  601,
 /*   340 */   601,   87,   87,   88,   88,   88,   88,  574,   86,   86,
 /*   350 */    86,   86,   85,   85,   84,   84,   84,   83,  330,  306,
 /*   360 */    77,  429,  638,  573,  589,  530,  240,  230,  242,  105,
 /*   370 */   249,  349,  248,  515,  588,  208,  460,  529,  564,  173,
 /*   380 */   634,  633,  970,  144,  430,    2,  424,  228,  380,  557,
 /*   390 */   606,  606,  190,  153,  159,  158,  514,   51,  632,  631,
 /*   400 */   630,   71,  536,  432,  954,  196,  610,  954,  614,   45,
 /*   410 */    18,   89,   90,   80,  600,  599,  601,  601,   87,   87,
 /*   420 */    88,   88,   88,   88,  261,   86,   86,   86,   86,   85,
 /*   430 */    85,   84,   84,   84,   83,  330,  306,  608,  608,  608,
 /*   440 */   542,  424,  402,  385,  241,  506,  451,  320,  211,  543,
 /*   450 */   164,  436,  386,  293,  451,  587,  108,  496,  111,  334,
 /*   460 */   391,  591,  424,  614,   27,  452,  453,  606,  606,   72,
 /*   470 */   257,   70,  259,  452,  339,  342,  564,  582,   68,  415,
 /*   480 */   469,  328,  327,   62,  614,   45,  110,  393,   89,   90,
 /*   490 */    80,  600,  599,  601,  601,   87,   87,   88,   88,   88,
 /*   500 */    88,  152,   86,   86,   86,   86,   85,   85,   84,   84,
 /*   510 */    84,   83,  330,  306,  110,  499,  520,  538,  402,  389,
 /*   520 */   424,  110,  566,  500,  593,  593,  454,   82,   79,  165,
 /*   530 */   424,  591,  384,  564,  340,  615,  188,  162,  424,  350,
 /*   540 */   616,  424,  614,   44,  606,  606,  445,  582,  300,  434,
 /*   550 */   151,   19,  614,    9,  568,  580,  348,  615,  469,  567,
 /*   560 */   614,   26,  616,  614,   45,   89,   90,   80,  600,  599,
 /*   570 */   601,  601,   87,   87,   88,   88,   88,   88,  411,   86,
 /*   580 */    86,   86,   86,   85,   85,   84,   84,   84,   83,  330,
 /*   590 */   306,  579,  110,  578,  521,  282,  433,  398,  400,  255,
 /*   600 */   486,   82,   79,  165,  487,  164,   82,   79,  165,  488,
 /*   610 */   488,  364,  387,  424,  544,  544,  509,  350,  362,  155,
 /*   620 */   191,  606,  606,  559,  642,  640,  333,   82,   79,  165,
 /*   630 */   305,  564,  507,  312,  357,  614,   45,  329,  596,  595,
 /*   640 */   194,  337,   89,   90,   80,  600,  599,  601,  601,   87,
 /*   650 */    87,   88,   88,   88,   88,  424,   86,   86,   86,   86,
 /*   660 */    85,   85,   84,   84,   84,   83,  330,  306,   20,  323,
 /*   670 */   150,  263,  211,  543,  421,  596,  595,  614,   22,  424,
 /*   680 */   193,  424,  284,  424,  391,  424,  509,  424,  577,  424,
 /*   690 */   186,  335,  424,  559,  424,  313,  120,  546,  606,  606,
 /*   700 */    67,  614,   47,  614,   50,  614,   48,  614,  100,  614,
 /*   710 */    99,  614,  101,  576,  614,  102,  614,  109,  326,   89,
 /*   720 */    90,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   730 */    88,   88,  424,   86,   86,   86,   86,   85,   85,   84,
 /*   740 */    84,   84,   83,  330,  306,  424,  311,  424,  585,   54,
 /*   750 */   424,  516,  517,  590,  614,  112,  424,  584,  424,  572,
 /*   760 */   424,  195,  424,  571,  424,   67,  424,  614,   94,  614,
 /*   770 */    98,  424,  614,   97,  264,  606,  606,  195,  614,   46,
 /*   780 */   614,   96,  614,   30,  614,   49,  614,  115,  614,  114,
 /*   790 */   418,  229,  388,  614,  113,  306,   89,   90,   80,  600,
 /*   800 */   599,  601,  601,   87,   87,   88,   88,   88,   88,  424,
 /*   810 */    86,   86,   86,   86,   85,   85,   84,   84,   84,   83,
 /*   820 */   330,  119,  424,  590,  110,  372,  606,  606,  195,   53,
 /*   830 */   250,  614,   29,  195,  472,  438,  729,  190,  302,  498,
 /*   840 */    14,  523,  641,    2,  614,   43,  306,   89,   90,   80,
 /*   850 */   600,  599,  601,  601,   87,   87,   88,   88,   88,   88,
 /*   860 */   424,   86,   86,   86,   86,   85,   85,   84,   84,   84,
 /*   870 */    83,  330,  424,  613,  964,  964,  354,  606,  606,  420,
 /*   880 */   312,   64,  614,   42,  391,  355,  283,  437,  301,  255,
 /*   890 */   414,  410,  495,  492,  614,   28,  471,  306,   89,   90,
 /*   900 */    80,  600,  599,  601,  601,   87,   87,   88,   88,   88,
 /*   910 */    88,  424,   86,   86,   86,   86,   85,   85,   84,   84,
 /*   920 */    84,   83,  330,  424,  110,  110,  110,  110,  606,  606,
 /*   930 */   110,  254,   13,  614,   41,  532,  531,  283,  481,  531,
 /*   940 */   457,  284,  119,  561,  356,  614,   40,  284,  306,   89,
 /*   950 */    78,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   960 */    88,   88,  424,   86,   86,   86,   86,   85,   85,   84,
 /*   970 */    84,   84,   83,  330,  110,  424,  341,  220,  555,  606,
 /*   980 */   606,  351,  555,  318,  614,   95,  413,  255,   83,  330,
 /*   990 */   284,  284,  255,  640,  333,  356,  255,  614,   39,  306,
 /*  1000 */   356,   90,   80,  600,  599,  601,  601,   87,   87,   88,
 /*  1010 */    88,   88,   88,  424,   86,   86,   86,   86,   85,   85,
 /*  1020 */    84,   84,   84,   83,  330,  424,  317,  316,  141,  465,
 /*  1030 */   606,  606,  219,  619,  463,  614,   10,  417,  462,  255,
 /*  1040 */   189,  510,  553,  351,  207,  363,  161,  614,   38,  315,
 /*  1050 */   218,  255,  255,   80,  600,  599,  601,  601,   87,   87,
 /*  1060 */    88,   88,   88,   88,  424,   86,   86,   86,   86,   85,
 /*  1070 */    85,   84,   84,   84,   83,  330,   76,  419,  255,    3,
 /*  1080 */   878,  461,  424,  247,  331,  331,  614,   37,  217,   76,
 /*  1090 */   419,  390,    3,  216,  215,  422,    4,  331,  331,  424,
 /*  1100 */   547,   12,  424,  545,  614,   36,  424,  541,  422,  424,
 /*  1110 */   540,  424,  214,  424,  408,  424,  539,  403,  605,  605,
 /*  1120 */   237,  614,   25,  119,  614,   24,  588,  408,  614,   45,
 /*  1130 */   118,  614,   35,  614,   34,  614,   33,  614,   23,  588,
 /*  1140 */    60,  223,  603,  602,  513,  378,   73,   74,  140,  139,
 /*  1150 */   424,  110,  265,   75,  426,  425,   59,  424,  610,   73,
 /*  1160 */    74,  549,  402,  404,  424,  373,   75,  426,  425,  604,
 /*  1170 */   138,  610,  614,   11,  392,   76,  419,  181,    3,  614,
 /*  1180 */    32,  271,  369,  331,  331,  493,  614,   31,  149,  608,
 /*  1190 */   608,  608,  607,   15,  422,  365,  614,    8,  137,  489,
 /*  1200 */   136,  190,  608,  608,  608,  607,   15,  485,  176,  135,
 /*  1210 */     7,  252,  477,  408,  174,  133,  175,  474,   57,   56,
 /*  1220 */   132,  130,  119,   76,  419,  588,    3,  468,  245,  464,
 /*  1230 */   171,  331,  331,  125,  123,  456,  447,  122,  446,  104,
 /*  1240 */   336,  231,  422,  166,  154,   73,   74,  332,  116,  431,
 /*  1250 */   121,  309,   75,  426,  425,  222,  106,  610,  308,  637,
 /*  1260 */   204,  408,  629,  627,  628,    6,  200,  428,  427,  290,
 /*  1270 */   203,  622,  201,  588,   62,   63,  289,   66,  419,  399,
 /*  1280 */     3,  401,  288,   92,  143,  331,  331,  287,  608,  608,
 /*  1290 */   608,  607,   15,   73,   74,  227,  422,  325,   69,  416,
 /*  1300 */    75,  426,  425,  612,  412,  610,  192,   61,  569,  209,
 /*  1310 */   396,  226,  278,  225,  383,  408,  527,  558,  276,  533,
 /*  1320 */   552,  528,  321,  523,  370,  508,  180,  588,  494,  179,
 /*  1330 */   366,  117,  253,  269,  522,  503,  608,  608,  608,  607,
 /*  1340 */    15,  551,  502,   58,  274,  524,  178,   73,   74,  304,
 /*  1350 */   501,  368,  303,  206,   75,  426,  425,  491,  360,  610,
 /*  1360 */   213,  177,  483,  131,  345,  298,  297,  296,  202,  294,
 /*  1370 */   480,  490,  466,  134,  172,  129,  444,  346,  470,  128,
 /*  1380 */   314,  459,  103,  127,  126,  148,  124,  167,  443,  235,
 /*  1390 */   608,  608,  608,  607,   15,  442,  439,  623,  234,  299,
 /*  1400 */   145,  583,  291,  377,  581,  160,  119,  156,  270,  636,
 /*  1410 */   971,  169,  279,  626,  520,  625,  473,  624,  170,  621,
 /*  1420 */   618,  119,  168,   55,  409,  423,  537,  609,  286,  285,
 /*  1430 */   405,  570,  560,  556,    5,   52,  458,  554,  147,  267,
 /*  1440 */   519,  504,  518,  406,  262,  239,  260,  512,  343,  511,
 /*  1450 */   258,  353,  565,  256,  224,  251,  359,  277,  275,  476,
 /*  1460 */   475,  246,  352,  244,  467,  455,  236,  233,  232,  307,
 /*  1470 */   441,  281,  205,  163,  397,  280,  535,  505,  330,  617,
 /*  1480 */   971,  971,  971,  971,  367,  971,  971,  971,  971,  971,
 /*  1490 */   971,  971,  971,  971,  971,  971,  338,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    19,   22,   22,   23,    1,   24,   26,   15,   27,   80,
 /*    10 */    81,   82,   83,   84,   85,   86,   87,   88,   89,   90,
 /*    20 */    91,   92,   93,   94,   95,  108,  109,  110,   27,   28,
 /*    30 */    23,   50,   51,   80,   81,   82,   83,  122,   85,   86,
 /*    40 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   22,
 /*    50 */    70,   23,   71,   72,   73,   74,   75,   76,   77,   78,
 /*    60 */    79,   80,   81,   82,   83,  122,   85,   86,   87,   88,
 /*    70 */    89,   90,   91,   92,   93,   94,   95,   19,   97,   91,
 /*    80 */    92,   93,   94,   95,   26,   85,   86,   87,   88,   89,
 /*    90 */    90,   91,   92,   93,   94,   95,   27,   28,   97,   98,
 /*   100 */    99,  122,  211,  102,  103,  104,   79,   19,   50,   51,
 /*   110 */    19,  122,   59,   55,  113,  224,  225,  226,   89,   90,
 /*   120 */    91,   92,   93,   94,   95,   23,   27,   28,   26,   71,
 /*   130 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   140 */    82,   83,   51,   85,   86,   87,   88,   89,   90,   91,
 /*   150 */    92,   93,   94,   95,   19,  132,  133,   58,   89,   90,
 /*   160 */    21,  108,  109,  110,   27,   28,   97,   98,   33,  100,
 /*   170 */     7,    8,  119,  120,   22,   19,  107,   42,  109,   27,
 /*   180 */    28,   27,   28,   95,   28,   50,   51,   99,  100,  101,
 /*   190 */   102,  103,  104,  105,   27,   28,   97,   98,  107,  152,
 /*   200 */   112,  132,  133,  112,   65,   69,   71,   72,   73,   74,
 /*   210 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   11,
 /*   220 */    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   230 */    95,   19,  101,   97,   97,   98,   24,  101,  122,  157,
 /*   240 */    12,   99,  103,  112,  102,  103,  104,  152,   22,   97,
 /*   250 */    98,   97,   98,   27,   28,  113,   27,   29,   91,  164,
 /*   260 */   165,  124,   50,   51,   97,   98,  219,   59,  132,  133,
 /*   270 */   134,   22,   23,   45,   66,   47,  212,  213,  124,  140,
 /*   280 */   132,  133,   19,   71,   72,   73,   74,   75,   76,   77,
 /*   290 */    78,   79,   80,   81,   82,   83,  152,   85,   86,   87,
 /*   300 */    88,   89,   90,   91,   92,   93,   94,   95,  164,  165,
 /*   310 */    27,   28,  230,   50,   51,  233,  108,  109,  110,   70,
 /*   320 */    16,   59,   23,   97,   98,   26,   97,   22,   66,  185,
 /*   330 */    12,  187,   27,   28,   71,   72,   73,   74,   75,   76,
 /*   340 */    77,   78,   79,   80,   81,   82,   83,   29,   85,   86,
 /*   350 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   19,
 /*   360 */    22,  148,  149,   45,   23,   47,   62,  154,   64,  156,
 /*   370 */   108,  109,  110,   37,   69,   23,  163,   59,   26,   26,
 /*   380 */    97,   98,  144,  145,  146,  147,  152,  200,   52,   23,
 /*   390 */    50,   51,   26,   22,   89,   90,   60,  210,    7,    8,
 /*   400 */     9,  138,   97,   22,   23,   26,  101,   26,  174,  175,
 /*   410 */   197,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*   420 */    80,   81,   82,   83,   16,   85,   86,   87,   88,   89,
 /*   430 */    90,   91,   92,   93,   94,   95,   19,  132,  133,  134,
 /*   440 */    23,  152,  208,  209,  140,  152,  152,  111,  195,  196,
 /*   450 */    98,   70,  163,  160,  152,   23,   22,  164,  165,  246,
 /*   460 */   207,   27,  152,  174,  175,  171,  172,   50,   51,  137,
 /*   470 */    62,  139,   64,  171,  172,  222,  124,   27,  138,   24,
 /*   480 */   163,   89,   90,  130,  174,  175,  197,  163,   71,   72,
 /*   490 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   500 */    83,   22,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   510 */    93,   94,   95,   19,  197,  181,  182,   23,  208,  209,
 /*   520 */   152,  197,   26,  189,  132,  133,  232,  224,  225,  226,
 /*   530 */   152,   97,   91,   26,  232,  116,  212,  213,  152,  222,
 /*   540 */   121,  152,  174,  175,   50,   51,  243,   97,   22,   23,
 /*   550 */    22,  234,  174,  175,  177,   23,  239,  116,  163,  177,
 /*   560 */   174,  175,  121,  174,  175,   71,   72,   73,   74,   75,
 /*   570 */    76,   77,   78,   79,   80,   81,   82,   83,   24,   85,
 /*   580 */    86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
 /*   590 */    19,   23,  197,   11,   23,  227,   70,  208,  220,  152,
 /*   600 */    31,  224,  225,  226,   35,   98,  224,  225,  226,  108,
 /*   610 */   109,  110,  115,  152,  117,  118,   27,  222,   49,  123,
 /*   620 */    24,   50,   51,   27,    0,    1,    2,  224,  225,  226,
 /*   630 */   166,  124,  168,  169,  239,  174,  175,  170,  171,  172,
 /*   640 */    22,  194,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   650 */    79,   80,   81,   82,   83,  152,   85,   86,   87,   88,
 /*   660 */    89,   90,   91,   92,   93,   94,   95,   19,   22,  208,
 /*   670 */    24,   23,  195,  196,  170,  171,  172,  174,  175,  152,
 /*   680 */    26,  152,  152,  152,  207,  152,   97,  152,   23,  152,
 /*   690 */    51,  244,  152,   97,  152,  247,  248,   23,   50,   51,
 /*   700 */    26,  174,  175,  174,  175,  174,  175,  174,  175,  174,
 /*   710 */   175,  174,  175,   23,  174,  175,  174,  175,  188,   71,
 /*   720 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   730 */    82,   83,  152,   85,   86,   87,   88,   89,   90,   91,
 /*   740 */    92,   93,   94,   95,   19,  152,  107,  152,   33,   24,
 /*   750 */   152,  100,  101,   27,  174,  175,  152,   42,  152,   23,
 /*   760 */   152,   26,  152,   23,  152,   26,  152,  174,  175,  174,
 /*   770 */   175,  152,  174,  175,   23,   50,   51,   26,  174,  175,
 /*   780 */   174,  175,  174,  175,  174,  175,  174,  175,  174,  175,
 /*   790 */   163,  119,  120,  174,  175,   19,   71,   72,   73,   74,
 /*   800 */    75,   76,   77,   78,   79,   80,   81,   82,   83,  152,
 /*   810 */    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   820 */    95,   66,  152,   97,  197,   23,   50,   51,   26,   53,
 /*   830 */    23,  174,  175,   26,   23,   23,   23,   26,   26,   26,
 /*   840 */    36,  106,  146,  147,  174,  175,   19,   71,   72,   73,
 /*   850 */    74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   860 */   152,   85,   86,   87,   88,   89,   90,   91,   92,   93,
 /*   870 */    94,   95,  152,  196,  119,  120,   19,   50,   51,  168,
 /*   880 */   169,   26,  174,  175,  207,   28,  152,  249,  250,  152,
 /*   890 */   163,  163,  163,  163,  174,  175,  163,   19,   71,   72,
 /*   900 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   910 */    83,  152,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   920 */    93,   94,   95,  152,  197,  197,  197,  197,   50,   51,
 /*   930 */   197,  194,   36,  174,  175,  191,  192,  152,  191,  192,
 /*   940 */   163,  152,   66,  124,  152,  174,  175,  152,   19,   71,
 /*   950 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   960 */    82,   83,  152,   85,   86,   87,   88,   89,   90,   91,
 /*   970 */    92,   93,   94,   95,  197,  152,  100,  188,  152,   50,
 /*   980 */    51,  152,  152,  188,  174,  175,  252,  152,   94,   95,
 /*   990 */   152,  152,  152,    1,    2,  152,  152,  174,  175,   19,
 /*  1000 */   152,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  1010 */    81,   82,   83,  152,   85,   86,   87,   88,   89,   90,
 /*  1020 */    91,   92,   93,   94,   95,  152,  188,  188,   22,  194,
 /*  1030 */    50,   51,  240,  173,  194,  174,  175,  252,  194,  152,
 /*  1040 */    36,  181,   28,  152,   23,  219,  122,  174,  175,  219,
 /*  1050 */   221,  152,  152,   73,   74,   75,   76,   77,   78,   79,
 /*  1060 */    80,   81,   82,   83,  152,   85,   86,   87,   88,   89,
 /*  1070 */    90,   91,   92,   93,   94,   95,   19,   20,  152,   22,
 /*  1080 */    23,  194,  152,  240,   27,   28,  174,  175,  240,   19,
 /*  1090 */    20,   26,   22,  194,  194,   38,   22,   27,   28,  152,
 /*  1100 */    23,   22,  152,  116,  174,  175,  152,   23,   38,  152,
 /*  1110 */    23,  152,  221,  152,   57,  152,   23,  163,   50,   51,
 /*  1120 */   194,  174,  175,   66,  174,  175,   69,   57,  174,  175,
 /*  1130 */    40,  174,  175,  174,  175,  174,  175,  174,  175,   69,
 /*  1140 */    22,   53,   74,   75,   30,   53,   89,   90,   22,   22,
 /*  1150 */   152,  197,   23,   96,   97,   98,   22,  152,  101,   89,
 /*  1160 */    90,   91,  208,  209,  152,   53,   96,   97,   98,  101,
 /*  1170 */    22,  101,  174,  175,  152,   19,   20,  105,   22,  174,
 /*  1180 */   175,  112,   19,   27,   28,   20,  174,  175,   24,  132,
 /*  1190 */   133,  134,  135,  136,   38,   44,  174,  175,  107,   61,
 /*  1200 */    54,   26,  132,  133,  134,  135,  136,   54,  107,   22,
 /*  1210 */     5,  140,    1,   57,   36,  111,  122,   28,   79,   79,
 /*  1220 */   131,  123,   66,   19,   20,   69,   22,    1,   16,   20,
 /*  1230 */   125,   27,   28,  123,  111,  120,   23,  131,   23,   16,
 /*  1240 */    68,  142,   38,   15,   22,   89,   90,    3,  167,    4,
 /*  1250 */   248,  251,   96,   97,   98,  180,  180,  101,  251,  151,
 /*  1260 */     6,   57,  151,   13,  151,   26,   25,  151,  161,  202,
 /*  1270 */   153,  162,  153,   69,  130,  128,  203,   19,   20,  127,
 /*  1280 */    22,  126,  204,  129,   22,   27,   28,  205,  132,  133,
 /*  1290 */   134,  135,  136,   89,   90,  231,   38,   95,  137,  179,
 /*  1300 */    96,   97,   98,  206,  179,  101,  122,  107,  159,  159,
 /*  1310 */   125,  231,  216,  228,  107,   57,  184,  217,  216,  176,
 /*  1320 */   217,  176,   48,  106,   18,  184,  158,   69,  159,  158,
 /*  1330 */    46,   71,  237,  176,  176,  176,  132,  133,  134,  135,
 /*  1340 */   136,  217,  176,  137,  216,  178,  158,   89,   90,  179,
 /*  1350 */   176,  159,  179,  159,   96,   97,   98,  159,  159,  101,
 /*  1360 */     5,  158,  202,   22,   18,   10,   11,   12,   13,   14,
 /*  1370 */   190,  238,   17,  190,  158,  193,   41,  159,  202,  193,
 /*  1380 */   159,  202,  245,  193,  193,  223,  190,   32,  159,   34,
 /*  1390 */   132,  133,  134,  135,  136,  159,   39,  155,   43,  150,
 /*  1400 */   223,  177,  201,  178,  177,  186,   66,  199,  177,  152,
 /*  1410 */   253,   56,  215,  152,  182,  152,  202,  152,   63,  152,
 /*  1420 */   152,   66,   67,  242,  229,  152,  174,  152,  152,  152,
 /*  1430 */   152,  152,  152,  152,  199,  242,  202,  152,  198,  152,
 /*  1440 */   152,  152,  183,  192,  152,  215,  152,  183,  215,  183,
 /*  1450 */   152,  241,  214,  152,  211,  152,  152,  211,  211,  152,
 /*  1460 */   152,  241,  152,  152,  152,  152,  152,  152,  152,  114,
 /*  1470 */   152,  152,  235,  152,  152,  152,  174,  187,   95,  174,
 /*  1480 */   253,  253,  253,  253,  236,  253,  253,  253,  253,  253,
 /*  1490 */   253,  253,  253,  253,  253,  253,  141,
};
#define YY_SHIFT_USE_DFLT (-86)
#define YY_SHIFT_COUNT (429)
#define YY_SHIFT_MIN   (-85)
#define YY_SHIFT_MAX   (1383)
static const short yy_shift_ofst[] = {
 /*     0 */   992, 1057, 1355, 1156, 1204, 1204,    1,  262,  -19,  135,
 /*    10 */   135,  776, 1204, 1204, 1204, 1204,   69,   69,   53,  208,
 /*    20 */   283,  755,   58,  725,  648,  571,  494,  417,  340,  263,
 /*    30 */   212,  827,  827,  827,  827,  827,  827,  827,  827,  827,
 /*    40 */   827,  827,  827,  827,  827,  827,  878,  827,  929,  980,
 /*    50 */   980, 1070, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    60 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    70 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    80 */  1258, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    90 */  1204, 1204, 1204, 1204,  -71,  -47,  -47,  -47,  -47,  -47,
 /*   100 */     0,   29,  -12,  283,  283,  139,   91,  392,  392,  894,
 /*   110 */   672,  726, 1383,  -86,  -86,  -86,   88,  318,  318,   99,
 /*   120 */   381,  -20,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   130 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   140 */   283,  283,  283,  283,  624,  876,  726,  672, 1340, 1340,
 /*   150 */  1340, 1340, 1340, 1340,  -86,  -86,  -86,  305,  136,  136,
 /*   160 */   142,  167,  226,  154,  137,  152,  283,  283,  283,  283,
 /*   170 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   180 */   283,  283,  283,  336,  336,  336,  283,  283,  352,  283,
 /*   190 */   283,  283,  283,  283,  228,  283,  283,  283,  283,  283,
 /*   200 */   283,  283,  283,  283,  283,  501,  569,  596,  596,  596,
 /*   210 */   507,  497,  441,  391,  353,  156,  156,  857,  353,  857,
 /*   220 */   735,  813,  639,  715,  156,  332,  715,  715,  496,  419,
 /*   230 */   646, 1357, 1184, 1184, 1335, 1335, 1184, 1341, 1260, 1144,
 /*   240 */  1346, 1346, 1346, 1346, 1184, 1306, 1144, 1341, 1260, 1260,
 /*   250 */  1144, 1184, 1306, 1206, 1284, 1184, 1184, 1306, 1184, 1306,
 /*   260 */  1184, 1306, 1262, 1207, 1207, 1207, 1274, 1262, 1207, 1217,
 /*   270 */  1207, 1274, 1207, 1207, 1185, 1200, 1185, 1200, 1185, 1200,
 /*   280 */  1184, 1184, 1161, 1262, 1202, 1202, 1262, 1154, 1155, 1147,
 /*   290 */  1152, 1144, 1241, 1239, 1250, 1250, 1254, 1254, 1254, 1254,
 /*   300 */   -86,  -86,  -86,  -86,  -86,  -86, 1068,  304,  526,  249,
 /*   310 */   408,  -83,  434,  812,   27,  811,  807,  802,  751,  589,
 /*   320 */   651,  163,  131,  674,  366,  450,  299,  148,   23,  102,
 /*   330 */   229,  -21, 1245, 1244, 1222, 1099, 1228, 1172, 1223, 1215,
 /*   340 */  1213, 1115, 1106, 1123, 1110, 1209, 1105, 1212, 1226, 1098,
 /*   350 */  1089, 1140, 1139, 1104, 1189, 1178, 1094, 1211, 1205, 1187,
 /*   360 */  1101, 1071, 1153, 1175, 1146, 1138, 1151, 1091, 1164, 1165,
 /*   370 */  1163, 1069, 1072, 1148, 1112, 1134, 1127, 1129, 1126, 1092,
 /*   380 */  1114, 1118, 1088, 1090, 1093, 1087, 1084,  987, 1079, 1077,
 /*   390 */  1074, 1065,  924, 1021, 1014, 1004, 1006,  819,  739,  896,
 /*   400 */   855,  804,  739,  740,  736,  690,  654,  665,  618,  582,
 /*   410 */   568,  528,  554,  379,  532,  479,  455,  379,  432,  371,
 /*   420 */   341,   28,  338,  116,  -11,  -57,  -85,    7,   -8,    3,
};
#define YY_REDUCE_USE_DFLT (-110)
#define YY_REDUCE_COUNT (305)
#define YY_REDUCE_MIN   (-109)
#define YY_REDUCE_MAX   (1323)
static const short yy_reduce_ofst[] = {
 /*     0 */   238,  954,  213,  289,  310,  234,  144,  317, -109,  382,
 /*    10 */   377,  303,  461,  389,  378,  368,  302,  294,  253,  395,
 /*    20 */   293,  324,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    30 */   403,  403,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    40 */   403,  403,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    50 */   403, 1022, 1012, 1005,  998,  963,  961,  959,  957,  950,
 /*    60 */   947,  930,  912,  873,  861,  823,  810,  771,  759,  720,
 /*    70 */   708,  670,  657,  619,  614,  612,  610,  608,  606,  604,
 /*    80 */   598,  595,  593,  580,  542,  540,  537,  535,  533,  531,
 /*    90 */   529,  527,  503,  386,  403,  403,  403,  403,  403,  403,
 /*   100 */   403,  403,  403,   95,  447,   82,  334,  504,  467,  403,
 /*   110 */   477,  464,  403,  403,  403,  403,  860,  747,  744,  785,
 /*   120 */   638,  638,  926,  891,  900,  899,  887,  844,  840,  835,
 /*   130 */   848,  830,  843,  829,  792,  839,  826,  737,  838,  795,
 /*   140 */   789,   47,  734,  530,  696,  777,  711,  677,  733,  730,
 /*   150 */   729,  728,  727,  627,  448,   64,  187, 1305, 1302, 1252,
 /*   160 */  1290, 1273, 1323, 1322, 1321, 1319, 1318, 1316, 1315, 1314,
 /*   170 */  1313, 1312, 1311, 1310, 1308, 1307, 1304, 1303, 1301, 1298,
 /*   180 */  1294, 1292, 1289, 1266, 1264, 1259, 1288, 1287, 1238, 1285,
 /*   190 */  1281, 1280, 1279, 1278, 1251, 1277, 1276, 1275, 1273, 1268,
 /*   200 */  1267, 1265, 1263, 1261, 1257, 1248, 1237, 1247, 1246, 1243,
 /*   210 */  1238, 1240, 1235, 1249, 1234, 1233, 1230, 1220, 1214, 1210,
 /*   220 */  1225, 1219, 1232, 1231, 1197, 1195, 1227, 1224, 1201, 1208,
 /*   230 */  1242, 1137, 1236, 1229, 1193, 1181, 1221, 1177, 1196, 1179,
 /*   240 */  1191, 1190, 1186, 1182, 1218, 1216, 1176, 1162, 1183, 1180,
 /*   250 */  1160, 1199, 1203, 1133, 1095, 1198, 1194, 1188, 1192, 1171,
 /*   260 */  1169, 1168, 1173, 1174, 1166, 1159, 1141, 1170, 1158, 1167,
 /*   270 */  1157, 1132, 1145, 1143, 1124, 1128, 1103, 1102, 1100, 1096,
 /*   280 */  1150, 1149, 1085, 1125, 1080, 1064, 1120, 1097, 1082, 1078,
 /*   290 */  1073, 1067, 1109, 1107, 1119, 1117, 1116, 1113, 1111, 1108,
 /*   300 */  1007, 1000, 1002, 1076, 1075, 1081,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   647,  964,  964,  964,  878,  878,  969,  964,  774,  802,
 /*    10 */   802,  938,  969,  969,  969,  876,  969,  969,  969,  964,
 /*    20 */   969,  778,  808,  969,  969,  969,  969,  969,  969,  969,
 /*    30 */   969,  937,  939,  816,  815,  918,  789,  813,  806,  810,
 /*    40 */   879,  872,  873,  871,  875,  880,  969,  809,  841,  856,
 /*    50 */   840,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    60 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    70 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    80 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    90 */   969,  969,  969,  969,  850,  855,  862,  854,  851,  843,
 /*   100 */   842,  844,  845,  969,  969,  673,  739,  969,  969,  846,
 /*   110 */   969,  685,  847,  859,  858,  857,  680,  969,  969,  969,
 /*   120 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   130 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   140 */   969,  969,  969,  969,  647,  964,  969,  969,  964,  964,
 /*   150 */   964,  964,  964,  964,  956,  778,  768,  969,  969,  969,
 /*   160 */   969,  969,  969,  969,  969,  969,  969,  944,  942,  969,
 /*   170 */   891,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   180 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   190 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   200 */   969,  969,  969,  969,  653,  969,  911,  774,  774,  774,
 /*   210 */   776,  754,  766,  655,  812,  791,  791,  923,  812,  923,
 /*   220 */   710,  733,  707,  802,  791,  874,  802,  802,  775,  766,
 /*   230 */   969,  949,  782,  782,  941,  941,  782,  821,  743,  812,
 /*   240 */   750,  750,  750,  750,  782,  670,  812,  821,  743,  743,
 /*   250 */   812,  782,  670,  917,  915,  782,  782,  670,  782,  670,
 /*   260 */   782,  670,  884,  741,  741,  741,  725,  884,  741,  710,
 /*   270 */   741,  725,  741,  741,  795,  790,  795,  790,  795,  790,
 /*   280 */   782,  782,  969,  884,  888,  888,  884,  807,  796,  805,
 /*   290 */   803,  812,  676,  728,  663,  663,  652,  652,  652,  652,
 /*   300 */   961,  961,  956,  712,  712,  695,  969,  969,  969,  969,
 /*   310 */   969,  969,  687,  969,  893,  969,  969,  969,  969,  969,
 /*   320 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   330 */   969,  828,  969,  648,  951,  969,  969,  948,  969,  969,
 /*   340 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   350 */   969,  969,  969,  969,  969,  969,  921,  969,  969,  969,
 /*   360 */   969,  969,  969,  914,  913,  969,  969,  969,  969,  969,
 /*   370 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   380 */   969,  969,  969,  969,  969,  969,  969,  757,  969,  969,
 /*   390 */   969,  761,  969,  969,  969,  969,  969,  969,  804,  969,
 /*   400 */   797,  969,  877,  969,  969,  969,  969,  969,  969,  969,
 /*   410 */   969,  969,  969,  966,  969,  969,  969,  965,  969,  969,
 /*   420 */   969,  969,  969,  830,  969,  829,  833,  969,  661,  969,
 /*   430 */   644,  649,  960,  963,  962,  959,  958,  957,  952,  950,
 /*   440 */   947,  946,  945,  943,  940,  936,  897,  895,  902,  901,
 /*   450 */   900,  899,  898,  896,  894,  892,  818,  817,  814,  811,
 /*   460 */   753,  935,  890,  752,  749,  748,  669,  953,  920,  929,
 /*   470 */   928,  927,  822,  926,  925,  924,  922,  919,  906,  820,
 /*   480 */   819,  744,  882,  881,  672,  910,  909,  908,  912,  916,
 /*   490 */   907,  784,  751,  671,  668,  675,  679,  731,  732,  740,
 /*   500 */   738,  737,  736,  735,  734,  730,  681,  686,  724,  709,
 /*   510 */   708,  717,  716,  722,  721,  720,  719,  718,  715,  714,
 /*   520 */   713,  706,  705,  711,  704,  727,  726,  723,  703,  747,
 /*   530 */   746,  745,  742,  702,  701,  700,  833,  699,  698,  838,
 /*   540 */   837,  866,  826,  755,  759,  758,  762,  763,  771,  770,
 /*   550 */   769,  780,  781,  793,  792,  824,  823,  794,  779,  773,
 /*   560 */   772,  788,  787,  786,  785,  777,  767,  799,  798,  868,
 /*   570 */   783,  867,  865,  934,  933,  932,  931,  930,  870,  967,
 /*   580 */   968,  887,  889,  886,  801,  800,  885,  869,  839,  836,
 /*   590 */   690,  691,  905,  904,  903,  693,  692,  689,  688,  863,
 /*   600 */   860,  852,  864,  861,  853,  849,  848,  834,  832,  831,
 /*   610 */   827,  835,  760,  756,  825,  765,  764,  697,  696,  694,
 /*   620 */   678,  677,  674,  667,  665,  664,  666,  662,  660,  659,
 /*   630 */   658,  657,  656,  684,  683,  682,  654,  651,  650,  646,
 /*   640 */   645,  643,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*       SEMI => nothing */
   27,  /*    EXPLAIN => ID */
   27,  /*      QUERY => ID */
   27,  /*       PLAN => ID */
   27,  /*      BEGIN => ID */
    0,  /* TRANSACTION => nothing */
   27,  /*   DEFERRED => ID */
   27,  /*  IMMEDIATE => ID */
   27,  /*  EXCLUSIVE => ID */
    0,  /*     COMMIT => nothing */
   27,  /*        END => ID */
   27,  /*   ROLLBACK => ID */
   27,  /*  SAVEPOINT => ID */
   27,  /*    RELEASE => ID */
    0,  /*         TO => nothing */
    0,  /*      TABLE => nothing */
    0,  /*     CREATE => nothing */
   27,  /*         IF => ID */
    0,  /*        NOT => nothing */
    0,  /*     EXISTS => nothing */
   27,  /*       TEMP => ID */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*         AS => nothing */
   27,  /*    WITHOUT => ID */
    0,  /*      COMMA => nothing */
    0,  /*         ID => nothing */
    0,  /*    INDEXED => nothing */
   27,  /*      ABORT => ID */
   27,  /*     ACTION => ID */
   27,  /*      AFTER => ID */
   27,  /*    ANALYZE => ID */
   27,  /*        ASC => ID */
   27,  /*     ATTACH => ID */
   27,  /*     BEFORE => ID */
   27,  /*         BY => ID */
   27,  /*    CASCADE => ID */
   27,  /*       CAST => ID */
   27,  /*   COLUMNKW => ID */
   27,  /*   CONFLICT => ID */
   27,  /*   DATABASE => ID */
   27,  /*       DESC => ID */
   27,  /*     DETACH => ID */
   27,  /*       EACH => ID */
   27,  /*       FAIL => ID */
   27,  /*        FOR => ID */
   27,  /*     IGNORE => ID */
   27,  /*  INITIALLY => ID */
   27,  /*    INSTEAD => ID */
   27,  /*    LIKE_KW => ID */
   27,  /*      MATCH => ID */
   27,  /*         NO => ID */
   27,  /*        KEY => ID */
   27,  /*         OF => ID */
   27,  /*     OFFSET => ID */
   27,  /*     PRAGMA => ID */
   27,  /*      RAISE => ID */
   27,  /*  RECURSIVE => ID */
   27,  /*    REPLACE => ID */
   27,  /*   RESTRICT => ID */
   27,  /*        ROW => ID */
   27,  /*    TRIGGER => ID */
   27,  /*     VACUUM => ID */
   27,  /*       VIEW => ID */
   27,  /*    VIRTUAL => ID */
   27,  /*       WITH => ID */
   27,  /*    REINDEX => ID */
   27,  /*     RENAME => ID */
   27,  /*   CTIME_KW => ID */
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
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  sqlite3ParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
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
void sqlite3ParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "SEMI",          "EXPLAIN",       "QUERY",       
  "PLAN",          "BEGIN",         "TRANSACTION",   "DEFERRED",    
  "IMMEDIATE",     "EXCLUSIVE",     "COMMIT",        "END",         
  "ROLLBACK",      "SAVEPOINT",     "RELEASE",       "TO",          
  "TABLE",         "CREATE",        "IF",            "NOT",         
  "EXISTS",        "TEMP",          "LP",            "RP",          
  "AS",            "WITHOUT",       "COMMA",         "ID",          
  "INDEXED",       "ABORT",         "ACTION",        "AFTER",       
  "ANALYZE",       "ASC",           "ATTACH",        "BEFORE",      
  "BY",            "CASCADE",       "CAST",          "COLUMNKW",    
  "CONFLICT",      "DATABASE",      "DESC",          "DETACH",      
  "EACH",          "FAIL",          "FOR",           "IGNORE",      
  "INITIALLY",     "INSTEAD",       "LIKE_KW",       "MATCH",       
  "NO",            "KEY",           "OF",            "OFFSET",      
  "PRAGMA",        "RAISE",         "RECURSIVE",     "REPLACE",     
  "RESTRICT",      "ROW",           "TRIGGER",       "VACUUM",      
  "VIEW",          "VIRTUAL",       "WITH",          "REINDEX",     
  "RENAME",        "CTIME_KW",      "ANY",           "OR",          
  "AND",           "IS",            "BETWEEN",       "IN",          
  "ISNULL",        "NOTNULL",       "NE",            "EQ",          
  "GT",            "LE",            "LT",            "GE",          
  "ESCAPE",        "BITAND",        "BITOR",         "LSHIFT",      
  "RSHIFT",        "PLUS",          "MINUS",         "STAR",        
  "SLASH",         "REM",           "CONCAT",        "COLLATE",     
  "BITNOT",        "STRING",        "JOIN_KW",       "CONSTRAINT",  
  "DEFAULT",       "NULL",          "PRIMARY",       "UNIQUE",      
  "CHECK",         "REFERENCES",    "AUTOINCR",      "ON",          
  "INSERT",        "DELETE",        "UPDATE",        "SET",         
  "DEFERRABLE",    "FOREIGN",       "DROP",          "UNION",       
  "ALL",           "EXCEPT",        "INTERSECT",     "SELECT",      
  "VALUES",        "DISTINCT",      "DOT",           "FROM",        
  "JOIN",          "USING",         "ORDER",         "GROUP",       
  "HAVING",        "LIMIT",         "WHERE",         "INTO",        
  "INTEGER",       "FLOAT",         "BLOB",          "VARIABLE",    
  "CASE",          "WHEN",          "THEN",          "ELSE",        
  "INDEX",         "ALTER",         "ADD",           "error",       
  "input",         "cmdlist",       "ecmd",          "explain",     
  "cmdx",          "cmd",           "transtype",     "trans_opt",   
  "nm",            "savepoint_opt",  "create_table",  "create_table_args",
  "createkw",      "temp",          "ifnotexists",   "dbnm",        
  "columnlist",    "conslist_opt",  "table_options",  "select",      
  "column",        "columnid",      "type",          "carglist",    
  "typetoken",     "typename",      "signed",        "plus_num",    
  "minus_num",     "ccons",         "term",          "expr",        
  "onconf",        "sortorder",     "autoinc",       "idxlist_opt", 
  "refargs",       "defer_subclause",  "refarg",        "refact",      
  "init_deferred_pred_opt",  "conslist",      "tconscomma",    "tcons",       
  "idxlist",       "defer_subclause_opt",  "orconf",        "resolvetype", 
  "raisetype",     "ifexists",      "fullname",      "selectnowith",
  "oneselect",     "with",          "multiselect_op",  "distinct",    
  "selcollist",    "from",          "where_opt",     "groupby_opt", 
  "having_opt",    "orderby_opt",   "limit_opt",     "values",      
  "nexprlist",     "exprlist",      "sclp",          "as",          
  "seltablist",    "stl_prefix",    "joinop",        "indexed_opt", 
  "on_opt",        "using_opt",     "joinop2",       "idlist",      
  "sortlist",      "setlist",       "insert_cmd",    "inscollist_opt",
  "likeop",        "between_op",    "in_op",         "case_operand",
  "case_exprlist",  "case_else",     "uniqueflag",    "collate",     
  "nmnum",         "trigger_decl",  "trigger_cmd_list",  "trigger_time",
  "trigger_event",  "foreach_clause",  "when_clause",   "trigger_cmd", 
  "trnm",          "tridxby",       "database_kw_opt",  "key_opt",     
  "add_column_fullname",  "kwcolumn_opt",  "create_vtab",   "vtabarglist", 
  "vtabarg",       "vtabargtoken",  "lp",            "anylist",     
  "wqlist",      
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= cmdlist",
 /*   1 */ "cmdlist ::= cmdlist ecmd",
 /*   2 */ "cmdlist ::= ecmd",
 /*   3 */ "ecmd ::= SEMI",
 /*   4 */ "ecmd ::= explain cmdx SEMI",
 /*   5 */ "explain ::=",
 /*   6 */ "explain ::= EXPLAIN",
 /*   7 */ "explain ::= EXPLAIN QUERY PLAN",
 /*   8 */ "cmdx ::= cmd",
 /*   9 */ "cmd ::= BEGIN transtype trans_opt",
 /*  10 */ "trans_opt ::=",
 /*  11 */ "trans_opt ::= TRANSACTION",
 /*  12 */ "trans_opt ::= TRANSACTION nm",
 /*  13 */ "transtype ::=",
 /*  14 */ "transtype ::= DEFERRED",
 /*  15 */ "transtype ::= IMMEDIATE",
 /*  16 */ "transtype ::= EXCLUSIVE",
 /*  17 */ "cmd ::= COMMIT trans_opt",
 /*  18 */ "cmd ::= END trans_opt",
 /*  19 */ "cmd ::= ROLLBACK trans_opt",
 /*  20 */ "savepoint_opt ::= SAVEPOINT",
 /*  21 */ "savepoint_opt ::=",
 /*  22 */ "cmd ::= SAVEPOINT nm",
 /*  23 */ "cmd ::= RELEASE savepoint_opt nm",
 /*  24 */ "cmd ::= ROLLBACK trans_opt TO savepoint_opt nm",
 /*  25 */ "cmd ::= create_table create_table_args",
 /*  26 */ "create_table ::= createkw temp TABLE ifnotexists nm dbnm",
 /*  27 */ "createkw ::= CREATE",
 /*  28 */ "ifnotexists ::=",
 /*  29 */ "ifnotexists ::= IF NOT EXISTS",
 /*  30 */ "temp ::= TEMP",
 /*  31 */ "temp ::=",
 /*  32 */ "create_table_args ::= LP columnlist conslist_opt RP table_options",
 /*  33 */ "create_table_args ::= AS select",
 /*  34 */ "table_options ::=",
 /*  35 */ "table_options ::= WITHOUT nm",
 /*  36 */ "columnlist ::= columnlist COMMA column",
 /*  37 */ "columnlist ::= column",
 /*  38 */ "column ::= columnid type carglist",
 /*  39 */ "columnid ::= nm",
 /*  40 */ "nm ::= ID|INDEXED",
 /*  41 */ "nm ::= STRING",
 /*  42 */ "nm ::= JOIN_KW",
 /*  43 */ "type ::=",
 /*  44 */ "type ::= typetoken",
 /*  45 */ "typetoken ::= typename",
 /*  46 */ "typetoken ::= typename LP signed RP",
 /*  47 */ "typetoken ::= typename LP signed COMMA signed RP",
 /*  48 */ "typename ::= ID|STRING",
 /*  49 */ "typename ::= typename ID|STRING",
 /*  50 */ "signed ::= plus_num",
 /*  51 */ "signed ::= minus_num",
 /*  52 */ "carglist ::= carglist ccons",
 /*  53 */ "carglist ::=",
 /*  54 */ "ccons ::= CONSTRAINT nm",
 /*  55 */ "ccons ::= DEFAULT term",
 /*  56 */ "ccons ::= DEFAULT LP expr RP",
 /*  57 */ "ccons ::= DEFAULT PLUS term",
 /*  58 */ "ccons ::= DEFAULT MINUS term",
 /*  59 */ "ccons ::= DEFAULT ID|INDEXED",
 /*  60 */ "ccons ::= NULL onconf",
 /*  61 */ "ccons ::= NOT NULL onconf",
 /*  62 */ "ccons ::= PRIMARY KEY sortorder onconf autoinc",
 /*  63 */ "ccons ::= UNIQUE onconf",
 /*  64 */ "ccons ::= CHECK LP expr RP",
 /*  65 */ "ccons ::= REFERENCES nm idxlist_opt refargs",
 /*  66 */ "ccons ::= defer_subclause",
 /*  67 */ "ccons ::= COLLATE ID|STRING",
 /*  68 */ "autoinc ::=",
 /*  69 */ "autoinc ::= AUTOINCR",
 /*  70 */ "refargs ::=",
 /*  71 */ "refargs ::= refargs refarg",
 /*  72 */ "refarg ::= MATCH nm",
 /*  73 */ "refarg ::= ON INSERT refact",
 /*  74 */ "refarg ::= ON DELETE refact",
 /*  75 */ "refarg ::= ON UPDATE refact",
 /*  76 */ "refact ::= SET NULL",
 /*  77 */ "refact ::= SET DEFAULT",
 /*  78 */ "refact ::= CASCADE",
 /*  79 */ "refact ::= RESTRICT",
 /*  80 */ "refact ::= NO ACTION",
 /*  81 */ "defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt",
 /*  82 */ "defer_subclause ::= DEFERRABLE init_deferred_pred_opt",
 /*  83 */ "init_deferred_pred_opt ::=",
 /*  84 */ "init_deferred_pred_opt ::= INITIALLY DEFERRED",
 /*  85 */ "init_deferred_pred_opt ::= INITIALLY IMMEDIATE",
 /*  86 */ "conslist_opt ::=",
 /*  87 */ "conslist_opt ::= COMMA conslist",
 /*  88 */ "conslist ::= conslist tconscomma tcons",
 /*  89 */ "conslist ::= tcons",
 /*  90 */ "tconscomma ::= COMMA",
 /*  91 */ "tconscomma ::=",
 /*  92 */ "tcons ::= CONSTRAINT nm",
 /*  93 */ "tcons ::= PRIMARY KEY LP idxlist autoinc RP onconf",
 /*  94 */ "tcons ::= UNIQUE LP idxlist RP onconf",
 /*  95 */ "tcons ::= CHECK LP expr RP onconf",
 /*  96 */ "tcons ::= FOREIGN KEY LP idxlist RP REFERENCES nm idxlist_opt refargs defer_subclause_opt",
 /*  97 */ "defer_subclause_opt ::=",
 /*  98 */ "defer_subclause_opt ::= defer_subclause",
 /*  99 */ "onconf ::=",
 /* 100 */ "onconf ::= ON CONFLICT resolvetype",
 /* 101 */ "orconf ::=",
 /* 102 */ "orconf ::= OR resolvetype",
 /* 103 */ "resolvetype ::= raisetype",
 /* 104 */ "resolvetype ::= IGNORE",
 /* 105 */ "resolvetype ::= REPLACE",
 /* 106 */ "cmd ::= DROP TABLE ifexists fullname",
 /* 107 */ "ifexists ::= IF EXISTS",
 /* 108 */ "ifexists ::=",
 /* 109 */ "cmd ::= createkw temp VIEW ifnotexists nm dbnm AS select",
 /* 110 */ "cmd ::= DROP VIEW ifexists fullname",
 /* 111 */ "cmd ::= select",
 /* 112 */ "select ::= with selectnowith",
 /* 113 */ "selectnowith ::= oneselect",
 /* 114 */ "selectnowith ::= selectnowith multiselect_op oneselect",
 /* 115 */ "multiselect_op ::= UNION",
 /* 116 */ "multiselect_op ::= UNION ALL",
 /* 117 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /* 118 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /* 119 */ "oneselect ::= values",
 /* 120 */ "values ::= VALUES LP nexprlist RP",
 /* 121 */ "values ::= values COMMA LP exprlist RP",
 /* 122 */ "distinct ::= DISTINCT",
 /* 123 */ "distinct ::= ALL",
 /* 124 */ "distinct ::=",
 /* 125 */ "sclp ::= selcollist COMMA",
 /* 126 */ "sclp ::=",
 /* 127 */ "selcollist ::= sclp expr as",
 /* 128 */ "selcollist ::= sclp STAR",
 /* 129 */ "selcollist ::= sclp nm DOT STAR",
 /* 130 */ "as ::= AS nm",
 /* 131 */ "as ::= ID|STRING",
 /* 132 */ "as ::=",
 /* 133 */ "from ::=",
 /* 134 */ "from ::= FROM seltablist",
 /* 135 */ "stl_prefix ::= seltablist joinop",
 /* 136 */ "stl_prefix ::=",
 /* 137 */ "seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt",
 /* 138 */ "seltablist ::= stl_prefix LP select RP as on_opt using_opt",
 /* 139 */ "seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt",
 /* 140 */ "dbnm ::=",
 /* 141 */ "dbnm ::= DOT nm",
 /* 142 */ "fullname ::= nm dbnm",
 /* 143 */ "joinop ::= COMMA|JOIN",
 /* 144 */ "joinop ::= JOIN_KW JOIN",
 /* 145 */ "joinop ::= JOIN_KW nm JOIN",
 /* 146 */ "joinop ::= JOIN_KW nm nm JOIN",
 /* 147 */ "on_opt ::= ON expr",
 /* 148 */ "on_opt ::=",
 /* 149 */ "indexed_opt ::=",
 /* 150 */ "indexed_opt ::= INDEXED BY nm",
 /* 151 */ "indexed_opt ::= NOT INDEXED",
 /* 152 */ "using_opt ::= USING LP idlist RP",
 /* 153 */ "using_opt ::=",
 /* 154 */ "orderby_opt ::=",
 /* 155 */ "orderby_opt ::= ORDER BY sortlist",
 /* 156 */ "sortlist ::= sortlist COMMA expr sortorder",
 /* 157 */ "sortlist ::= expr sortorder",
 /* 158 */ "sortorder ::= ASC",
 /* 159 */ "sortorder ::= DESC",
 /* 160 */ "sortorder ::=",
 /* 161 */ "groupby_opt ::=",
 /* 162 */ "groupby_opt ::= GROUP BY nexprlist",
 /* 163 */ "having_opt ::=",
 /* 164 */ "having_opt ::= HAVING expr",
 /* 165 */ "limit_opt ::=",
 /* 166 */ "limit_opt ::= LIMIT expr",
 /* 167 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /* 168 */ "limit_opt ::= LIMIT expr COMMA expr",
 /* 169 */ "cmd ::= with DELETE FROM fullname indexed_opt where_opt",
 /* 170 */ "where_opt ::=",
 /* 171 */ "where_opt ::= WHERE expr",
 /* 172 */ "cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt",
 /* 173 */ "setlist ::= setlist COMMA nm EQ expr",
 /* 174 */ "setlist ::= nm EQ expr",
 /* 175 */ "cmd ::= with insert_cmd INTO fullname inscollist_opt select",
 /* 176 */ "cmd ::= with insert_cmd INTO fullname inscollist_opt DEFAULT VALUES",
 /* 177 */ "insert_cmd ::= INSERT orconf",
 /* 178 */ "insert_cmd ::= REPLACE",
 /* 179 */ "inscollist_opt ::=",
 /* 180 */ "inscollist_opt ::= LP idlist RP",
 /* 181 */ "idlist ::= idlist COMMA nm",
 /* 182 */ "idlist ::= nm",
 /* 183 */ "expr ::= term",
 /* 184 */ "expr ::= LP expr RP",
 /* 185 */ "term ::= NULL",
 /* 186 */ "expr ::= ID|INDEXED",
 /* 187 */ "expr ::= JOIN_KW",
 /* 188 */ "expr ::= nm DOT nm",
 /* 189 */ "expr ::= nm DOT nm DOT nm",
 /* 190 */ "term ::= INTEGER|FLOAT|BLOB",
 /* 191 */ "term ::= STRING",
 /* 192 */ "expr ::= VARIABLE",
 /* 193 */ "expr ::= expr COLLATE ID|STRING",
 /* 194 */ "expr ::= CAST LP expr AS typetoken RP",
 /* 195 */ "expr ::= ID|INDEXED LP distinct exprlist RP",
 /* 196 */ "expr ::= ID|INDEXED LP STAR RP",
 /* 197 */ "term ::= CTIME_KW",
 /* 198 */ "expr ::= expr AND expr",
 /* 199 */ "expr ::= expr OR expr",
 /* 200 */ "expr ::= expr LT|GT|GE|LE expr",
 /* 201 */ "expr ::= expr EQ|NE expr",
 /* 202 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /* 203 */ "expr ::= expr PLUS|MINUS expr",
 /* 204 */ "expr ::= expr STAR|SLASH|REM expr",
 /* 205 */ "expr ::= expr CONCAT expr",
 /* 206 */ "likeop ::= LIKE_KW|MATCH",
 /* 207 */ "likeop ::= NOT LIKE_KW|MATCH",
 /* 208 */ "expr ::= expr likeop expr",
 /* 209 */ "expr ::= expr likeop expr ESCAPE expr",
 /* 210 */ "expr ::= expr ISNULL|NOTNULL",
 /* 211 */ "expr ::= expr NOT NULL",
 /* 212 */ "expr ::= expr IS expr",
 /* 213 */ "expr ::= expr IS NOT expr",
 /* 214 */ "expr ::= NOT expr",
 /* 215 */ "expr ::= BITNOT expr",
 /* 216 */ "expr ::= MINUS expr",
 /* 217 */ "expr ::= PLUS expr",
 /* 218 */ "between_op ::= BETWEEN",
 /* 219 */ "between_op ::= NOT BETWEEN",
 /* 220 */ "expr ::= expr between_op expr AND expr",
 /* 221 */ "in_op ::= IN",
 /* 222 */ "in_op ::= NOT IN",
 /* 223 */ "expr ::= expr in_op LP exprlist RP",
 /* 224 */ "expr ::= LP select RP",
 /* 225 */ "expr ::= expr in_op LP select RP",
 /* 226 */ "expr ::= expr in_op nm dbnm",
 /* 227 */ "expr ::= EXISTS LP select RP",
 /* 228 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 229 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 230 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 231 */ "case_else ::= ELSE expr",
 /* 232 */ "case_else ::=",
 /* 233 */ "case_operand ::= expr",
 /* 234 */ "case_operand ::=",
 /* 235 */ "exprlist ::= nexprlist",
 /* 236 */ "exprlist ::=",
 /* 237 */ "nexprlist ::= nexprlist COMMA expr",
 /* 238 */ "nexprlist ::= expr",
 /* 239 */ "cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP idxlist RP where_opt",
 /* 240 */ "uniqueflag ::= UNIQUE",
 /* 241 */ "uniqueflag ::=",
 /* 242 */ "idxlist_opt ::=",
 /* 243 */ "idxlist_opt ::= LP idxlist RP",
 /* 244 */ "idxlist ::= idxlist COMMA nm collate sortorder",
 /* 245 */ "idxlist ::= nm collate sortorder",
 /* 246 */ "collate ::=",
 /* 247 */ "collate ::= COLLATE ID|STRING",
 /* 248 */ "cmd ::= DROP INDEX ifexists fullname",
 /* 249 */ "cmd ::= VACUUM",
 /* 250 */ "cmd ::= VACUUM nm",
 /* 251 */ "cmd ::= PRAGMA nm dbnm",
 /* 252 */ "cmd ::= PRAGMA nm dbnm EQ nmnum",
 /* 253 */ "cmd ::= PRAGMA nm dbnm LP nmnum RP",
 /* 254 */ "cmd ::= PRAGMA nm dbnm EQ minus_num",
 /* 255 */ "cmd ::= PRAGMA nm dbnm LP minus_num RP",
 /* 256 */ "nmnum ::= plus_num",
 /* 257 */ "nmnum ::= nm",
 /* 258 */ "nmnum ::= ON",
 /* 259 */ "nmnum ::= DELETE",
 /* 260 */ "nmnum ::= DEFAULT",
 /* 261 */ "plus_num ::= PLUS INTEGER|FLOAT",
 /* 262 */ "plus_num ::= INTEGER|FLOAT",
 /* 263 */ "minus_num ::= MINUS INTEGER|FLOAT",
 /* 264 */ "cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END",
 /* 265 */ "trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause",
 /* 266 */ "trigger_time ::= BEFORE",
 /* 267 */ "trigger_time ::= AFTER",
 /* 268 */ "trigger_time ::= INSTEAD OF",
 /* 269 */ "trigger_time ::=",
 /* 270 */ "trigger_event ::= DELETE|INSERT",
 /* 271 */ "trigger_event ::= UPDATE",
 /* 272 */ "trigger_event ::= UPDATE OF idlist",
 /* 273 */ "foreach_clause ::=",
 /* 274 */ "foreach_clause ::= FOR EACH ROW",
 /* 275 */ "when_clause ::=",
 /* 276 */ "when_clause ::= WHEN expr",
 /* 277 */ "trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI",
 /* 278 */ "trigger_cmd_list ::= trigger_cmd SEMI",
 /* 279 */ "trnm ::= nm",
 /* 280 */ "trnm ::= nm DOT nm",
 /* 281 */ "tridxby ::=",
 /* 282 */ "tridxby ::= INDEXED BY nm",
 /* 283 */ "tridxby ::= NOT INDEXED",
 /* 284 */ "trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt",
 /* 285 */ "trigger_cmd ::= insert_cmd INTO trnm inscollist_opt select",
 /* 286 */ "trigger_cmd ::= DELETE FROM trnm tridxby where_opt",
 /* 287 */ "trigger_cmd ::= select",
 /* 288 */ "expr ::= RAISE LP IGNORE RP",
 /* 289 */ "expr ::= RAISE LP raisetype COMMA nm RP",
 /* 290 */ "raisetype ::= ROLLBACK",
 /* 291 */ "raisetype ::= ABORT",
 /* 292 */ "raisetype ::= FAIL",
 /* 293 */ "cmd ::= DROP TRIGGER ifexists fullname",
 /* 294 */ "cmd ::= ATTACH database_kw_opt expr AS expr key_opt",
 /* 295 */ "cmd ::= DETACH database_kw_opt expr",
 /* 296 */ "key_opt ::=",
 /* 297 */ "key_opt ::= KEY expr",
 /* 298 */ "database_kw_opt ::= DATABASE",
 /* 299 */ "database_kw_opt ::=",
 /* 300 */ "cmd ::= REINDEX",
 /* 301 */ "cmd ::= REINDEX nm dbnm",
 /* 302 */ "cmd ::= ANALYZE",
 /* 303 */ "cmd ::= ANALYZE nm dbnm",
 /* 304 */ "cmd ::= ALTER TABLE fullname RENAME TO nm",
 /* 305 */ "cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column",
 /* 306 */ "add_column_fullname ::= fullname",
 /* 307 */ "kwcolumn_opt ::=",
 /* 308 */ "kwcolumn_opt ::= COLUMNKW",
 /* 309 */ "cmd ::= create_vtab",
 /* 310 */ "cmd ::= create_vtab LP vtabarglist RP",
 /* 311 */ "create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm",
 /* 312 */ "vtabarglist ::= vtabarg",
 /* 313 */ "vtabarglist ::= vtabarglist COMMA vtabarg",
 /* 314 */ "vtabarg ::=",
 /* 315 */ "vtabarg ::= vtabarg vtabargtoken",
 /* 316 */ "vtabargtoken ::= ANY",
 /* 317 */ "vtabargtoken ::= lp anylist RP",
 /* 318 */ "lp ::= LP",
 /* 319 */ "anylist ::=",
 /* 320 */ "anylist ::= anylist LP anylist RP",
 /* 321 */ "anylist ::= anylist ANY",
 /* 322 */ "with ::=",
 /* 323 */ "with ::= WITH wqlist",
 /* 324 */ "with ::= WITH RECURSIVE wqlist",
 /* 325 */ "wqlist ::= nm idxlist_opt AS LP select RP",
 /* 326 */ "wqlist ::= wqlist COMMA nm idxlist_opt AS LP select RP",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
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
** to sqlite3Parser and sqlite3ParserFree.
*/
void *sqlite3ParserAlloc(void *(*mallocProc)(u64)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (u64)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  sqlite3ParserARG_FETCH;
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
    case 163: /* select */
    case 195: /* selectnowith */
    case 196: /* oneselect */
    case 207: /* values */
{
#line 406 "parse.y"
sqlite3SelectDelete(pParse->db, (yypminor->yy3));
#line 1418 "parse.c"
}
      break;
    case 174: /* term */
    case 175: /* expr */
{
#line 812 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy346).pExpr);
#line 1426 "parse.c"
}
      break;
    case 179: /* idxlist_opt */
    case 188: /* idxlist */
    case 200: /* selcollist */
    case 203: /* groupby_opt */
    case 205: /* orderby_opt */
    case 208: /* nexprlist */
    case 209: /* exprlist */
    case 210: /* sclp */
    case 220: /* sortlist */
    case 221: /* setlist */
    case 228: /* case_exprlist */
{
#line 1216 "parse.y"
sqlite3ExprListDelete(pParse->db, (yypminor->yy14));
#line 1443 "parse.c"
}
      break;
    case 194: /* fullname */
    case 201: /* from */
    case 212: /* seltablist */
    case 213: /* stl_prefix */
{
#line 630 "parse.y"
sqlite3SrcListDelete(pParse->db, (yypminor->yy65));
#line 1453 "parse.c"
}
      break;
    case 197: /* with */
    case 252: /* wqlist */
{
#line 1477 "parse.y"
sqlite3WithDelete(pParse->db, (yypminor->yy59));
#line 1461 "parse.c"
}
      break;
    case 202: /* where_opt */
    case 204: /* having_opt */
    case 216: /* on_opt */
    case 227: /* case_operand */
    case 229: /* case_else */
    case 238: /* when_clause */
    case 243: /* key_opt */
{
#line 739 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy132));
#line 1474 "parse.c"
}
      break;
    case 217: /* using_opt */
    case 219: /* idlist */
    case 223: /* inscollist_opt */
{
#line 662 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy408));
#line 1483 "parse.c"
}
      break;
    case 234: /* trigger_cmd_list */
    case 239: /* trigger_cmd */
{
#line 1313 "parse.y"
sqlite3DeleteTriggerStep(pParse->db, (yypminor->yy473));
#line 1491 "parse.c"
}
      break;
    case 236: /* trigger_event */
{
#line 1299 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy378).b);
#line 1498 "parse.c"
}
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
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  /* There is no mechanism by which the parser stack can be popped below
  ** empty in SQLite.  */
  if( NEVER(pParser->yyidx<0) ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
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
**       obtained from sqlite3ParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void sqlite3ParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  /* In SQLite, we never try to destroy a parser that was not successfully
  ** created in the first place. */
  if( NEVER(pParser==0) ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int sqlite3ParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

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
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
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
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
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
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   sqlite3ParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 37 "parse.y"

  UNUSED_PARAMETER(yypMinor); /* Silence some compiler warnings */
  sqlite3ErrorMsg(pParse, "parser stack overflow");
#line 1688 "parse.c"
   sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
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
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 144, 1 },
  { 145, 2 },
  { 145, 1 },
  { 146, 1 },
  { 146, 3 },
  { 147, 0 },
  { 147, 1 },
  { 147, 3 },
  { 148, 1 },
  { 149, 3 },
  { 151, 0 },
  { 151, 1 },
  { 151, 2 },
  { 150, 0 },
  { 150, 1 },
  { 150, 1 },
  { 150, 1 },
  { 149, 2 },
  { 149, 2 },
  { 149, 2 },
  { 153, 1 },
  { 153, 0 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 2 },
  { 154, 6 },
  { 156, 1 },
  { 158, 0 },
  { 158, 3 },
  { 157, 1 },
  { 157, 0 },
  { 155, 5 },
  { 155, 2 },
  { 162, 0 },
  { 162, 2 },
  { 160, 3 },
  { 160, 1 },
  { 164, 3 },
  { 165, 1 },
  { 152, 1 },
  { 152, 1 },
  { 152, 1 },
  { 166, 0 },
  { 166, 1 },
  { 168, 1 },
  { 168, 4 },
  { 168, 6 },
  { 169, 1 },
  { 169, 2 },
  { 170, 1 },
  { 170, 1 },
  { 167, 2 },
  { 167, 0 },
  { 173, 2 },
  { 173, 2 },
  { 173, 4 },
  { 173, 3 },
  { 173, 3 },
  { 173, 2 },
  { 173, 2 },
  { 173, 3 },
  { 173, 5 },
  { 173, 2 },
  { 173, 4 },
  { 173, 4 },
  { 173, 1 },
  { 173, 2 },
  { 178, 0 },
  { 178, 1 },
  { 180, 0 },
  { 180, 2 },
  { 182, 2 },
  { 182, 3 },
  { 182, 3 },
  { 182, 3 },
  { 183, 2 },
  { 183, 2 },
  { 183, 1 },
  { 183, 1 },
  { 183, 2 },
  { 181, 3 },
  { 181, 2 },
  { 184, 0 },
  { 184, 2 },
  { 184, 2 },
  { 161, 0 },
  { 161, 2 },
  { 185, 3 },
  { 185, 1 },
  { 186, 1 },
  { 186, 0 },
  { 187, 2 },
  { 187, 7 },
  { 187, 5 },
  { 187, 5 },
  { 187, 10 },
  { 189, 0 },
  { 189, 1 },
  { 176, 0 },
  { 176, 3 },
  { 190, 0 },
  { 190, 2 },
  { 191, 1 },
  { 191, 1 },
  { 191, 1 },
  { 149, 4 },
  { 193, 2 },
  { 193, 0 },
  { 149, 8 },
  { 149, 4 },
  { 149, 1 },
  { 163, 2 },
  { 195, 1 },
  { 195, 3 },
  { 198, 1 },
  { 198, 2 },
  { 198, 1 },
  { 196, 9 },
  { 196, 1 },
  { 207, 4 },
  { 207, 5 },
  { 199, 1 },
  { 199, 1 },
  { 199, 0 },
  { 210, 2 },
  { 210, 0 },
  { 200, 3 },
  { 200, 2 },
  { 200, 4 },
  { 211, 2 },
  { 211, 1 },
  { 211, 0 },
  { 201, 0 },
  { 201, 2 },
  { 213, 2 },
  { 213, 0 },
  { 212, 7 },
  { 212, 7 },
  { 212, 7 },
  { 159, 0 },
  { 159, 2 },
  { 194, 2 },
  { 214, 1 },
  { 214, 2 },
  { 214, 3 },
  { 214, 4 },
  { 216, 2 },
  { 216, 0 },
  { 215, 0 },
  { 215, 3 },
  { 215, 2 },
  { 217, 4 },
  { 217, 0 },
  { 205, 0 },
  { 205, 3 },
  { 220, 4 },
  { 220, 2 },
  { 177, 1 },
  { 177, 1 },
  { 177, 0 },
  { 203, 0 },
  { 203, 3 },
  { 204, 0 },
  { 204, 2 },
  { 206, 0 },
  { 206, 2 },
  { 206, 4 },
  { 206, 4 },
  { 149, 6 },
  { 202, 0 },
  { 202, 2 },
  { 149, 8 },
  { 221, 5 },
  { 221, 3 },
  { 149, 6 },
  { 149, 7 },
  { 222, 2 },
  { 222, 1 },
  { 223, 0 },
  { 223, 3 },
  { 219, 3 },
  { 219, 1 },
  { 175, 1 },
  { 175, 3 },
  { 174, 1 },
  { 175, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 5 },
  { 174, 1 },
  { 174, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 6 },
  { 175, 5 },
  { 175, 4 },
  { 174, 1 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 224, 1 },
  { 224, 2 },
  { 175, 3 },
  { 175, 5 },
  { 175, 2 },
  { 175, 3 },
  { 175, 3 },
  { 175, 4 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 225, 1 },
  { 225, 2 },
  { 175, 5 },
  { 226, 1 },
  { 226, 2 },
  { 175, 5 },
  { 175, 3 },
  { 175, 5 },
  { 175, 4 },
  { 175, 4 },
  { 175, 5 },
  { 228, 5 },
  { 228, 4 },
  { 229, 2 },
  { 229, 0 },
  { 227, 1 },
  { 227, 0 },
  { 209, 1 },
  { 209, 0 },
  { 208, 3 },
  { 208, 1 },
  { 149, 12 },
  { 230, 1 },
  { 230, 0 },
  { 179, 0 },
  { 179, 3 },
  { 188, 5 },
  { 188, 3 },
  { 231, 0 },
  { 231, 2 },
  { 149, 4 },
  { 149, 1 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 6 },
  { 149, 5 },
  { 149, 6 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 171, 2 },
  { 171, 1 },
  { 172, 2 },
  { 149, 5 },
  { 233, 11 },
  { 235, 1 },
  { 235, 1 },
  { 235, 2 },
  { 235, 0 },
  { 236, 1 },
  { 236, 1 },
  { 236, 3 },
  { 237, 0 },
  { 237, 3 },
  { 238, 0 },
  { 238, 2 },
  { 234, 3 },
  { 234, 2 },
  { 240, 1 },
  { 240, 3 },
  { 241, 0 },
  { 241, 3 },
  { 241, 2 },
  { 239, 7 },
  { 239, 5 },
  { 239, 5 },
  { 239, 1 },
  { 175, 4 },
  { 175, 6 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 149, 4 },
  { 149, 6 },
  { 149, 3 },
  { 243, 0 },
  { 243, 2 },
  { 242, 1 },
  { 242, 0 },
  { 149, 1 },
  { 149, 3 },
  { 149, 1 },
  { 149, 3 },
  { 149, 6 },
  { 149, 6 },
  { 244, 1 },
  { 245, 0 },
  { 245, 1 },
  { 149, 1 },
  { 149, 4 },
  { 246, 8 },
  { 247, 1 },
  { 247, 3 },
  { 248, 0 },
  { 248, 2 },
  { 249, 1 },
  { 249, 3 },
  { 250, 1 },
  { 251, 0 },
  { 251, 4 },
  { 251, 2 },
  { 197, 0 },
  { 197, 2 },
  { 197, 3 },
  { 252, 6 },
  { 252, 8 },
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
  sqlite3ParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 5: /* explain ::= */
#line 105 "parse.y"
{ sqlite3BeginParse(pParse, 0); }
#line 2129 "parse.c"
        break;
      case 6: /* explain ::= EXPLAIN */
#line 107 "parse.y"
{ sqlite3BeginParse(pParse, 1); }
#line 2134 "parse.c"
        break;
      case 7: /* explain ::= EXPLAIN QUERY PLAN */
#line 108 "parse.y"
{ sqlite3BeginParse(pParse, 2); }
#line 2139 "parse.c"
        break;
      case 8: /* cmdx ::= cmd */
#line 110 "parse.y"
{ sqlite3FinishCoding(pParse); }
#line 2144 "parse.c"
        break;
      case 9: /* cmd ::= BEGIN transtype trans_opt */
#line 115 "parse.y"
{sqlite3BeginTransaction(pParse, yymsp[-1].minor.yy328);}
#line 2149 "parse.c"
        break;
      case 13: /* transtype ::= */
#line 120 "parse.y"
{yygotominor.yy328 = TK_DEFERRED;}
#line 2154 "parse.c"
        break;
      case 14: /* transtype ::= DEFERRED */
      case 15: /* transtype ::= IMMEDIATE */ yytestcase(yyruleno==15);
      case 16: /* transtype ::= EXCLUSIVE */ yytestcase(yyruleno==16);
      case 115: /* multiselect_op ::= UNION */ yytestcase(yyruleno==115);
      case 117: /* multiselect_op ::= EXCEPT|INTERSECT */ yytestcase(yyruleno==117);
#line 121 "parse.y"
{yygotominor.yy328 = yymsp[0].major;}
#line 2163 "parse.c"
        break;
      case 17: /* cmd ::= COMMIT trans_opt */
      case 18: /* cmd ::= END trans_opt */ yytestcase(yyruleno==18);
#line 124 "parse.y"
{sqlite3CommitTransaction(pParse);}
#line 2169 "parse.c"
        break;
      case 19: /* cmd ::= ROLLBACK trans_opt */
#line 126 "parse.y"
{sqlite3RollbackTransaction(pParse);}
#line 2174 "parse.c"
        break;
      case 22: /* cmd ::= SAVEPOINT nm */
#line 130 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_BEGIN, &yymsp[0].minor.yy0);
}
#line 2181 "parse.c"
        break;
      case 23: /* cmd ::= RELEASE savepoint_opt nm */
#line 133 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_RELEASE, &yymsp[0].minor.yy0);
}
#line 2188 "parse.c"
        break;
      case 24: /* cmd ::= ROLLBACK trans_opt TO savepoint_opt nm */
#line 136 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_ROLLBACK, &yymsp[0].minor.yy0);
}
#line 2195 "parse.c"
        break;
      case 26: /* create_table ::= createkw temp TABLE ifnotexists nm dbnm */
#line 143 "parse.y"
{
   sqlite3StartTable(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,yymsp[-4].minor.yy328,0,0,yymsp[-2].minor.yy328);
}
#line 2202 "parse.c"
        break;
      case 27: /* createkw ::= CREATE */
#line 146 "parse.y"
{
  pParse->db->lookaside.bEnabled = 0;
  yygotominor.yy0 = yymsp[0].minor.yy0;
}
#line 2210 "parse.c"
        break;
      case 28: /* ifnotexists ::= */
      case 31: /* temp ::= */ yytestcase(yyruleno==31);
      case 68: /* autoinc ::= */ yytestcase(yyruleno==68);
      case 81: /* defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt */ yytestcase(yyruleno==81);
      case 83: /* init_deferred_pred_opt ::= */ yytestcase(yyruleno==83);
      case 85: /* init_deferred_pred_opt ::= INITIALLY IMMEDIATE */ yytestcase(yyruleno==85);
      case 97: /* defer_subclause_opt ::= */ yytestcase(yyruleno==97);
      case 108: /* ifexists ::= */ yytestcase(yyruleno==108);
      case 218: /* between_op ::= BETWEEN */ yytestcase(yyruleno==218);
      case 221: /* in_op ::= IN */ yytestcase(yyruleno==221);
#line 151 "parse.y"
{yygotominor.yy328 = 0;}
#line 2224 "parse.c"
        break;
      case 29: /* ifnotexists ::= IF NOT EXISTS */
      case 30: /* temp ::= TEMP */ yytestcase(yyruleno==30);
      case 69: /* autoinc ::= AUTOINCR */ yytestcase(yyruleno==69);
      case 84: /* init_deferred_pred_opt ::= INITIALLY DEFERRED */ yytestcase(yyruleno==84);
      case 107: /* ifexists ::= IF EXISTS */ yytestcase(yyruleno==107);
      case 219: /* between_op ::= NOT BETWEEN */ yytestcase(yyruleno==219);
      case 222: /* in_op ::= NOT IN */ yytestcase(yyruleno==222);
#line 152 "parse.y"
{yygotominor.yy328 = 1;}
#line 2235 "parse.c"
        break;
      case 32: /* create_table_args ::= LP columnlist conslist_opt RP table_options */
#line 158 "parse.y"
{
  sqlite3EndTable(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,yymsp[0].minor.yy186,0);
}
#line 2242 "parse.c"
        break;
      case 33: /* create_table_args ::= AS select */
#line 161 "parse.y"
{
  sqlite3EndTable(pParse,0,0,0,yymsp[0].minor.yy3);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy3);
}
#line 2250 "parse.c"
        break;
      case 34: /* table_options ::= */
#line 166 "parse.y"
{yygotominor.yy186 = 0;}
#line 2255 "parse.c"
        break;
      case 35: /* table_options ::= WITHOUT nm */
#line 167 "parse.y"
{
  if( yymsp[0].minor.yy0.n==5 && sqlite3_strnicmp(yymsp[0].minor.yy0.z,"rowid",5)==0 ){
    yygotominor.yy186 = TF_WithoutRowid;
  }else{
    yygotominor.yy186 = 0;
    sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
  }
}
#line 2267 "parse.c"
        break;
      case 38: /* column ::= columnid type carglist */
#line 183 "parse.y"
{
  yygotominor.yy0.z = yymsp[-2].minor.yy0.z;
  yygotominor.yy0.n = (int)(pParse->sLastToken.z-yymsp[-2].minor.yy0.z) + pParse->sLastToken.n;
}
#line 2275 "parse.c"
        break;
      case 39: /* columnid ::= nm */
#line 187 "parse.y"
{
  sqlite3AddColumn(pParse,&yymsp[0].minor.yy0);
  yygotominor.yy0 = yymsp[0].minor.yy0;
  pParse->constraintName.n = 0;
}
#line 2284 "parse.c"
        break;
      case 40: /* nm ::= ID|INDEXED */
      case 41: /* nm ::= STRING */ yytestcase(yyruleno==41);
      case 42: /* nm ::= JOIN_KW */ yytestcase(yyruleno==42);
      case 45: /* typetoken ::= typename */ yytestcase(yyruleno==45);
      case 48: /* typename ::= ID|STRING */ yytestcase(yyruleno==48);
      case 130: /* as ::= AS nm */ yytestcase(yyruleno==130);
      case 131: /* as ::= ID|STRING */ yytestcase(yyruleno==131);
      case 141: /* dbnm ::= DOT nm */ yytestcase(yyruleno==141);
      case 150: /* indexed_opt ::= INDEXED BY nm */ yytestcase(yyruleno==150);
      case 247: /* collate ::= COLLATE ID|STRING */ yytestcase(yyruleno==247);
      case 256: /* nmnum ::= plus_num */ yytestcase(yyruleno==256);
      case 257: /* nmnum ::= nm */ yytestcase(yyruleno==257);
      case 258: /* nmnum ::= ON */ yytestcase(yyruleno==258);
      case 259: /* nmnum ::= DELETE */ yytestcase(yyruleno==259);
      case 260: /* nmnum ::= DEFAULT */ yytestcase(yyruleno==260);
      case 261: /* plus_num ::= PLUS INTEGER|FLOAT */ yytestcase(yyruleno==261);
      case 262: /* plus_num ::= INTEGER|FLOAT */ yytestcase(yyruleno==262);
      case 263: /* minus_num ::= MINUS INTEGER|FLOAT */ yytestcase(yyruleno==263);
      case 279: /* trnm ::= nm */ yytestcase(yyruleno==279);
#line 247 "parse.y"
{yygotominor.yy0 = yymsp[0].minor.yy0;}
#line 2307 "parse.c"
        break;
      case 44: /* type ::= typetoken */
#line 257 "parse.y"
{sqlite3AddColumnType(pParse,&yymsp[0].minor.yy0);}
#line 2312 "parse.c"
        break;
      case 46: /* typetoken ::= typename LP signed RP */
#line 259 "parse.y"
{
  yygotominor.yy0.z = yymsp[-3].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy0.z);
}
#line 2320 "parse.c"
        break;
      case 47: /* typetoken ::= typename LP signed COMMA signed RP */
#line 263 "parse.y"
{
  yygotominor.yy0.z = yymsp[-5].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy0.z);
}
#line 2328 "parse.c"
        break;
      case 49: /* typename ::= typename ID|STRING */
#line 269 "parse.y"
{yygotominor.yy0.z=yymsp[-1].minor.yy0.z; yygotominor.yy0.n=yymsp[0].minor.yy0.n+(int)(yymsp[0].minor.yy0.z-yymsp[-1].minor.yy0.z);}
#line 2333 "parse.c"
        break;
      case 54: /* ccons ::= CONSTRAINT nm */
      case 92: /* tcons ::= CONSTRAINT nm */ yytestcase(yyruleno==92);
#line 278 "parse.y"
{pParse->constraintName = yymsp[0].minor.yy0;}
#line 2339 "parse.c"
        break;
      case 55: /* ccons ::= DEFAULT term */
      case 57: /* ccons ::= DEFAULT PLUS term */ yytestcase(yyruleno==57);
#line 279 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[0].minor.yy346);}
#line 2345 "parse.c"
        break;
      case 56: /* ccons ::= DEFAULT LP expr RP */
#line 280 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[-1].minor.yy346);}
#line 2350 "parse.c"
        break;
      case 58: /* ccons ::= DEFAULT MINUS term */
#line 282 "parse.y"
{
  ExprSpan v;
  v.pExpr = sqlite3PExpr(pParse, TK_UMINUS, yymsp[0].minor.yy346.pExpr, 0, 0);
  v.zStart = yymsp[-1].minor.yy0.z;
  v.zEnd = yymsp[0].minor.yy346.zEnd;
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2361 "parse.c"
        break;
      case 59: /* ccons ::= DEFAULT ID|INDEXED */
#line 289 "parse.y"
{
  ExprSpan v;
  spanExpr(&v, pParse, TK_STRING, &yymsp[0].minor.yy0);
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2370 "parse.c"
        break;
      case 61: /* ccons ::= NOT NULL onconf */
#line 299 "parse.y"
{sqlite3AddNotNull(pParse, yymsp[0].minor.yy328);}
#line 2375 "parse.c"
        break;
      case 62: /* ccons ::= PRIMARY KEY sortorder onconf autoinc */
#line 301 "parse.y"
{sqlite3AddPrimaryKey(pParse,0,yymsp[-1].minor.yy328,yymsp[0].minor.yy328,yymsp[-2].minor.yy328);}
#line 2380 "parse.c"
        break;
      case 63: /* ccons ::= UNIQUE onconf */
#line 302 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,0,yymsp[0].minor.yy328,0,0,0,0);}
#line 2385 "parse.c"
        break;
      case 64: /* ccons ::= CHECK LP expr RP */
#line 303 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-1].minor.yy346.pExpr);}
#line 2390 "parse.c"
        break;
      case 65: /* ccons ::= REFERENCES nm idxlist_opt refargs */
#line 305 "parse.y"
{sqlite3CreateForeignKey(pParse,0,&yymsp[-2].minor.yy0,yymsp[-1].minor.yy14,yymsp[0].minor.yy328);}
#line 2395 "parse.c"
        break;
      case 66: /* ccons ::= defer_subclause */
#line 306 "parse.y"
{sqlite3DeferForeignKey(pParse,yymsp[0].minor.yy328);}
#line 2400 "parse.c"
        break;
      case 67: /* ccons ::= COLLATE ID|STRING */
#line 307 "parse.y"
{sqlite3AddCollateType(pParse, &yymsp[0].minor.yy0);}
#line 2405 "parse.c"
        break;
      case 70: /* refargs ::= */
#line 320 "parse.y"
{ yygotominor.yy328 = OE_None*0x0101; /* EV: R-19803-45884 */}
#line 2410 "parse.c"
        break;
      case 71: /* refargs ::= refargs refarg */
#line 321 "parse.y"
{ yygotominor.yy328 = (yymsp[-1].minor.yy328 & ~yymsp[0].minor.yy429.mask) | yymsp[0].minor.yy429.value; }
#line 2415 "parse.c"
        break;
      case 72: /* refarg ::= MATCH nm */
      case 73: /* refarg ::= ON INSERT refact */ yytestcase(yyruleno==73);
#line 323 "parse.y"
{ yygotominor.yy429.value = 0;     yygotominor.yy429.mask = 0x000000; }
#line 2421 "parse.c"
        break;
      case 74: /* refarg ::= ON DELETE refact */
#line 325 "parse.y"
{ yygotominor.yy429.value = yymsp[0].minor.yy328;     yygotominor.yy429.mask = 0x0000ff; }
#line 2426 "parse.c"
        break;
      case 75: /* refarg ::= ON UPDATE refact */
#line 326 "parse.y"
{ yygotominor.yy429.value = yymsp[0].minor.yy328<<8;  yygotominor.yy429.mask = 0x00ff00; }
#line 2431 "parse.c"
        break;
      case 76: /* refact ::= SET NULL */
#line 328 "parse.y"
{ yygotominor.yy328 = OE_SetNull;  /* EV: R-33326-45252 */}
#line 2436 "parse.c"
        break;
      case 77: /* refact ::= SET DEFAULT */
#line 329 "parse.y"
{ yygotominor.yy328 = OE_SetDflt;  /* EV: R-33326-45252 */}
#line 2441 "parse.c"
        break;
      case 78: /* refact ::= CASCADE */
#line 330 "parse.y"
{ yygotominor.yy328 = OE_Cascade;  /* EV: R-33326-45252 */}
#line 2446 "parse.c"
        break;
      case 79: /* refact ::= RESTRICT */
#line 331 "parse.y"
{ yygotominor.yy328 = OE_Restrict; /* EV: R-33326-45252 */}
#line 2451 "parse.c"
        break;
      case 80: /* refact ::= NO ACTION */
#line 332 "parse.y"
{ yygotominor.yy328 = OE_None;     /* EV: R-33326-45252 */}
#line 2456 "parse.c"
        break;
      case 82: /* defer_subclause ::= DEFERRABLE init_deferred_pred_opt */
      case 98: /* defer_subclause_opt ::= defer_subclause */ yytestcase(yyruleno==98);
      case 100: /* onconf ::= ON CONFLICT resolvetype */ yytestcase(yyruleno==100);
      case 103: /* resolvetype ::= raisetype */ yytestcase(yyruleno==103);
#line 335 "parse.y"
{yygotominor.yy328 = yymsp[0].minor.yy328;}
#line 2464 "parse.c"
        break;
      case 86: /* conslist_opt ::= */
#line 341 "parse.y"
{yygotominor.yy0.n = 0; yygotominor.yy0.z = 0;}
#line 2469 "parse.c"
        break;
      case 87: /* conslist_opt ::= COMMA conslist */
#line 342 "parse.y"
{yygotominor.yy0 = yymsp[-1].minor.yy0;}
#line 2474 "parse.c"
        break;
      case 90: /* tconscomma ::= COMMA */
#line 345 "parse.y"
{pParse->constraintName.n = 0;}
#line 2479 "parse.c"
        break;
      case 93: /* tcons ::= PRIMARY KEY LP idxlist autoinc RP onconf */
#line 349 "parse.y"
{sqlite3AddPrimaryKey(pParse,yymsp[-3].minor.yy14,yymsp[0].minor.yy328,yymsp[-2].minor.yy328,0);}
#line 2484 "parse.c"
        break;
      case 94: /* tcons ::= UNIQUE LP idxlist RP onconf */
#line 351 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,yymsp[-2].minor.yy14,yymsp[0].minor.yy328,0,0,0,0);}
#line 2489 "parse.c"
        break;
      case 95: /* tcons ::= CHECK LP expr RP onconf */
#line 353 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-2].minor.yy346.pExpr);}
#line 2494 "parse.c"
        break;
      case 96: /* tcons ::= FOREIGN KEY LP idxlist RP REFERENCES nm idxlist_opt refargs defer_subclause_opt */
#line 355 "parse.y"
{
    sqlite3CreateForeignKey(pParse, yymsp[-6].minor.yy14, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy14, yymsp[-1].minor.yy328);
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy328);
}
#line 2502 "parse.c"
        break;
      case 99: /* onconf ::= */
#line 369 "parse.y"
{yygotominor.yy328 = OE_Default;}
#line 2507 "parse.c"
        break;
      case 101: /* orconf ::= */
#line 371 "parse.y"
{yygotominor.yy186 = OE_Default;}
#line 2512 "parse.c"
        break;
      case 102: /* orconf ::= OR resolvetype */
#line 372 "parse.y"
{yygotominor.yy186 = (u8)yymsp[0].minor.yy328;}
#line 2517 "parse.c"
        break;
      case 104: /* resolvetype ::= IGNORE */
#line 374 "parse.y"
{yygotominor.yy328 = OE_Ignore;}
#line 2522 "parse.c"
        break;
      case 105: /* resolvetype ::= REPLACE */
#line 375 "parse.y"
{yygotominor.yy328 = OE_Replace;}
#line 2527 "parse.c"
        break;
      case 106: /* cmd ::= DROP TABLE ifexists fullname */
#line 379 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy65, 0, yymsp[-1].minor.yy328);
}
#line 2534 "parse.c"
        break;
      case 109: /* cmd ::= createkw temp VIEW ifnotexists nm dbnm AS select */
#line 389 "parse.y"
{
  sqlite3CreateView(pParse, &yymsp[-7].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, yymsp[0].minor.yy3, yymsp[-6].minor.yy328, yymsp[-4].minor.yy328);
}
#line 2541 "parse.c"
        break;
      case 110: /* cmd ::= DROP VIEW ifexists fullname */
#line 392 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy65, 1, yymsp[-1].minor.yy328);
}
#line 2548 "parse.c"
        break;
      case 111: /* cmd ::= select */
#line 399 "parse.y"
{
  SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0};
  sqlite3Select(pParse, yymsp[0].minor.yy3, &dest);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy3);
}
#line 2557 "parse.c"
        break;
      case 112: /* select ::= with selectnowith */
#line 436 "parse.y"
{
  Select *p = yymsp[0].minor.yy3;
  if( p ){
    p->pWith = yymsp[-1].minor.yy59;
    parserDoubleLinkSelect(pParse, p);
  }else{
    sqlite3WithDelete(pParse->db, yymsp[-1].minor.yy59);
  }
  yygotominor.yy3 = p;
}
#line 2571 "parse.c"
        break;
      case 113: /* selectnowith ::= oneselect */
      case 119: /* oneselect ::= values */ yytestcase(yyruleno==119);
#line 447 "parse.y"
{yygotominor.yy3 = yymsp[0].minor.yy3;}
#line 2577 "parse.c"
        break;
      case 114: /* selectnowith ::= selectnowith multiselect_op oneselect */
#line 449 "parse.y"
{
  Select *pRhs = yymsp[0].minor.yy3;
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    parserDoubleLinkSelect(pParse, pRhs);
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)yymsp[-1].minor.yy328;
    pRhs->pPrior = yymsp[-2].minor.yy3;
    pRhs->selFlags &= ~SF_MultiValue;
    if( yymsp[-1].minor.yy328!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, yymsp[-2].minor.yy3);
  }
  yygotominor.yy3 = pRhs;
}
#line 2601 "parse.c"
        break;
      case 116: /* multiselect_op ::= UNION ALL */
#line 471 "parse.y"
{yygotominor.yy328 = TK_ALL;}
#line 2606 "parse.c"
        break;
      case 118: /* oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt */
#line 475 "parse.y"
{
  yygotominor.yy3 = sqlite3SelectNew(pParse,yymsp[-6].minor.yy14,yymsp[-5].minor.yy65,yymsp[-4].minor.yy132,yymsp[-3].minor.yy14,yymsp[-2].minor.yy132,yymsp[-1].minor.yy14,yymsp[-7].minor.yy381,yymsp[0].minor.yy476.pLimit,yymsp[0].minor.yy476.pOffset);
#if SELECTTRACE_ENABLED
  /* Populate the Select.zSelName[] string that is used to help with
  ** query planner debugging, to differentiate between multiple Select
  ** objects in a complex query.
  **
  ** If the SELECT keyword is immediately followed by a C-style comment
  ** then extract the first few alphanumeric characters from within that
  ** comment to be the zSelName value.  Otherwise, the label is #N where
  ** is an integer that is incremented with each SELECT statement seen.
  */
  if( yygotominor.yy3!=0 ){
    const char *z = yymsp[-8].minor.yy0.z+6;
    int i;
    sqlite3_snprintf(sizeof(yygotominor.yy3->zSelName), yygotominor.yy3->zSelName, "#%d",
                     ++pParse->nSelect);
    while( z[0]==' ' ) z++;
    if( z[0]=='/' && z[1]=='*' ){
      z += 2;
      while( z[0]==' ' ) z++;
      for(i=0; sqlite3Isalnum(z[i]); i++){}
      sqlite3_snprintf(sizeof(yygotominor.yy3->zSelName), yygotominor.yy3->zSelName, "%.*s", i, z);
    }
  }
#endif /* SELECTRACE_ENABLED */
}
#line 2637 "parse.c"
        break;
      case 120: /* values ::= VALUES LP nexprlist RP */
#line 506 "parse.y"
{
  yygotominor.yy3 = sqlite3SelectNew(pParse,yymsp[-1].minor.yy14,0,0,0,0,0,SF_Values,0,0);
}
#line 2644 "parse.c"
        break;
      case 121: /* values ::= values COMMA LP exprlist RP */
#line 509 "parse.y"
{
  Select *pRight, *pLeft = yymsp[-4].minor.yy3;
  pRight = sqlite3SelectNew(pParse,yymsp[-1].minor.yy14,0,0,0,0,0,SF_Values|SF_MultiValue,0,0);
  if( ALWAYS(pLeft) ) pLeft->selFlags &= ~SF_MultiValue;
  if( pRight ){
    pRight->op = TK_ALL;
    pLeft = yymsp[-4].minor.yy3;
    pRight->pPrior = pLeft;
    yygotominor.yy3 = pRight;
  }else{
    yygotominor.yy3 = pLeft;
  }
}
#line 2661 "parse.c"
        break;
      case 122: /* distinct ::= DISTINCT */
#line 527 "parse.y"
{yygotominor.yy381 = SF_Distinct;}
#line 2666 "parse.c"
        break;
      case 123: /* distinct ::= ALL */
      case 124: /* distinct ::= */ yytestcase(yyruleno==124);
#line 528 "parse.y"
{yygotominor.yy381 = 0;}
#line 2672 "parse.c"
        break;
      case 125: /* sclp ::= selcollist COMMA */
      case 243: /* idxlist_opt ::= LP idxlist RP */ yytestcase(yyruleno==243);
#line 540 "parse.y"
{yygotominor.yy14 = yymsp[-1].minor.yy14;}
#line 2678 "parse.c"
        break;
      case 126: /* sclp ::= */
      case 154: /* orderby_opt ::= */ yytestcase(yyruleno==154);
      case 161: /* groupby_opt ::= */ yytestcase(yyruleno==161);
      case 236: /* exprlist ::= */ yytestcase(yyruleno==236);
      case 242: /* idxlist_opt ::= */ yytestcase(yyruleno==242);
#line 541 "parse.y"
{yygotominor.yy14 = 0;}
#line 2687 "parse.c"
        break;
      case 127: /* selcollist ::= sclp expr as */
#line 542 "parse.y"
{
   yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy14, yymsp[-1].minor.yy346.pExpr);
   if( yymsp[0].minor.yy0.n>0 ) sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[0].minor.yy0, 1);
   sqlite3ExprListSetSpan(pParse,yygotominor.yy14,&yymsp[-1].minor.yy346);
}
#line 2696 "parse.c"
        break;
      case 128: /* selcollist ::= sclp STAR */
#line 547 "parse.y"
{
  Expr *p = sqlite3Expr(pParse->db, TK_ALL, 0);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-1].minor.yy14, p);
}
#line 2704 "parse.c"
        break;
      case 129: /* selcollist ::= sclp nm DOT STAR */
#line 551 "parse.y"
{
  Expr *pRight = sqlite3PExpr(pParse, TK_ALL, 0, 0, &yymsp[0].minor.yy0);
  Expr *pLeft = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *pDot = sqlite3PExpr(pParse, TK_DOT, pLeft, pRight, 0);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy14, pDot);
}
#line 2714 "parse.c"
        break;
      case 132: /* as ::= */
#line 564 "parse.y"
{yygotominor.yy0.n = 0;}
#line 2719 "parse.c"
        break;
      case 133: /* from ::= */
#line 576 "parse.y"
{yygotominor.yy65 = sqlite3DbMallocZero(pParse->db, sizeof(*yygotominor.yy65));}
#line 2724 "parse.c"
        break;
      case 134: /* from ::= FROM seltablist */
#line 577 "parse.y"
{
  yygotominor.yy65 = yymsp[0].minor.yy65;
  sqlite3SrcListShiftJoinType(yygotominor.yy65);
}
#line 2732 "parse.c"
        break;
      case 135: /* stl_prefix ::= seltablist joinop */
#line 585 "parse.y"
{
   yygotominor.yy65 = yymsp[-1].minor.yy65;
   if( ALWAYS(yygotominor.yy65 && yygotominor.yy65->nSrc>0) ) yygotominor.yy65->a[yygotominor.yy65->nSrc-1].jointype = (u8)yymsp[0].minor.yy328;
}
#line 2740 "parse.c"
        break;
      case 136: /* stl_prefix ::= */
#line 589 "parse.y"
{yygotominor.yy65 = 0;}
#line 2745 "parse.c"
        break;
      case 137: /* seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt */
#line 591 "parse.y"
{
  yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,&yymsp[-5].minor.yy0,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,0,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
  sqlite3SrcListIndexedBy(pParse, yygotominor.yy65, &yymsp[-2].minor.yy0);
}
#line 2753 "parse.c"
        break;
      case 138: /* seltablist ::= stl_prefix LP select RP as on_opt using_opt */
#line 597 "parse.y"
{
    yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,yymsp[-4].minor.yy3,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
  }
#line 2760 "parse.c"
        break;
      case 139: /* seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt */
#line 601 "parse.y"
{
    if( yymsp[-6].minor.yy65==0 && yymsp[-2].minor.yy0.n==0 && yymsp[-1].minor.yy132==0 && yymsp[0].minor.yy408==0 ){
      yygotominor.yy65 = yymsp[-4].minor.yy65;
    }else if( yymsp[-4].minor.yy65->nSrc==1 ){
      yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
      if( yygotominor.yy65 ){
        struct SrcList_item *pNew = &yygotominor.yy65->a[yygotominor.yy65->nSrc-1];
        struct SrcList_item *pOld = yymsp[-4].minor.yy65->a;
        pNew->zName = pOld->zName;
        pNew->zDatabase = pOld->zDatabase;
        pNew->pSelect = pOld->pSelect;
        pOld->zName = pOld->zDatabase = 0;
        pOld->pSelect = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-4].minor.yy65);
    }else{
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(yymsp[-4].minor.yy65);
      pSubquery = sqlite3SelectNew(pParse,0,yymsp[-4].minor.yy65,0,0,0,0,SF_NestedFrom,0,0);
      yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,pSubquery,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
    }
  }
#line 2786 "parse.c"
        break;
      case 140: /* dbnm ::= */
      case 149: /* indexed_opt ::= */ yytestcase(yyruleno==149);
#line 626 "parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=0;}
#line 2792 "parse.c"
        break;
      case 142: /* fullname ::= nm dbnm */
#line 631 "parse.y"
{yygotominor.yy65 = sqlite3SrcListAppend(pParse->db,0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);}
#line 2797 "parse.c"
        break;
      case 143: /* joinop ::= COMMA|JOIN */
#line 635 "parse.y"
{ yygotominor.yy328 = JT_INNER; }
#line 2802 "parse.c"
        break;
      case 144: /* joinop ::= JOIN_KW JOIN */
#line 636 "parse.y"
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0); }
#line 2807 "parse.c"
        break;
      case 145: /* joinop ::= JOIN_KW nm JOIN */
#line 637 "parse.y"
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,0); }
#line 2812 "parse.c"
        break;
      case 146: /* joinop ::= JOIN_KW nm nm JOIN */
#line 639 "parse.y"
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0); }
#line 2817 "parse.c"
        break;
      case 147: /* on_opt ::= ON expr */
      case 164: /* having_opt ::= HAVING expr */ yytestcase(yyruleno==164);
      case 171: /* where_opt ::= WHERE expr */ yytestcase(yyruleno==171);
      case 231: /* case_else ::= ELSE expr */ yytestcase(yyruleno==231);
      case 233: /* case_operand ::= expr */ yytestcase(yyruleno==233);
#line 643 "parse.y"
{yygotominor.yy132 = yymsp[0].minor.yy346.pExpr;}
#line 2826 "parse.c"
        break;
      case 148: /* on_opt ::= */
      case 163: /* having_opt ::= */ yytestcase(yyruleno==163);
      case 170: /* where_opt ::= */ yytestcase(yyruleno==170);
      case 232: /* case_else ::= */ yytestcase(yyruleno==232);
      case 234: /* case_operand ::= */ yytestcase(yyruleno==234);
#line 644 "parse.y"
{yygotominor.yy132 = 0;}
#line 2835 "parse.c"
        break;
      case 151: /* indexed_opt ::= NOT INDEXED */
#line 659 "parse.y"
{yygotominor.yy0.z=0; yygotominor.yy0.n=1;}
#line 2840 "parse.c"
        break;
      case 152: /* using_opt ::= USING LP idlist RP */
      case 180: /* inscollist_opt ::= LP idlist RP */ yytestcase(yyruleno==180);
#line 663 "parse.y"
{yygotominor.yy408 = yymsp[-1].minor.yy408;}
#line 2846 "parse.c"
        break;
      case 153: /* using_opt ::= */
      case 179: /* inscollist_opt ::= */ yytestcase(yyruleno==179);
#line 664 "parse.y"
{yygotominor.yy408 = 0;}
#line 2852 "parse.c"
        break;
      case 155: /* orderby_opt ::= ORDER BY sortlist */
      case 162: /* groupby_opt ::= GROUP BY nexprlist */ yytestcase(yyruleno==162);
      case 235: /* exprlist ::= nexprlist */ yytestcase(yyruleno==235);
#line 673 "parse.y"
{yygotominor.yy14 = yymsp[0].minor.yy14;}
#line 2859 "parse.c"
        break;
      case 156: /* sortlist ::= sortlist COMMA expr sortorder */
#line 674 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy14,yymsp[-1].minor.yy346.pExpr);
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
#line 2867 "parse.c"
        break;
      case 157: /* sortlist ::= expr sortorder */
#line 678 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0,yymsp[-1].minor.yy346.pExpr);
  if( yygotominor.yy14 && ALWAYS(yygotominor.yy14->a) ) yygotominor.yy14->a[0].sortOrder = (u8)yymsp[0].minor.yy328;
}
#line 2875 "parse.c"
        break;
      case 158: /* sortorder ::= ASC */
      case 160: /* sortorder ::= */ yytestcase(yyruleno==160);
#line 685 "parse.y"
{yygotominor.yy328 = SQLITE_SO_ASC;}
#line 2881 "parse.c"
        break;
      case 159: /* sortorder ::= DESC */
#line 686 "parse.y"
{yygotominor.yy328 = SQLITE_SO_DESC;}
#line 2886 "parse.c"
        break;
      case 165: /* limit_opt ::= */
#line 712 "parse.y"
{yygotominor.yy476.pLimit = 0; yygotominor.yy476.pOffset = 0;}
#line 2891 "parse.c"
        break;
      case 166: /* limit_opt ::= LIMIT expr */
#line 713 "parse.y"
{yygotominor.yy476.pLimit = yymsp[0].minor.yy346.pExpr; yygotominor.yy476.pOffset = 0;}
#line 2896 "parse.c"
        break;
      case 167: /* limit_opt ::= LIMIT expr OFFSET expr */
#line 715 "parse.y"
{yygotominor.yy476.pLimit = yymsp[-2].minor.yy346.pExpr; yygotominor.yy476.pOffset = yymsp[0].minor.yy346.pExpr;}
#line 2901 "parse.c"
        break;
      case 168: /* limit_opt ::= LIMIT expr COMMA expr */
#line 717 "parse.y"
{yygotominor.yy476.pOffset = yymsp[-2].minor.yy346.pExpr; yygotominor.yy476.pLimit = yymsp[0].minor.yy346.pExpr;}
#line 2906 "parse.c"
        break;
      case 169: /* cmd ::= with DELETE FROM fullname indexed_opt where_opt */
#line 731 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy59, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-2].minor.yy65, &yymsp[-1].minor.yy0);
  sqlite3DeleteFrom(pParse,yymsp[-2].minor.yy65,yymsp[0].minor.yy132);
}
#line 2915 "parse.c"
        break;
      case 172: /* cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt */
#line 758 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-7].minor.yy59, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-4].minor.yy65, &yymsp[-3].minor.yy0);
  sqlite3ExprListCheckLength(pParse,yymsp[-1].minor.yy14,"set list"); 
  sqlite3Update(pParse,yymsp[-4].minor.yy65,yymsp[-1].minor.yy14,yymsp[0].minor.yy132,yymsp[-5].minor.yy186);
}
#line 2925 "parse.c"
        break;
      case 173: /* setlist ::= setlist COMMA nm EQ expr */
#line 769 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[0].minor.yy346.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
}
#line 2933 "parse.c"
        break;
      case 174: /* setlist ::= nm EQ expr */
#line 773 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy346.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
}
#line 2941 "parse.c"
        break;
      case 175: /* cmd ::= with insert_cmd INTO fullname inscollist_opt select */
#line 780 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy59, 1);
  sqlite3Insert(pParse, yymsp[-2].minor.yy65, yymsp[0].minor.yy3, yymsp[-1].minor.yy408, yymsp[-4].minor.yy186);
}
#line 2949 "parse.c"
        break;
      case 176: /* cmd ::= with insert_cmd INTO fullname inscollist_opt DEFAULT VALUES */
#line 785 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-6].minor.yy59, 1);
  sqlite3Insert(pParse, yymsp[-3].minor.yy65, 0, yymsp[-2].minor.yy408, yymsp[-5].minor.yy186);
}
#line 2957 "parse.c"
        break;
      case 177: /* insert_cmd ::= INSERT orconf */
#line 791 "parse.y"
{yygotominor.yy186 = yymsp[0].minor.yy186;}
#line 2962 "parse.c"
        break;
      case 178: /* insert_cmd ::= REPLACE */
#line 792 "parse.y"
{yygotominor.yy186 = OE_Replace;}
#line 2967 "parse.c"
        break;
      case 181: /* idlist ::= idlist COMMA nm */
#line 802 "parse.y"
{yygotominor.yy408 = sqlite3IdListAppend(pParse->db,yymsp[-2].minor.yy408,&yymsp[0].minor.yy0);}
#line 2972 "parse.c"
        break;
      case 182: /* idlist ::= nm */
#line 804 "parse.y"
{yygotominor.yy408 = sqlite3IdListAppend(pParse->db,0,&yymsp[0].minor.yy0);}
#line 2977 "parse.c"
        break;
      case 183: /* expr ::= term */
#line 835 "parse.y"
{yygotominor.yy346 = yymsp[0].minor.yy346;}
#line 2982 "parse.c"
        break;
      case 184: /* expr ::= LP expr RP */
#line 836 "parse.y"
{yygotominor.yy346.pExpr = yymsp[-1].minor.yy346.pExpr; spanSet(&yygotominor.yy346,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);}
#line 2987 "parse.c"
        break;
      case 185: /* term ::= NULL */
      case 190: /* term ::= INTEGER|FLOAT|BLOB */ yytestcase(yyruleno==190);
      case 191: /* term ::= STRING */ yytestcase(yyruleno==191);
#line 837 "parse.y"
{spanExpr(&yygotominor.yy346, pParse, yymsp[0].major, &yymsp[0].minor.yy0);}
#line 2994 "parse.c"
        break;
      case 186: /* expr ::= ID|INDEXED */
      case 187: /* expr ::= JOIN_KW */ yytestcase(yyruleno==187);
#line 838 "parse.y"
{spanExpr(&yygotominor.yy346, pParse, TK_ID, &yymsp[0].minor.yy0);}
#line 3000 "parse.c"
        break;
      case 188: /* expr ::= nm DOT nm */
#line 840 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp2, 0);
  spanSet(&yygotominor.yy346,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3010 "parse.c"
        break;
      case 189: /* expr ::= nm DOT nm DOT nm */
#line 846 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-4].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp3 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  Expr *temp4 = sqlite3PExpr(pParse, TK_DOT, temp2, temp3, 0);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp4, 0);
  spanSet(&yygotominor.yy346,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3022 "parse.c"
        break;
      case 192: /* expr ::= VARIABLE */
#line 856 "parse.y"
{
  if( yymsp[0].minor.yy0.n>=2 && yymsp[0].minor.yy0.z[0]=='#' && sqlite3Isdigit(yymsp[0].minor.yy0.z[1]) ){
    /* When doing a nested parse, one can include terms in an expression
    ** that look like this:   #1 #2 ...  These terms refer to registers
    ** in the virtual machine.  #N is the N-th register. */
    if( pParse->nested==0 ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &yymsp[0].minor.yy0);
      yygotominor.yy346.pExpr = 0;
    }else{
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_REGISTER, 0, 0, &yymsp[0].minor.yy0);
      if( yygotominor.yy346.pExpr ) sqlite3GetInt32(&yymsp[0].minor.yy0.z[1], &yygotominor.yy346.pExpr->iTable);
    }
  }else{
    spanExpr(&yygotominor.yy346, pParse, TK_VARIABLE, &yymsp[0].minor.yy0);
    sqlite3ExprAssignVarNumber(pParse, yygotominor.yy346.pExpr);
  }
  spanSet(&yygotominor.yy346, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 3044 "parse.c"
        break;
      case 193: /* expr ::= expr COLLATE ID|STRING */
#line 874 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy346.pExpr, &yymsp[0].minor.yy0, 1);
  yygotominor.yy346.zStart = yymsp[-2].minor.yy346.zStart;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3053 "parse.c"
        break;
      case 194: /* expr ::= CAST LP expr AS typetoken RP */
#line 880 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_CAST, yymsp[-3].minor.yy346.pExpr, 0, &yymsp[-1].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3061 "parse.c"
        break;
      case 195: /* expr ::= ID|INDEXED LP distinct exprlist RP */
#line 885 "parse.y"
{
  if( yymsp[-1].minor.yy14 && yymsp[-1].minor.yy14->nExpr>pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", &yymsp[-4].minor.yy0);
  }
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy14, &yymsp[-4].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy381 && yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->flags |= EP_Distinct;
  }
}
#line 3075 "parse.c"
        break;
      case 196: /* expr ::= ID|INDEXED LP STAR RP */
#line 895 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3083 "parse.c"
        break;
      case 197: /* term ::= CTIME_KW */
#line 899 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0);
  spanSet(&yygotominor.yy346, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 3091 "parse.c"
        break;
      case 198: /* expr ::= expr AND expr */
      case 199: /* expr ::= expr OR expr */ yytestcase(yyruleno==199);
      case 200: /* expr ::= expr LT|GT|GE|LE expr */ yytestcase(yyruleno==200);
      case 201: /* expr ::= expr EQ|NE expr */ yytestcase(yyruleno==201);
      case 202: /* expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr */ yytestcase(yyruleno==202);
      case 203: /* expr ::= expr PLUS|MINUS expr */ yytestcase(yyruleno==203);
      case 204: /* expr ::= expr STAR|SLASH|REM expr */ yytestcase(yyruleno==204);
      case 205: /* expr ::= expr CONCAT expr */ yytestcase(yyruleno==205);
#line 921 "parse.y"
{spanBinaryExpr(&yygotominor.yy346,pParse,yymsp[-1].major,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy346);}
#line 3103 "parse.c"
        break;
      case 206: /* likeop ::= LIKE_KW|MATCH */
#line 934 "parse.y"
{yygotominor.yy96.eOperator = yymsp[0].minor.yy0; yygotominor.yy96.bNot = 0;}
#line 3108 "parse.c"
        break;
      case 207: /* likeop ::= NOT LIKE_KW|MATCH */
#line 935 "parse.y"
{yygotominor.yy96.eOperator = yymsp[0].minor.yy0; yygotominor.yy96.bNot = 1;}
#line 3113 "parse.c"
        break;
      case 208: /* expr ::= expr likeop expr */
#line 936 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[0].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy96.eOperator);
  if( yymsp[-1].minor.yy96.bNot ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-2].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
  if( yygotominor.yy346.pExpr ) yygotominor.yy346.pExpr->flags |= EP_InfixFunc;
}
#line 3127 "parse.c"
        break;
      case 209: /* expr ::= expr likeop expr ESCAPE expr */
#line 946 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-4].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy96.eOperator);
  if( yymsp[-3].minor.yy96.bNot ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
  if( yygotominor.yy346.pExpr ) yygotominor.yy346.pExpr->flags |= EP_InfixFunc;
}
#line 3142 "parse.c"
        break;
      case 210: /* expr ::= expr ISNULL|NOTNULL */
#line 974 "parse.y"
{spanUnaryPostfix(&yygotominor.yy346,pParse,yymsp[0].major,&yymsp[-1].minor.yy346,&yymsp[0].minor.yy0);}
#line 3147 "parse.c"
        break;
      case 211: /* expr ::= expr NOT NULL */
#line 975 "parse.y"
{spanUnaryPostfix(&yygotominor.yy346,pParse,TK_NOTNULL,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy0);}
#line 3152 "parse.c"
        break;
      case 212: /* expr ::= expr IS expr */
#line 996 "parse.y"
{
  spanBinaryExpr(&yygotominor.yy346,pParse,TK_IS,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy346);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy346.pExpr, yygotominor.yy346.pExpr, TK_ISNULL);
}
#line 3160 "parse.c"
        break;
      case 213: /* expr ::= expr IS NOT expr */
#line 1000 "parse.y"
{
  spanBinaryExpr(&yygotominor.yy346,pParse,TK_ISNOT,&yymsp[-3].minor.yy346,&yymsp[0].minor.yy346);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy346.pExpr, yygotominor.yy346.pExpr, TK_NOTNULL);
}
#line 3168 "parse.c"
        break;
      case 214: /* expr ::= NOT expr */
      case 215: /* expr ::= BITNOT expr */ yytestcase(yyruleno==215);
#line 1023 "parse.y"
{spanUnaryPrefix(&yygotominor.yy346,pParse,yymsp[-1].major,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
#line 3174 "parse.c"
        break;
      case 216: /* expr ::= MINUS expr */
#line 1026 "parse.y"
{spanUnaryPrefix(&yygotominor.yy346,pParse,TK_UMINUS,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
#line 3179 "parse.c"
        break;
      case 217: /* expr ::= PLUS expr */
#line 1028 "parse.y"
{spanUnaryPrefix(&yygotominor.yy346,pParse,TK_UPLUS,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
#line 3184 "parse.c"
        break;
      case 220: /* expr ::= expr between_op expr AND expr */
#line 1033 "parse.y"
{
  ExprList *pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_BETWEEN, yymsp[-4].minor.yy346.pExpr, 0, 0);
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->x.pList = pList;
  }else{
    sqlite3ExprListDelete(pParse->db, pList);
  } 
  if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
}
#line 3201 "parse.c"
        break;
      case 223: /* expr ::= expr in_op LP exprlist RP */
#line 1050 "parse.y"
{
    if( yymsp[-1].minor.yy14==0 ){
      /* Expressions of the form
      **
      **      expr1 IN ()
      **      expr1 NOT IN ()
      **
      ** simplify to constants 0 (false) and 1 (true), respectively,
      ** regardless of the value of expr1.
      */
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_INTEGER, 0, 0, &sqlite3IntTokens[yymsp[-3].minor.yy328]);
      sqlite3ExprDelete(pParse->db, yymsp[-4].minor.yy346.pExpr);
    }else if( yymsp[-1].minor.yy14->nExpr==1 ){
      /* Expressions of the form:
      **
      **      expr1 IN (?1)
      **      expr1 NOT IN (?2)
      **
      ** with exactly one value on the RHS can be simplified to something
      ** like this:
      **
      **      expr1 == ?1
      **      expr1 <> ?2
      **
      ** But, the RHS of the == or <> is marked with the EP_Generic flag
      ** so that it may not contribute to the computation of comparison
      ** affinity or the collating sequence to use for comparison.  Otherwise,
      ** the semantics would be subtly different from IN or NOT IN.
      */
      Expr *pRHS = yymsp[-1].minor.yy14->a[0].pExpr;
      yymsp[-1].minor.yy14->a[0].pExpr = 0;
      sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
      /* pRHS cannot be NULL because a malloc error would have been detected
      ** before now and control would have never reached this point */
      if( ALWAYS(pRHS) ){
        pRHS->flags &= ~EP_Collate;
        pRHS->flags |= EP_Generic;
      }
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, yymsp[-3].minor.yy328 ? TK_NE : TK_EQ, yymsp[-4].minor.yy346.pExpr, pRHS, 0);
    }else{
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy346.pExpr, 0, 0);
      if( yygotominor.yy346.pExpr ){
        yygotominor.yy346.pExpr->x.pList = yymsp[-1].minor.yy14;
        sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
      }else{
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
      }
      if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    }
    yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3257 "parse.c"
        break;
      case 224: /* expr ::= LP select RP */
#line 1102 "parse.y"
{
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_SELECT, 0, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    yygotominor.yy346.zStart = yymsp[-2].minor.yy0.z;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3273 "parse.c"
        break;
      case 225: /* expr ::= expr in_op LP select RP */
#line 1114 "parse.y"
{
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy346.pExpr, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3290 "parse.c"
        break;
      case 226: /* expr ::= expr in_op nm dbnm */
#line 1127 "parse.y"
{
    SrcList *pSrc = sqlite3SrcListAppend(pParse->db, 0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-3].minor.yy346.pExpr, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = sqlite3SelectNew(pParse, 0,pSrc,0,0,0,0,0,0,0);
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SrcListDelete(pParse->db, pSrc);
    }
    if( yymsp[-2].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    yygotominor.yy346.zStart = yymsp[-3].minor.yy346.zStart;
    yygotominor.yy346.zEnd = yymsp[0].minor.yy0.z ? &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] : &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n];
  }
#line 3308 "parse.c"
        break;
      case 227: /* expr ::= EXISTS LP select RP */
#line 1141 "parse.y"
{
    Expr *p = yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_EXISTS, 0, 0, 0);
    if( p ){
      p->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(p, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, p);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    yygotominor.yy346.zStart = yymsp[-3].minor.yy0.z;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3324 "parse.c"
        break;
      case 228: /* expr ::= CASE case_operand case_exprlist case_else END */
#line 1156 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_CASE, yymsp[-3].minor.yy132, 0, 0);
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->x.pList = yymsp[-1].minor.yy132 ? sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy14,yymsp[-1].minor.yy132) : yymsp[-2].minor.yy14;
    sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
  }else{
    sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy14);
    sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy132);
  }
  yygotominor.yy346.zStart = yymsp[-4].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3340 "parse.c"
        break;
      case 229: /* case_exprlist ::= case_exprlist WHEN expr THEN expr */
#line 1170 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy14, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yygotominor.yy14, yymsp[0].minor.yy346.pExpr);
}
#line 3348 "parse.c"
        break;
      case 230: /* case_exprlist ::= WHEN expr THEN expr */
#line 1174 "parse.y"
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yygotominor.yy14, yymsp[0].minor.yy346.pExpr);
}
#line 3356 "parse.c"
        break;
      case 237: /* nexprlist ::= nexprlist COMMA expr */
#line 1195 "parse.y"
{yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy14,yymsp[0].minor.yy346.pExpr);}
#line 3361 "parse.c"
        break;
      case 238: /* nexprlist ::= expr */
#line 1197 "parse.y"
{yygotominor.yy14 = sqlite3ExprListAppend(pParse,0,yymsp[0].minor.yy346.pExpr);}
#line 3366 "parse.c"
        break;
      case 239: /* cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP idxlist RP where_opt */
#line 1203 "parse.y"
{
  sqlite3CreateIndex(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, 
                     sqlite3SrcListAppend(pParse->db,0,&yymsp[-4].minor.yy0,0), yymsp[-2].minor.yy14, yymsp[-10].minor.yy328,
                      &yymsp[-11].minor.yy0, yymsp[0].minor.yy132, SQLITE_SO_ASC, yymsp[-8].minor.yy328);
}
#line 3375 "parse.c"
        break;
      case 240: /* uniqueflag ::= UNIQUE */
      case 291: /* raisetype ::= ABORT */ yytestcase(yyruleno==291);
#line 1210 "parse.y"
{yygotominor.yy328 = OE_Abort;}
#line 3381 "parse.c"
        break;
      case 241: /* uniqueflag ::= */
#line 1211 "parse.y"
{yygotominor.yy328 = OE_None;}
#line 3386 "parse.c"
        break;
      case 244: /* idxlist ::= idxlist COMMA nm collate sortorder */
#line 1220 "parse.y"
{
  Expr *p = sqlite3ExprAddCollateToken(pParse, 0, &yymsp[-1].minor.yy0, 1);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy14, p);
  sqlite3ExprListSetName(pParse,yygotominor.yy14,&yymsp[-2].minor.yy0,1);
  sqlite3ExprListCheckLength(pParse, yygotominor.yy14, "index");
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
#line 3397 "parse.c"
        break;
      case 245: /* idxlist ::= nm collate sortorder */
#line 1227 "parse.y"
{
  Expr *p = sqlite3ExprAddCollateToken(pParse, 0, &yymsp[-1].minor.yy0, 1);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0, p);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
  sqlite3ExprListCheckLength(pParse, yygotominor.yy14, "index");
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
#line 3408 "parse.c"
        break;
      case 246: /* collate ::= */
#line 1236 "parse.y"
{yygotominor.yy0.z = 0; yygotominor.yy0.n = 0;}
#line 3413 "parse.c"
        break;
      case 248: /* cmd ::= DROP INDEX ifexists fullname */
#line 1242 "parse.y"
{sqlite3DropIndex(pParse, yymsp[0].minor.yy65, yymsp[-1].minor.yy328);}
#line 3418 "parse.c"
        break;
      case 249: /* cmd ::= VACUUM */
      case 250: /* cmd ::= VACUUM nm */ yytestcase(yyruleno==250);
#line 1248 "parse.y"
{sqlite3Vacuum(pParse);}
#line 3424 "parse.c"
        break;
      case 251: /* cmd ::= PRAGMA nm dbnm */
#line 1256 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,0,0);}
#line 3429 "parse.c"
        break;
      case 252: /* cmd ::= PRAGMA nm dbnm EQ nmnum */
#line 1257 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,0);}
#line 3434 "parse.c"
        break;
      case 253: /* cmd ::= PRAGMA nm dbnm LP nmnum RP */
#line 1258 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,0);}
#line 3439 "parse.c"
        break;
      case 254: /* cmd ::= PRAGMA nm dbnm EQ minus_num */
#line 1260 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,1);}
#line 3444 "parse.c"
        break;
      case 255: /* cmd ::= PRAGMA nm dbnm LP minus_num RP */
#line 1262 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,1);}
#line 3449 "parse.c"
        break;
      case 264: /* cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END */
#line 1278 "parse.y"
{
  Token all;
  all.z = yymsp[-3].minor.yy0.z;
  all.n = (int)(yymsp[0].minor.yy0.z - yymsp[-3].minor.yy0.z) + yymsp[0].minor.yy0.n;
  sqlite3FinishTrigger(pParse, yymsp[-1].minor.yy473, &all);
}
#line 3459 "parse.c"
        break;
      case 265: /* trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause */
#line 1287 "parse.y"
{
  sqlite3BeginTrigger(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, yymsp[-5].minor.yy328, yymsp[-4].minor.yy378.a, yymsp[-4].minor.yy378.b, yymsp[-2].minor.yy65, yymsp[0].minor.yy132, yymsp[-10].minor.yy328, yymsp[-8].minor.yy328);
  yygotominor.yy0 = (yymsp[-6].minor.yy0.n==0?yymsp[-7].minor.yy0:yymsp[-6].minor.yy0);
}
#line 3467 "parse.c"
        break;
      case 266: /* trigger_time ::= BEFORE */
      case 269: /* trigger_time ::= */ yytestcase(yyruleno==269);
#line 1293 "parse.y"
{ yygotominor.yy328 = TK_BEFORE; }
#line 3473 "parse.c"
        break;
      case 267: /* trigger_time ::= AFTER */
#line 1294 "parse.y"
{ yygotominor.yy328 = TK_AFTER;  }
#line 3478 "parse.c"
        break;
      case 268: /* trigger_time ::= INSTEAD OF */
#line 1295 "parse.y"
{ yygotominor.yy328 = TK_INSTEAD;}
#line 3483 "parse.c"
        break;
      case 270: /* trigger_event ::= DELETE|INSERT */
      case 271: /* trigger_event ::= UPDATE */ yytestcase(yyruleno==271);
#line 1300 "parse.y"
{yygotominor.yy378.a = yymsp[0].major; yygotominor.yy378.b = 0;}
#line 3489 "parse.c"
        break;
      case 272: /* trigger_event ::= UPDATE OF idlist */
#line 1302 "parse.y"
{yygotominor.yy378.a = TK_UPDATE; yygotominor.yy378.b = yymsp[0].minor.yy408;}
#line 3494 "parse.c"
        break;
      case 275: /* when_clause ::= */
      case 296: /* key_opt ::= */ yytestcase(yyruleno==296);
#line 1309 "parse.y"
{ yygotominor.yy132 = 0; }
#line 3500 "parse.c"
        break;
      case 276: /* when_clause ::= WHEN expr */
      case 297: /* key_opt ::= KEY expr */ yytestcase(yyruleno==297);
#line 1310 "parse.y"
{ yygotominor.yy132 = yymsp[0].minor.yy346.pExpr; }
#line 3506 "parse.c"
        break;
      case 277: /* trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI */
#line 1314 "parse.y"
{
  assert( yymsp[-2].minor.yy473!=0 );
  yymsp[-2].minor.yy473->pLast->pNext = yymsp[-1].minor.yy473;
  yymsp[-2].minor.yy473->pLast = yymsp[-1].minor.yy473;
  yygotominor.yy473 = yymsp[-2].minor.yy473;
}
#line 3516 "parse.c"
        break;
      case 278: /* trigger_cmd_list ::= trigger_cmd SEMI */
#line 1320 "parse.y"
{ 
  assert( yymsp[-1].minor.yy473!=0 );
  yymsp[-1].minor.yy473->pLast = yymsp[-1].minor.yy473;
  yygotominor.yy473 = yymsp[-1].minor.yy473;
}
#line 3525 "parse.c"
        break;
      case 280: /* trnm ::= nm DOT nm */
#line 1332 "parse.y"
{
  yygotominor.yy0 = yymsp[0].minor.yy0;
  sqlite3ErrorMsg(pParse, 
        "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
        "statements within triggers");
}
#line 3535 "parse.c"
        break;
      case 282: /* tridxby ::= INDEXED BY nm */
#line 1344 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the INDEXED BY clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3544 "parse.c"
        break;
      case 283: /* tridxby ::= NOT INDEXED */
#line 1349 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the NOT INDEXED clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3553 "parse.c"
        break;
      case 284: /* trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt */
#line 1362 "parse.y"
{ yygotominor.yy473 = sqlite3TriggerUpdateStep(pParse->db, &yymsp[-4].minor.yy0, yymsp[-1].minor.yy14, yymsp[0].minor.yy132, yymsp[-5].minor.yy186); }
#line 3558 "parse.c"
        break;
      case 285: /* trigger_cmd ::= insert_cmd INTO trnm inscollist_opt select */
#line 1366 "parse.y"
{yygotominor.yy473 = sqlite3TriggerInsertStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy408, yymsp[0].minor.yy3, yymsp[-4].minor.yy186);}
#line 3563 "parse.c"
        break;
      case 286: /* trigger_cmd ::= DELETE FROM trnm tridxby where_opt */
#line 1370 "parse.y"
{yygotominor.yy473 = sqlite3TriggerDeleteStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[0].minor.yy132);}
#line 3568 "parse.c"
        break;
      case 287: /* trigger_cmd ::= select */
#line 1373 "parse.y"
{yygotominor.yy473 = sqlite3TriggerSelectStep(pParse->db, yymsp[0].minor.yy3); }
#line 3573 "parse.c"
        break;
      case 288: /* expr ::= RAISE LP IGNORE RP */
#line 1376 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, 0); 
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->affinity = OE_Ignore;
  }
  yygotominor.yy346.zStart = yymsp[-3].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3585 "parse.c"
        break;
      case 289: /* expr ::= RAISE LP raisetype COMMA nm RP */
#line 1384 "parse.y"
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, &yymsp[-1].minor.yy0); 
  if( yygotominor.yy346.pExpr ) {
    yygotominor.yy346.pExpr->affinity = (char)yymsp[-3].minor.yy328;
  }
  yygotominor.yy346.zStart = yymsp[-5].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3597 "parse.c"
        break;
      case 290: /* raisetype ::= ROLLBACK */
#line 1395 "parse.y"
{yygotominor.yy328 = OE_Rollback;}
#line 3602 "parse.c"
        break;
      case 292: /* raisetype ::= FAIL */
#line 1397 "parse.y"
{yygotominor.yy328 = OE_Fail;}
#line 3607 "parse.c"
        break;
      case 293: /* cmd ::= DROP TRIGGER ifexists fullname */
#line 1402 "parse.y"
{
  sqlite3DropTrigger(pParse,yymsp[0].minor.yy65,yymsp[-1].minor.yy328);
}
#line 3614 "parse.c"
        break;
      case 294: /* cmd ::= ATTACH database_kw_opt expr AS expr key_opt */
#line 1409 "parse.y"
{
  sqlite3Attach(pParse, yymsp[-3].minor.yy346.pExpr, yymsp[-1].minor.yy346.pExpr, yymsp[0].minor.yy132);
}
#line 3621 "parse.c"
        break;
      case 295: /* cmd ::= DETACH database_kw_opt expr */
#line 1412 "parse.y"
{
  sqlite3Detach(pParse, yymsp[0].minor.yy346.pExpr);
}
#line 3628 "parse.c"
        break;
      case 300: /* cmd ::= REINDEX */
#line 1427 "parse.y"
{sqlite3Reindex(pParse, 0, 0);}
#line 3633 "parse.c"
        break;
      case 301: /* cmd ::= REINDEX nm dbnm */
#line 1428 "parse.y"
{sqlite3Reindex(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3638 "parse.c"
        break;
      case 302: /* cmd ::= ANALYZE */
#line 1433 "parse.y"
{sqlite3Analyze(pParse, 0, 0);}
#line 3643 "parse.c"
        break;
      case 303: /* cmd ::= ANALYZE nm dbnm */
#line 1434 "parse.y"
{sqlite3Analyze(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3648 "parse.c"
        break;
      case 304: /* cmd ::= ALTER TABLE fullname RENAME TO nm */
#line 1439 "parse.y"
{
  sqlite3AlterRenameTable(pParse,yymsp[-3].minor.yy65,&yymsp[0].minor.yy0);
}
#line 3655 "parse.c"
        break;
      case 305: /* cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column */
#line 1442 "parse.y"
{
  sqlite3AlterFinishAddColumn(pParse, &yymsp[0].minor.yy0);
}
#line 3662 "parse.c"
        break;
      case 306: /* add_column_fullname ::= fullname */
#line 1445 "parse.y"
{
  pParse->db->lookaside.bEnabled = 0;
  sqlite3AlterBeginAddColumn(pParse, yymsp[0].minor.yy65);
}
#line 3670 "parse.c"
        break;
      case 309: /* cmd ::= create_vtab */
#line 1455 "parse.y"
{sqlite3VtabFinishParse(pParse,0);}
#line 3675 "parse.c"
        break;
      case 310: /* cmd ::= create_vtab LP vtabarglist RP */
#line 1456 "parse.y"
{sqlite3VtabFinishParse(pParse,&yymsp[0].minor.yy0);}
#line 3680 "parse.c"
        break;
      case 311: /* create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm */
#line 1458 "parse.y"
{
    sqlite3VtabBeginParse(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy328);
}
#line 3687 "parse.c"
        break;
      case 314: /* vtabarg ::= */
#line 1463 "parse.y"
{sqlite3VtabArgInit(pParse);}
#line 3692 "parse.c"
        break;
      case 316: /* vtabargtoken ::= ANY */
      case 317: /* vtabargtoken ::= lp anylist RP */ yytestcase(yyruleno==317);
      case 318: /* lp ::= LP */ yytestcase(yyruleno==318);
#line 1465 "parse.y"
{sqlite3VtabArgExtend(pParse,&yymsp[0].minor.yy0);}
#line 3699 "parse.c"
        break;
      case 322: /* with ::= */
#line 1480 "parse.y"
{yygotominor.yy59 = 0;}
#line 3704 "parse.c"
        break;
      case 323: /* with ::= WITH wqlist */
      case 324: /* with ::= WITH RECURSIVE wqlist */ yytestcase(yyruleno==324);
#line 1482 "parse.y"
{ yygotominor.yy59 = yymsp[0].minor.yy59; }
#line 3710 "parse.c"
        break;
      case 325: /* wqlist ::= nm idxlist_opt AS LP select RP */
#line 1485 "parse.y"
{
  yygotominor.yy59 = sqlite3WithAdd(pParse, 0, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy14, yymsp[-1].minor.yy3);
}
#line 3717 "parse.c"
        break;
      case 326: /* wqlist ::= wqlist COMMA nm idxlist_opt AS LP select RP */
#line 1488 "parse.y"
{
  yygotominor.yy59 = sqlite3WithAdd(pParse, yymsp[-7].minor.yy59, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy14, yymsp[-1].minor.yy3);
}
#line 3724 "parse.c"
        break;
      default:
      /* (0) input ::= cmdlist */ yytestcase(yyruleno==0);
      /* (1) cmdlist ::= cmdlist ecmd */ yytestcase(yyruleno==1);
      /* (2) cmdlist ::= ecmd */ yytestcase(yyruleno==2);
      /* (3) ecmd ::= SEMI */ yytestcase(yyruleno==3);
      /* (4) ecmd ::= explain cmdx SEMI */ yytestcase(yyruleno==4);
      /* (10) trans_opt ::= */ yytestcase(yyruleno==10);
      /* (11) trans_opt ::= TRANSACTION */ yytestcase(yyruleno==11);
      /* (12) trans_opt ::= TRANSACTION nm */ yytestcase(yyruleno==12);
      /* (20) savepoint_opt ::= SAVEPOINT */ yytestcase(yyruleno==20);
      /* (21) savepoint_opt ::= */ yytestcase(yyruleno==21);
      /* (25) cmd ::= create_table create_table_args */ yytestcase(yyruleno==25);
      /* (36) columnlist ::= columnlist COMMA column */ yytestcase(yyruleno==36);
      /* (37) columnlist ::= column */ yytestcase(yyruleno==37);
      /* (43) type ::= */ yytestcase(yyruleno==43);
      /* (50) signed ::= plus_num */ yytestcase(yyruleno==50);
      /* (51) signed ::= minus_num */ yytestcase(yyruleno==51);
      /* (52) carglist ::= carglist ccons */ yytestcase(yyruleno==52);
      /* (53) carglist ::= */ yytestcase(yyruleno==53);
      /* (60) ccons ::= NULL onconf */ yytestcase(yyruleno==60);
      /* (88) conslist ::= conslist tconscomma tcons */ yytestcase(yyruleno==88);
      /* (89) conslist ::= tcons */ yytestcase(yyruleno==89);
      /* (91) tconscomma ::= */ yytestcase(yyruleno==91);
      /* (273) foreach_clause ::= */ yytestcase(yyruleno==273);
      /* (274) foreach_clause ::= FOR EACH ROW */ yytestcase(yyruleno==274);
      /* (281) tridxby ::= */ yytestcase(yyruleno==281);
      /* (298) database_kw_opt ::= DATABASE */ yytestcase(yyruleno==298);
      /* (299) database_kw_opt ::= */ yytestcase(yyruleno==299);
      /* (307) kwcolumn_opt ::= */ yytestcase(yyruleno==307);
      /* (308) kwcolumn_opt ::= COLUMNKW */ yytestcase(yyruleno==308);
      /* (312) vtabarglist ::= vtabarg */ yytestcase(yyruleno==312);
      /* (313) vtabarglist ::= vtabarglist COMMA vtabarg */ yytestcase(yyruleno==313);
      /* (315) vtabarg ::= vtabarg vtabargtoken */ yytestcase(yyruleno==315);
      /* (319) anylist ::= */ yytestcase(yyruleno==319);
      /* (320) anylist ::= anylist LP anylist RP */ yytestcase(yyruleno==320);
      /* (321) anylist ::= anylist ANY */ yytestcase(yyruleno==321);
        break;
  };
  assert( yyruleno>=0 && yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  sqlite3ParserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 32 "parse.y"

  UNUSED_PARAMETER(yymajor);  /* Silence some compiler warnings */
  assert( TOKEN.z[0] );  /* The tokenizer always gives us a token */
  sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &TOKEN);
#line 3827 "parse.c"
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3ParserAlloc" which describes the current state of the parser.
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
void sqlite3Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3ParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  sqlite3ParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
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
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
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
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
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
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
