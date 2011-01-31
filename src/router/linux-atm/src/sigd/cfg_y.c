/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "cfg_y.y"

/* cfg.y - configuration language */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#include "atm.h"
#include "atmd.h"

#include "proto.h"
#include "io.h"
#include "trace.h"
#include "policy.h"


static RULE *rule;
static SIG_ENTITY *curr_sig = &_entity;

void yyerror(const char *s);
void yywarn(const char *s);
int yylex(void);

static int hex2num(char digit)
{
    if (isdigit(digit)) return digit-'0';
    if (islower(digit)) return toupper(digit)-'A'+10;
    return digit-'A'+10;
}


static void put_address(char *address)
{
    char *mask;

    mask = strchr(address,'/');
    if (mask) *mask++ = 0;
    if (text2atm(address,(struct sockaddr *) &rule->addr,sizeof(rule->addr),
      T2A_SVC | T2A_WILDCARD | T2A_NAME | T2A_LOCAL) < 0) {
	yyerror("invalid address");
	return;
    }
    if (!mask) rule->mask = -1;
    else rule->mask = strtol(mask,NULL,10);
    add_rule(rule);
}



/* Line 189 of yacc.c  */
#line 130 "cfg_y.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_LEVEL = 258,
     TOK_DEBUG = 259,
     TOK_INFO = 260,
     TOK_WARN = 261,
     TOK_ERROR = 262,
     TOK_FATAL = 263,
     TOK_SIG = 264,
     TOK_UNI30 = 265,
     TOK_UNI31 = 266,
     TOK_UNI40 = 267,
     TOK_Q2963_1 = 268,
     TOK_SAAL = 269,
     TOK_VC = 270,
     TOK_IO = 271,
     TOK_MODE = 272,
     TOK_USER = 273,
     TOK_NET = 274,
     TOK_SWITCH = 275,
     TOK_VPCI = 276,
     TOK_ITF = 277,
     TOK_PCR = 278,
     TOK_TRACE = 279,
     TOK_POLICY = 280,
     TOK_ALLOW = 281,
     TOK_REJECT = 282,
     TOK_ENTITY = 283,
     TOK_DEFAULT = 284,
     TOK_NUMBER = 285,
     TOK_MAX_RATE = 286,
     TOK_DUMP_DIR = 287,
     TOK_LOGFILE = 288,
     TOK_QOS = 289,
     TOK_FROM = 290,
     TOK_TO = 291,
     TOK_ROUTE = 292,
     TOK_PVC = 293
   };
#endif
/* Tokens.  */
#define TOK_LEVEL 258
#define TOK_DEBUG 259
#define TOK_INFO 260
#define TOK_WARN 261
#define TOK_ERROR 262
#define TOK_FATAL 263
#define TOK_SIG 264
#define TOK_UNI30 265
#define TOK_UNI31 266
#define TOK_UNI40 267
#define TOK_Q2963_1 268
#define TOK_SAAL 269
#define TOK_VC 270
#define TOK_IO 271
#define TOK_MODE 272
#define TOK_USER 273
#define TOK_NET 274
#define TOK_SWITCH 275
#define TOK_VPCI 276
#define TOK_ITF 277
#define TOK_PCR 278
#define TOK_TRACE 279
#define TOK_POLICY 280
#define TOK_ALLOW 281
#define TOK_REJECT 282
#define TOK_ENTITY 283
#define TOK_DEFAULT 284
#define TOK_NUMBER 285
#define TOK_MAX_RATE 286
#define TOK_DUMP_DIR 287
#define TOK_LOGFILE 288
#define TOK_QOS 289
#define TOK_FROM 290
#define TOK_TO 291
#define TOK_ROUTE 292
#define TOK_PVC 293




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 58 "cfg_y.y"

    int num;
    char *str;
    struct sockaddr_atmpvc pvc;



/* Line 214 of yacc.c  */
#line 250 "cfg_y.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 262 "cfg_y.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   108

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  31
/* YYNRULES -- Number of rules.  */
#define YYNRULES  79
/* YYNRULES -- Number of states.  */
#define YYNSTATES  117

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   293

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
       2,     2,     2,    39,     2,    40,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     6,     7,    10,    11,    14,    17,    20,
      23,    26,    29,    32,    33,    38,    39,    43,    44,    47,
      52,    55,    57,    59,    61,    63,    65,    69,    70,    73,
      75,    79,    80,    83,    85,    89,    90,    93,    95,    99,
     100,   103,   105,   109,   110,   113,   116,   121,   123,   125,
     127,   129,   131,   134,   137,   140,   143,   146,   148,   150,
     153,   155,   157,   160,   161,   163,   165,   167,   169,   171,
     173,   175,   177,   179,   182,   183,   187,   189,   191,   193
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      42,     0,    -1,    43,    44,    -1,    -1,    45,    43,    -1,
      -1,    46,    44,    -1,     3,    66,    -1,     9,    51,    -1,
      14,    53,    -1,    16,    55,    -1,     4,    57,    -1,    25,
      59,    -1,    -1,    28,    38,    47,    48,    -1,    -1,    39,
      49,    40,    -1,    -1,    50,    49,    -1,    21,    30,    22,
      30,    -1,    17,    67,    -1,    34,    -1,    31,    -1,    37,
      -1,    29,    -1,    61,    -1,    39,    52,    40,    -1,    -1,
      61,    52,    -1,    62,    -1,    39,    54,    40,    -1,    -1,
      62,    54,    -1,    63,    -1,    39,    56,    40,    -1,    -1,
      63,    56,    -1,    64,    -1,    39,    58,    40,    -1,    -1,
      64,    58,    -1,    68,    -1,    39,    60,    40,    -1,    -1,
      68,    60,    -1,     3,    66,    -1,    21,    30,    22,    30,
      -1,    10,    -1,    11,    -1,    12,    -1,    13,    -1,    19,
      -1,    17,    67,    -1,     3,    66,    -1,     3,    66,    -1,
      15,    38,    -1,    23,    30,    -1,    34,    -1,    31,    -1,
       3,    66,    -1,    32,    -1,    33,    -1,    24,    65,    -1,
      -1,    30,    -1,     4,    -1,     5,    -1,     6,    -1,     7,
      -1,     8,    -1,    18,    -1,    19,    -1,    20,    -1,     3,
      66,    -1,    -1,    70,    69,    71,    -1,    26,    -1,    27,
      -1,    35,    -1,    36,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    79,    79,    82,    83,    86,    87,   111,   115,   116,
     117,   118,   119,   124,   123,   145,   146,   149,   150,   154,
     158,   159,   163,   167,   181,   188,   189,   192,   193,   197,
     198,   201,   202,   206,   207,   210,   211,   215,   216,   219,
     220,   224,   225,   228,   229,   233,   239,   243,   252,   261,
     270,   279,   284,   288,   296,   300,   304,   309,   313,   320,
     324,   329,   333,   340,   343,   350,   354,   358,   362,   366,
     373,   377,   381,   388,   393,   392,   401,   405,   412,   417
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_LEVEL", "TOK_DEBUG", "TOK_INFO",
  "TOK_WARN", "TOK_ERROR", "TOK_FATAL", "TOK_SIG", "TOK_UNI30",
  "TOK_UNI31", "TOK_UNI40", "TOK_Q2963_1", "TOK_SAAL", "TOK_VC", "TOK_IO",
  "TOK_MODE", "TOK_USER", "TOK_NET", "TOK_SWITCH", "TOK_VPCI", "TOK_ITF",
  "TOK_PCR", "TOK_TRACE", "TOK_POLICY", "TOK_ALLOW", "TOK_REJECT",
  "TOK_ENTITY", "TOK_DEFAULT", "TOK_NUMBER", "TOK_MAX_RATE",
  "TOK_DUMP_DIR", "TOK_LOGFILE", "TOK_QOS", "TOK_FROM", "TOK_TO",
  "TOK_ROUTE", "TOK_PVC", "'{'", "'}'", "$accept", "all", "global",
  "local", "item", "entity", "$@1", "opt_options", "options", "option",
  "sig", "sig_items", "saal", "saal_items", "io", "io_items", "debug",
  "debug_items", "policy", "policy_items", "sig_item", "saal_item",
  "io_item", "debug_item", "opt_trace_size", "level", "mode",
  "policy_item", "$@2", "action", "direction", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   123,
     125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    43,    43,    44,    44,    45,    45,    45,
      45,    45,    45,    47,    46,    48,    48,    49,    49,    50,
      50,    50,    50,    50,    50,    51,    51,    52,    52,    53,
      53,    54,    54,    55,    55,    56,    56,    57,    57,    58,
      58,    59,    59,    60,    60,    61,    61,    61,    61,    61,
      61,    61,    61,    62,    63,    63,    63,    63,    63,    64,
      64,    64,    64,    65,    65,    66,    66,    66,    66,    66,
      67,    67,    67,    68,    69,    68,    70,    70,    71,    71
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     2,     0,     2,     2,     2,     2,
       2,     2,     2,     0,     4,     0,     3,     0,     2,     4,
       2,     1,     1,     1,     1,     1,     3,     0,     2,     1,
       3,     0,     2,     1,     3,     0,     2,     1,     3,     0,
       2,     1,     3,     0,     2,     2,     4,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     1,     1,     2,
       1,     1,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     3,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     0,     0,     0,     0,     0,     5,     3,
      65,    66,    67,    68,    69,     7,     0,    63,    60,    61,
      39,    11,    37,     0,    47,    48,    49,    50,     0,    51,
       0,    27,     8,    25,     0,    31,     9,    29,     0,     0,
       0,    58,    57,    35,    10,    33,     0,    76,    77,    43,
      12,    41,    74,     1,     0,     2,     5,     4,    59,    64,
      62,     0,    39,    45,    70,    71,    72,    52,     0,     0,
      27,    53,     0,    31,    54,    55,    56,     0,    35,    73,
       0,    43,     0,    13,     6,    38,    40,     0,    26,    28,
      30,    32,    34,    36,    42,    44,    78,    79,    75,    15,
      46,    17,    14,     0,     0,    24,    22,    21,    23,     0,
      17,    20,     0,    16,    18,     0,    19
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     7,     8,    55,     9,    56,    99,   102,   109,   110,
      32,    69,    36,    72,    44,    77,    21,    61,    50,    80,
      70,    73,    78,    62,    60,    15,    67,    81,    82,    52,
      98
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -31
static const yytype_int8 yypact[] =
{
      53,    69,     1,    -3,    -1,    -2,     0,    11,   -16,    53,
     -31,   -31,   -31,   -31,   -31,   -31,    69,   -15,   -31,   -31,
      28,   -31,   -31,    69,   -31,   -31,   -31,   -31,    46,   -31,
     -13,    80,   -31,   -31,    69,    16,   -31,   -31,    69,   -18,
      -6,   -31,   -31,    20,   -31,   -31,    69,   -31,   -31,    19,
     -31,   -31,   -31,   -31,   -10,   -31,   -16,   -31,   -31,   -31,
     -31,     2,    28,   -31,   -31,   -31,   -31,   -31,     8,     4,
      80,   -31,     7,    16,   -31,   -31,   -31,     9,    20,   -31,
      10,    19,   -30,   -31,   -31,   -31,   -31,    23,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,    31,
     -31,    51,   -31,    46,    49,   -31,   -31,   -31,   -31,    15,
      51,   -31,    36,   -31,   -31,    54,   -31
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -31,   -31,    72,    30,   -31,   -31,   -31,   -31,   -23,   -31,
     -31,    24,   -31,    22,   -31,    18,   -31,    27,   -31,    17,
      97,    98,    99,   101,   -31,    25,     3,   102,   -31,   -31,
     -31
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      23,    38,    34,    46,    16,    96,    97,    24,    25,    26,
      27,    53,    54,    39,    28,    59,    29,    68,    30,    34,
      75,    40,    46,    38,    76,    17,    47,    48,    83,    41,
      87,    16,    42,    18,    19,    39,    31,    43,    35,    49,
      20,    58,    85,    40,    88,    47,    48,    90,    63,    92,
      94,    41,    17,   100,    42,   113,     1,     2,   115,    71,
      18,    19,     3,    74,    64,    65,    66,     4,   103,     5,
     101,    79,   104,    10,    11,    12,    13,    14,     6,   112,
     105,    57,   106,    23,   116,   107,    84,   114,   108,    86,
      24,    25,    26,    27,    89,    91,    93,    28,    95,    29,
      33,    30,    37,    22,    45,     0,   111,     0,    51
};

static const yytype_int8 yycheck[] =
{
       3,     3,     3,     3,     3,    35,    36,    10,    11,    12,
      13,     0,    28,    15,    17,    30,    19,    30,    21,     3,
      38,    23,     3,     3,    30,    24,    26,    27,    38,    31,
      22,     3,    34,    32,    33,    15,    39,    39,    39,    39,
      39,    16,    40,    23,    40,    26,    27,    40,    23,    40,
      40,    31,    24,    30,    34,    40,     3,     4,    22,    34,
      32,    33,     9,    38,    18,    19,    20,    14,    17,    16,
      39,    46,    21,     4,     5,     6,     7,     8,    25,    30,
      29,     9,    31,     3,    30,    34,    56,   110,    37,    62,
      10,    11,    12,    13,    70,    73,    78,    17,    81,    19,
       3,    21,     4,     2,     5,    -1,   103,    -1,     6
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     9,    14,    16,    25,    42,    43,    45,
       4,     5,     6,     7,     8,    66,     3,    24,    32,    33,
      39,    57,    64,     3,    10,    11,    12,    13,    17,    19,
      21,    39,    51,    61,     3,    39,    53,    62,     3,    15,
      23,    31,    34,    39,    55,    63,     3,    26,    27,    39,
      59,    68,    70,     0,    28,    44,    46,    43,    66,    30,
      65,    58,    64,    66,    18,    19,    20,    67,    30,    52,
      61,    66,    54,    62,    66,    38,    30,    56,    63,    66,
      60,    68,    69,    38,    44,    40,    58,    22,    40,    52,
      40,    54,    40,    56,    40,    60,    35,    36,    71,    47,
      30,    39,    48,    17,    21,    29,    31,    34,    37,    49,
      50,    67,    30,    40,    49,    22,    30
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

/* Line 1464 of yacc.c  */
#line 88 "cfg_y.y"
    {
	    if (!curr_sig->uni)
		curr_sig->uni =
#if defined(UNI30) || defined(DYNAMIC_UNI)
		  S_UNI30
#endif
#ifdef UNI31
		  S_UNI31
#ifdef ALLOW_UNI30
		  | S_UNI30
#endif
#endif
#ifdef UNI40
		  S_UNI40
#ifdef Q2963_1
		  | S_Q2963_1
#endif
#endif
		  ;
	}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 112 "cfg_y.y"
    {
	    set_verbosity(NULL,(yyvsp[(2) - (2)].num));
	}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 124 "cfg_y.y"
    {
	    SIG_ENTITY *sig,**walk;

	    if (atmpvc_addr_in_use(_entity.signaling_pvc))
		yyerror("can't use  io vc  and  entity ...  in the same "
		  "configuration");
	    if (entities == &_entity) entities = NULL;
	    for (sig = entities; sig; sig = sig->next)
		if (atm_equal((struct sockaddr *) &sig->signaling_pvc,
		  (struct sockaddr *) &(yyvsp[(2) - (2)].pvc),0,0))
		    yyerror("duplicate PVC address");
	    curr_sig = alloc_t(SIG_ENTITY);
	    *curr_sig = _entity;
	    curr_sig->signaling_pvc = (yyvsp[(2) - (2)].pvc);
	    curr_sig->next = NULL;
	    for (walk = &entities; *walk; walk = &(*walk)->next);
	    *walk = curr_sig;
	}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 155 "cfg_y.y"
    {
	    enter_vpci(curr_sig,(yyvsp[(2) - (4)].num),(yyvsp[(4) - (4)].num));
	}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 160 "cfg_y.y"
    {
	    curr_sig->sig_qos = (yyvsp[(1) - (1)].str);
	}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 164 "cfg_y.y"
    {
	    curr_sig->max_rate = (yyvsp[(1) - (1)].num);
	}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 168 "cfg_y.y"
    {
	     struct sockaddr_atmsvc addr;
	     char *mask;

	    mask = strchr((yyvsp[(1) - (1)].str),'/');
	    if (mask) *mask++ = 0;
	    if (text2atm((yyvsp[(1) - (1)].str),(struct sockaddr *) &addr,sizeof(addr),
	    T2A_SVC | T2A_WILDCARD | T2A_NAME | T2A_LOCAL) < 0) {
		yyerror("invalid address");
		YYABORT;
	    }
	    add_route(curr_sig,&addr,mask ? strtol(mask,NULL,10) : INT_MAX);
	}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 182 "cfg_y.y"
    {
	    add_route(curr_sig,NULL,0);
	}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 234 "cfg_y.y"
    {
	    set_verbosity("UNI",(yyvsp[(2) - (2)].num));
	    set_verbosity("KERNEL",(yyvsp[(2) - (2)].num));
	    set_verbosity("SAP",(yyvsp[(2) - (2)].num));
	}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 240 "cfg_y.y"
    {
	    enter_vpci(curr_sig,(yyvsp[(2) - (4)].num),(yyvsp[(4) - (4)].num));
	}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 244 "cfg_y.y"
    {
#if defined(UNI30) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni & ~S_UNI31) yyerror("UNI mode is already set");
	    curr_sig->uni |= S_UNI30;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 253 "cfg_y.y"
    {
#if defined(UNI31) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni & ~S_UNI30) yyerror("UNI mode is already set");
	    curr_sig->uni |= S_UNI31;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 262 "cfg_y.y"
    {
#if defined(UNI40) || defined(DYNAMIC_UNI)
	    if (curr_sig->uni) yyerror("UNI mode is already set");
	    curr_sig->uni = S_UNI40;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 271 "cfg_y.y"
    {
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	    if (!(curr_sig->uni & S_UNI40)) yyerror("Incompatible UNI mode");
	    curr_sig->uni |= S_Q2963_1;
#else
	    yyerror("Sorry, not supported yet");
#endif
	}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 280 "cfg_y.y"
    {
	    yywarn("sig net  is obsolete, please use  sig mode net  instead");
	    curr_sig->mode = sm_net;
	}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 289 "cfg_y.y"
    {
	    set_verbosity("SSCF",(yyvsp[(2) - (2)].num));
	    set_verbosity("SSCOP",(yyvsp[(2) - (2)].num));
	}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 297 "cfg_y.y"
    {
	    set_verbosity("IO",(yyvsp[(2) - (2)].num));
	}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 301 "cfg_y.y"
    {
	    curr_sig->signaling_pvc = (yyvsp[(2) - (2)].pvc);
	}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 305 "cfg_y.y"
    {
	    yywarn("io pcr  is obsolete, please use  io qos  instead");
	    curr_sig->sig_pcr = (yyvsp[(2) - (2)].num);
	}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 310 "cfg_y.y"
    {
	    curr_sig->sig_qos = (yyvsp[(1) - (1)].str);
	}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 314 "cfg_y.y"
    {
	    curr_sig->max_rate = (yyvsp[(1) - (1)].num);
	}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 321 "cfg_y.y"
    {
	    set_verbosity(NULL,(yyvsp[(2) - (2)].num));
	}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 325 "cfg_y.y"
    {
	    dump_dir = (yyvsp[(1) - (1)].str);
	    if (!trace_size) trace_size = DEFAULT_TRACE_SIZE;
	}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 330 "cfg_y.y"
    {
	    set_logfile((yyvsp[(1) - (1)].str));
	}
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 334 "cfg_y.y"
    {
	    trace_size = (yyvsp[(2) - (2)].num);
	}
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 340 "cfg_y.y"
    {
	    (yyval.num) = DEFAULT_TRACE_SIZE;
	}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 344 "cfg_y.y"
    {
	    (yyval.num) = (yyvsp[(1) - (1)].num);
	}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 351 "cfg_y.y"
    {
	    (yyval.num) = DIAG_DEBUG;
	}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 355 "cfg_y.y"
    {
	    (yyval.num) = DIAG_INFO;
	}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 359 "cfg_y.y"
    {
	    (yyval.num) = DIAG_WARN;
	}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 363 "cfg_y.y"
    {
	    (yyval.num) = DIAG_ERROR;
	}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 367 "cfg_y.y"
    {
	    (yyval.num) = DIAG_FATAL;
	}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 374 "cfg_y.y"
    {
	    curr_sig->mode = sm_user;
	}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 378 "cfg_y.y"
    {
	    curr_sig->mode = sm_net;
	}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 382 "cfg_y.y"
    {
	    curr_sig->mode = sm_switch;
	}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 389 "cfg_y.y"
    {
	    set_verbosity("POLICY",(yyvsp[(2) - (2)].num));
	}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 393 "cfg_y.y"
    {
	    rule = alloc_t(RULE);
	    rule->type = (yyvsp[(1) - (1)].num);
	}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 402 "cfg_y.y"
    {
	    (yyval.num) = ACL_ALLOW;
	}
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 406 "cfg_y.y"
    {
	    (yyval.num) = ACL_REJECT;
	}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 413 "cfg_y.y"
    {
	    rule->type |= ACL_IN;
	    put_address((yyvsp[(1) - (1)].str));
	}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 418 "cfg_y.y"
    {
	    rule->type |= ACL_OUT;
	    put_address((yyvsp[(1) - (1)].str));
	}
    break;



/* Line 1464 of yacc.c  */
#line 2013 "cfg_y.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



