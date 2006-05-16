/* A Bison parser, made from keynote.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse knparse
#define yylex knlex
#define yyerror knerror
#define yylval knlval
#define yychar knchar
#define yydebug kndebug
#define yynerrs knnerrs
# define	NUM	257
# define	KOF	258
# define	FLOAT	259
# define	STRING	260
# define	VARIABLE	261
# define	TRUE	262
# define	FALSE	263
# define	OPENPAREN	264
# define	CLOSEPAREN	265
# define	EQQ	266
# define	COMMA	267
# define	ACTSTR	268
# define	LOCINI	269
# define	KEYPRE	270
# define	KNVERSION	271
# define	DOTT	272
# define	SIGNERKEY	273
# define	HINT	274
# define	OPENBLOCK	275
# define	CLOSEBLOCK	276
# define	SIGNATUREENTRY	277
# define	PRIVATEKEY	278
# define	SEMICOLON	279
# define	EQ	280
# define	NE	281
# define	LT	282
# define	GT	283
# define	LE	284
# define	GE	285
# define	REGEXP	286
# define	OR	287
# define	AND	288
# define	NOT	289
# define	PLUS	290
# define	MINUS	291
# define	MULT	292
# define	DIV	293
# define	MOD	294
# define	EXP	295
# define	UNARYMINUS	296
# define	DEREF	297
# define	OPENNUM	298
# define	OPENFLT	299


#line 20 "keynote.y"
#ifndef YYSTYPE
typedef union {
    char   *string;
    double  doubval;
    int     intval;
    int     bool;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#line 44 "keynote.y"

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "header.h"
#include "keynote.h"
#include "assertion.h"

static int *keynote_kth_array = (int *) NULL;
static int keylistcount = 0;

static int   resolve_assertion(char *);
static int   keynote_init_kth(void);
static int   isfloatstring(char *);
static int   checkexception(int);
static char *my_lookup(char *);
static int   intpow(int, int);
static int   get_kth(int);
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		172
#define	YYFLAG		-32768
#define	YYNTBASE	46

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 299 ? yytranslate[x] : 81)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     5,     6,    10,    11,    15,    16,    20,
      21,    25,    26,    30,    31,    35,    36,    38,    40,    42,
      43,    48,    49,    54,    58,    59,    60,    67,    69,    73,
      75,    76,    78,    82,    83,    89,    91,    92,    93,    98,
     102,   104,   106,   110,   114,   115,   120,   121,   126,   129,
     131,   133,   135,   137,   139,   143,   147,   151,   155,   159,
     163,   167,   171,   175,   179,   183,   187,   191,   195,   199,
     203,   206,   210,   212,   215,   219,   223,   227,   231,   235,
     238,   242,   244,   247,   251,   255,   259,   263,   267,   271,
     275,   279,   281,   283,   287,   289
};
static const short yyrhs[] =
{
      -1,    15,    47,    63,     0,     0,    14,    48,    66,     0,
       0,    16,    49,    54,     0,     0,    19,    50,    62,     0,
       0,    23,    51,    62,     0,     0,    17,    52,     6,     0,
       0,    24,    53,     6,     0,     0,    55,     0,    62,     0,
      56,     0,     0,    55,    34,    57,    55,     0,     0,    55,
      33,    58,    55,     0,    10,    56,    11,     0,     0,     0,
       4,    59,    10,    60,    61,    11,     0,    62,     0,    62,
      13,    61,     0,    79,     0,     0,    64,     0,     7,    12,
       6,     0,     0,     7,    12,     6,    65,    64,     0,    67,
       0,     0,     0,    69,    68,    25,    67,     0,    71,    20,
      70,     0,    71,     0,    79,     0,    21,    67,    22,     0,
      10,    71,    11,     0,     0,    71,    34,    72,    71,     0,
       0,    71,    33,    73,    71,     0,    35,    71,     0,    74,
       0,    75,     0,    78,     0,     8,     0,     9,     0,    76,
      28,    76,     0,    76,    29,    76,     0,    76,    26,    76,
       0,    76,    30,    76,     0,    76,    31,    76,     0,    76,
      27,    76,     0,    77,    28,    77,     0,    77,    29,    77,
       0,    77,    30,    77,     0,    77,    31,    77,     0,    76,
      36,    76,     0,    76,    37,    76,     0,    76,    38,    76,
       0,    76,    39,    76,     0,    76,    40,    76,     0,    76,
      41,    76,     0,    37,    76,     0,    10,    76,    11,     0,
       3,     0,    44,    80,     0,    77,    36,    77,     0,    77,
      37,    77,     0,    77,    38,    77,     0,    77,    39,    77,
       0,    77,    41,    77,     0,    37,    77,     0,    10,    77,
      11,     0,     5,     0,    45,    80,     0,    79,    26,    79,
       0,    79,    27,    79,     0,    79,    28,    79,     0,    79,
      29,    79,     0,    79,    30,    79,     0,    79,    31,    79,
       0,    79,    32,    79,     0,    79,    18,    79,     0,    80,
       0,     6,     0,    10,    79,    11,     0,     7,     0,    43,
      79,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    76,    76,    78,    78,    79,    79,    81,    81,    82,
      82,    84,    84,    90,    90,    95,    98,   102,   103,   105,
     105,   114,   114,   123,   124,   124,   124,   142,   150,   159,
     197,   198,   200,   234,   234,   269,   274,   275,   275,   289,
     296,   304,   317,   320,   321,   321,   326,   326,   331,   332,
     333,   334,   335,   336,   338,   339,   340,   341,   342,   343,
     345,   346,   347,   348,   350,   351,   352,   353,   361,   369,
     370,   371,   372,   373,   388,   389,   390,   391,   399,   403,
     404,   405,   406,   422,   434,   446,   458,   470,   482,   494,
     583,   607,   609,   610,   611,   637
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "NUM", "KOF", "FLOAT", "STRING", "VARIABLE", 
  "TRUE", "FALSE", "OPENPAREN", "CLOSEPAREN", "EQQ", "COMMA", "ACTSTR", 
  "LOCINI", "KEYPRE", "KNVERSION", "DOTT", "SIGNERKEY", "HINT", 
  "OPENBLOCK", "CLOSEBLOCK", "SIGNATUREENTRY", "PRIVATEKEY", "SEMICOLON", 
  "EQ", "NE", "LT", "GT", "LE", "GE", "REGEXP", "OR", "AND", "NOT", 
  "PLUS", "MINUS", "MULT", "DIV", "MOD", "EXP", "UNARYMINUS", "DEREF", 
  "OPENNUM", "OPENFLT", "grammarswitch", "@1", "@2", "@3", "@4", "@5", 
  "@6", "@7", "keypredicate", "notemptykeypredicate", "keyexp", "@8", 
  "@9", "@10", "@11", "keylist", "key", "localinit", "localconstants", 
  "@12", "program", "prog", "@13", "notemptyprog", "afterhint", "expr", 
  "@14", "@15", "numexp", "floatexp", "numex", "floatex", "stringexp", 
  "str", "strnotconcat", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    47,    46,    48,    46,    49,    46,    50,    46,    51,
      46,    52,    46,    53,    46,    54,    54,    55,    55,    57,
      56,    58,    56,    56,    59,    60,    56,    61,    61,    62,
      63,    63,    64,    65,    64,    66,    67,    68,    67,    69,
      69,    70,    70,    71,    72,    71,    73,    71,    71,    71,
      71,    71,    71,    71,    74,    74,    74,    74,    74,    74,
      75,    75,    75,    75,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    78,    78,    78,    78,    78,    78,    78,
      79,    79,    80,    80,    80,    80
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     1,     1,     1,     0,
       4,     0,     4,     3,     0,     0,     6,     1,     3,     1,
       0,     1,     3,     0,     5,     1,     0,     0,     4,     3,
       1,     1,     3,     3,     0,     4,     0,     4,     2,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     1,     2,     3,     3,     3,     3,     3,     2,
       3,     1,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     1,     2
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     3,     1,     5,    11,     7,     9,    13,    36,    30,
      15,     0,     0,     0,     0,    72,    81,    92,    94,    52,
      53,     0,     0,     0,     0,     0,     0,     4,    35,    37,
      40,    49,    50,     0,     0,    51,     0,    91,     0,     2,
      31,    24,     0,     6,    16,    18,    17,    29,    12,     0,
       8,    10,    14,     0,     0,     0,     0,    48,     0,    70,
      79,    95,    73,    82,     0,     0,    46,    44,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,    29,    21,    19,     0,    43,    71,    80,    93,     0,
       0,    36,    36,    39,    41,     0,     0,     0,     0,    56,
      59,    54,    55,    57,    58,    64,    65,    66,    67,    68,
      69,     0,     0,    60,    61,    62,    63,    74,    75,    76,
      77,    78,    90,    83,    84,    85,    86,    87,    88,    89,
      32,    25,    23,     0,     0,    38,     0,    47,    45,     0,
       0,    22,    20,    42,    34,     0,    27,    26,     0,    28,
       0,     0,     0
};

static const short yydefgoto[] =
{
     170,     9,     8,    10,    12,    13,    11,    14,    43,    44,
      45,   154,   153,    98,   160,   165,    46,    39,    40,   159,
      27,    28,    64,    29,   113,    30,   116,   115,    31,    32,
      33,    34,    35,    47,    37
};

static const short yypact[] =
{
     120,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    18,    -1,
     114,     6,   119,   119,     8,-32768,-32768,-32768,-32768,-32768,
  -32768,    18,    18,    36,   119,   119,   119,-32768,-32768,-32768,
      97,-32768,-32768,   180,   201,-32768,   196,-32768,    -3,-32768,
  -32768,-32768,   114,-32768,    94,-32768,-32768,     0,-32768,   119,
  -32768,-32768,-32768,    25,   138,   122,   173,-32768,    36,-32768,
  -32768,-32768,-32768,-32768,    10,   102,-32768,-32768,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
       3,     3,     3,     3,     3,     3,     3,     3,     3,   119,
     119,   119,   119,   119,   119,   119,   119,    14,    28,    94,
      46,    32,-32768,-32768,    32,-32768,-32768,-32768,-32768,    -7,
     144,    18,    18,-32768,     0,    18,    18,    12,    12,    38,
      38,    38,    38,    38,    38,   132,   132,    42,    42,    42,
  -32768,     3,     3,   151,   151,   151,   151,    13,    13,    78,
      78,-32768,-32768,     0,     0,     0,     0,     0,     0,     0,
     115,-32768,-32768,   114,   114,-32768,   110,   104,-32768,    -1,
     119,   106,-32768,-32768,-32768,   143,   182,-32768,   119,-32768,
     186,   197,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -40,
     154,-32768,-32768,-32768,-32768,    30,   -12,-32768,    53,-32768,
  -32768,    35,-32768,-32768,-32768,   -11,-32768,-32768,-32768,-32768,
      24,   -16,-32768,    -5,   168
};


#define	YYLAST		242


static const short yytable[] =
{
      50,    51,    99,    36,   106,    55,    38,    60,    16,    97,
      53,    57,    48,   131,    52,    15,    56,    36,    89,    61,
     150,    15,   117,    16,    17,    18,    19,    20,    21,    74,
      75,    76,    77,    78,    79,   111,   105,   101,   151,    15,
     132,    16,   110,   108,   104,    54,    58,    59,    26,   118,
      89,    86,    87,    22,    88,    23,    25,   152,    66,    67,
     114,    24,    25,    26,   133,   134,   135,   136,   137,   138,
     139,   140,   141,    23,    74,    75,    76,    77,    78,    79,
      25,    26,   109,    79,   142,   143,   144,   145,   146,   147,
     148,   149,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   157,   158,    36,    36,    17,    18,
      36,    36,    49,   161,   162,   110,    60,    65,    41,    88,
      17,    18,   -33,   112,    42,    17,    18,   102,   103,    49,
      66,    67,   163,   107,     1,     2,     3,     4,    67,     5,
     103,   109,    59,     6,     7,    24,   155,   156,   166,   106,
      80,    81,    82,    83,   167,   107,   166,    24,    84,    85,
      86,    87,    24,    88,    68,    69,    70,    71,    72,    73,
      76,    77,    78,    79,    74,    75,    76,    77,    78,    79,
      84,    85,    86,    87,   108,    88,   171,    84,    85,    86,
      87,    89,    88,    62,    63,   168,   100,   172,   169,    90,
      91,    92,    93,    94,    95,    96,    68,    69,    70,    71,
      72,    73,   164,     0,    89,     0,    74,    75,    76,    77,
      78,    79,    90,    91,    92,    93,    94,    95,    96,    80,
      81,    82,    83,     0,     0,     0,     0,    84,    85,    86,
      87,     0,    88
};

static const short yycheck[] =
{
      12,    13,    42,     8,    11,    21,     7,    23,     5,    12,
      21,    22,     6,    10,     6,     3,    21,    22,    18,    24,
       6,     3,    10,     5,     6,     7,     8,     9,    10,    36,
      37,    38,    39,    40,    41,    25,    11,    42,    10,     3,
      37,     5,    58,    11,    49,    21,    10,    23,    45,    37,
      18,    38,    39,    35,    41,    37,    44,    11,    33,    34,
      65,    43,    44,    45,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    37,    36,    37,    38,    39,    40,    41,
      44,    45,    58,    41,    89,    90,    91,    92,    93,    94,
      95,    96,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,   115,   116,   111,   112,     6,     7,
     115,   116,    10,   153,   154,   131,   132,    20,     4,    41,
       6,     7,     7,    21,    10,     6,     7,    33,    34,    10,
      33,    34,    22,    11,    14,    15,    16,    17,    34,    19,
      34,   117,   118,    23,    24,    43,   111,   112,   160,    11,
      28,    29,    30,    31,    11,    11,   168,    43,    36,    37,
      38,    39,    43,    41,    26,    27,    28,    29,    30,    31,
      38,    39,    40,    41,    36,    37,    38,    39,    40,    41,
      36,    37,    38,    39,    11,    41,     0,    36,    37,    38,
      39,    18,    41,    25,    26,    13,    42,     0,   168,    26,
      27,    28,    29,    30,    31,    32,    26,    27,    28,    29,
      30,    31,   159,    -1,    18,    -1,    36,    37,    38,    39,
      40,    41,    26,    27,    28,    29,    30,    31,    32,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    36,    37,    38,
      39,    -1,    41
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 76 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 3:
#line 78 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 5:
#line 79 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 7:
#line 81 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 9:
#line 82 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 11:
#line 84 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 12:
#line 85 "keynote.y"
{ keynote_lex_remove(yyvsp[0].string);
				 if (strcmp(yyvsp[0].string, KEYNOTE_VERSION_STRING))
				   keynote_errno = ERROR_SYNTAX;
				 free(yyvsp[0].string);
			       }
    break;
case 13:
#line 90 "keynote.y"
{ keynote_exceptionflag = keynote_donteval = 0; }
    break;
case 14:
#line 91 "keynote.y"
{ keynote_lex_remove(yyvsp[0].string);
			         keynote_privkey = yyvsp[0].string;
			       }
    break;
case 15:
#line 95 "keynote.y"
{ keynote_returnvalue = 0;
                                return 0; 
                              }
    break;
case 16:
#line 98 "keynote.y"
{ keynote_returnvalue = yyvsp[0].intval;
				return 0;
                              }
    break;
case 17:
#line 102 "keynote.y"
{ yyval.intval = yyvsp[0].intval; }
    break;
case 18:
#line 103 "keynote.y"
{ yyval.intval = yyvsp[0].intval; }
    break;
case 19:
#line 105 "keynote.y"
{ if ((yyvsp[-1].intval == 0) && !keynote_justrecord)
                                     keynote_donteval = 1;
                                 }
    break;
case 20:
#line 108 "keynote.y"
{ if (yyvsp[-3].intval > yyvsp[0].intval)
		     yyval.intval = yyvsp[0].intval;
		   else
	       	     yyval.intval = yyvsp[-3].intval;
		   keynote_donteval = 0;
                 }
    break;
case 21:
#line 114 "keynote.y"
{ if ((yyvsp[-1].intval == (keynote_current_session->ks_values_num - 1)) && !keynote_justrecord)
	                             keynote_donteval = 1;
       	                         }
    break;
case 22:
#line 117 "keynote.y"
{ if (yyvsp[-3].intval >= yyvsp[0].intval)
		     yyval.intval = yyvsp[-3].intval;
		   else
		     yyval.intval = yyvsp[0].intval;
		   keynote_donteval = 0;
                 }
    break;
case 23:
#line 123 "keynote.y"
{ yyval.intval = yyvsp[-1].intval; }
    break;
case 24:
#line 124 "keynote.y"
{ keylistcount = 0; }
    break;
case 25:
#line 124 "keynote.y"
{
			 if (!keynote_justrecord && !keynote_donteval)
 	                   if (keynote_init_kth() == -1)
			     return -1;
                       }
    break;
case 26:
#line 129 "keynote.y"
{
			      if (keylistcount < yyvsp[-5].intval)
			      {
				  keynote_errno = ERROR_SYNTAX;
				  return -1;
			      }

			    if (!keynote_justrecord && !keynote_donteval)
			      yyval.intval = get_kth(yyvsp[-5].intval);
			    else
			      yyval.intval = 0;
			  }
    break;
case 27:
#line 143 "keynote.y"
{ /* Don't do anything if we're just recording */ 
              if (!keynote_justrecord && !keynote_donteval)
		if ((yyvsp[0].intval < keynote_current_session->ks_values_num) && (yyvsp[0].intval >= 0))
		  keynote_kth_array[yyvsp[0].intval]++;

	      keylistcount++;
            }
    break;
case 28:
#line 151 "keynote.y"
{ /* Don't do anything if we're just recording */ 
	      if (!keynote_justrecord && !keynote_donteval)
		if ((yyvsp[-2].intval < keynote_current_session->ks_values_num) && (yyvsp[-2].intval >= 0))
		  keynote_kth_array[yyvsp[-2].intval]++;

	      keylistcount++;
            }
    break;
case 29:
#line 159 "keynote.y"
{
		   if (keynote_donteval)
		     yyval.intval = 0;
		   else
		   {
		       keynote_lex_remove(yyvsp[0].string);
		       if (keynote_justrecord)
		       {
			   if (keynote_keylist_add(&keynote_keypred_keylist,
						   yyvsp[0].string) == -1)
			   {
			       free(yyvsp[0].string);
			       return -1;
			   }

			   yyval.intval = 0;
		       }
		       else
			 switch (keynote_in_action_authorizers(yyvsp[0].string, KEYNOTE_ALGORITHM_UNSPEC))
			 {
			     case -1:
				 free(yyvsp[0].string);
				 return -1;
				 
			     case RESULT_TRUE:
				 free(yyvsp[0].string);
				 yyval.intval = keynote_current_session->ks_values_num -
				      1;
				 break;
				 
			     default:
				 yyval.intval = resolve_assertion(yyvsp[0].string);
				 free(yyvsp[0].string);
				 break;
			 }
		   }
                 }
    break;
case 32:
#line 201 "keynote.y"
{
            int i;

            keynote_lex_remove(yyvsp[-2].string);
	    keynote_lex_remove(yyvsp[0].string);
 
	    /*
	     * Variable names starting with underscores are illegal here.
	     */
	    if (yyvsp[-2].string[0] == '_')
	    {
		free(yyvsp[-2].string);
		free(yyvsp[0].string);
		keynote_errno = ERROR_SYNTAX;
		return -1;
	    }
	    
	    /* If the identifier already exists, report error. */
	    if (keynote_env_lookup(yyvsp[-2].string, &keynote_init_list, 1) != (char *) NULL)
	    {
		free(yyvsp[-2].string);
		free(yyvsp[0].string);
		keynote_errno = ERROR_SYNTAX;
		return -1;
	    }

	    i = keynote_env_add(yyvsp[-2].string, yyvsp[0].string, &keynote_init_list, 1, 0);
	    free(yyvsp[-2].string);
	    free(yyvsp[0].string);

	    if (i != RESULT_TRUE)
	      return -1;
	  }
    break;
case 33:
#line 235 "keynote.y"
{
            int i;

	    keynote_lex_remove(yyvsp[-2].string);
	    keynote_lex_remove(yyvsp[0].string);

	    /*
	     * Variable names starting with underscores are illegal here.
	     */
	    if (yyvsp[-2].string[0] == '_')
	    {
		free(yyvsp[-2].string);
		free(yyvsp[0].string);
		keynote_errno = ERROR_SYNTAX;
		return -1;
	    }
	 
	    /* If the identifier already exists, report error. */
	    if (keynote_env_lookup(yyvsp[-2].string, &keynote_init_list, 1) != (char *) NULL)
	    {
		free(yyvsp[-2].string);
		free(yyvsp[0].string);
		keynote_errno = ERROR_SYNTAX;
		return -1;
	    }

	    i = keynote_env_add(yyvsp[-2].string, yyvsp[0].string, &keynote_init_list, 1, 0);
	    free(yyvsp[-2].string);
	    free(yyvsp[0].string);

	    if (i != RESULT_TRUE)
	      return -1;
	  }
    break;
case 35:
#line 269 "keynote.y"
{ 
	        keynote_returnvalue = yyvsp[0].intval;
		return 0;
	      }
    break;
case 36:
#line 274 "keynote.y"
{ yyval.intval = 0; }
    break;
case 37:
#line 275 "keynote.y"
{
			  /* 
			   * Cleanup envlist of additions such as 
			   * regexp results
			   */
			  keynote_env_cleanup(&keynote_temp_list, 1);
                    }
    break;
case 38:
#line 282 "keynote.y"
{
		      if (yyvsp[-3].intval > yyvsp[0].intval)
			yyval.intval = yyvsp[-3].intval;
		      else
			yyval.intval = yyvsp[0].intval;
                    }
    break;
case 39:
#line 290 "keynote.y"
{
		if (checkexception(yyvsp[-2].bool))
		  yyval.intval = yyvsp[0].intval;
		else
		  yyval.intval = 0;
	      }
    break;
case 40:
#line 297 "keynote.y"
{
		if (checkexception(yyvsp[0].bool))
		  yyval.intval = keynote_current_session->ks_values_num - 1;
		else
		  yyval.intval = 0;
	      }
    break;
case 41:
#line 304 "keynote.y"
{  if (keynote_exceptionflag || keynote_donteval)
		    yyval.intval = 0;
		  else
		  {
		      keynote_lex_remove(yyvsp[0].string);

		      yyval.intval = keynote_retindex(yyvsp[0].string);
		      if (yyval.intval == -1)   /* Invalid return value */
			yyval.intval = 0;

		      free(yyvsp[0].string);
		  }
                }
    break;
case 42:
#line 317 "keynote.y"
{ yyval.intval = yyvsp[-1].intval; }
    break;
case 43:
#line 320 "keynote.y"
{ yyval.bool = yyvsp[-1].bool; }
    break;
case 44:
#line 321 "keynote.y"
{ if (yyvsp[-1].bool == 0)
	               keynote_donteval = 1;
	           }
    break;
case 45:
#line 323 "keynote.y"
{ yyval.bool = (yyvsp[-3].bool && yyvsp[0].bool);
		                          keynote_donteval = 0;
		                        }
    break;
case 46:
#line 326 "keynote.y"
{ if (yyvsp[-1].bool)
	              keynote_donteval = 1; 
	          }
    break;
case 47:
#line 328 "keynote.y"
{ yyval.bool = (yyvsp[-3].bool || yyvsp[0].bool);
		                          keynote_donteval = 0;
                                        }
    break;
case 48:
#line 331 "keynote.y"
{ yyval.bool = !(yyvsp[0].bool); }
    break;
case 49:
#line 332 "keynote.y"
{ yyval.bool = yyvsp[0].bool; }
    break;
case 50:
#line 333 "keynote.y"
{ yyval.bool = yyvsp[0].bool; }
    break;
case 51:
#line 334 "keynote.y"
{ yyval.bool = yyvsp[0].bool; }
    break;
case 52:
#line 335 "keynote.y"
{ yyval.bool = 1; }
    break;
case 53:
#line 336 "keynote.y"
{ yyval.bool = 0; }
    break;
case 54:
#line 338 "keynote.y"
{ yyval.bool = yyvsp[-2].intval < yyvsp[0].intval; }
    break;
case 55:
#line 339 "keynote.y"
{ yyval.bool = yyvsp[-2].intval > yyvsp[0].intval; }
    break;
case 56:
#line 340 "keynote.y"
{ yyval.bool = yyvsp[-2].intval == yyvsp[0].intval; }
    break;
case 57:
#line 341 "keynote.y"
{ yyval.bool = yyvsp[-2].intval <= yyvsp[0].intval; }
    break;
case 58:
#line 342 "keynote.y"
{ yyval.bool = yyvsp[-2].intval >= yyvsp[0].intval; }
    break;
case 59:
#line 343 "keynote.y"
{ yyval.bool = yyvsp[-2].intval != yyvsp[0].intval; }
    break;
case 60:
#line 345 "keynote.y"
{ yyval.bool = yyvsp[-2].doubval < yyvsp[0].doubval; }
    break;
case 61:
#line 346 "keynote.y"
{ yyval.bool = yyvsp[-2].doubval > yyvsp[0].doubval; }
    break;
case 62:
#line 347 "keynote.y"
{ yyval.bool = yyvsp[-2].doubval <= yyvsp[0].doubval; }
    break;
case 63:
#line 348 "keynote.y"
{ yyval.bool = yyvsp[-2].doubval >= yyvsp[0].doubval; }
    break;
case 64:
#line 350 "keynote.y"
{ yyval.intval = yyvsp[-2].intval + yyvsp[0].intval; }
    break;
case 65:
#line 351 "keynote.y"
{ yyval.intval = yyvsp[-2].intval - yyvsp[0].intval; }
    break;
case 66:
#line 352 "keynote.y"
{ yyval.intval = yyvsp[-2].intval * yyvsp[0].intval; }
    break;
case 67:
#line 353 "keynote.y"
{ if (yyvsp[0].intval == 0)
	                      {
				  if (!keynote_donteval)
				    keynote_exceptionflag = 1;
			      }
	                      else
			        yyval.intval = (yyvsp[-2].intval / yyvsp[0].intval);
			    }
    break;
case 68:
#line 361 "keynote.y"
{ if (yyvsp[0].intval == 0)
	                      {
				  if (!keynote_donteval)
				    keynote_exceptionflag = 1;
			      }
	                      else
			        yyval.intval = yyvsp[-2].intval % yyvsp[0].intval;
			    }
    break;
case 69:
#line 369 "keynote.y"
{ yyval.intval = intpow(yyvsp[-2].intval, yyvsp[0].intval); }
    break;
case 70:
#line 370 "keynote.y"
{ yyval.intval = -(yyvsp[0].intval); }
    break;
case 71:
#line 371 "keynote.y"
{ yyval.intval = yyvsp[-1].intval; }
    break;
case 72:
#line 372 "keynote.y"
{ yyval.intval = yyvsp[0].intval; }
    break;
case 73:
#line 373 "keynote.y"
{ if (keynote_exceptionflag ||
					      keynote_donteval)
	                                    yyval.intval = 0;
 	                                  else
					  {
					      keynote_lex_remove(yyvsp[0].string);

					      if (!isfloatstring(yyvsp[0].string))
						yyval.intval = 0;
					      else
						yyval.intval = (int) floor(atof(yyvsp[0].string));
					      free(yyvsp[0].string);
					  }
					}
    break;
case 74:
#line 388 "keynote.y"
{ yyval.doubval = (yyvsp[-2].doubval + yyvsp[0].doubval); }
    break;
case 75:
#line 389 "keynote.y"
{ yyval.doubval = (yyvsp[-2].doubval - yyvsp[0].doubval); }
    break;
case 76:
#line 390 "keynote.y"
{ yyval.doubval = (yyvsp[-2].doubval * yyvsp[0].doubval); }
    break;
case 77:
#line 391 "keynote.y"
{ if (yyvsp[0].doubval == 0)
	                                  {
					      if (!keynote_donteval)
						keynote_exceptionflag = 1;
					  }
	                                  else
			        	   yyval.doubval = (yyvsp[-2].doubval / yyvsp[0].doubval);
					}
    break;
case 78:
#line 399 "keynote.y"
{ if (!keynote_exceptionflag &&
						      !keynote_donteval)
	                                            yyval.doubval = pow(yyvsp[-2].doubval, yyvsp[0].doubval);
	                                        }
    break;
case 79:
#line 403 "keynote.y"
{ yyval.doubval = -(yyvsp[0].doubval); }
    break;
case 80:
#line 404 "keynote.y"
{ yyval.doubval = yyvsp[-1].doubval; }
    break;
case 81:
#line 405 "keynote.y"
{ yyval.doubval = yyvsp[0].doubval; }
    break;
case 82:
#line 406 "keynote.y"
{
	                                  if (keynote_exceptionflag ||
					      keynote_donteval)
					    yyval.doubval = 0.0;
					  else
					  {
					      keynote_lex_remove(yyvsp[0].string);
					  
					      if (!isfloatstring(yyvsp[0].string))
						yyval.doubval = 0.0;
					      else
						yyval.doubval = atof(yyvsp[0].string);
					      free(yyvsp[0].string);
					  }
	                                }
    break;
case 83:
#line 422 "keynote.y"
{
                        if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) == 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 84:
#line 434 "keynote.y"
{
	                if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) != 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 85:
#line 446 "keynote.y"
{
	                if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) < 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 86:
#line 458 "keynote.y"
{
	                if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) > 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 87:
#line 470 "keynote.y"
{
	                if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) <= 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 88:
#line 482 "keynote.y"
{
	                if (keynote_exceptionflag || keynote_donteval)
			  yyval.bool = 0;
			else
			{
			    yyval.bool = strcmp(yyvsp[-2].string, yyvsp[0].string) >= 0 ? 1 : 0;
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			}
		      }
    break;
case 89:
#line 495 "keynote.y"
{
	      regmatch_t pmatch[32];
	      char grp[4], *gr;
	      regex_t preg;
	      int i;

	      if (keynote_exceptionflag || keynote_donteval)
		yyval.bool = 0;
	      else
	      {
		  keynote_lex_remove(yyvsp[-2].string);
		  keynote_lex_remove(yyvsp[0].string);

		  memset(pmatch, 0, sizeof(pmatch));
		  memset(grp, 0, sizeof(grp));

#if HAVE_REGCOMP
		  if (regcomp(&preg, yyvsp[0].string, REG_EXTENDED))
		  {
#else /* HAVE_REGCOMP */
#error "This system does not have regcomp()."
#endif /* HAVE_REGCOMP */
		      free(yyvsp[-2].string);
		      free(yyvsp[0].string);
		      keynote_exceptionflag = 1;
		  }
		  else
		  {
		      /* Clean-up residuals from previous regexps */
		      keynote_env_cleanup(&keynote_temp_list, 1);

		      free(yyvsp[0].string);
		      i = regexec(&preg, yyvsp[-2].string, 32, pmatch, 0);
		      yyval.bool = (i == 0 ? 1 : 0);
		      if (i == 0)
		      {
#if !defined(HAVE_SNPRINTF)
			  sprintf(grp, "%d", preg.re_nsub);
#else /* !HAVE_SNPRINTF */
			  snprintf(grp, 3, "%d", preg.re_nsub);
#endif /* !HAVE_SNPRINTF */
			  if (keynote_env_add("_0", grp, &keynote_temp_list,
					      1, 0) != RESULT_TRUE)
			  {
			      free(yyvsp[-2].string);
			      regfree(&preg);
			      return -1;
			  }

			  for (i = 1; i < 32 && pmatch[i].rm_so != -1; i++)
			  {
			      gr = calloc(pmatch[i].rm_eo - pmatch[i].rm_so +
					  1, sizeof(char));
			      if (gr == (char *) NULL)
			      {
				  free(yyvsp[-2].string);
				  regfree(&preg);
				  keynote_errno = ERROR_MEMORY;
				  return -1;
			      }

			      strncpy(gr, yyvsp[-2].string + pmatch[i].rm_so,
				      pmatch[i].rm_eo - pmatch[i].rm_so);
			      gr[pmatch[i].rm_eo - pmatch[i].rm_so] = '\0';
#if !defined(HAVE_SNPRINTF)
			      sprintf(grp, "_%d", i);
#else /* !HAVE_SNPRINTF */
			      snprintf(grp, 3, "_%d", i);
#endif /* !HAVE_SNPRINTF */
			      if (keynote_env_add(grp, gr, &keynote_temp_list,
						  1, 0) == -1)
			      {
				  free(yyvsp[-2].string);
				  regfree(&preg);
				  free(gr);
				  return -1;
			      }
			      else
				free(gr);
			  }
		      }

		      regfree(&preg);
		      free(yyvsp[-2].string);
		  }
	      }
	    }
    break;
case 90:
#line 583 "keynote.y"
{  if (keynote_exceptionflag || keynote_donteval)
			  yyval.string = (char *) NULL;
			else
			{
			    yyval.string = calloc(strlen(yyvsp[-2].string) + strlen(yyvsp[0].string) + 1,
					sizeof(char));
			    keynote_lex_remove(yyvsp[-2].string);
			    keynote_lex_remove(yyvsp[0].string);
			    if (yyval.string == (char *) NULL)
			    {
				free(yyvsp[-2].string);
				free(yyvsp[0].string);
				keynote_errno = ERROR_MEMORY;
				return -1;
			    }
 
			    strcpy(yyval.string, yyvsp[-2].string);
			    strcpy(yyval.string + strlen(yyvsp[-2].string), yyvsp[0].string);
			    free(yyvsp[-2].string);
			    free(yyvsp[0].string);
			    if (keynote_lex_add(yyval.string, LEXTYPE_CHAR) == -1)
			      return -1;
			}
		      }
    break;
case 91:
#line 607 "keynote.y"
{ yyval.string = yyvsp[0].string; }
    break;
case 92:
#line 609 "keynote.y"
{ yyval.string = yyvsp[0].string; }
    break;
case 93:
#line 610 "keynote.y"
{ yyval.string = yyvsp[-1].string; }
    break;
case 94:
#line 611 "keynote.y"
{  if (keynote_exceptionflag || keynote_donteval)
	                     yyval.string = (char *) NULL;
 	                   else
			   {
			       yyval.string = my_lookup(yyvsp[0].string);
			       keynote_lex_remove(yyvsp[0].string);
			       free(yyvsp[0].string);
			       if (yyval.string == (char *) NULL)
			       {
				   if (keynote_errno)
				     return -1;
				   yyval.string = strdup("");
			       }
			       else
				 yyval.string = strdup(yyval.string);

			       if (yyval.string == (char *) NULL)
			       {
				   keynote_errno = ERROR_MEMORY;
				   return -1;
			       }

			       if (keynote_lex_add(yyval.string, LEXTYPE_CHAR) == -1)
				 return -1;
			   }
	                 }
    break;
case 95:
#line 637 "keynote.y"
{  if (keynote_exceptionflag || keynote_donteval)
			      yyval.string = (char *) NULL;
			    else
			    {
				yyval.string = my_lookup(yyvsp[0].string);
				keynote_lex_remove(yyvsp[0].string);
				free(yyvsp[0].string);
				if (yyval.string == (char *) NULL)
				{
				    if (keynote_errno)
				      return -1;
				    yyval.string = strdup("");
				}
				else
				  yyval.string = strdup(yyval.string);

				if (yyval.string == (char *) NULL)
				{
				    keynote_errno = ERROR_MEMORY;
				    return -1;
				}

				if (keynote_lex_add(yyval.string, LEXTYPE_CHAR) == -1)
				  return -1;
			    }
			 }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 663 "keynote.y"


/*
 * Find all assertions signed by s and give us the one with the highest
 * return value.
 */
static int
resolve_assertion(char *s)
{
    int i, alg = KEYNOTE_ALGORITHM_NONE, p = 0;
    void *key = (void *) s;
    struct assertion *as;
    struct keylist *kl;

    kl = keynote_keylist_find(keynote_current_assertion->as_keylist, s);
    if (kl != (struct keylist *) NULL)
    {
	alg = kl->key_alg;
	key = kl->key_key;
    }

    for (i = 0;; i++)
    {
	as = keynote_find_assertion(key, i, alg);
	if (as == (struct assertion *) NULL)  /* Gone through all of them */
	  return p;

	if (as->as_kresult == KRESULT_DONE)
	  if (p < as->as_result)
	    p = as->as_result;

	/* Short circuit if we find an assertion with maximum return value */
	if (p == (keynote_current_session->ks_values_num - 1))
	  return p;
    }

    return 0;
}

/* 
 * Environment variable lookup. 
 */
static char *
my_lookup(char *s)
{
    struct keynote_session *ks = keynote_current_session;
    char *ret;

    if (!strcmp(s, "_MIN_TRUST"))
    {
	keynote_used_variable = 1;
	return ks->ks_values[0];
    }
    else
    {
	if (!strcmp(s, "_MAX_TRUST"))
	{
	    keynote_used_variable = 1;
	    return ks->ks_values[ks->ks_values_num - 1];
	}
	else
	{
	    if (!strcmp(s, "_VALUES"))
	    {
		keynote_used_variable = 1;
		return keynote_env_lookup("_VALUES", ks->ks_env_table,
					  HASHTABLESIZE);
	    }
	    else
	    {
		if (!strcmp(s, "_ACTION_AUTHORIZERS"))
		{
		    keynote_used_variable = 1;
		    return keynote_env_lookup("_ACTION_AUTHORIZERS",
					      ks->ks_env_table, HASHTABLESIZE);
		}
	    }
	}
    }

    /* Temporary list (regexp results) */
    ret = keynote_env_lookup(s, &keynote_temp_list, 1);
    if (ret != (char *) NULL)
      return ret;
    else
      if (keynote_errno != 0)
	return (char *) NULL;

    /* Local-Constants */
    ret = keynote_env_lookup(s, &keynote_init_list, 1);
    if (ret != (char *) NULL)
      return ret;
    else
      if (keynote_errno != 0)
	return (char *) NULL;

    keynote_used_variable = 1;

    /* Action environment */
    ret = keynote_env_lookup(s, ks->ks_env_table, HASHTABLESIZE);
    if (ret != (char *) NULL)
      return ret;
    else
      if (keynote_errno != 0)
	return (char *) NULL;

    /* Regex table */
    return keynote_env_lookup(s, &(ks->ks_env_regex), 1);
}

/*
 * If we had an exception, the boolean expression should return false.
 * Otherwise, return the result of the expression (the argument).
 */
static int
checkexception(int i)
{
    if (keynote_exceptionflag)
    {
	keynote_exceptionflag = 0;
	return 0;
    }
    else
      return i;
}


/* 
 * Integer exponentation -- copied from Schneier's AC2, page 244. 
 */
static int
intpow(int x, int y)
{
    int s = 1;
    
    /* 
     * x^y with y < 0 is equivalent to 1/(x^y), which for
     * integer arithmetic is 0.
     */
    if (y < 0)
      return 0;

    while (y)
    {
	if (y & 1)
	  s *= x;
	
	y >>= 1;
	x *= x;
    }

    return s;
}

/* 
 * Check whether the string is a floating point number. 
 */
static int
isfloatstring(char *s)
{
    int i, point = 0;
    
    for (i = strlen(s) - 1; i >= 0; i--)
      if (!isdigit((int) s[i]))
      {
	  if (s[i] == '.')
	  {
	      if (point == 1)
	        return 0;
	      else
	        point = 1;
	  }
	  else
	    return 0;
      }

    return 1;
}

/*
 * Initialize array for threshold search.
 */
static int
keynote_init_kth(void)
{
    int i = keynote_current_session->ks_values_num;
    
    if (i == -1)
      return -1;
    
    keynote_kth_array = (int *) calloc(i, sizeof(int));
    if (keynote_kth_array == (int *) NULL)
    {
	keynote_errno = ERROR_MEMORY;
	return -1;
    }

    return RESULT_TRUE;
}

/*
 * Get the k-th best return value.
 */
static int
get_kth(int k)
{
    int i;

    for (i = keynote_current_session->ks_values_num - 1; i >= 0; i--)
    {
	k -= keynote_kth_array[i];
	
	if (k <= 0)
	  return i;
    }

    return 0;
}

/*
 * Cleanup array.
 */
void
keynote_cleanup_kth(void)
{
    if (keynote_kth_array != (int *) NULL)
    {
	free(keynote_kth_array);
	keynote_kth_array = (int *) NULL;
    }
}

void
knerror(char *s)
{}
