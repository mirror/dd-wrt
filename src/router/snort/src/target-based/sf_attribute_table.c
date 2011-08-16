
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

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

/* Substitute the variable and function names.  */
#define yyparse         sfat_parse
#define yylex           sfat_lex
#define yyerror         sfat_error
#define yylval          sfat_lval
#define yychar          sfat_char
#define yydebug         sfat_debug
#define yynerrs         sfat_nerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 32 "sf_attribute_table.y"

#ifdef TARGET_BASED
#include <stdlib.h>
#include <string.h>
#include "snort.h"
#include "util.h"
#include "sftarget_reader.h"
#include "log.h"
#include "debug.h"
#include "sf_types.h"

#define YYSTACK_USE_ALLOCA 0

/* define the initial stack-sizes */

#ifdef YYMAXDEPTH
#undef YYMAXDEPTH
#define YYMAXDEPTH  70000
#else
#define YYMAXDEPTH  70000
#endif

extern ServiceClient sfat_client_or_service;
extern char *sfat_grammar_error;

extern int sfat_lex();
extern void sfat_error(char*);


/* Line 189 of yacc.c  */
#line 111 "sf_attribute_table.c"

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
     SF_AT_COMMENT = 258,
     SF_AT_WHITESPACE = 259,
     SF_START_SNORT_ATTRIBUTES = 260,
     SF_END_SNORT_ATTRIBUTES = 261,
     SF_AT_START_MAP_TABLE = 262,
     SF_AT_END_MAP_TABLE = 263,
     SF_AT_START_ENTRY = 264,
     SF_AT_END_ENTRY = 265,
     SF_AT_START_ENTRY_ID = 266,
     SF_AT_END_ENTRY_ID = 267,
     SF_AT_START_ENTRY_VALUE = 268,
     SF_AT_END_ENTRY_VALUE = 269,
     SF_AT_START_ATTRIBUTE_TABLE = 270,
     SF_AT_END_ATTRIBUTE_TABLE = 271,
     SF_AT_START_HOST = 272,
     SF_AT_END_HOST = 273,
     SF_AT_START_HOST_IP = 274,
     SF_AT_END_HOST_IP = 275,
     SF_AT_STRING = 276,
     SF_AT_NUMERIC = 277,
     SF_AT_IPv6 = 278,
     SF_AT_IPv6Cidr = 279,
     SF_AT_START_OS = 280,
     SF_AT_END_OS = 281,
     SF_AT_START_ATTRIBUTE_VALUE = 282,
     SF_AT_END_ATTRIBUTE_VALUE = 283,
     SF_AT_START_ATTRIBUTE_ID = 284,
     SF_AT_END_ATTRIBUTE_ID = 285,
     SF_AT_START_CONFIDENCE = 286,
     SF_AT_END_CONFIDENCE = 287,
     SF_AT_START_NAME = 288,
     SF_AT_END_NAME = 289,
     SF_AT_START_VENDOR = 290,
     SF_AT_END_VENDOR = 291,
     SF_AT_START_VERSION = 292,
     SF_AT_END_VERSION = 293,
     SF_AT_START_FRAG_POLICY = 294,
     SF_AT_END_FRAG_POLICY = 295,
     SF_AT_START_STREAM_POLICY = 296,
     SF_AT_END_STREAM_POLICY = 297,
     SF_AT_START_SERVICES = 298,
     SF_AT_END_SERVICES = 299,
     SF_AT_START_SERVICE = 300,
     SF_AT_END_SERVICE = 301,
     SF_AT_START_CLIENTS = 302,
     SF_AT_END_CLIENTS = 303,
     SF_AT_START_CLIENT = 304,
     SF_AT_END_CLIENT = 305,
     SF_AT_START_IPPROTO = 306,
     SF_AT_END_IPPROTO = 307,
     SF_AT_START_PORT = 308,
     SF_AT_END_PORT = 309,
     SF_AT_START_PROTOCOL = 310,
     SF_AT_END_PROTOCOL = 311,
     SF_AT_START_APPLICATION = 312,
     SF_AT_END_APPLICATION = 313
   };
#endif
/* Tokens.  */
#define SF_AT_COMMENT 258
#define SF_AT_WHITESPACE 259
#define SF_START_SNORT_ATTRIBUTES 260
#define SF_END_SNORT_ATTRIBUTES 261
#define SF_AT_START_MAP_TABLE 262
#define SF_AT_END_MAP_TABLE 263
#define SF_AT_START_ENTRY 264
#define SF_AT_END_ENTRY 265
#define SF_AT_START_ENTRY_ID 266
#define SF_AT_END_ENTRY_ID 267
#define SF_AT_START_ENTRY_VALUE 268
#define SF_AT_END_ENTRY_VALUE 269
#define SF_AT_START_ATTRIBUTE_TABLE 270
#define SF_AT_END_ATTRIBUTE_TABLE 271
#define SF_AT_START_HOST 272
#define SF_AT_END_HOST 273
#define SF_AT_START_HOST_IP 274
#define SF_AT_END_HOST_IP 275
#define SF_AT_STRING 276
#define SF_AT_NUMERIC 277
#define SF_AT_IPv6 278
#define SF_AT_IPv6Cidr 279
#define SF_AT_START_OS 280
#define SF_AT_END_OS 281
#define SF_AT_START_ATTRIBUTE_VALUE 282
#define SF_AT_END_ATTRIBUTE_VALUE 283
#define SF_AT_START_ATTRIBUTE_ID 284
#define SF_AT_END_ATTRIBUTE_ID 285
#define SF_AT_START_CONFIDENCE 286
#define SF_AT_END_CONFIDENCE 287
#define SF_AT_START_NAME 288
#define SF_AT_END_NAME 289
#define SF_AT_START_VENDOR 290
#define SF_AT_END_VENDOR 291
#define SF_AT_START_VERSION 292
#define SF_AT_END_VERSION 293
#define SF_AT_START_FRAG_POLICY 294
#define SF_AT_END_FRAG_POLICY 295
#define SF_AT_START_STREAM_POLICY 296
#define SF_AT_END_STREAM_POLICY 297
#define SF_AT_START_SERVICES 298
#define SF_AT_END_SERVICES 299
#define SF_AT_START_SERVICE 300
#define SF_AT_END_SERVICE 301
#define SF_AT_START_CLIENTS 302
#define SF_AT_END_CLIENTS 303
#define SF_AT_START_CLIENT 304
#define SF_AT_END_CLIENT 305
#define SF_AT_START_IPPROTO 306
#define SF_AT_END_IPPROTO 307
#define SF_AT_START_PORT 308
#define SF_AT_END_PORT 309
#define SF_AT_START_PROTOCOL 310
#define SF_AT_END_PROTOCOL 311
#define SF_AT_START_APPLICATION 312
#define SF_AT_END_APPLICATION 313




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 62 "sf_attribute_table.y"

  char stringValue[STD_BUF];
  uint32_t numericValue;
  AttributeData data;
  MapData mapEntry;



/* Line 214 of yacc.c  */
#line 272 "sf_attribute_table.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 284 "sf_attribute_table.c"

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
# if YYENABLE_NLS
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
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   133

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  82
/* YYNRULES -- Number of states.  */
#define YYNSTATES  151

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   313

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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,    10,    14,    18,    19,    22,    26,
      28,    30,    33,    37,    41,    45,    46,    49,    53,    55,
      57,    62,    66,    70,    73,    77,    81,    83,    86,    88,
      90,    92,    94,    96,   100,   104,   108,   112,   116,   118,
     121,   124,   126,   129,   131,   135,   139,   143,   147,   151,
     153,   155,   156,   159,   163,   165,   167,   169,   172,   176,
     180,   184,   188,   192,   196,   200,   204,   208,   212,   217,
     221,   225,   227,   229,   230,   233,   237,   239,   241,   243,
     246,   248,   251
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      60,     0,    -1,    61,    -1,     5,    62,    70,     6,    -1,
       5,    70,     6,    -1,     7,    63,     8,    -1,    -1,    64,
      63,    -1,    65,    67,    66,    -1,     9,    -1,    10,    -1,
      69,    68,    -1,    13,    21,    14,    -1,    11,    22,    12,
      -1,    15,    71,    16,    -1,    -1,    71,    72,    -1,    73,
      75,    74,    -1,    17,    -1,    18,    -1,    76,    77,    90,
     104,    -1,    76,    77,   104,    -1,    76,    77,    90,    -1,
      76,    77,    -1,    19,    21,    20,    -1,    25,    78,    26,
      -1,    79,    -1,    78,    79,    -1,    80,    -1,    81,    -1,
      82,    -1,    84,    -1,    83,    -1,    33,    85,    34,    -1,
      35,    85,    36,    -1,    37,    85,    38,    -1,    39,    21,
      40,    -1,    41,    21,    42,    -1,    86,    -1,    86,    89,
      -1,    87,    89,    -1,    87,    -1,    88,    89,    -1,    88,
      -1,    27,    21,    28,    -1,    27,    22,    28,    -1,    29,
      22,    30,    -1,    31,    22,    32,    -1,    91,    93,    92,
      -1,    43,    -1,    44,    -1,    -1,    94,    93,    -1,    95,
      97,    96,    -1,    45,    -1,    46,    -1,    98,    -1,    98,
     102,    -1,    99,   100,   101,    -1,    99,   101,   100,    -1,
     100,    99,   101,    -1,   100,   101,    99,    -1,   101,   100,
      99,    -1,   101,    99,   100,    -1,    51,    85,    52,    -1,
      55,    85,    56,    -1,    53,    85,    54,    -1,    57,    85,
      58,    -1,    57,    85,   103,    58,    -1,    37,    85,    38,
      -1,   105,   107,   106,    -1,    47,    -1,    48,    -1,    -1,
     108,   107,    -1,   109,   111,   110,    -1,    49,    -1,    50,
      -1,   112,    -1,   112,   102,    -1,   100,    -1,    99,   100,
      -1,   100,    99,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   147,   147,   153,   158,   165,   171,   174,   177,   185,
     188,   191,   198,   205,   213,   219,   222,   225,   235,   242,
     245,   250,   255,   260,   267,   282,   284,   284,   286,   286,
     286,   286,   286,   289,   297,   305,   313,   321,   329,   335,
     341,   347,   353,   373,   395,   401,   407,   414,   421,   427,
     434,   440,   443,   449,   457,   464,   470,   474,   480,   485,
     490,   495,   500,   505,   512,   520,   528,   536,   543,   552,
     560,   566,   573,   579,   582,   588,   596,   603,   609,   613,
     619,   624,   629
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SF_AT_COMMENT", "SF_AT_WHITESPACE",
  "SF_START_SNORT_ATTRIBUTES", "SF_END_SNORT_ATTRIBUTES",
  "SF_AT_START_MAP_TABLE", "SF_AT_END_MAP_TABLE", "SF_AT_START_ENTRY",
  "SF_AT_END_ENTRY", "SF_AT_START_ENTRY_ID", "SF_AT_END_ENTRY_ID",
  "SF_AT_START_ENTRY_VALUE", "SF_AT_END_ENTRY_VALUE",
  "SF_AT_START_ATTRIBUTE_TABLE", "SF_AT_END_ATTRIBUTE_TABLE",
  "SF_AT_START_HOST", "SF_AT_END_HOST", "SF_AT_START_HOST_IP",
  "SF_AT_END_HOST_IP", "SF_AT_STRING", "SF_AT_NUMERIC", "SF_AT_IPv6",
  "SF_AT_IPv6Cidr", "SF_AT_START_OS", "SF_AT_END_OS",
  "SF_AT_START_ATTRIBUTE_VALUE", "SF_AT_END_ATTRIBUTE_VALUE",
  "SF_AT_START_ATTRIBUTE_ID", "SF_AT_END_ATTRIBUTE_ID",
  "SF_AT_START_CONFIDENCE", "SF_AT_END_CONFIDENCE", "SF_AT_START_NAME",
  "SF_AT_END_NAME", "SF_AT_START_VENDOR", "SF_AT_END_VENDOR",
  "SF_AT_START_VERSION", "SF_AT_END_VERSION", "SF_AT_START_FRAG_POLICY",
  "SF_AT_END_FRAG_POLICY", "SF_AT_START_STREAM_POLICY",
  "SF_AT_END_STREAM_POLICY", "SF_AT_START_SERVICES", "SF_AT_END_SERVICES",
  "SF_AT_START_SERVICE", "SF_AT_END_SERVICE", "SF_AT_START_CLIENTS",
  "SF_AT_END_CLIENTS", "SF_AT_START_CLIENT", "SF_AT_END_CLIENT",
  "SF_AT_START_IPPROTO", "SF_AT_END_IPPROTO", "SF_AT_START_PORT",
  "SF_AT_END_PORT", "SF_AT_START_PROTOCOL", "SF_AT_END_PROTOCOL",
  "SF_AT_START_APPLICATION", "SF_AT_END_APPLICATION", "$accept",
  "AttributeGrammar", "SnortAttributes", "MappingTable",
  "ListOfMapEntries", "MapEntry", "MapEntryStart", "MapEntryEnd",
  "MapEntryData", "MapValue", "MapId", "AttributeTable", "ListOfHosts",
  "HostEntry", "HostEntryStart", "HostEntryEnd", "HostEntryData", "IpCidr",
  "HostOS", "OSAttributes", "OSAttribute", "OSName", "OSVendor",
  "OSVersion", "OSFragPolicy", "OSStreamPolicy", "AttributeInfo",
  "AttributeValueString", "AttributeValueNumber", "AttributeId",
  "AttributeConfidence", "ServiceList", "ServiceListStart",
  "ServiceListEnd", "ServiceListData", "Service", "ServiceStart",
  "ServiceEnd", "ServiceData", "ServiceDataRequired", "IPProtocol",
  "Protocol", "Port", "Application", "Version", "ClientList",
  "ClientListStart", "ClientListEnd", "ClientListData", "Client",
  "ClientStart", "ClientEnd", "ClientData", "ClientDataRequired", 0
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    59,    60,    61,    61,    62,    63,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    71,    72,    73,    74,
      75,    75,    75,    75,    76,    77,    78,    78,    79,    79,
      79,    79,    79,    80,    81,    82,    83,    84,    85,    85,
      85,    85,    85,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    93,    94,    95,    96,    97,    97,    98,    98,
      98,    98,    98,    98,    99,   100,   101,   102,   102,   103,
     104,   105,   106,   107,   107,   108,   109,   110,   111,   111,
     112,   112,   112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     4,     3,     3,     0,     2,     3,     1,
       1,     2,     3,     3,     3,     0,     2,     3,     1,     1,
       4,     3,     3,     2,     3,     3,     1,     2,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     1,     2,
       2,     1,     2,     1,     3,     3,     3,     3,     3,     1,
       1,     0,     2,     3,     1,     1,     1,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     3,
       3,     1,     1,     0,     2,     3,     1,     1,     1,     2,
       1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     2,     6,    15,     0,     0,     1,     9,
       0,     6,     0,     0,     0,     4,     5,     7,     0,     0,
       0,    14,    18,    16,     0,     3,     0,    10,     8,     0,
      11,     0,     0,     0,    13,     0,     0,    19,    17,     0,
      23,    12,    24,     0,     0,     0,     0,     0,     0,    26,
      28,    29,    30,    32,    31,    49,    71,    22,    51,    21,
      73,     0,     0,     0,    38,    41,    43,     0,     0,     0,
       0,    25,    27,    20,    54,     0,    51,     0,    76,     0,
      73,     0,     0,     0,     0,    33,     0,    39,    40,    42,
      34,    35,    36,    37,    50,    48,    52,     0,     0,     0,
       0,    56,     0,     0,     0,    72,    70,    74,     0,    80,
       0,    78,    44,    45,    46,     0,     0,     0,     0,    55,
      53,     0,    57,     0,     0,     0,     0,     0,     0,    81,
      82,    77,    75,    79,    47,    64,    66,    65,     0,    58,
      59,    60,    61,    63,    62,     0,    67,     0,     0,    68,
      69
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     6,    10,    11,    12,    28,    19,    30,
      20,     7,    13,    23,    24,    38,    32,    33,    40,    48,
      49,    50,    51,    52,    53,    54,    63,    64,    65,    66,
      87,    57,    58,    95,    75,    76,    77,   120,   100,   101,
     102,   103,   104,   122,   147,    59,    60,   106,    79,    80,
      81,   132,   110,   111
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -97
static const yytype_int8 yypact[] =
{
      -2,    26,    22,   -97,    28,   -97,    27,    54,   -97,   -97,
      38,    28,    50,    40,    56,   -97,   -97,   -97,    41,    55,
      51,   -97,   -97,   -97,    47,   -97,    57,   -97,   -97,    46,
     -97,    49,    53,    43,   -97,    58,    59,   -97,   -97,   -21,
      -8,   -97,   -97,    -6,    -6,    -6,    52,    60,   -22,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,    29,    30,   -97,
      25,   -13,    61,    44,    62,    62,    62,    48,    42,    45,
      63,   -97,   -97,   -97,   -97,    64,    30,   -17,   -97,    34,
      25,   -11,    66,    67,    68,   -97,    65,   -97,   -97,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,    -6,    -6,    -6,
      69,    31,   -10,    -1,   -11,   -97,   -97,   -97,    35,    70,
      36,    31,   -97,   -97,   -97,    71,    37,    72,    73,   -97,
     -97,    -6,   -97,    39,    35,    39,    70,    35,    70,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -27,   -97,
     -97,   -97,   -97,   -97,   -97,    -6,   -97,    33,    74,   -97,
     -97
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -97,   -97,   -97,   -97,    85,   -97,   -97,   -97,   -97,   -97,
     -97,    91,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,
      75,   -97,   -97,   -97,   -97,   -97,   -44,   -97,   -97,   -97,
      -7,   -97,   -97,   -97,    23,   -97,   -97,   -97,   -97,   -97,
     -79,   -76,   -96,    -9,   -97,    76,   -97,   -97,    20,   -97,
     -97,   -97,   -97,   -97
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      67,    68,   108,     1,    71,   109,   124,   126,    82,    83,
     145,    43,    43,    44,    44,    45,    45,    46,    46,    47,
      47,    61,     8,    62,   125,   127,   123,   139,   128,   141,
     130,   146,   129,     4,    97,    55,    98,     9,    99,    56,
      97,     5,     5,    98,    99,    99,    16,   142,   140,   144,
      97,   143,    98,   116,   117,   118,    21,    22,    88,    89,
      15,    18,    25,    26,    29,    27,    31,    35,    39,    34,
      36,    37,    41,    69,    78,    74,    56,   138,    85,    42,
      91,    70,   105,    84,    90,    92,   131,   115,   121,   135,
      99,   149,    98,    86,   112,   113,    17,    14,   114,    96,
     107,   148,   133,   134,     0,    93,     0,     0,    94,     0,
       0,     0,   150,     0,     0,   119,     0,     0,     0,     0,
       0,    97,     0,    72,     0,     0,   136,     0,     0,   137,
       0,     0,     0,    73
};

static const yytype_int16 yycheck[] =
{
      44,    45,    81,     5,    26,    81,   102,   103,    21,    22,
      37,    33,    33,    35,    35,    37,    37,    39,    39,    41,
      41,    27,     0,    29,   103,   104,   102,   123,   104,   125,
     109,    58,   108,     7,    51,    43,    53,     9,    55,    47,
      51,    15,    15,    53,    55,    55,     8,   126,   124,   128,
      51,   127,    53,    97,    98,    99,    16,    17,    65,    66,
       6,    11,     6,    22,    13,    10,    19,    21,    25,    12,
      21,    18,    14,    21,    49,    45,    47,   121,    34,    20,
      38,    21,    48,    22,    36,    40,    50,    22,    57,    52,
      55,    58,    53,    31,    28,    28,    11,     6,    30,    76,
      80,   145,   111,    32,    -1,    42,    -1,    -1,    44,    -1,
      -1,    -1,    38,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    48,    -1,    -1,    54,    -1,    -1,    56,
      -1,    -1,    -1,    57
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,    60,    61,     7,    15,    62,    70,     0,     9,
      63,    64,    65,    71,    70,     6,     8,    63,    11,    67,
      69,    16,    17,    72,    73,     6,    22,    10,    66,    13,
      68,    19,    75,    76,    12,    21,    21,    18,    74,    25,
      77,    14,    20,    33,    35,    37,    39,    41,    78,    79,
      80,    81,    82,    83,    84,    43,    47,    90,    91,   104,
     105,    27,    29,    85,    86,    87,    88,    85,    85,    21,
      21,    26,    79,   104,    45,    93,    94,    95,    49,   107,
     108,   109,    21,    22,    22,    34,    31,    89,    89,    89,
      36,    38,    40,    42,    44,    92,    93,    51,    53,    55,
      97,    98,    99,   100,   101,    48,   106,   107,    99,   100,
     111,   112,    28,    28,    30,    22,    85,    85,    85,    46,
      96,    57,   102,   100,   101,    99,   101,    99,   100,   100,
      99,    50,   110,   102,    32,    52,    54,    56,    85,   101,
     100,   101,    99,   100,    99,    37,    58,   103,    85,    58,
      38
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
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

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
# if YYLTYPE_IS_TRIVIAL
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
        case 2:

/* Line 1455 of yacc.c  */
#line 148 "sf_attribute_table.y"
    {
    YYACCEPT;
  }
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 154 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "SnortAttributes: Got Attribute Map & Table\n"););
  }
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 159 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "SnortAttributes: Got Attribute Table\n"););
  }
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 166 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Got Attribute Map\n"););
  }
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 171 "sf_attribute_table.y"
    {
     DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Empty Mapping Table\n"););
   }
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 178 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapEntry: Name: %s, Id %d\n",
        (yyvsp[(2) - (3)].mapEntry).s_mapvalue, (yyvsp[(2) - (3)].mapEntry).l_mapid););
    SFAT_AddMapEntry(&(yyvsp[(2) - (3)].mapEntry));
  }
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 192 "sf_attribute_table.y"
    {
    (yyval.mapEntry).l_mapid = (yyvsp[(1) - (2)].numericValue);
    SnortStrncpy((yyval.mapEntry).s_mapvalue, (yyvsp[(2) - (2)].stringValue), STD_BUF);
  }
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 199 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapValue: %s\n", (yyvsp[(2) - (3)].stringValue));)
    SnortStrncpy((yyval.stringValue), (yyvsp[(2) - (3)].stringValue), STD_BUF);
  }
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 206 "sf_attribute_table.y"
    {
    (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapId: %d\n", (yyvsp[(2) - (3)].numericValue)););
  }
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 214 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Got Attribute Table\n"););
  }
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 219 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyHostEntry\n"););
  }
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 226 "sf_attribute_table.y"
    {
    if (SFAT_AddHostEntryToMap() != SFAT_OK)
    {
        YYABORT;
    }
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Host Added\n"););
  }
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 236 "sf_attribute_table.y"
    {
    /* Callback to create a host entry object */
    SFAT_CreateHostEntry();
  }
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 246 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData\n"););
  }
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 251 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Services\n"););
  }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 256 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Clients\n"););
  }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 261 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Services or Clients\n"););
  }
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 268 "sf_attribute_table.y"
    {
    /* Convert IP/CIDR to Snort IPCidr Object */
    /* determine the number of bits (done in SetHostIp4) */
#ifdef SUP_IP6
    if (SFAT_SetHostIp((yyvsp[(2) - (3)].stringValue)) != SFAT_OK)
#else
    if (SFAT_SetHostIp4((yyvsp[(2) - (3)].stringValue)) != SFAT_OK)
#endif
    {
        YYABORT;
    }
  }
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 290 "sf_attribute_table.y"
    {
    /* Copy OSName */
    DEBUG_WRAP(PrintAttributeData("OS:Name", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_OS);
  }
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 298 "sf_attribute_table.y"
    {
    /* Copy OSVendor */
    DEBUG_WRAP(PrintAttributeData("OS:Vendor", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_VENDOR);
  }
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 306 "sf_attribute_table.y"
    {
    /* Copy OSVersion */
    DEBUG_WRAP(PrintAttributeData("OS:Version", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_VERSION);
  }
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 314 "sf_attribute_table.y"
    {
    /* Copy OSFragPolicy */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "OS:FragPolicy: %s\n", (yyvsp[(2) - (3)].stringValue)););
    SFAT_SetOSPolicy((yyvsp[(2) - (3)].stringValue), HOST_INFO_FRAG_POLICY);
  }
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 322 "sf_attribute_table.y"
    {
    /* Copy OSStreamPolicy */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "OS:StreamPolicy: %s\n", (yyvsp[(2) - (3)].stringValue)););
    SFAT_SetOSPolicy((yyvsp[(2) - (3)].stringValue), HOST_INFO_STREAM_POLICY);
  }
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 330 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = 100;
        SnortStrncpy((yyval.data).value.s_value, (yyvsp[(1) - (1)].stringValue), STD_BUF);
  }
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 336 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = (yyvsp[(2) - (2)].numericValue);
        SnortStrncpy((yyval.data).value.s_value, (yyvsp[(1) - (2)].stringValue), STD_BUF);
  }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 342 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = (yyvsp[(2) - (2)].numericValue);
        SnortSnprintf((yyval.data).value.s_value, STD_BUF, "%d", (yyvsp[(1) - (2)].numericValue));
  }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 348 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = 100;
        SnortSnprintf((yyval.data).value.s_value, STD_BUF, "%d", (yyvsp[(1) - (1)].numericValue));
  }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 354 "sf_attribute_table.y"
    {
        char *mapped_name;
        (yyval.data).confidence = (yyvsp[(2) - (2)].numericValue);
        mapped_name = SFAT_LookupAttributeNameById((yyvsp[(1) - (2)].numericValue));
        if (!mapped_name)
        {
            (yyval.data).type = ATTRIBUTE_ID; 
            (yyval.data).value.l_value = (yyvsp[(1) - (2)].numericValue);
            //FatalError("Unknown/Invalid Attribute ID %d\n", $1);
            sfat_grammar_error = "Unknown/Invalid Attribute ID";
            YYABORT;
        }
        else
        {
            /* Copy String */
            (yyval.data).type = ATTRIBUTE_NAME; 
            SnortStrncpy((yyval.data).value.s_value, mapped_name, STD_BUF);
        }
  }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 374 "sf_attribute_table.y"
    {
        char *mapped_name;
        (yyval.data).confidence = 100;
        mapped_name = SFAT_LookupAttributeNameById((yyvsp[(1) - (1)].numericValue));
        if (!mapped_name)
        {
            (yyval.data).type = ATTRIBUTE_ID; 
            (yyval.data).value.l_value = (yyvsp[(1) - (1)].numericValue);
            //FatalError("Unknown/Invalid Attribute ID %d\n", $1);
            sfat_grammar_error = "Unknown/Invalid Attribute ID";
            YYABORT;
        }
        else
        {
            /* Copy String */
            (yyval.data).type = ATTRIBUTE_NAME; 
            SnortStrncpy((yyval.data).value.s_value, mapped_name, STD_BUF);
        }
  }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 396 "sf_attribute_table.y"
    {
        SnortStrncpy((yyval.stringValue), (yyvsp[(2) - (3)].stringValue), STD_BUF);
  }
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 402 "sf_attribute_table.y"
    {
        (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
  }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 408 "sf_attribute_table.y"
    {
        /* Copy numeric */
        (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
      }
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 415 "sf_attribute_table.y"
    {
    /* Copy numeric */
    (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
  }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 422 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "ServiceList (complete)\n"););
  }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 428 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Start ServiceList\n"););
    sfat_client_or_service = ATTRIBUTE_SERVICE;
  }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 435 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "End ServiceList\n"););
  }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 440 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyService\n"););
  }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 444 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service ServiceListData\n"););
  }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 450 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Adding Complete\n"););
    SFAT_AddApplicationData();
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Added\n"););
  }
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 458 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Start\n"););
    SFAT_CreateApplicationEntry();
  }
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 465 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service End\n"););
  }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 471 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data (no application)\n"););
  }
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 475 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data (application)\n"););
  }
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 481 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (IPProto Proto Port)\n"););
  }
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 486 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (IPProto Port Proto)\n"););
  }
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 491 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Proto IPProto Port)\n"););
  }
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 496 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Proto Port IPProto)\n"););
  }
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 501 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Port Proto IPProto)\n"););
  }
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 506 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Port IPProto Proto)\n"););
  }
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 513 "sf_attribute_table.y"
    {
    /* Store IPProto Info */
    DEBUG_WRAP(PrintAttributeData("IPProto", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_IPPROTO);
  }
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 521 "sf_attribute_table.y"
    {
    /* Store Protocol Info */
    DEBUG_WRAP(PrintAttributeData("Protocol", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_PROTO);
  }
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 529 "sf_attribute_table.y"
    {
    /* Store Port Info */
    DEBUG_WRAP(PrintAttributeData("Port", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_PORT);
  }
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 537 "sf_attribute_table.y"
    {
    /* Store Application Info */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Application\n"));
    DEBUG_WRAP(PrintAttributeData("Application", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_APPLICATION);
  }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 544 "sf_attribute_table.y"
    {
    /* Store Application Info */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Application with Version\n"));
    DEBUG_WRAP(PrintAttributeData("Application", &(yyvsp[(2) - (4)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (4)].data), APPLICATION_ENTRY_APPLICATION);
  }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 553 "sf_attribute_table.y"
    {
    /* Store Version Info */
    DEBUG_WRAP(PrintAttributeData("Version", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_VERSION);
  }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 561 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "ClientList (complete)\n"););
  }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 567 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Start ClientList\n"););
    sfat_client_or_service = ATTRIBUTE_CLIENT;
  }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 574 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "End ClientList\n"););
  }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 579 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyClient\n"););
  }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 583 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client ClientListData\n"););
  }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 589 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Adding Complete\n"););
    SFAT_AddApplicationData();
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Added\n"););
  }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 597 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Start\n"););
    SFAT_CreateApplicationEntry();
  }
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 604 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client End\n"););
  }
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 610 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data (no application)\n"););
  }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 614 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data (application)\n"););
  }
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 620 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (Proto)\n"););
  }
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 625 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (IPProto Proto)\n"););
  }
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 630 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (Proto IPProto)\n"););
  }
    break;



/* Line 1455 of yacc.c  */
#line 2356 "sf_attribute_table.c"
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



/* Line 1675 of yacc.c  */
#line 635 "sf_attribute_table.y"

/*
int yywrap(void)
{
    return 1;
}
*/
#endif /* TARGET_BASED */

