/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         sfat_parse
#define yylex           sfat_lex
#define yyerror         sfat_error
#define yylval          sfat_lval
#define yychar          sfat_char
#define yydebug         sfat_debug
#define yynerrs         sfat_nerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 33 "sf_attribute_table.y"

#ifdef TARGET_BASED
#include <stdlib.h>
#include <string.h>
#include "sftarget_reader.h"
#include "snort_debug.h"

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

/* Line 371 of yacc.c  */
#line 100 "sf_attribute_table.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "sf_attribute_table.h".  */
#ifndef YY_SFAT_SF_ATTRIBUTE_TABLE_H_INCLUDED
# define YY_SFAT_SF_ATTRIBUTE_TABLE_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int sfat_debug;
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
/* Line 387 of yacc.c  */
#line 59 "sf_attribute_table.y"

  char stringValue[STD_BUF];
  uint32_t numericValue;
  AttributeData data;
  MapData mapEntry;


/* Line 387 of yacc.c  */
#line 267 "sf_attribute_table.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE sfat_lval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int sfat_parse (void *YYPARSE_PARAM);
#else
int sfat_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int sfat_parse (void);
#else
int sfat_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_SFAT_SF_ATTRIBUTE_TABLE_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 295 "sf_attribute_table.c"

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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   133

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  83
/* YYNRULES -- Number of states.  */
#define YYNSTATES  152

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
     121,   124,   126,   129,   131,   135,   138,   142,   146,   150,
     154,   156,   158,   159,   162,   166,   168,   170,   172,   175,
     179,   183,   187,   191,   195,   199,   203,   207,   211,   215,
     220,   224,   228,   230,   232,   233,   236,   240,   242,   244,
     246,   249,   251,   254
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
      -1,    27,    21,    28,    -1,    27,    28,    -1,    27,    22,
      28,    -1,    29,    22,    30,    -1,    31,    22,    32,    -1,
      91,    93,    92,    -1,    43,    -1,    44,    -1,    -1,    94,
      93,    -1,    95,    97,    96,    -1,    45,    -1,    46,    -1,
      98,    -1,    98,   102,    -1,    99,   100,   101,    -1,    99,
     101,   100,    -1,   100,    99,   101,    -1,   100,   101,    99,
      -1,   101,   100,    99,    -1,   101,    99,   100,    -1,    51,
      85,    52,    -1,    55,    85,    56,    -1,    53,    85,    54,
      -1,    57,    85,    58,    -1,    57,    85,   103,    58,    -1,
      37,    85,    38,    -1,   105,   107,   106,    -1,    47,    -1,
      48,    -1,    -1,   108,   107,    -1,   109,   111,   110,    -1,
      49,    -1,    50,    -1,   112,    -1,   112,   102,    -1,   100,
      -1,    99,   100,    -1,   100,    99,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   144,   144,   150,   155,   162,   168,   171,   174,   182,
     185,   188,   195,   202,   210,   216,   219,   222,   232,   239,
     242,   247,   252,   257,   264,   275,   277,   277,   279,   279,
     279,   279,   279,   282,   290,   298,   306,   314,   322,   328,
     334,   340,   346,   366,   388,   394,   398,   404,   411,   418,
     424,   431,   437,   440,   446,   454,   461,   467,   471,   477,
     482,   487,   492,   497,   502,   509,   517,   525,   533,   540,
     549,   557,   563,   570,   576,   579,   585,   593,   600,   606,
     610,   616,   621,   626
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
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
  "ClientStart", "ClientEnd", "ClientData", "ClientDataRequired", YY_NULL
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
      85,    85,    85,    85,    86,    87,    87,    88,    89,    90,
      91,    92,    93,    93,    94,    95,    96,    97,    97,    98,
      98,    98,    98,    98,    98,    99,   100,   101,   102,   102,
     103,   104,   105,   106,   107,   107,   108,   109,   110,   111,
     111,   112,   112,   112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     4,     3,     3,     0,     2,     3,     1,
       1,     2,     3,     3,     3,     0,     2,     3,     1,     1,
       4,     3,     3,     2,     3,     3,     1,     2,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     1,     2,
       2,     1,     2,     1,     3,     2,     3,     3,     3,     3,
       1,     1,     0,     2,     3,     1,     1,     1,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     3,     1,     1,     0,     2,     3,     1,     1,     1,
       2,     1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     2,     6,    15,     0,     0,     1,     9,
       0,     6,     0,     0,     0,     4,     5,     7,     0,     0,
       0,    14,    18,    16,     0,     3,     0,    10,     8,     0,
      11,     0,     0,     0,    13,     0,     0,    19,    17,     0,
      23,    12,    24,     0,     0,     0,     0,     0,     0,    26,
      28,    29,    30,    32,    31,    50,    72,    22,    52,    21,
      74,     0,     0,     0,    38,    41,    43,     0,     0,     0,
       0,    25,    27,    20,    55,     0,    52,     0,    77,     0,
      74,     0,     0,     0,    45,     0,    33,     0,    39,    40,
      42,    34,    35,    36,    37,    51,    49,    53,     0,     0,
       0,     0,    57,     0,     0,     0,    73,    71,    75,     0,
      81,     0,    79,    44,    46,    47,     0,     0,     0,     0,
      56,    54,     0,    58,     0,     0,     0,     0,     0,     0,
      82,    83,    78,    76,    80,    48,    65,    67,    66,     0,
      59,    60,    61,    62,    64,    63,     0,    68,     0,     0,
      69,    70
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     6,    10,    11,    12,    28,    19,    30,
      20,     7,    13,    23,    24,    38,    32,    33,    40,    48,
      49,    50,    51,    52,    53,    54,    63,    64,    65,    66,
      88,    57,    58,    96,    75,    76,    77,   121,   101,   102,
     103,   104,   105,   123,   148,    59,    60,   107,    79,    80,
      81,   133,   111,   112
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -97
static const yytype_int8 yypact[] =
{
       1,    25,     9,   -97,    28,   -97,    30,    41,   -97,   -97,
      43,    28,    51,    42,    57,   -97,   -97,   -97,    44,    54,
      52,   -97,   -97,   -97,    48,   -97,    56,   -97,   -97,    49,
     -97,    50,    55,    47,   -97,    60,    59,   -97,   -97,   -23,
      -4,   -97,   -97,    -7,    -7,    -7,    61,    62,   -22,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,    22,    31,   -97,
      26,    13,    58,    53,    46,    46,    46,    45,    63,    64,
      65,   -97,   -97,   -97,   -97,    40,    31,    -9,   -97,    37,
      26,     2,    66,    67,   -97,    68,   -97,    69,   -97,   -97,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,    -7,    -7,
      -7,    70,    29,   -32,   -15,     2,   -97,   -97,   -97,    33,
      38,    71,    29,   -97,   -97,   -97,    73,    72,    36,    74,
     -97,   -97,    -7,   -97,    39,    33,    39,    38,    33,    38,
     -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -34,
     -97,   -97,   -97,   -97,   -97,   -97,    -7,   -97,    35,    75,
     -97,   -97
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -97,   -97,   -97,   -97,    85,   -97,   -97,   -97,   -97,   -97,
     -97,    91,   -97,   -97,   -97,   -97,   -97,   -97,   -97,   -97,
      77,   -97,   -97,   -97,   -97,   -97,   -44,   -97,   -97,   -97,
      -5,   -97,   -97,   -97,    23,   -97,   -97,   -97,   -97,   -97,
     -79,   -76,   -96,   -12,   -97,    76,   -97,   -97,    32,   -97,
     -97,   -97,   -97,   -97
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      67,    68,   109,   146,    71,   110,     1,   125,   127,     8,
      43,    43,    44,    44,    45,    45,    46,    46,    47,    47,
      61,    99,    62,   100,   147,   126,   128,   124,   140,   129,
     142,   131,     4,   130,    82,    83,    98,     9,    99,    55,
       5,    84,    98,    56,    99,     5,   100,    15,   143,   141,
     145,    16,   144,    98,   117,   118,   119,   100,    21,    22,
      89,    90,    18,    25,    27,    29,    26,    31,    34,    56,
      35,    36,    39,    37,    41,    78,    74,    87,   139,    42,
      85,    91,    69,    70,    95,   106,   122,    86,   100,    98,
     137,   116,    99,   150,   113,   114,    17,    14,   115,    97,
     134,    92,   149,     0,    93,   135,     0,    94,     0,     0,
       0,     0,   108,   151,     0,     0,   120,     0,     0,     0,
       0,   132,     0,     0,   136,    72,     0,     0,     0,     0,
     138,     0,     0,    73
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-97)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      44,    45,    81,    37,    26,    81,     5,   103,   104,     0,
      33,    33,    35,    35,    37,    37,    39,    39,    41,    41,
      27,    53,    29,    55,    58,   104,   105,   103,   124,   105,
     126,   110,     7,   109,    21,    22,    51,     9,    53,    43,
      15,    28,    51,    47,    53,    15,    55,     6,   127,   125,
     129,     8,   128,    51,    98,    99,   100,    55,    16,    17,
      65,    66,    11,     6,    10,    13,    22,    19,    12,    47,
      21,    21,    25,    18,    14,    49,    45,    31,   122,    20,
      22,    36,    21,    21,    44,    48,    57,    34,    55,    51,
      54,    22,    53,    58,    28,    28,    11,     6,    30,    76,
     112,    38,   146,    -1,    40,    32,    -1,    42,    -1,    -1,
      -1,    -1,    80,    38,    -1,    -1,    46,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    52,    48,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    57
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
     108,   109,    21,    22,    28,    22,    34,    31,    89,    89,
      89,    36,    38,    40,    42,    44,    92,    93,    51,    53,
      55,    97,    98,    99,   100,   101,    48,   106,   107,    99,
     100,   111,   112,    28,    28,    30,    22,    85,    85,    85,
      46,    96,    57,   102,   100,   101,    99,   101,    99,   100,
     100,    99,    50,   110,   102,    32,    52,    54,    56,    85,
     101,   100,   101,    99,   100,    99,    37,    58,   103,    85,
      58,    38
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

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

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

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
/* Line 1792 of yacc.c  */
#line 145 "sf_attribute_table.y"
    {
    YYACCEPT;
  }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 151 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "SnortAttributes: Got Attribute Map & Table\n"););
  }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 156 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "SnortAttributes: Got Attribute Table\n"););
  }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 163 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Got Attribute Map\n"););
  }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 168 "sf_attribute_table.y"
    {
     DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Empty Mapping Table\n"););
   }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 175 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapEntry: Name: %s, Id %d\n",
        (yyvsp[(2) - (3)].mapEntry).s_mapvalue, (yyvsp[(2) - (3)].mapEntry).l_mapid););
    SFAT_AddMapEntry(&(yyvsp[(2) - (3)].mapEntry));
  }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 189 "sf_attribute_table.y"
    {
    (yyval.mapEntry).l_mapid = (yyvsp[(1) - (2)].numericValue);
    SnortStrncpy((yyval.mapEntry).s_mapvalue, (yyvsp[(2) - (2)].stringValue), STD_BUF);
  }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 196 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapValue: %s\n", (yyvsp[(2) - (3)].stringValue));)
    SnortStrncpy((yyval.stringValue), (yyvsp[(2) - (3)].stringValue), STD_BUF);
  }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 203 "sf_attribute_table.y"
    {
    (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "MapId: %d\n", (yyvsp[(2) - (3)].numericValue)););
  }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 211 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Got Attribute Table\n"););
  }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 216 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyHostEntry\n"););
  }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 223 "sf_attribute_table.y"
    {
    if (SFAT_AddHostEntryToMap() != SFAT_OK)
    {
        YYABORT;
    }
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Host Added\n"););
  }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 233 "sf_attribute_table.y"
    {
    /* Callback to create a host entry object */
    SFAT_CreateHostEntry();
  }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 243 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData\n"););
  }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 248 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Services\n"););
  }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 253 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Clients\n"););
  }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 258 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "HostEntryData: No Services or Clients\n"););
  }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 265 "sf_attribute_table.y"
    {
    /* Convert IP/CIDR to Snort IPCidr Object */
    /* determine the number of bits (done in SetHostIp4) */
    if (SFAT_SetHostIp((yyvsp[(2) - (3)].stringValue)) != SFAT_OK)
    {
        YYABORT;
    }
  }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 283 "sf_attribute_table.y"
    {
    /* Copy OSName */
    DEBUG_WRAP(PrintAttributeData("OS:Name", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_OS);
  }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 291 "sf_attribute_table.y"
    {
    /* Copy OSVendor */
    DEBUG_WRAP(PrintAttributeData("OS:Vendor", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_VENDOR);
  }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 299 "sf_attribute_table.y"
    {
    /* Copy OSVersion */
    DEBUG_WRAP(PrintAttributeData("OS:Version", &(yyvsp[(2) - (3)].data)););
    SFAT_SetOSAttribute(&(yyvsp[(2) - (3)].data), HOST_INFO_VERSION);
  }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 307 "sf_attribute_table.y"
    {
    /* Copy OSFragPolicy */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "OS:FragPolicy: %s\n", (yyvsp[(2) - (3)].stringValue)););
    SFAT_SetOSPolicy((yyvsp[(2) - (3)].stringValue), HOST_INFO_FRAG_POLICY);
  }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 315 "sf_attribute_table.y"
    {
    /* Copy OSStreamPolicy */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "OS:StreamPolicy: %s\n", (yyvsp[(2) - (3)].stringValue)););
    SFAT_SetOSPolicy((yyvsp[(2) - (3)].stringValue), HOST_INFO_STREAM_POLICY);
  }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 323 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = 100;
        SnortStrncpy((yyval.data).value.s_value, (yyvsp[(1) - (1)].stringValue), STD_BUF);
  }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 329 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = (yyvsp[(2) - (2)].numericValue);
        SnortStrncpy((yyval.data).value.s_value, (yyvsp[(1) - (2)].stringValue), STD_BUF);
  }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 335 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = (yyvsp[(2) - (2)].numericValue);
        SnortSnprintf((yyval.data).value.s_value, STD_BUF, "%d", (yyvsp[(1) - (2)].numericValue));
  }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 341 "sf_attribute_table.y"
    {
        (yyval.data).type = ATTRIBUTE_NAME; 
        (yyval.data).confidence = 100;
        SnortSnprintf((yyval.data).value.s_value, STD_BUF, "%d", (yyvsp[(1) - (1)].numericValue));
  }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 347 "sf_attribute_table.y"
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
/* Line 1792 of yacc.c  */
#line 367 "sf_attribute_table.y"
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
/* Line 1792 of yacc.c  */
#line 389 "sf_attribute_table.y"
    {
        SnortStrncpy((yyval.stringValue), (yyvsp[(2) - (3)].stringValue), STD_BUF);
  }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 395 "sf_attribute_table.y"
    {
        (yyval.numericValue) = 0;
  }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 399 "sf_attribute_table.y"
    {
        (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
  }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 405 "sf_attribute_table.y"
    {
        /* Copy numeric */
        (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
      }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 412 "sf_attribute_table.y"
    {
    /* Copy numeric */
    (yyval.numericValue) = (yyvsp[(2) - (3)].numericValue);
  }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 419 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "ServiceList (complete)\n"););
  }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 425 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Start ServiceList\n"););
    sfat_client_or_service = ATTRIBUTE_SERVICE;
  }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 432 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "End ServiceList\n"););
  }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 437 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyService\n"););
  }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 441 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service ServiceListData\n"););
  }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 447 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Adding Complete\n"););
    SFAT_AddApplicationData();
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Added\n"););
  }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 455 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Start\n"););
    SFAT_CreateApplicationEntry();
  }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 462 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service End\n"););
  }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 468 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data (no application)\n"););
  }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 472 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data (application)\n"););
  }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 478 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (IPProto Proto Port)\n"););
  }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 483 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (IPProto Port Proto)\n"););
  }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 488 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Proto IPProto Port)\n"););
  }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 493 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Proto Port IPProto)\n"););
  }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 498 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Port Proto IPProto)\n"););
  }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 503 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Service Data Required (Port IPProto Proto)\n"););
  }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 510 "sf_attribute_table.y"
    {
    /* Store IPProto Info */
    DEBUG_WRAP(PrintAttributeData("IPProto", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_IPPROTO);
  }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 518 "sf_attribute_table.y"
    {
    /* Store Protocol Info */
    DEBUG_WRAP(PrintAttributeData("Protocol", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_PROTO);
  }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 526 "sf_attribute_table.y"
    {
    /* Store Port Info */
    DEBUG_WRAP(PrintAttributeData("Port", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_PORT);
  }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 534 "sf_attribute_table.y"
    {
    /* Store Application Info */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Application\n"));
    DEBUG_WRAP(PrintAttributeData("Application", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_APPLICATION);
  }
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 541 "sf_attribute_table.y"
    {
    /* Store Application Info */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Application with Version\n"));
    DEBUG_WRAP(PrintAttributeData("Application", &(yyvsp[(2) - (4)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (4)].data), APPLICATION_ENTRY_APPLICATION);
  }
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 550 "sf_attribute_table.y"
    {
    /* Store Version Info */
    DEBUG_WRAP(PrintAttributeData("Version", &(yyvsp[(2) - (3)].data)););
    SFAT_SetApplicationAttribute(&(yyvsp[(2) - (3)].data), APPLICATION_ENTRY_VERSION);
  }
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 558 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "ClientList (complete)\n"););
  }
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 564 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Start ClientList\n"););
    sfat_client_or_service = ATTRIBUTE_CLIENT;
  }
    break;

  case 73:
/* Line 1792 of yacc.c  */
#line 571 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "End ClientList\n"););
  }
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 576 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "EmptyClient\n"););
  }
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 580 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client ClientListData\n"););
  }
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 586 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Adding Complete\n"););
    SFAT_AddApplicationData();
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Added\n"););
  }
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 594 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Start\n"););
    SFAT_CreateApplicationEntry();
  }
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 601 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client End\n"););
  }
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 607 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data (no application)\n"););
  }
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 611 "sf_attribute_table.y"
    {
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data (application)\n"););
  }
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 617 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (Proto)\n"););
  }
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 622 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (IPProto Proto)\n"););
  }
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 627 "sf_attribute_table.y"
    {
    /* Order independent */
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Client Data Required (Proto IPProto)\n"););
  }
    break;


/* Line 1792 of yacc.c  */
#line 2301 "sf_attribute_table.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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


/* Line 2055 of yacc.c  */
#line 632 "sf_attribute_table.y"

/*
int yywrap(void)
{
    return 1;
}
*/
#endif /* TARGET_BASED */
