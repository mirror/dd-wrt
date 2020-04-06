/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yydebug         zenddebug
#define yynerrs         zendnerrs


/* Copy the first part of user declarations.  */


/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Zend Technologies Ltd. (http://www.zend.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@php.net>                                 |
   |          Zeev Suraski <zeev@php.net>                                 |
   |          Nikita Popov <nikic@php.net>                                |
   +----------------------------------------------------------------------+
*/

#include "zend_compile.h"
#include "zend.h"
#include "zend_list.h"
#include "zend_globals.h"
#include "zend_API.h"
#include "zend_constants.h"
#include "zend_language_scanner.h"
#include "zend_exceptions.h"

#define YYSIZE_T size_t
#define yytnamerr zend_yytnamerr
static YYSIZE_T zend_yytnamerr(char*, const char*);

#define YYSTYPE zend_parser_stack_elem

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif




# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "zend_language_parser.h".  */
#ifndef YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
# define YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int zenddebug;
#endif
/* "%code requires" blocks.  */





/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    PREC_ARROW_FUNCTION = 258,
    T_INCLUDE = 259,
    T_INCLUDE_ONCE = 260,
    T_REQUIRE = 261,
    T_REQUIRE_ONCE = 262,
    T_LOGICAL_OR = 263,
    T_LOGICAL_XOR = 264,
    T_LOGICAL_AND = 265,
    T_PRINT = 266,
    T_YIELD = 267,
    T_DOUBLE_ARROW = 268,
    T_YIELD_FROM = 269,
    T_PLUS_EQUAL = 270,
    T_MINUS_EQUAL = 271,
    T_MUL_EQUAL = 272,
    T_DIV_EQUAL = 273,
    T_CONCAT_EQUAL = 274,
    T_MOD_EQUAL = 275,
    T_AND_EQUAL = 276,
    T_OR_EQUAL = 277,
    T_XOR_EQUAL = 278,
    T_SL_EQUAL = 279,
    T_SR_EQUAL = 280,
    T_POW_EQUAL = 281,
    T_COALESCE_EQUAL = 282,
    T_COALESCE = 283,
    T_BOOLEAN_OR = 284,
    T_BOOLEAN_AND = 285,
    T_IS_EQUAL = 286,
    T_IS_NOT_EQUAL = 287,
    T_IS_IDENTICAL = 288,
    T_IS_NOT_IDENTICAL = 289,
    T_SPACESHIP = 290,
    T_IS_SMALLER_OR_EQUAL = 291,
    T_IS_GREATER_OR_EQUAL = 292,
    T_SL = 293,
    T_SR = 294,
    T_INSTANCEOF = 295,
    T_INT_CAST = 296,
    T_DOUBLE_CAST = 297,
    T_STRING_CAST = 298,
    T_ARRAY_CAST = 299,
    T_OBJECT_CAST = 300,
    T_BOOL_CAST = 301,
    T_UNSET_CAST = 302,
    T_POW = 303,
    T_NEW = 304,
    T_CLONE = 305,
    T_NOELSE = 306,
    T_ELSEIF = 307,
    T_ELSE = 308,
    T_LNUMBER = 309,
    T_DNUMBER = 310,
    T_STRING = 311,
    T_VARIABLE = 312,
    T_INLINE_HTML = 313,
    T_ENCAPSED_AND_WHITESPACE = 314,
    T_CONSTANT_ENCAPSED_STRING = 315,
    T_STRING_VARNAME = 316,
    T_NUM_STRING = 317,
    T_EVAL = 318,
    T_INC = 319,
    T_DEC = 320,
    T_EXIT = 321,
    T_IF = 322,
    T_ENDIF = 323,
    T_ECHO = 324,
    T_DO = 325,
    T_WHILE = 326,
    T_ENDWHILE = 327,
    T_FOR = 328,
    T_ENDFOR = 329,
    T_FOREACH = 330,
    T_ENDFOREACH = 331,
    T_DECLARE = 332,
    T_ENDDECLARE = 333,
    T_AS = 334,
    T_SWITCH = 335,
    T_ENDSWITCH = 336,
    T_CASE = 337,
    T_DEFAULT = 338,
    T_BREAK = 339,
    T_CONTINUE = 340,
    T_GOTO = 341,
    T_FUNCTION = 342,
    T_FN = 343,
    T_CONST = 344,
    T_RETURN = 345,
    T_TRY = 346,
    T_CATCH = 347,
    T_FINALLY = 348,
    T_THROW = 349,
    T_USE = 350,
    T_INSTEADOF = 351,
    T_GLOBAL = 352,
    T_STATIC = 353,
    T_ABSTRACT = 354,
    T_FINAL = 355,
    T_PRIVATE = 356,
    T_PROTECTED = 357,
    T_PUBLIC = 358,
    T_VAR = 359,
    T_UNSET = 360,
    T_ISSET = 361,
    T_EMPTY = 362,
    T_HALT_COMPILER = 363,
    T_CLASS = 364,
    T_TRAIT = 365,
    T_INTERFACE = 366,
    T_EXTENDS = 367,
    T_IMPLEMENTS = 368,
    T_OBJECT_OPERATOR = 369,
    T_LIST = 370,
    T_ARRAY = 371,
    T_CALLABLE = 372,
    T_LINE = 373,
    T_FILE = 374,
    T_DIR = 375,
    T_CLASS_C = 376,
    T_TRAIT_C = 377,
    T_METHOD_C = 378,
    T_FUNC_C = 379,
    T_COMMENT = 380,
    T_DOC_COMMENT = 381,
    T_OPEN_TAG = 382,
    T_OPEN_TAG_WITH_ECHO = 383,
    T_CLOSE_TAG = 384,
    T_WHITESPACE = 385,
    T_START_HEREDOC = 386,
    T_END_HEREDOC = 387,
    T_DOLLAR_OPEN_CURLY_BRACES = 388,
    T_CURLY_OPEN = 389,
    T_PAAMAYIM_NEKUDOTAYIM = 390,
    T_NAMESPACE = 391,
    T_NS_C = 392,
    T_NS_SEPARATOR = 393,
    T_ELLIPSIS = 394,
    T_BAD_CHARACTER = 395,
    T_ERROR = 396
  };
#endif

/* Value type.  */



ZEND_API int zendparse (void);

#endif /* !YY_ZEND_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */



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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

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
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   7174

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  170
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  140
/* YYNRULES -- Number of rules.  */
#define YYNRULES  504
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  955

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   396

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,   168,     2,   169,    53,    36,     2,
     159,   160,    51,    48,   164,    49,    50,    52,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   161,
      42,    15,    44,    29,    64,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   165,     2,   166,    35,     2,   167,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   162,    34,   163,    56,     2,     2,     2,
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
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    31,    32,    33,    37,    38,    39,    40,
      41,    43,    45,    46,    47,    55,    57,    58,    59,    60,
      61,    62,    63,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   270,   270,   274,   274,   274,   274,   274,   274,   274,
     274,   275,   275,   275,   275,   275,   275,   275,   275,   275,
     275,   275,   275,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   277,   277,   277,   277,   277,   277,   277,
     277,   277,   277,   278,   278,   278,   278,   278,   278,   278,
     278,   278,   278,   278,   279,   279,   279,   279,   279,   279,
     279,   279,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   284,   285,   285,   285,   285,   285,   285,   289,   290,
     298,   299,   303,   304,   308,   309,   310,   314,   315,   316,
     317,   318,   319,   323,   326,   326,   329,   329,   332,   333,
     334,   335,   336,   340,   341,   345,   347,   352,   354,   358,
     360,   364,   366,   371,   373,   378,   380,   385,   386,   390,
     392,   397,   398,   402,   403,   407,   410,   415,   416,   417,
     418,   419,   420,   427,   428,   429,   430,   432,   434,   436,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     449,   453,   452,   456,   457,   459,   460,   461,   466,   467,
     472,   473,   477,   478,   482,   483,   487,   491,   498,   499,
     503,   504,   508,   508,   511,   511,   517,   518,   523,   524,
     528,   528,   534,   534,   540,   541,   545,   546,   550,   551,
     555,   556,   557,   558,   562,   563,   567,   568,   572,   573,
     577,   578,   579,   580,   584,   585,   587,   592,   593,   598,
     599,   604,   607,   613,   614,   619,   622,   628,   629,   635,
     636,   641,   643,   648,   650,   656,   657,   661,   662,   666,
     667,   668,   672,   673,   677,   678,   682,   684,   689,   690,
     694,   695,   699,   705,   706,   710,   711,   716,   719,   724,
     727,   729,   731,   738,   739,   743,   744,   745,   749,   751,
     756,   757,   761,   766,   768,   770,   772,   777,   779,   783,
     788,   789,   793,   794,   798,   799,   804,   805,   810,   811,
     812,   813,   814,   815,   819,   820,   824,   826,   831,   832,
     836,   840,   844,   845,   848,   852,   853,   857,   858,   862,
     862,   872,   874,   879,   881,   883,   885,   887,   889,   890,
     892,   894,   896,   898,   900,   902,   904,   906,   908,   910,
     912,   914,   916,   917,   918,   919,   920,   922,   924,   926,
     928,   930,   931,   932,   933,   934,   935,   936,   937,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   948,   950,
     952,   954,   956,   958,   960,   962,   964,   966,   971,   972,
     974,   976,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,  1001,  1006,  1015,  1019,  1023,  1027,  1031,  1035,  1036,
    1040,  1041,  1045,  1046,  1050,  1051,  1055,  1057,  1059,  1061,
    1066,  1069,  1073,  1074,  1078,  1079,  1084,  1085,  1086,  1091,
    1092,  1097,  1098,  1099,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1116,  1117,  1118,  1119,
    1123,  1124,  1126,  1131,  1132,  1136,  1140,  1141,  1142,  1146,
    1147,  1148,  1152,  1154,  1156,  1158,  1160,  1162,  1166,  1168,
    1170,  1175,  1176,  1177,  1181,  1183,  1188,  1190,  1192,  1194,
    1196,  1198,  1203,  1204,  1205,  1209,  1210,  1211,  1215,  1220,
    1221,  1225,  1227,  1232,  1234,  1236,  1238,  1240,  1242,  1245,
    1251,  1253,  1255,  1257,  1262,  1264,  1267,  1270,  1272,  1274,
    1277,  1281,  1282,  1283,  1284,  1289,  1290,  1291,  1293,  1295,
    1297,  1299,  1304,  1305,  1310
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "PREC_ARROW_FUNCTION",
  "\"include (T_INCLUDE)\"", "\"include_once (T_INCLUDE_ONCE)\"",
  "\"require (T_REQUIRE)\"", "\"require_once (T_REQUIRE_ONCE)\"",
  "\"or (T_LOGICAL_OR)\"", "\"xor (T_LOGICAL_XOR)\"",
  "\"and (T_LOGICAL_AND)\"", "\"print (T_PRINT)\"", "\"yield (T_YIELD)\"",
  "\"=> (T_DOUBLE_ARROW)\"", "\"yield from (T_YIELD_FROM)\"", "'='",
  "\"+= (T_PLUS_EQUAL)\"", "\"-= (T_MINUS_EQUAL)\"",
  "\"*= (T_MUL_EQUAL)\"", "\"/= (T_DIV_EQUAL)\"",
  "\".= (T_CONCAT_EQUAL)\"", "\"%= (T_MOD_EQUAL)\"",
  "\"&= (T_AND_EQUAL)\"", "\"|= (T_OR_EQUAL)\"", "\"^= (T_XOR_EQUAL)\"",
  "\"<<= (T_SL_EQUAL)\"", "\">>= (T_SR_EQUAL)\"", "\"**= (T_POW_EQUAL)\"",
  "\"?""?= (T_COALESCE_EQUAL)\"", "'?'", "':'", "\"?? (T_COALESCE)\"",
  "\"|| (T_BOOLEAN_OR)\"", "\"&& (T_BOOLEAN_AND)\"", "'|'", "'^'", "'&'",
  "\"== (T_IS_EQUAL)\"", "\"!= (T_IS_NOT_EQUAL)\"",
  "\"=== (T_IS_IDENTICAL)\"", "\"!== (T_IS_NOT_IDENTICAL)\"",
  "\"<=> (T_SPACESHIP)\"", "'<'", "\"<= (T_IS_SMALLER_OR_EQUAL)\"", "'>'",
  "\">= (T_IS_GREATER_OR_EQUAL)\"", "\"<< (T_SL)\"", "\">> (T_SR)\"",
  "'+'", "'-'", "'.'", "'*'", "'/'", "'%'", "'!'",
  "\"instanceof (T_INSTANCEOF)\"", "'~'", "\"(int) (T_INT_CAST)\"",
  "\"(double) (T_DOUBLE_CAST)\"", "\"(string) (T_STRING_CAST)\"",
  "\"(array) (T_ARRAY_CAST)\"", "\"(object) (T_OBJECT_CAST)\"",
  "\"(bool) (T_BOOL_CAST)\"", "\"(unset) (T_UNSET_CAST)\"", "'@'",
  "\"** (T_POW)\"", "\"new (T_NEW)\"", "\"clone (T_CLONE)\"", "T_NOELSE",
  "\"elseif (T_ELSEIF)\"", "\"else (T_ELSE)\"",
  "\"integer number (T_LNUMBER)\"",
  "\"floating-point number (T_DNUMBER)\"", "\"identifier (T_STRING)\"",
  "\"variable (T_VARIABLE)\"", "T_INLINE_HTML",
  "\"quoted-string and whitespace (T_ENCAPSED_AND_WHITESPACE)\"",
  "\"quoted-string (T_CONSTANT_ENCAPSED_STRING)\"",
  "\"variable name (T_STRING_VARNAME)\"", "\"number (T_NUM_STRING)\"",
  "\"eval (T_EVAL)\"", "\"++ (T_INC)\"", "\"-- (T_DEC)\"",
  "\"exit (T_EXIT)\"", "\"if (T_IF)\"", "\"endif (T_ENDIF)\"",
  "\"echo (T_ECHO)\"", "\"do (T_DO)\"", "\"while (T_WHILE)\"",
  "\"endwhile (T_ENDWHILE)\"", "\"for (T_FOR)\"", "\"endfor (T_ENDFOR)\"",
  "\"foreach (T_FOREACH)\"", "\"endforeach (T_ENDFOREACH)\"",
  "\"declare (T_DECLARE)\"", "\"enddeclare (T_ENDDECLARE)\"",
  "\"as (T_AS)\"", "\"switch (T_SWITCH)\"", "\"endswitch (T_ENDSWITCH)\"",
  "\"case (T_CASE)\"", "\"default (T_DEFAULT)\"", "\"break (T_BREAK)\"",
  "\"continue (T_CONTINUE)\"", "\"goto (T_GOTO)\"",
  "\"function (T_FUNCTION)\"", "\"fn (T_FN)\"", "\"const (T_CONST)\"",
  "\"return (T_RETURN)\"", "\"try (T_TRY)\"", "\"catch (T_CATCH)\"",
  "\"finally (T_FINALLY)\"", "\"throw (T_THROW)\"", "\"use (T_USE)\"",
  "\"insteadof (T_INSTEADOF)\"", "\"global (T_GLOBAL)\"",
  "\"static (T_STATIC)\"", "\"abstract (T_ABSTRACT)\"",
  "\"final (T_FINAL)\"", "\"private (T_PRIVATE)\"",
  "\"protected (T_PROTECTED)\"", "\"public (T_PUBLIC)\"",
  "\"var (T_VAR)\"", "\"unset (T_UNSET)\"", "\"isset (T_ISSET)\"",
  "\"empty (T_EMPTY)\"", "\"__halt_compiler (T_HALT_COMPILER)\"",
  "\"class (T_CLASS)\"", "\"trait (T_TRAIT)\"",
  "\"interface (T_INTERFACE)\"", "\"extends (T_EXTENDS)\"",
  "\"implements (T_IMPLEMENTS)\"", "\"-> (T_OBJECT_OPERATOR)\"",
  "\"list (T_LIST)\"", "\"array (T_ARRAY)\"", "\"callable (T_CALLABLE)\"",
  "\"__LINE__ (T_LINE)\"", "\"__FILE__ (T_FILE)\"", "\"__DIR__ (T_DIR)\"",
  "\"__CLASS__ (T_CLASS_C)\"", "\"__TRAIT__ (T_TRAIT_C)\"",
  "\"__METHOD__ (T_METHOD_C)\"", "\"__FUNCTION__ (T_FUNC_C)\"",
  "\"comment (T_COMMENT)\"", "\"doc comment (T_DOC_COMMENT)\"",
  "\"open tag (T_OPEN_TAG)\"",
  "\"open tag with echo (T_OPEN_TAG_WITH_ECHO)\"",
  "\"close tag (T_CLOSE_TAG)\"", "\"whitespace (T_WHITESPACE)\"",
  "\"heredoc start (T_START_HEREDOC)\"", "\"heredoc end (T_END_HEREDOC)\"",
  "\"${ (T_DOLLAR_OPEN_CURLY_BRACES)\"", "\"{$ (T_CURLY_OPEN)\"",
  "\":: (T_PAAMAYIM_NEKUDOTAYIM)\"", "\"namespace (T_NAMESPACE)\"",
  "\"__NAMESPACE__ (T_NS_C)\"", "\"\\\\ (T_NS_SEPARATOR)\"",
  "\"... (T_ELLIPSIS)\"", "\"invalid character (T_BAD_CHARACTER)\"",
  "T_ERROR", "'('", "')'", "';'", "'{'", "'}'", "','", "'['", "']'", "'`'",
  "'\"'", "'$'", "$accept", "start", "reserved_non_modifiers",
  "semi_reserved", "identifier", "top_statement_list", "namespace_name",
  "name", "top_statement", "$@1", "$@2", "use_type",
  "group_use_declaration", "mixed_group_use_declaration", "possible_comma",
  "inline_use_declarations", "unprefixed_use_declarations",
  "use_declarations", "inline_use_declaration",
  "unprefixed_use_declaration", "use_declaration", "const_list",
  "inner_statement_list", "inner_statement", "statement", "$@3",
  "catch_list", "catch_name_list", "finally_statement", "unset_variables",
  "unset_variable", "function_declaration_statement", "is_reference",
  "is_variadic", "class_declaration_statement", "@4", "@5",
  "class_modifiers", "class_modifier", "trait_declaration_statement", "@6",
  "interface_declaration_statement", "@7", "extends_from",
  "interface_extends_list", "implements_list", "foreach_variable",
  "for_statement", "foreach_statement", "declare_statement",
  "switch_case_list", "case_list", "case_separator", "while_statement",
  "if_stmt_without_else", "if_stmt", "alt_if_stmt_without_else",
  "alt_if_stmt", "parameter_list", "non_empty_parameter_list", "parameter",
  "optional_type", "type_expr", "type", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "class_statement", "name_list", "trait_adaptations",
  "trait_adaptation_list", "trait_adaptation", "trait_precedence",
  "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "class_const_list", "class_const_decl",
  "const_decl", "echo_expr_list", "echo_expr", "for_exprs",
  "non_empty_for_exprs", "anonymous_class", "@8", "new_expr", "expr",
  "inline_function", "fn", "function", "backup_doc_comment",
  "backup_fn_flags", "backup_lex_pos", "returns_ref", "lexical_vars",
  "lexical_var_list", "lexical_var", "function_call", "class_name",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "dereferencable_scalar", "scalar", "constant", "optional_expr",
  "variable_class_name", "dereferencable", "callable_expr",
  "callable_variable", "variable", "simple_variable", "static_member",
  "new_variable", "member_name", "property_name", "array_pair_list",
  "possible_array_pair", "non_empty_array_pair_list", "array_pair",
  "encaps_list", "encaps_var", "encaps_var_offset",
  "internal_functions_in_yacc", "isset_variables", "isset_variable", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    61,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,    63,
      58,   283,   284,   285,   124,    94,    38,   286,   287,   288,
     289,   290,    60,   291,    62,   292,   293,   294,    43,    45,
      46,    42,    47,    37,    33,   295,   126,   296,   297,   298,
     299,   300,   301,   302,    64,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,    40,
      41,    59,   123,   125,    44,    91,    93,    96,    34,    36
};
# endif

#define YYPACT_NINF -753

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-753)))

#define YYTABLE_NINF -477

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-477)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -753,   100,  1156,  -753,  5216,  5216,  5216,  5216,  5216,  5216,
    5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,
    5216,  5216,  5216,   206,  5216,  -753,  -753,    90,  -753,  -753,
    -753,   -29,   351,   351,    23,    37,  5216,  4084,    84,   123,
     158,   166,   171,  5216,  5216,   115,  -753,  -753,   264,  5216,
     201,  5216,    -5,     5,   209,  -753,  -753,   220,   239,   252,
     255,  -753,  -753,  -753,   263,   268,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,   234,    25,  -753,   298,  5216,  -753,  -753,
    4222,   327,   361,   -16,   238,   -55,  -753,  -753,  -753,  -753,
     -23,  -753,  -753,  -753,   399,  -753,   304,  -753,  -753,  5916,
    -753,   397,   397,  -753,   321,   205,  -753,   316,   333,    60,
     330,   338,  4567,  -753,  -753,  -753,  -753,   404,   348,  6958,
     397,  6958,  6958,  6958,  5440,  7046,  5440,   440,   440,    47,
     440,   440,   440,   440,   440,   440,   440,   440,   440,  -753,
    -753,  -753,  -753,   365,   330,  -753,   235,  -753,  -753,  5216,
    5216,  4222,   391,   316,   390,   390,  5216,  -753,  5216,   -12,
    -753,  6958,   463,  5216,  5216,  5216,   264,  5216,  6958,   392,
     398,   413,   545,   182,  -753,   419,  -753,  5965,  -753,  -753,
     298,   -18,    56,   423,   191,  -753,  -753,   262,  -753,  -753,
     553,   318,  -753,  -753,   351,  5216,  5216,   411,   509,   510,
     512,  4222,  4222,   -39,   231,  -753,  4498,   351,   341,  -753,
     298,   174,   424,   238,  6013,  1294,   351,   429,  5216,  6862,
     426,  -753,   430,  -753,    10,   428,   364,    10,   227,  5216,
    -753,   520,  4360,  -753,  -753,  -753,   437,  4084,   439,   579,
     450,  5216,  5216,  5216,  4664,  5216,  5216,  5216,  5216,  5216,
    5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,
    5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,   233,  5216,
    -753,  -753,   453,   540,  2978,  5216,  2978,    36,  5216,  5216,
    -753,  4802,  5216,  5216,  5216,  5216,  5216,  5216,  5216,  5216,
    5216,  5216,  5216,  5216,  5216,  -753,  -753,  -753,  5216,   330,
       5,  -753,  -753,    36,     5,  5216,  5216,  6062,  6110,   454,
     464,  6158,  -753,  5216,   455,  6206,   466,   459,  6958,  6796,
      70,  6254,  -753,  -753,  -753,  5216,  -753,   264,  -753,  1432,
    -753,    76,  -753,   552,   -20,   298,   119,   467,   322,  -753,
    -753,    59,  -753,     5,  5216,  -753,   557,   468,  -753,   198,
    6958,   473,  -753,  6302,   478,   514,  -753,   515,   485,   486,
     574,   421,  -753,  -753,   -74,  5580,   488,  -753,  -753,  -753,
     238,  -753,   487,  -753,   438,   493,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,   162,  4222,  6958,  4940,   638,  4222,  -753,
    -753,  5628,  -753,  5216,  -753,   491,  -753,  6958,   583,  5216,
    -753,  5216,  -753,  -753,  7005,  2962,  5440,  5216,  6910,  6648,
    6504,  7078,  7109,  4270,  4407,  4711,  4711,  4711,  4711,  4711,
    1008,  1008,  1008,  1008,   733,   733,    62,    62,    62,    47,
      47,    47,  -753,   440,    66,  -753,   499,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  5216,  -753,  -753,   500,   501,   330,   504,   500,
     501,   330,  -753,  5216,  -753,   330,  5676,   511,   351,  5440,
    5440,  5440,  5440,  5440,  5440,  5440,  5440,  5440,  5440,  5440,
    5440,  5440,  5440,  5440,   514,  -753,  -753,  -753,  5724,   513,
    -753,   506,  -753,  -753,  3282,  -753,  5216,  3448,  5216,  5216,
      51,  -753,    13,  6958,  -753,  -753,   -17,  -753,   250,   122,
      -3,  -753,  -753,   298,   190,  -753,  -753,  6958,  -753,   351,
     507,  5216,   521,  -753,  -753,    74,   536,   523,    74,  -753,
     660,  -753,  -753,   607,  -753,  -753,  -753,   525,  -753,  5216,
    -753,  -753,  -753,   880,   532,   533,   351,   537,  6958,  5216,
    -753,  -753,  6958,  5078,   535,   514,  6350,  6398,  1570,  6648,
    5216,   276,  -753,  -753,  -753,   538,   539,  -753,   663,  -753,
    -753,   542,    66,  5772,  -753,  -753,  -753,  5820,  -753,  -753,
    -753,   390,   536,  -753,  -753,  -753,  -753,  6446,  -753,  -753,
    -753,   541,  6958,   351,   546,  4222,    -6,    32,  3614,   548,
     550,  -753,  -753,   422,   250,   298,   543,  -753,  -753,     1,
     298,  -753,   551,  -753,  -753,  -753,    74,  -753,  -753,  -753,
     556,   544,  5216,  -753,  -753,   410,  1018,  -753,   554,   660,
     270,  4222,  5440,  -753,  -753,   536,  4084,   694,   564,  6648,
    -753,   696,    34,  -753,   571,    66,   568,  -753,  -753,  -753,
    3116,   572,  1708,  5216,    33,  4222,   581,    51,  3780,  -753,
    -753,  -753,  -753,   448,  -753,   -28,   575,   576,  -753,   543,
    -753,   250,   585,   298,   588,  -753,  -753,   556,   587,   407,
      74,  -753,  5440,   600,  -753,  -753,   604,  -753,  -753,  -753,
    -753,    34,  -753,  -753,  -753,   695,   605,   656,   608,  -753,
     610,   613,   614,    35,   615,  -753,  -753,  -753,  1846,   479,
     617,  5216,    17,    22,  -753,    74,  -753,   616,  -753,  -753,
     588,   298,   626,  -753,    74,  -753,  -753,  -753,  -753,  -753,
    -753,  -753,  -753,  -753,    34,   300,   711,  -753,  -753,   418,
    -753,   660,   628,  3116,  -753,   778,   777,   696,   635,   696,
    -753,  -753,  3946,  -753,  3780,  1984,   634,   643,  -753,  5868,
    -753,  -753,  -753,  -753,  -753,    42,  2122,  -753,   633,  -753,
    -753,   624,   -56,   725,  6524,   397,  -753,  -753,  -753,  -753,
    5216,  -753,    45,  -753,   639,  -753,  -753,  -753,  -753,   644,
    -753,  -753,  -753,  3116,    74,   640,  -753,  -753,  -753,  -753,
    5307,  -753,   799,   335,  -753,   804,   337,  -753,  6524,   691,
    -753,  6958,   658,   747,  -753,   212,  -753,   661,  -753,  2260,
    -753,  3116,  -753,   671,   726,   348,  -753,  -753,   682,  5445,
    -753,   674,   675,   741,   727,  5216,  -753,  -753,   725,  5216,
    -753,  6524,  -753,  -753,  5216,  -753,  -753,  -753,    45,  -753,
     678,  -753,  6524,  -753,  -753,  -753,  -753,  6662,    74,  6958,
    -753,  6958,  -753,   683,  6958,  2398,  -753,  2536,  -753,  2674,
    -753,  -753,  -753,  6524,   556,  -753,  -753,    66,  -753,  -753,
    -753,  -753,  -753,   681,  -753,  -753,   696,  -753,   379,  -753,
    -753,  -753,  2812,  -753,  -753
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      81,     0,     2,     1,     0,     0,     0,     0,     0,   375,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   414,   415,    82,   451,   146,
     413,     0,     0,     0,   404,     0,     0,     0,     0,     0,
       0,     0,     0,   433,   433,     0,   384,   383,     0,   433,
       0,     0,     0,     0,   400,   178,   179,     0,     0,     0,
       0,   174,   180,   182,     0,     0,   416,   417,   418,   423,
     419,   420,   421,     0,    96,   422,     0,     0,   153,   126,
     469,   406,     0,     0,    84,   430,    80,    87,    88,    89,
       0,   176,    90,    91,   213,   134,     0,   135,   358,     0,
     379,   388,   388,   447,     0,   428,   372,   429,     0,   435,
       0,   448,   303,   442,   449,   362,    82,   400,     0,   497,
     388,   498,   500,   501,   374,   376,   378,   343,   344,   345,
     346,   363,   364,   365,   366,   367,   368,   369,   371,   400,
     299,   401,   302,   402,   409,   456,   403,   308,   157,     0,
       0,   469,   438,     0,   323,   325,   433,   370,     0,     0,
     293,   294,     0,     0,   295,     0,     0,     0,   434,     0,
       0,     0,     0,     0,   124,     0,   126,     0,   103,   104,
       0,   119,     0,     0,     0,   121,   116,     0,   241,   242,
     245,     0,   244,   380,     0,     0,     0,     0,     0,     0,
       0,   469,   469,   484,     0,   425,     0,     0,     0,   482,
       0,    94,     0,    86,     0,     0,     0,     0,     0,   474,
       0,   472,   468,   470,   407,     0,   408,     0,     0,     0,
     453,     0,     0,   396,   172,   177,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     147,   389,     0,   385,     0,   433,     0,     0,     0,   433,
     399,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   322,   324,   385,     0,   409,
       0,   410,   301,     0,     0,     0,   433,     0,     0,     0,
       0,     0,   145,     0,     0,     0,     0,   296,   298,     0,
       0,     0,   140,   141,   156,     0,   102,     0,   142,     0,
     155,   119,   122,     0,     0,     0,   119,     0,     0,    98,
     100,     0,   143,     0,     0,   144,     0,   109,   164,   436,
     504,   109,   502,     0,     0,   184,   385,   186,     0,     0,
       0,     0,   424,   483,     0,     0,   436,   481,   427,   480,
      85,    93,     0,    81,   357,     0,   133,   125,   127,   128,
     129,   130,   131,   436,   469,   477,     0,   412,   469,   373,
     426,     0,    83,     0,   234,   109,   236,   238,     0,     0,
     214,     0,   126,   217,   328,   330,   329,     0,     0,   361,
     326,   327,   331,   333,   332,   349,   350,   347,   348,   355,
     351,   352,   353,   354,   341,   342,   335,   336,   334,   337,
     339,   340,   356,   338,   225,   385,     0,     3,     4,     6,
       7,     8,     9,    10,    46,    47,    11,    12,    13,    16,
      17,    78,     5,    14,    15,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    49,    50,    51,
      52,    53,    41,    42,    43,    70,    44,    45,    30,    31,
      32,    33,    34,    35,    36,    72,    73,    74,    75,    76,
      77,    37,    38,    39,    40,    61,    59,    60,    56,    57,
      48,    54,    55,    66,    67,    68,    62,    63,    65,    64,
      58,    69,     0,    71,    79,   431,   454,     0,     0,   432,
     455,     0,   465,     0,   467,   450,     0,     0,     0,   306,
     309,   310,   311,   313,   314,   315,   316,   317,   318,   319,
     320,   312,   321,   377,   184,   460,   459,   461,     0,     0,
     499,   437,   412,   405,     0,   292,     0,     0,   295,     0,
       0,   151,     0,   385,   123,   158,     0,   120,     0,   119,
       0,    99,   101,     0,   119,   115,   240,   246,   243,   110,
       0,   110,     0,   496,    92,     0,   188,     0,     0,   385,
       0,   411,   486,     0,   491,   494,   492,     0,   488,     0,
     487,   490,    81,     0,     0,     0,     0,     0,   473,     0,
     471,   452,   239,   110,     0,   184,     0,     0,     0,   360,
       0,     0,   229,   230,   231,     0,   219,   221,   168,   226,
     227,     0,   225,     0,   397,   444,   398,     0,   446,   445,
     443,   307,   188,   458,   457,   126,   211,     0,   126,   209,
     136,     0,   297,     0,     0,   469,     0,   436,     0,   204,
     204,   139,   291,   162,     0,     0,   109,   112,   117,     0,
       0,   165,     0,   503,   495,   185,     0,   385,   248,   253,
     187,     0,     0,   493,   485,     0,     0,    97,     0,   479,
     436,   469,   305,   237,   235,   188,     0,     0,     0,   359,
     228,   232,   225,   169,   170,   225,     0,   463,   466,   385,
     215,     0,     0,   295,   436,   469,     0,     0,     0,   126,
     198,   152,   204,     0,   204,     0,     0,     0,   154,   109,
     118,   110,     0,     0,   109,   114,   148,   189,     0,   274,
       0,   248,   304,     0,    95,   132,     0,   385,   212,   126,
     218,     0,   385,   222,   171,     0,     0,   390,     0,   137,
       0,     0,     0,   412,     0,   126,   196,   149,     0,     0,
       0,     0,     0,     0,   200,     0,   126,     0,   111,   107,
     109,   110,     0,   248,     0,   281,   282,   283,   280,   279,
     278,   273,   181,   247,   225,     0,   272,   276,   254,   274,
     489,   478,     0,   216,   233,     0,   223,   232,     0,   232,
     248,   210,     0,   192,     0,     0,     0,     0,   202,     0,
     207,   208,   126,   201,   160,     0,     0,   108,     0,   113,
     105,   274,     0,     0,     0,   388,   277,   183,   248,   386,
       0,   386,     0,   386,   274,   126,   194,   138,   150,     0,
     199,   203,   126,   206,     0,     0,   163,   106,   175,   255,
       0,   251,   385,     0,   285,     0,     0,   289,     0,   274,
     387,   224,     0,     0,   394,     0,   393,     0,   300,     0,
     197,   205,   161,     0,    82,    58,   256,   267,     0,     0,
     258,     0,     0,     0,   268,     0,   286,   249,     0,     0,
     250,     0,   385,   173,     0,   126,   395,   391,     0,   126,
       0,   126,     0,   257,   259,   260,   261,     0,     0,   385,
     284,   385,   288,     0,   386,     0,   392,     0,   195,     0,
     269,   263,   264,   266,   262,   287,   290,   225,   382,   386,
     386,   159,   265,     0,   167,   381,   232,   386,     0,   270,
     126,   386,     0,   252,   271
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -753,  -753,   -73,  -753,  -239,  -365,    -1,   -22,  -753,  -753,
    -753,   791,  -753,  -753,  -313,   181,   114,   666,   118,  -177,
     516,   684,  -174,  -753,     4,  -753,  -753,  -753,  -753,  -753,
     273,     3,  -753,  -753,    27,  -753,  -753,  -753,   763,    30,
    -753,    31,  -753,  -477,  -753,  -456,   138,  -753,    44,  -753,
    -753,  -591,    40,  -753,  -753,  -753,  -753,  -753,  -628,  -753,
     154,    68,   109,   242,  -743,   -45,  -753,   251,  -753,   522,
    -753,   524,  -687,  -753,  -640,  -753,  -753,   -21,  -753,  -753,
    -753,  -753,  -753,  -753,  -753,  -753,  -752,  -753,   -32,  -753,
     -30,   547,  -753,   559,  -519,  -753,  -753,  -753,  -753,     6,
      26,  -753,    -2,  -245,  -707,  -753,   -68,  -753,  -753,   -35,
    -753,     8,   601,  -753,  -753,   577,    29,  -753,   108,    41,
    -753,  -753,  -753,  -753,   144,    65,  -753,  -753,   599,   578,
    -142,   490,  -753,  -753,   462,   414,  -753,  -753,  -753,   299
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   513,   514,   865,     2,    84,    85,    86,   372,
     212,   665,   337,   183,   580,   666,   734,   184,   667,   185,
     186,   173,   215,   377,   378,   658,   663,   825,   728,   347,
     348,   379,   704,   755,   380,   398,   198,    90,    91,   381,
     199,   382,   200,   586,   589,   677,   656,   847,   767,   721,
     661,   723,   822,   650,    94,    95,    96,    97,   625,   626,
     627,   628,   629,   630,   752,   301,   395,   396,   187,   188,
     191,   192,   739,   793,   680,   861,   889,   890,   891,   892,
     893,   894,   951,   794,   795,   796,   797,   863,   864,   866,
     867,   174,   159,   160,   316,   317,   142,   299,    98,    99,
     100,   101,   120,   436,   870,   904,   272,   809,   875,   876,
     103,   104,   144,   157,   225,   302,   105,   106,   107,   169,
     108,   109,   110,   111,   112,   113,   114,   146,   517,   525,
     220,   221,   222,   223,   208,   209,   597,   115,   351,   352
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     102,   141,   329,   332,   706,    88,    87,   717,   603,   309,
     119,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,    89,
     147,   143,    92,    93,   273,   515,   737,   519,   582,   651,
     233,   162,   161,   659,   836,  -190,  -191,   820,  -193,   168,
     168,   181,   297,   392,   799,   168,   392,   177,    28,   358,
     359,   152,   152,   621,   841,   280,   843,   642,   116,   725,
     392,   771,   772,   211,   392,   213,   854,   756,   333,    28,
     193,   873,   614,   214,   203,   170,   219,   653,   145,   598,
     175,   599,   360,    55,    56,   621,   831,  -401,   116,   178,
       3,   179,   268,   234,   232,   859,   860,   116,   740,   522,
      28,   587,   269,   265,   266,   267,   855,   268,   189,   874,
     148,   771,   772,   844,   116,    28,   361,   269,    30,   116,
     149,   769,   116,   773,   872,   774,   877,   334,   695,   116,
     153,   153,   568,   193,   832,   664,   229,   116,   230,   312,
     180,   869,   313,    83,   718,   307,   308,   219,   332,   670,
     206,   207,   168,   733,   311,   933,   139,   622,   623,   315,
     318,   319,   333,   321,    83,   660,   154,   155,   821,   331,
     210,   336,   156,   654,    65,   823,   709,   118,   171,    76,
     631,   277,  -190,  -191,   761,  -193,   158,   310,   523,   622,
     623,   350,   353,   947,   118,    83,    76,   219,   219,   370,
     150,   335,   365,   102,   573,   333,   655,   938,   333,   118,
      83,    76,   278,   152,   385,   279,  -220,   118,   618,    76,
     561,   566,   944,   945,   327,   391,   152,   686,   397,   747,
     948,   400,   605,   163,   953,   152,   141,   404,   405,   406,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     418,   419,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   570,   433,   143,   669,   934,   116,
      28,   168,   164,   190,   526,   168,   333,   529,   530,   531,
     532,   533,   534,   535,   536,   537,   538,   539,   540,   541,
     542,   203,   153,   367,   543,   203,   116,    28,   203,   943,
     204,   548,   168,    46,    47,   153,   518,   165,   662,   161,
     527,   139,  -476,   116,   153,   166,  -476,   102,  -476,   231,
     167,   563,   140,   145,   569,   371,  -438,   172,   349,   516,
     574,   520,   524,   326,   681,   231,   327,   549,   139,   116,
     577,   366,   340,   732,   178,   341,   179,  -438,  -166,   118,
     383,    76,  -166,   176,  -441,   545,   303,  -438,   524,   547,
    -438,   116,   907,   238,   239,    83,   908,   206,   207,   194,
     362,   206,   207,   205,   206,   207,   118,   304,    76,   240,
     219,   668,   608,   231,   219,   390,   332,   305,   195,   612,
     306,   203,    83,   224,    46,   616,   834,   617,   189,   622,
     623,   196,   624,   619,   197,   203,   777,   367,   241,   242,
     243,   782,   201,   342,   116,    28,   343,   202,    30,   118,
    -475,    76,   738,   271,  -475,   203,  -475,   227,   203,   244,
     367,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   758,   268,   139,   828,   236,   237,
     593,   710,   634,   274,   712,   269,   636,   206,   207,   345,
     638,   275,   346,   572,    65,   276,   341,   668,   730,   232,
     368,   206,   207,   735,   594,   595,   897,  -439,   900,   898,
     596,   901,   802,   210,   118,   269,    76,   805,    46,    47,
     150,   206,   207,   716,   206,   207,   151,   300,   633,   784,
      83,  -436,   785,   786,   787,   788,   789,   790,   791,   637,
     784,   726,   727,   785,   786,   787,   788,   789,   790,   791,
     949,   950,  -436,   226,   228,   768,   770,   771,   772,   746,
    -441,   314,  -436,   322,   668,  -436,   735,   152,   646,   323,
     325,   649,   647,   675,   318,   652,   679,   574,   344,  -437,
     792,   354,   574,   762,   324,   803,   743,   817,   771,   772,
     328,   837,   355,   356,   339,   357,   373,   350,   384,   152,
    -437,   815,   387,   392,   388,   389,   399,  -440,   401,   624,
    -437,   102,   826,  -437,   829,   685,    88,    87,   152,   402,
     624,   403,   434,   435,   556,   692,   102,   896,   363,   397,
     552,   887,   369,   559,   553,   567,   699,   558,   571,   902,
      89,   190,   579,    92,    93,   152,   153,   581,   363,   584,
     369,   363,   369,   585,   588,   590,   591,   592,   853,   602,
     887,   601,   604,   609,   679,   613,   615,   923,   632,  -462,
    -464,   219,   720,   574,   574,  -440,   676,   672,   153,   574,
     635,   879,   641,   930,   935,   682,   936,   640,   881,   644,
     624,   674,   152,   624,   102,   678,   683,   153,   742,    88,
      87,   684,   688,   689,   942,   694,   691,   219,   701,   703,
     748,   705,   713,   702,   657,   715,   741,   731,   102,   722,
     102,   724,   736,    89,   153,   745,    92,    93,   798,   318,
     740,   219,   766,   349,   749,   750,   751,   754,   757,   624,
     574,   925,   574,   759,   775,   927,   784,   929,   776,   785,
     786,   787,   788,   789,   790,   791,   152,   763,   779,   783,
     690,   784,   781,   824,   785,   786,   787,   788,   789,   790,
     791,   153,   679,   800,   801,   807,   102,   868,   808,   806,
     810,   811,   624,   812,   813,   814,   952,   819,   818,   827,
     574,   262,   263,   264,   265,   266,   267,   858,   268,   830,
     838,   839,   840,   835,   842,   850,   857,   714,   269,   862,
     883,   102,   878,   784,   851,   880,   785,   786,   787,   788,
     789,   790,   791,   102,   895,  -275,   846,  -275,   766,   899,
     905,   906,   -78,   909,   102,   153,   785,   786,   787,   788,
     789,   790,   882,   911,   912,   915,   916,   917,   888,   928,
     918,   946,   937,   182,   932,   729,   871,   780,   338,   778,
     320,   102,   671,   235,   903,   764,   753,   575,   848,   852,
     804,   657,   833,   700,   693,   576,   920,   888,   914,   432,
     578,   922,   555,   926,   564,   521,   544,   102,   610,   102,
     673,   546,     0,     0,     4,     5,     6,     7,     0,     0,
       0,     8,     9,     0,    10,     0,   679,     0,     0,     0,
       0,   919,     0,     0,     0,   921,     0,     0,     0,     0,
     924,     0,     0,     0,     0,   624,     0,     0,     0,     0,
       0,     0,     0,   102,     0,   102,     0,   102,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
     102,    25,    26,    27,    28,    29,     0,    30,     0,     0,
      31,    32,    33,    34,    35,     0,    36,    37,    38,     0,
      39,     0,    40,     0,    41,     0,     0,    42,     0,     0,
       0,    43,    44,    45,    46,    47,    48,    49,    50,     0,
       0,    51,    52,     0,    53,    54,    55,    56,     0,     0,
       0,     0,    57,    58,    59,    60,    61,    62,    63,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     4,     5,     6,     7,     0,     0,    73,     8,
       9,     0,    10,    74,    75,    76,     0,     0,     0,    77,
       0,    78,    79,   687,     0,    80,     0,    81,    82,    83,
    -477,  -477,  -477,  -477,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,     0,     0,    11,    12,     0,     0,
       0,     0,    13,   269,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,    27,    28,    29,     0,    30,     0,     0,    31,    32,
      33,    34,    35,     0,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,    47,    48,    49,    50,     0,     0,    51,
      52,     0,    53,    54,    55,    56,     0,     0,     0,     0,
      57,    58,    59,    60,    61,    62,    63,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       4,     5,     6,     7,     0,     0,    73,     8,     9,     0,
      10,    74,    75,    76,     0,     0,     0,    77,     0,    78,
      79,   744,     0,    80,     0,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,    27,
      28,    29,     0,    30,     0,     0,    31,    32,    33,    34,
      35,     0,    36,    37,    38,     0,    39,     0,    40,     0,
      41,     0,     0,    42,     0,     0,     0,    43,    44,    45,
      46,    47,    48,    49,    50,     0,     0,    51,    52,     0,
      53,    54,    55,    56,     0,     0,     0,     0,    57,    58,
      59,    60,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     4,     5,
       6,     7,     0,     0,    73,     8,     9,     0,    10,    74,
      75,    76,     0,     0,     0,    77,     0,    78,    79,     0,
       0,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
      23,    24,     0,     0,     0,    25,    26,    27,    28,    29,
       0,    30,     0,     0,    31,    32,    33,    34,    35,     0,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,    47,
       0,    49,    50,     0,     0,    51,     0,     0,    53,    54,
      55,    56,     0,     0,     0,     0,    57,    58,    59,   375,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     4,     5,     6,     7,
       0,     0,    73,     8,     9,     0,    10,   118,    75,    76,
       0,     0,     0,    77,     0,    78,    79,   376,     0,    80,
       0,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,    27,    28,    29,     0,    30,
       0,     0,    31,    32,    33,    34,    35,     0,    36,    37,
      38,     0,    39,     0,    40,     0,    41,     0,     0,    42,
       0,     0,     0,    43,    44,    45,    46,    47,     0,    49,
      50,     0,     0,    51,     0,     0,    53,    54,    55,    56,
       0,     0,     0,     0,    57,    58,    59,   375,    61,    62,
      63,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     4,     5,     6,     7,     0,     0,
      73,     8,     9,     0,    10,   118,    75,    76,     0,     0,
       0,    77,     0,    78,    79,   565,     0,    80,     0,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
       0,    25,    26,    27,    28,    29,     0,    30,     0,     0,
      31,    32,    33,    34,    35,   698,    36,    37,    38,     0,
      39,     0,    40,     0,    41,     0,     0,    42,     0,     0,
       0,    43,    44,    45,    46,    47,     0,    49,    50,     0,
       0,    51,     0,     0,    53,    54,    55,    56,     0,     0,
       0,     0,    57,    58,    59,   375,    61,    62,    63,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     4,     5,     6,     7,     0,     0,    73,     8,
       9,     0,    10,   118,    75,    76,     0,     0,     0,    77,
       0,    78,    79,     0,     0,    80,     0,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,    27,    28,    29,     0,    30,     0,     0,    31,    32,
      33,    34,    35,     0,    36,    37,    38,   760,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,    47,     0,    49,    50,     0,     0,    51,
       0,     0,    53,    54,    55,    56,     0,     0,     0,     0,
      57,    58,    59,   375,    61,    62,    63,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       4,     5,     6,     7,     0,     0,    73,     8,     9,     0,
      10,   118,    75,    76,     0,     0,     0,    77,     0,    78,
      79,     0,     0,    80,     0,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,    27,
      28,    29,     0,    30,     0,     0,    31,    32,    33,    34,
      35,     0,    36,    37,    38,     0,    39,     0,    40,     0,
      41,   816,     0,    42,     0,     0,     0,    43,    44,    45,
      46,    47,     0,    49,    50,     0,     0,    51,     0,     0,
      53,    54,    55,    56,     0,     0,     0,     0,    57,    58,
      59,   375,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     4,     5,
       6,     7,     0,     0,    73,     8,     9,     0,    10,   118,
      75,    76,     0,     0,     0,    77,     0,    78,    79,     0,
       0,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
      23,    24,     0,     0,     0,    25,    26,    27,    28,    29,
       0,    30,     0,     0,    31,    32,    33,    34,    35,     0,
      36,    37,    38,     0,    39,     0,    40,   849,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,    47,
       0,    49,    50,     0,     0,    51,     0,     0,    53,    54,
      55,    56,     0,     0,     0,     0,    57,    58,    59,   375,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     4,     5,     6,     7,
       0,     0,    73,     8,     9,     0,    10,   118,    75,    76,
       0,     0,     0,    77,     0,    78,    79,     0,     0,    80,
       0,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,    27,    28,    29,     0,    30,
       0,     0,    31,    32,    33,    34,    35,     0,    36,    37,
      38,     0,    39,     0,    40,     0,    41,     0,     0,    42,
       0,     0,     0,    43,    44,    45,    46,    47,     0,    49,
      50,     0,     0,    51,     0,     0,    53,    54,    55,    56,
       0,     0,     0,     0,    57,    58,    59,   375,    61,    62,
      63,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     4,     5,     6,     7,     0,     0,
      73,     8,     9,     0,    10,   118,    75,    76,     0,     0,
       0,    77,     0,    78,    79,   856,     0,    80,     0,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
       0,    25,    26,    27,    28,    29,     0,    30,     0,     0,
      31,    32,    33,    34,    35,     0,    36,    37,    38,     0,
      39,   910,    40,     0,    41,     0,     0,    42,     0,     0,
       0,    43,    44,    45,    46,    47,     0,    49,    50,     0,
       0,    51,     0,     0,    53,    54,    55,    56,     0,     0,
       0,     0,    57,    58,    59,   375,    61,    62,    63,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     4,     5,     6,     7,     0,     0,    73,     8,
       9,     0,    10,   118,    75,    76,     0,     0,     0,    77,
       0,    78,    79,     0,     0,    80,     0,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,    27,    28,    29,     0,    30,     0,     0,    31,    32,
      33,    34,    35,     0,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,    47,     0,    49,    50,     0,     0,    51,
       0,     0,    53,    54,    55,    56,     0,     0,     0,     0,
      57,    58,    59,   375,    61,    62,    63,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       4,     5,     6,     7,     0,     0,    73,     8,     9,     0,
      10,   118,    75,    76,     0,     0,     0,    77,     0,    78,
      79,   939,     0,    80,     0,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,    27,
      28,    29,     0,    30,     0,     0,    31,    32,    33,    34,
      35,     0,    36,    37,    38,     0,    39,     0,    40,     0,
      41,     0,     0,    42,     0,     0,     0,    43,    44,    45,
      46,    47,     0,    49,    50,     0,     0,    51,     0,     0,
      53,    54,    55,    56,     0,     0,     0,     0,    57,    58,
      59,   375,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     4,     5,
       6,     7,     0,     0,    73,     8,     9,     0,    10,   118,
      75,    76,     0,     0,     0,    77,     0,    78,    79,   940,
       0,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
      23,    24,     0,     0,     0,    25,    26,    27,    28,    29,
       0,    30,     0,     0,    31,    32,    33,    34,    35,     0,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,    47,
       0,    49,    50,     0,     0,    51,     0,     0,    53,    54,
      55,    56,     0,     0,     0,     0,    57,    58,    59,   375,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     4,     5,     6,     7,
       0,     0,    73,     8,     9,     0,    10,   118,    75,    76,
       0,     0,     0,    77,     0,    78,    79,   941,     0,    80,
       0,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,    27,    28,    29,     0,    30,
       0,     0,    31,    32,    33,    34,    35,     0,    36,    37,
      38,     0,    39,     0,    40,     0,    41,     0,     0,    42,
       0,     0,     0,    43,    44,    45,    46,    47,     0,    49,
      50,     0,     0,    51,     0,     0,    53,    54,    55,    56,
       0,     0,     0,     0,    57,    58,    59,   375,    61,    62,
      63,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
      73,     0,     0,     0,     0,   118,    75,    76,     0,     0,
       0,    77,   243,    78,    79,   954,     0,    80,     0,    81,
      82,    83,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   244,     0,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,     0,   268,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,     0,     0,
       0,     0,     0,   446,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,   448,     0,   449,   450,     0,
       0,   451,    28,     0,     0,     0,     0,     0,   452,     0,
       0,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,     0,   495,   496,   497,   498,   499,     0,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
       4,     5,     6,     7,     0,     0,     0,     8,     9,     0,
      10,   510,   511,     0,     0,     0,     0,     0,     0,     0,
     512,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,    27,
      28,    29,     0,    30,     0,     0,    31,    32,    33,    34,
      35,     0,    36,    37,    38,     0,    39,     0,    40,     0,
      41,     0,     0,    42,     0,     0,     0,    43,    44,    45,
      46,    47,     0,    49,    50,     0,     0,    51,     0,     0,
      53,    54,    55,    56,     0,     0,     0,     0,    57,    58,
      59,   375,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,   118,
      75,    76,     0,     0,     0,    77,     0,    78,    79,     0,
       0,    80,     0,    81,    82,    83,     4,     5,     6,     7,
       0,     0,     0,     8,     9,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   645,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,    27,    28,    29,     0,    30,
       0,     0,    31,    32,    33,    34,    35,     0,    36,    37,
      38,     0,    39,     0,    40,     0,    41,     0,     0,    42,
       0,     0,     0,    43,    44,    45,    46,    47,     0,    49,
      50,     0,     0,    51,     0,     0,    53,    54,     0,     0,
       0,     0,     0,     0,    57,    58,    59,     0,     0,     0,
       0,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
      73,     0,     0,     0,     0,   118,    75,    76,     0,     0,
       0,    77,     0,    78,    79,     0,     0,    80,     0,    81,
      82,    83,     4,     5,     6,     7,     0,     0,     0,     8,
       9,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   648,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,    27,    28,    29,     0,    30,     0,     0,    31,    32,
      33,    34,    35,     0,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,    47,     0,    49,    50,     0,     0,    51,
       0,     0,    53,    54,     0,     0,     0,     0,     0,     0,
      57,    58,    59,     0,     0,     0,     0,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,    73,     0,     0,     0,
       0,   118,    75,    76,     0,     0,     0,    77,     0,    78,
      79,     0,     0,    80,     0,    81,    82,    83,     4,     5,
       6,     7,     0,     0,     0,     8,     9,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   719,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
      23,    24,     0,     0,     0,    25,    26,    27,    28,    29,
       0,    30,     0,     0,    31,    32,    33,    34,    35,     0,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,    47,
       0,    49,    50,     0,     0,    51,     0,     0,    53,    54,
       0,     0,     0,     0,     0,     0,    57,    58,    59,     0,
       0,     0,     0,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,    73,     0,     0,     0,     0,   118,    75,    76,
       0,     0,     0,    77,     0,    78,    79,     0,     0,    80,
       0,    81,    82,    83,     4,     5,     6,     7,     0,     0,
       0,     8,     9,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     765,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
       0,    25,    26,    27,    28,    29,     0,    30,     0,     0,
      31,    32,    33,    34,    35,     0,    36,    37,    38,     0,
      39,     0,    40,     0,    41,     0,     0,    42,     0,     0,
       0,    43,    44,    45,    46,    47,     0,    49,    50,     0,
       0,    51,     0,     0,    53,    54,     0,     0,     0,     0,
       0,     0,    57,    58,    59,     0,     0,     0,     0,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,   118,    75,    76,     0,     0,     0,    77,
       0,    78,    79,     0,     0,    80,     0,    81,    82,    83,
       4,     5,     6,     7,     0,     0,     0,     8,     9,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   845,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,    27,
      28,    29,     0,    30,     0,     0,    31,    32,    33,    34,
      35,     0,    36,    37,    38,     0,    39,     0,    40,     0,
      41,     0,     0,    42,     0,     0,     0,    43,    44,    45,
      46,    47,     0,    49,    50,     0,     0,    51,     0,     0,
      53,    54,     0,     0,     0,     0,     0,     0,    57,    58,
      59,     0,     0,     0,     0,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     4,     5,
       6,     7,     0,     0,    73,     8,     9,     0,    10,   118,
      75,    76,     0,     0,     0,    77,     0,    78,    79,     0,
       0,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
      23,    24,     0,     0,     0,    25,    26,    27,    28,    29,
       0,    30,     0,     0,    31,    32,    33,    34,    35,     0,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,    47,
       0,    49,    50,     0,     0,    51,     0,     0,    53,    54,
       0,     0,     0,     0,     0,     0,    57,    58,    59,     0,
       0,     0,     0,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     4,     5,     6,     7,
       0,     0,    73,     8,     9,     0,    10,   118,    75,    76,
       0,     0,     0,    77,     0,    78,    79,     0,     0,    80,
       0,    81,    82,    83,     0,     0,     0,     0,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,   116,    28,     0,     0,    30,
       0,     0,    31,    32,    33,    34,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,    46,    47,     0,     0,
       0,     0,     0,     0,     0,   269,     0,   117,     0,     0,
       0,     0,     0,     0,     0,    58,    59,     0,     0,     0,
       0,     0,     0,     0,   217,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     4,     5,     6,     7,     0,     0,
      73,     8,     9,     0,    10,   118,    75,    76,   218,     0,
       0,    77,     0,     0,     0,     0,     0,    80,     0,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
       0,    25,    26,   116,    28,     0,     0,    30,     0,     0,
      31,    32,    33,    34,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,     0,   268,     0,    46,    47,     0,     0,     0,     0,
       0,     0,   269,     0,     0,   117,     0,     0,     0,     0,
       0,     0,     0,    58,    59,     0,     0,     0,     0,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     4,     5,     6,     7,     0,     0,    73,     8,
       9,     0,    10,   118,    75,    76,   393,     0,     0,    77,
     394,     0,     0,     0,     0,    80,     0,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,   116,    28,     0,     0,    30,   364,     0,    31,    32,
      33,    34,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,     0,     0,     0,     0,     0,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,    73,     0,   295,   296,
       0,   118,    75,    76,     0,     0,     0,    77,     0,     0,
       0,     0,     0,    80,     0,    81,    82,    83,     4,     5,
       6,     7,     0,     0,     0,     8,     9,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   407,     0,     0,     0,  -436,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,  -436,
      14,    15,    16,    17,    18,    19,    20,    21,    22,  -436,
      23,    24,  -436,     0,     0,    25,    26,   116,    28,     0,
       0,    30,     0,     0,    31,    32,    33,    34,  -477,  -477,
    -477,  -477,  -477,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,     0,   268,     0,    46,    47,
       0,     0,     0,     0,     0,     0,   269,     0,     0,   117,
       0,     0,     0,     0,     0,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     4,     5,     6,     7,
       0,     0,    73,     8,     9,     0,    10,   118,    75,    76,
       0,     0,     0,    77,     0,     0,     0,     0,     0,    80,
       0,    81,    82,    83,     0,     0,     0,     0,   528,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,    23,    24,
       0,     0,     0,    25,    26,   116,    28,     0,     0,    30,
       0,     0,    31,    32,    33,    34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    46,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,    58,    59,     0,     0,     0,
       0,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     4,     5,     6,     7,     0,     0,
      73,     8,     9,     0,    10,   118,    75,    76,     0,     0,
       0,    77,     0,     0,     0,     0,     0,    80,     0,    81,
      82,    83,     0,     0,     0,     0,   606,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,    23,    24,     0,     0,
       0,    25,    26,   116,    28,     0,     0,    30,     0,     0,
      31,    32,    33,    34,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   117,     0,     0,     0,     0,
       0,     0,     0,    58,    59,     0,     0,     0,     0,     0,
       0,     0,   607,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     4,     5,     6,     7,     0,     0,    73,     8,
       9,     0,    10,   118,    75,    76,     0,     0,     0,    77,
       0,     0,     0,     0,     0,    80,     0,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,    23,    24,     0,     0,     0,    25,
      26,   116,    28,     0,     0,    30,     0,     0,    31,    32,
      33,    34,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   117,     0,     0,     0,     0,     0,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       4,     5,     6,     7,     0,     0,    73,     8,     9,     0,
      10,   118,    75,    76,   393,     0,     0,    77,     0,     0,
       0,     0,     0,    80,     0,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,    23,    24,     0,     0,     0,    25,    26,   116,
      28,     0,     0,    30,     0,     0,    31,    32,    33,    34,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   437,   438,   439,   440,   441,   442,   443,   444,   445,
      46,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,     0,     0,     0,     0,     0,     0,     0,    58,
      59,     0,     0,     0,     0,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,   446,     0,    73,     0,     0,     0,     0,   118,
      75,    76,     0,   447,   448,    77,   449,   450,     0,     0,
     884,    80,     0,    81,    82,    83,     0,   452,     0,     0,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,     0,   495,   496,   497,   498,   499,     0,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   437,
     438,   439,   440,   441,   442,   443,   444,   445,     0,     0,
     885,   511,    76,     0,     0,     0,     0,     0,     0,   244,
     886,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,     0,     0,     0,     0,
     446,     0,     0,     0,     0,   269,     0,     0,     0,     0,
       0,   447,   448,     0,   449,   450,     0,     0,   884,     0,
       0,     0,     0,     0,     0,   452,     0,     0,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
       0,   495,   496,   497,   498,   499,     0,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   509,     0,   241,   242,
     243,     0,     0,     0,     0,     0,     0,     0,   885,   511,
      76,     0,     0,     0,     0,     0,     0,     0,   913,   244,
       0,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,   241,   242,   243,     0,
       0,     0,     0,     0,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,   241,   242,   243,     0,     0,     0,
       0,     0,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,   241,   242,   243,     0,     0,     0,     0,     0,
       0,   269,     0,   600,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   244,     0,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,     0,   268,
     241,   242,   243,     0,     0,     0,     0,     0,     0,   269,
       0,   611,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   244,     0,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,     0,   268,   241,   242,
     243,     0,     0,     0,     0,     0,     0,   269,     0,   639,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
       0,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,   241,   242,   243,     0,
       0,     0,     0,     0,     0,   269,     0,   643,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,   820,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,   241,   242,   243,     0,     0,     0,
       0,     0,     0,   269,     0,   707,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,     0,   241,   242,   243,     0,     0,     0,     0,
       0,   269,     0,   708,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,     0,
     268,   241,   242,   243,     0,     0,     0,     0,     0,   821,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   244,     0,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,     0,   268,     0,
     241,   242,   243,     0,     0,     0,     0,   270,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   244,     0,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,     0,   268,   241,   242,
     243,     0,     0,     0,     0,     0,   330,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
       0,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,   241,   242,   243,     0,
       0,     0,     0,   374,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,   241,   242,   243,     0,     0,     0,
       0,     0,   550,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,   241,   242,   243,     0,     0,     0,     0,     0,
     551,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   244,     0,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,     0,   268,
     241,   242,   243,     0,     0,     0,     0,     0,   554,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   244,     0,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,     0,   268,   241,   242,
     243,     0,     0,     0,     0,     0,   557,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
       0,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,   241,   242,   243,     0,
       0,     0,     0,     0,   562,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,   241,   242,   243,     0,     0,     0,
       0,     0,   583,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,     0,     0,     0,     0,     0,     0,     0,     0,
     696,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   697,   268,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   446,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     447,   448,     0,   449,   450,     0,     0,   451,     0,     0,
       0,     0,     0,     0,   452,     0,   711,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,     0,
     495,   496,   497,   498,   499,     0,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,     0,   510,   511,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,     0,     0,     0,   446,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   447,   448,
       0,   449,   450,     0,     0,   931,     0,     0,     0,     0,
       0,     0,   452,     0,     0,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   785,   786,   787,
     788,   789,   790,   491,   492,   493,   494,     0,   495,   496,
     497,   498,   499,     0,   500,   501,   502,   503,   504,   505,
     506,   507,   508,   509,   241,   242,   243,     0,     0,     0,
       0,     0,     0,     0,     0,   510,   511,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
     241,   242,   243,     0,     0,   386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   244,   560,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,     0,   268,   241,   242,
     243,     0,     0,     0,     0,     0,     0,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   244,
     620,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,     0,   268,   241,   242,   243,     0,
       0,     0,     0,     0,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,   242,   243,     0,     0,     0,     0,
       0,     0,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   298,
     268,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,     0,     0,     0,     0,   244,     0,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
       0,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,     0,   268,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,     0,   268,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269
};

static const yytype_int16 yycheck[] =
{
       2,    23,   176,   180,   632,     2,     2,    13,   373,   151,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     2,
      24,    23,     2,     2,   102,   274,   676,   276,   351,   558,
      85,    37,    36,    30,   796,    13,    13,    30,    13,    43,
      44,    52,   120,    73,   741,    49,    73,    51,    74,   201,
     202,    32,    33,    29,   807,   110,   809,   544,    73,   660,
      73,    99,   100,    74,    73,    76,    34,   705,    96,    74,
      54,    36,   395,    77,    74,    44,    80,    36,    23,   163,
      49,   165,   131,   116,   117,    29,   783,   152,    73,   104,
       0,   106,    55,   126,   159,   161,   162,    73,   164,    73,
      74,   356,    65,    51,    52,    53,    74,    55,    53,    74,
      30,    99,   100,   810,    73,    74,   165,    65,    77,    73,
     159,   722,    73,   724,   841,   163,   843,   155,   615,    73,
      32,    33,   162,   117,   784,   162,   162,    73,    83,   161,
     155,   838,   164,   169,   160,   149,   150,   151,   335,   162,
     150,   151,   156,   162,   158,   917,   115,   133,   134,   163,
     164,   165,    96,   167,   169,   162,    32,    33,   161,   180,
     155,   182,   159,   132,   133,   163,   642,   153,    73,   155,
     435,   131,   160,   160,   713,   160,   159,   156,   162,   133,
     134,   195,   196,   946,   153,   169,   155,   201,   202,   210,
     159,   155,   206,   215,   155,    96,   165,   924,    96,   153,
     169,   155,   162,   194,   218,   165,   160,   153,   402,   155,
     160,   155,   939,   940,   164,   229,   207,   602,   232,   695,
     947,   237,   384,   159,   951,   216,   268,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   155,   269,   268,   155,   918,    73,
      74,   275,   159,    74,   278,   279,    96,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,    74,   194,    76,   298,    74,    73,    74,    74,   937,
      76,   305,   306,   104,   105,   207,   275,   159,   563,   313,
     279,   115,   160,    73,   216,   159,   164,   329,   166,   155,
     159,   325,   126,   268,   335,   161,   131,    73,   194,   274,
     341,   276,   277,   161,   589,   155,   164,   306,   115,    73,
     344,   207,   161,   666,   104,   164,   106,   152,   160,   153,
     216,   155,   164,   162,   159,   300,   131,   162,   303,   304,
     165,    73,   160,    69,    70,   169,   164,   150,   151,   159,
     149,   150,   151,   149,   150,   151,   153,   152,   155,    85,
     384,   568,   386,   155,   388,   168,   573,   162,   159,   393,
     165,    74,   169,    76,   104,   399,   106,   401,   343,   133,
     134,   159,   434,   407,   159,    74,   729,    76,     8,     9,
      10,   734,   159,   161,    73,    74,   164,   159,    77,   153,
     160,   155,   677,    36,   164,    74,   166,    76,    74,    29,
      76,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,   709,    55,   115,   780,    69,    70,
      49,   645,   517,   152,   648,    65,   521,   150,   151,   161,
     525,   165,   164,   161,   133,   152,   164,   664,   665,   159,
     149,   150,   151,   670,    73,    74,   161,   159,   161,   164,
      79,   164,   747,   155,   153,    65,   155,   752,   104,   105,
     159,   150,   151,   655,   150,   151,   165,   152,   512,   112,
     169,   131,   115,   116,   117,   118,   119,   120,   121,   523,
     112,   109,   110,   115,   116,   117,   118,   119,   120,   121,
     161,   162,   152,    81,    82,   719,    98,    99,   100,   691,
     159,    88,   162,   161,   731,   165,   733,   528,   554,   161,
      15,   557,   556,   585,   558,   559,   588,   568,    15,   131,
     163,   160,   573,   715,   161,   749,   166,    98,    99,   100,
     161,   163,    73,    73,   161,    73,   162,   581,   159,   560,
     152,   765,   166,    73,   164,   167,   159,   159,   159,   621,
     162,   603,   776,   165,   781,   599,   603,   603,   579,    30,
     632,   161,   159,    73,   159,   609,   618,   862,   204,   613,
     166,   860,   208,   164,   160,    73,   620,   161,   161,   868,
     603,    74,   164,   603,   603,   606,   528,   164,   224,   161,
     226,   227,   228,   129,   129,   160,   160,    73,   822,   162,
     889,   163,   159,    15,   676,   164,    73,   902,   159,   159,
     159,   655,   658,   664,   665,   159,   130,   160,   560,   670,
     166,   845,   528,   912,   919,    15,   921,   166,   852,   166,
     702,   160,   653,   705,   686,   162,    79,   579,   682,   686,
     686,   166,   160,   160,   933,   160,   159,   691,   160,    36,
     696,   159,   161,   164,   560,   159,   162,   164,   710,   161,
     712,   161,   161,   686,   606,   161,   686,   686,   740,   713,
     164,   715,   718,   579,    30,   161,    30,   156,   160,   751,
     731,   905,   733,   161,   159,   909,   112,   911,   162,   115,
     116,   117,   118,   119,   120,   121,   717,   166,   163,   162,
     606,   112,   164,   775,   115,   116,   117,   118,   119,   120,
     121,   653,   784,   163,   160,   160,   768,   835,   112,    74,
     162,   161,   794,   160,   160,   160,   950,   771,   161,   163,
     781,    48,    49,    50,    51,    52,    53,   163,    55,   163,
     162,    13,    15,   795,   159,   161,   163,   653,    65,    74,
     160,   803,   163,   112,   161,   161,   115,   116,   117,   118,
     119,   120,   121,   815,    15,   104,   812,   106,   814,    15,
     162,    74,    96,   162,   826,   717,   115,   116,   117,   118,
     119,   120,   854,   162,   152,   161,   161,    96,   860,   161,
     113,   160,   159,    52,   917,   664,   840,   733,   182,   731,
     166,   853,   579,    90,   163,   717,   702,   341,   814,   819,
     751,   717,   794,   621,   613,   343,   898,   889,   889,   268,
     346,   901,   313,   908,   327,   276,   299,   879,   388,   881,
     581,   303,    -1,    -1,     4,     5,     6,     7,    -1,    -1,
      -1,    11,    12,    -1,    14,    -1,   918,    -1,    -1,    -1,
      -1,   895,    -1,    -1,    -1,   899,    -1,    -1,    -1,    -1,
     904,    -1,    -1,    -1,    -1,   937,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   925,    -1,   927,    -1,   929,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
     952,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,   105,   106,   107,   108,    -1,
      -1,   111,   112,    -1,   114,   115,   116,   117,    -1,    -1,
      -1,    -1,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,     4,     5,     6,     7,    -1,    -1,   148,    11,
      12,    -1,    14,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,   161,   162,   163,    -1,   165,    -1,   167,   168,   169,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    65,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,   105,   106,   107,   108,    -1,    -1,   111,
     112,    -1,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
       4,     5,     6,     7,    -1,    -1,   148,    11,    12,    -1,
      14,   153,   154,   155,    -1,    -1,    -1,   159,    -1,   161,
     162,   163,    -1,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,   108,    -1,    -1,   111,   112,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,     4,     5,
       6,     7,    -1,    -1,   148,    11,    12,    -1,    14,   153,
     154,   155,    -1,    -1,    -1,   159,    -1,   161,   162,    -1,
      -1,   165,    -1,   167,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
      -1,   107,   108,    -1,    -1,   111,    -1,    -1,   114,   115,
     116,   117,    -1,    -1,    -1,    -1,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
      -1,    -1,   148,    11,    12,    -1,    14,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,   161,   162,   163,    -1,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,    -1,   107,
     108,    -1,    -1,   111,    -1,    -1,   114,   115,   116,   117,
      -1,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,     4,     5,     6,     7,    -1,    -1,
     148,    11,    12,    -1,    14,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,   161,   162,   163,    -1,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,   105,    -1,   107,   108,    -1,
      -1,   111,    -1,    -1,   114,   115,   116,   117,    -1,    -1,
      -1,    -1,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,     4,     5,     6,     7,    -1,    -1,   148,    11,
      12,    -1,    14,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,   161,   162,    -1,    -1,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    86,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,   105,    -1,   107,   108,    -1,    -1,   111,
      -1,    -1,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
       4,     5,     6,     7,    -1,    -1,   148,    11,    12,    -1,
      14,   153,   154,   155,    -1,    -1,    -1,   159,    -1,   161,
     162,    -1,    -1,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    95,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,    -1,   107,   108,    -1,    -1,   111,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,     4,     5,
       6,     7,    -1,    -1,   148,    11,    12,    -1,    14,   153,
     154,   155,    -1,    -1,    -1,   159,    -1,   161,   162,    -1,
      -1,   165,    -1,   167,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    92,    93,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
      -1,   107,   108,    -1,    -1,   111,    -1,    -1,   114,   115,
     116,   117,    -1,    -1,    -1,    -1,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
      -1,    -1,   148,    11,    12,    -1,    14,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,   161,   162,    -1,    -1,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,    -1,   107,
     108,    -1,    -1,   111,    -1,    -1,   114,   115,   116,   117,
      -1,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,     4,     5,     6,     7,    -1,    -1,
     148,    11,    12,    -1,    14,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,   161,   162,   163,    -1,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    86,    87,    88,    -1,
      90,    91,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,   105,    -1,   107,   108,    -1,
      -1,   111,    -1,    -1,   114,   115,   116,   117,    -1,    -1,
      -1,    -1,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,     4,     5,     6,     7,    -1,    -1,   148,    11,
      12,    -1,    14,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,   161,   162,    -1,    -1,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,   105,    -1,   107,   108,    -1,    -1,   111,
      -1,    -1,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
       4,     5,     6,     7,    -1,    -1,   148,    11,    12,    -1,
      14,   153,   154,   155,    -1,    -1,    -1,   159,    -1,   161,
     162,   163,    -1,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,    -1,   107,   108,    -1,    -1,   111,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,     4,     5,
       6,     7,    -1,    -1,   148,    11,    12,    -1,    14,   153,
     154,   155,    -1,    -1,    -1,   159,    -1,   161,   162,   163,
      -1,   165,    -1,   167,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
      -1,   107,   108,    -1,    -1,   111,    -1,    -1,   114,   115,
     116,   117,    -1,    -1,    -1,    -1,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
      -1,    -1,   148,    11,    12,    -1,    14,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,   161,   162,   163,    -1,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,    -1,   107,
     108,    -1,    -1,   111,    -1,    -1,   114,   115,   116,   117,
      -1,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,    -1,    -1,
      -1,   159,    10,   161,   162,   163,    -1,   165,    -1,   167,
     168,   169,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    67,    -1,    69,    70,    -1,
      -1,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,   129,   130,    -1,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
       4,     5,     6,     7,    -1,    -1,    -1,    11,    12,    -1,
      14,   153,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,    -1,   107,   108,    -1,    -1,   111,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,    -1,    -1,    -1,   159,    -1,   161,   162,    -1,
      -1,   165,    -1,   167,   168,   169,     4,     5,     6,     7,
      -1,    -1,    -1,    11,    12,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,    -1,   107,
     108,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,   122,   123,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,   161,   162,    -1,    -1,   165,    -1,   167,
     168,   169,     4,     5,     6,     7,    -1,    -1,    -1,    11,
      12,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,   105,    -1,   107,   108,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
     122,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,    -1,    -1,    -1,   159,    -1,   161,
     162,    -1,    -1,   165,    -1,   167,   168,   169,     4,     5,
       6,     7,    -1,    -1,    -1,    11,    12,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
      -1,   107,   108,    -1,    -1,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,   122,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,   161,   162,    -1,    -1,   165,
      -1,   167,   168,   169,     4,     5,     6,     7,    -1,    -1,
      -1,    11,    12,    -1,    14,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,   105,    -1,   107,   108,    -1,
      -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,   161,   162,    -1,    -1,   165,    -1,   167,   168,   169,
       4,     5,     6,     7,    -1,    -1,    -1,    11,    12,    -1,
      14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,    -1,   107,   108,    -1,    -1,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,   122,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,     4,     5,
       6,     7,    -1,    -1,   148,    11,    12,    -1,    14,   153,
     154,   155,    -1,    -1,    -1,   159,    -1,   161,   162,    -1,
      -1,   165,    -1,   167,   168,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
      -1,   107,   108,    -1,    -1,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,   122,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
      -1,    -1,   148,    11,    12,    -1,    14,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,   161,   162,    -1,    -1,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,   104,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,     4,     5,     6,     7,    -1,    -1,
     148,    11,    12,    -1,    14,   153,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    -1,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,   104,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,     4,     5,     6,     7,    -1,    -1,   148,    11,
      12,    -1,    14,   153,   154,   155,   156,    -1,    -1,   159,
     160,    -1,    -1,    -1,    -1,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    -1,    -1,    77,    78,    -1,    80,    81,
      82,    83,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    81,    82,
      -1,   153,   154,   155,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,   165,    -1,   167,   168,   169,     4,     5,
       6,     7,    -1,    -1,    -1,    11,    12,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,   131,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,   152,
      56,    57,    58,    59,    60,    61,    62,    63,    64,   162,
      66,    67,   165,    -1,    -1,    71,    72,    73,    74,    -1,
      -1,    77,    -1,    -1,    80,    81,    82,    83,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,   104,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
      -1,    -1,   148,    11,    12,    -1,    14,   153,   154,   155,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,   165,
      -1,   167,   168,   169,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,
      -1,    -1,    80,    81,    82,    83,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,   140,   141,     4,     5,     6,     7,    -1,    -1,
     148,    11,    12,    -1,    14,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,   165,    -1,   167,
     168,   169,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    -1,    -1,    77,    -1,    -1,
      80,    81,    82,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
     140,   141,     4,     5,     6,     7,    -1,    -1,   148,    11,
      12,    -1,    14,   153,   154,   155,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,   165,    -1,   167,   168,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    -1,    -1,    77,    -1,    -1,    80,    81,
      82,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,   140,   141,
       4,     5,     6,     7,    -1,    -1,   148,    11,    12,    -1,
      14,   153,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,   165,    -1,   167,   168,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,    77,    -1,    -1,    80,    81,    82,    83,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,     5,     6,     7,     8,     9,    10,    11,    12,
     104,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    55,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,    -1,    66,    67,   159,    69,    70,    -1,    -1,
      73,   165,    -1,   167,   168,   169,    -1,    80,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,    -1,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    -1,    -1,
     153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     163,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    66,    67,    -1,    69,    70,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,    -1,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,    -1,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,   154,
     155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     8,     9,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,     8,     9,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
       8,     9,    10,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     8,     9,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,     8,     9,    10,    -1,    -1,    -1,    -1,
      -1,    65,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,     8,     9,    10,    -1,    -1,    -1,    -1,    -1,   161,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
       8,     9,    10,    -1,    -1,    -1,    -1,   161,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,   161,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     8,     9,    10,    -1,
      -1,    -1,    -1,   160,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,   160,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,     8,     9,    10,    -1,    -1,    -1,    -1,    -1,
     160,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
       8,     9,    10,    -1,    -1,    -1,    -1,    -1,   160,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,   160,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     8,     9,    10,    -1,
      -1,    -1,    -1,    -1,   160,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,   160,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     160,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,   160,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    69,    70,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    -1,   160,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,    -1,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    -1,    -1,   153,   154,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    67,
      -1,    69,    70,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,    -1,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,     8,     9,    10,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   153,   154,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       8,     9,    10,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    96,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     8,     9,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     9,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    13,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   171,   175,     0,     4,     5,     6,     7,    11,    12,
      14,    48,    49,    54,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    66,    67,    71,    72,    73,    74,    75,
      77,    80,    81,    82,    83,    84,    86,    87,    88,    90,
      92,    94,    97,   101,   102,   103,   104,   105,   106,   107,
     108,   111,   112,   114,   115,   116,   117,   122,   123,   124,
     125,   126,   127,   128,   132,   133,   135,   136,   137,   138,
     139,   140,   141,   148,   153,   154,   155,   159,   161,   162,
     165,   167,   168,   169,   176,   177,   178,   194,   201,   204,
     207,   208,   209,   211,   224,   225,   226,   227,   268,   269,
     270,   271,   272,   280,   281,   286,   287,   288,   290,   291,
     292,   293,   294,   295,   296,   307,    73,   115,   153,   269,
     272,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   115,
     126,   177,   266,   281,   282,   295,   297,   269,    30,   159,
     159,   165,   286,   288,   294,   294,   159,   283,   159,   262,
     263,   269,   194,   159,   159,   159,   159,   159,   269,   289,
     289,    73,    73,   191,   261,   289,   162,   269,   104,   106,
     155,   176,   181,   183,   187,   189,   190,   238,   239,   295,
      74,   240,   241,   270,   159,   159,   159,   159,   206,   210,
     212,   159,   159,    74,    76,   149,   150,   151,   304,   305,
     155,   176,   180,   176,   269,   192,    36,   132,   156,   269,
     300,   301,   302,   303,    76,   284,   304,    76,   304,   162,
     295,   155,   159,   235,   126,   208,    69,    70,    69,    70,
      85,     8,     9,    10,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    55,    65,
     161,    36,   276,   276,   152,   165,   152,   131,   162,   165,
     235,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    81,    82,   276,    13,   267,
     152,   235,   285,   131,   152,   162,   165,   269,   269,   300,
     289,   269,   161,   164,    88,   269,   264,   265,   269,   269,
     191,   269,   161,   161,   161,    15,   161,   164,   161,   192,
     161,   176,   189,    96,   155,   155,   176,   182,   187,   161,
     161,   164,   161,   164,    15,   161,   164,   199,   200,   294,
     269,   308,   309,   269,   160,    73,    73,    73,   300,   300,
     131,   165,   149,   305,    78,   269,   294,    76,   149,   305,
     176,   161,   179,   162,   160,   125,   163,   193,   194,   201,
     204,   209,   211,   294,   159,   269,    13,   166,   164,   167,
     168,   269,    73,   156,   160,   236,   237,   269,   205,   159,
     194,   159,    30,   161,   269,   269,   269,    30,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   282,   269,   159,    73,   273,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    55,    66,    67,    69,
      70,    73,    80,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   126,   127,   128,   129,   130,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     153,   154,   162,   172,   173,   174,   295,   298,   289,   174,
     295,   298,    73,   162,   295,   299,   269,   289,    36,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   285,   295,   299,   295,   269,   289,
     160,   160,   166,   160,   160,   263,   159,   160,   161,   164,
      96,   160,   160,   269,   261,   163,   155,    73,   162,   176,
     155,   161,   161,   155,   176,   190,   239,   269,   241,   164,
     184,   164,   184,   160,   161,   129,   213,   273,   129,   214,
     160,   160,    73,    49,    73,    74,    79,   306,   163,   165,
     163,   163,   162,   175,   159,   300,    36,   132,   269,    15,
     301,   163,   269,   164,   184,    73,   269,   269,   192,   269,
      30,    29,   133,   134,   177,   228,   229,   230,   231,   232,
     233,   273,   159,   269,   235,   166,   235,   269,   235,   163,
     166,   294,   213,   163,   166,    30,   194,   269,    30,   194,
     223,   264,   269,    36,   132,   165,   216,   294,   195,    30,
     162,   220,   273,   196,   162,   181,   185,   188,   189,   155,
     162,   200,   160,   309,   160,   177,   130,   215,   162,   177,
     244,   273,    15,    79,   166,   269,   175,   163,   160,   160,
     294,   159,   269,   237,   160,   213,   160,   160,    85,   269,
     233,   160,   164,    36,   202,   159,   228,   163,   163,   215,
     192,   160,   192,   161,   294,   159,   300,    13,   160,    30,
     194,   219,   161,   221,   161,   221,   109,   110,   198,   185,
     189,   164,   184,   162,   186,   189,   161,   244,   273,   242,
     164,   162,   269,   166,   163,   161,   300,   215,   194,    30,
     161,    30,   234,   230,   156,   203,   228,   160,   273,   161,
      89,   264,   300,   166,   216,    30,   194,   218,   192,   221,
      98,    99,   100,   221,   163,   159,   162,   184,   188,   163,
     186,   164,   184,   162,   112,   115,   116,   117,   118,   119,
     120,   121,   163,   243,   253,   254,   255,   256,   177,   242,
     163,   160,   273,   192,   232,   273,    74,   160,   112,   277,
     162,   161,   160,   160,   160,   192,    95,    98,   161,   269,
      30,   161,   222,   163,   177,   197,   192,   163,   184,   189,
     163,   242,   244,   231,   106,   272,   256,   163,   162,    13,
      15,   234,   159,   234,   242,    30,   194,   217,   218,    93,
     161,   161,   222,   192,    34,    74,   163,   163,   163,   161,
     162,   245,    74,   257,   258,   174,   259,   260,   276,   242,
     274,   269,   274,    36,    74,   278,   279,   274,   163,   192,
     161,   192,   177,   160,    73,   153,   163,   174,   177,   246,
     247,   248,   249,   250,   251,    15,   273,   161,   164,    15,
     161,   164,   174,   163,   275,   162,    74,   160,   164,   162,
      91,   162,   152,   163,   247,   161,   161,    96,   113,   269,
     258,   269,   260,   273,   269,   192,   279,   192,   161,   192,
     174,    73,   172,   256,   244,   273,   273,   159,   274,   163,
     163,   163,   174,   228,   274,   274,   160,   234,   274,   161,
     162,   252,   192,   274,   163
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   170,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   173,   173,   173,   173,   173,   173,   173,   174,   174,
     175,   175,   176,   176,   177,   177,   177,   178,   178,   178,
     178,   178,   178,   178,   179,   178,   180,   178,   178,   178,
     178,   178,   178,   181,   181,   182,   182,   183,   183,   184,
     184,   185,   185,   186,   186,   187,   187,   188,   188,   189,
     189,   190,   190,   191,   191,   192,   192,   193,   193,   193,
     193,   193,   193,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   195,   194,   194,   194,   194,   194,   194,   196,   196,
     197,   197,   198,   198,   199,   199,   200,   201,   202,   202,
     203,   203,   205,   204,   206,   204,   207,   207,   208,   208,
     210,   209,   212,   211,   213,   213,   214,   214,   215,   215,
     216,   216,   216,   216,   217,   217,   218,   218,   219,   219,
     220,   220,   220,   220,   221,   221,   221,   222,   222,   223,
     223,   224,   224,   225,   225,   226,   226,   227,   227,   228,
     228,   229,   229,   230,   230,   231,   231,   232,   232,   233,
     233,   233,   234,   234,   235,   235,   236,   236,   237,   237,
     238,   238,   239,   240,   240,   241,   241,   242,   242,   243,
     243,   243,   243,   244,   244,   245,   245,   245,   246,   246,
     247,   247,   248,   249,   249,   249,   249,   250,   250,   251,
     252,   252,   253,   253,   254,   254,   255,   255,   256,   256,
     256,   256,   256,   256,   257,   257,   258,   258,   259,   259,
     260,   261,   262,   262,   263,   264,   264,   265,   265,   267,
     266,   268,   268,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   270,   270,   271,   272,   273,   274,   275,   276,   276,
     277,   277,   278,   278,   279,   279,   280,   280,   280,   280,
     281,   281,   282,   282,   283,   283,   284,   284,   284,   285,
     285,   286,   286,   286,   287,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     288,   288,   288,   289,   289,   290,   291,   291,   291,   292,
     292,   292,   293,   293,   293,   293,   293,   293,   294,   294,
     294,   295,   295,   295,   296,   296,   297,   297,   297,   297,
     297,   297,   298,   298,   298,   299,   299,   299,   300,   301,
     301,   302,   302,   303,   303,   303,   303,   303,   303,   303,
     304,   304,   304,   304,   305,   305,   305,   305,   305,   305,
     305,   306,   306,   306,   306,   307,   307,   307,   307,   307,
     307,   307,   308,   308,   309
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     0,     1,     3,     1,     3,     2,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       3,     4,     3,     1,     1,     6,     7,     6,     7,     0,
       1,     3,     1,     3,     1,     3,     1,     1,     2,     1,
       3,     1,     2,     3,     1,     2,     0,     1,     1,     1,
       1,     1,     4,     3,     1,     1,     5,     7,     9,     5,
       3,     3,     3,     3,     3,     3,     1,     2,     6,     7,
       9,     0,     6,     1,     6,     3,     3,     2,     0,     9,
       1,     3,     0,     4,     1,     3,     1,    13,     0,     1,
       0,     1,     0,    10,     0,     9,     1,     2,     1,     1,
       0,     7,     0,     8,     0,     2,     0,     2,     0,     2,
       1,     2,     4,     3,     1,     4,     1,     4,     1,     4,
       3,     4,     4,     5,     0,     5,     4,     1,     1,     1,
       4,     5,     6,     1,     3,     6,     7,     3,     6,     1,
       0,     1,     3,     4,     6,     0,     1,     1,     2,     1,
       1,     1,     0,     2,     2,     4,     1,     3,     1,     2,
       3,     1,     1,     3,     1,     1,     3,     2,     0,     4,
       4,     3,    12,     1,     3,     1,     2,     3,     1,     2,
       2,     2,     3,     3,     3,     4,     3,     1,     1,     3,
       1,     3,     1,     1,     0,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     4,     3,     1,
       4,     4,     3,     1,     1,     0,     1,     3,     1,     0,
       9,     3,     2,     1,     6,     5,     3,     4,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     5,
       4,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     3,     2,     1,     2,     4,     2,     1,
       2,    13,    12,     1,     1,     0,     0,     0,     0,     1,
       0,     4,     3,     1,     1,     2,     2,     4,     4,     2,
       1,     1,     1,     1,     0,     3,     0,     1,     1,     0,
       1,     4,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     1,     1,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     1,     1,     4,     4,     4,     4,     1,     1,     1,
       3,     1,     4,     2,     3,     3,     1,     4,     4,     3,
       3,     3,     1,     3,     1,     1,     3,     1,     1,     0,
       1,     3,     1,     3,     1,     4,     2,     2,     6,     4,
       2,     2,     1,     2,     1,     4,     3,     3,     3,     6,
       3,     1,     1,     2,     1,     5,     4,     2,     2,     4,
       2,     2,     1,     3,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 71: /* "integer number (T_LNUMBER)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 72: /* "floating-point number (T_DNUMBER)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 73: /* "identifier (T_STRING)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 74: /* "variable (T_VARIABLE)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 75: /* T_INLINE_HTML  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 76: /* "quoted-string and whitespace (T_ENCAPSED_AND_WHITESPACE)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 77: /* "quoted-string (T_CONSTANT_ENCAPSED_STRING)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 78: /* "variable name (T_STRING_VARNAME)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 79: /* "number (T_NUM_STRING)"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 174: /* identifier  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 175: /* top_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 176: /* namespace_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 177: /* name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 178: /* top_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 182: /* group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 183: /* mixed_group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 185: /* inline_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 186: /* unprefixed_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 187: /* use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 188: /* inline_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 189: /* unprefixed_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 190: /* use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 191: /* const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 192: /* inner_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 193: /* inner_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 194: /* statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 196: /* catch_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 197: /* catch_name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 198: /* finally_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 199: /* unset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 200: /* unset_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 201: /* function_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 204: /* class_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 209: /* trait_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 211: /* interface_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 213: /* extends_from  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 214: /* interface_extends_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 215: /* implements_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 216: /* foreach_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 217: /* for_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 218: /* foreach_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 219: /* declare_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 220: /* switch_case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 221: /* case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 223: /* while_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 224: /* if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 225: /* if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 226: /* alt_if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 227: /* alt_if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 228: /* parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 229: /* non_empty_parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 230: /* parameter  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 231: /* optional_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 232: /* type_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 233: /* type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 234: /* return_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 235: /* argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 236: /* non_empty_argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 237: /* argument  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 238: /* global_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 239: /* global_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 240: /* static_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 241: /* static_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 242: /* class_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 243: /* class_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 244: /* name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 245: /* trait_adaptations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 246: /* trait_adaptation_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 247: /* trait_adaptation  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 248: /* trait_precedence  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 249: /* trait_alias  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 250: /* trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 251: /* absolute_trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 252: /* method_body  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 257: /* property_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 258: /* property  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 259: /* class_const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 260: /* class_const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 261: /* const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 262: /* echo_expr_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 263: /* echo_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 264: /* for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 265: /* non_empty_for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 266: /* anonymous_class  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 268: /* new_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 269: /* expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 270: /* inline_function  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 273: /* backup_doc_comment  */

      { if (((*yyvaluep).str)) zend_string_release_ex(((*yyvaluep).str), 0); }

        break;

    case 277: /* lexical_vars  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 278: /* lexical_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 279: /* lexical_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 280: /* function_call  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 281: /* class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 282: /* class_name_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 283: /* exit_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 284: /* backticks_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 285: /* ctor_arguments  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 286: /* dereferencable_scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 287: /* scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 288: /* constant  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 289: /* optional_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 290: /* variable_class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 291: /* dereferencable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 292: /* callable_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 293: /* callable_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 294: /* variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 295: /* simple_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 296: /* static_member  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 297: /* new_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 298: /* member_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 299: /* property_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 300: /* array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 301: /* possible_array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 302: /* non_empty_array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 303: /* array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 304: /* encaps_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 305: /* encaps_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 306: /* encaps_var_offset  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 307: /* internal_functions_in_yacc  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 308: /* isset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 309: /* isset_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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
      yychar = yylex (&yylval);
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
     '$$ = $1'.

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

    { CG(ast) = (yyvsp[0].ast); }

    break;

  case 78:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 79:

    {
			zval zv;
			zend_lex_tstring(&zv);
			(yyval.ast) = zend_ast_create_zval(&zv);
		}

    break;

  case 80:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 81:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 82:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 83:

    { (yyval.ast) = zend_ast_append_str((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 84:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }

    break;

  case 85:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }

    break;

  case 86:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }

    break;

  case 87:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 88:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 89:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 90:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 91:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 92:

    { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }

    break;

  case 93:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }

    break;

  case 94:

    { RESET_DOC_COMMENT(); }

    break;

  case 95:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 96:

    { RESET_DOC_COMMENT(); }

    break;

  case 97:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }

    break;

  case 98:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 99:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 100:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 101:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 102:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 103:

    { (yyval.num) = ZEND_SYMBOL_FUNCTION; }

    break;

  case 104:

    { (yyval.num) = ZEND_SYMBOL_CONST; }

    break;

  case 105:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 106:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 107:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}

    break;

  case 108:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 111:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 112:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 113:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 114:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 115:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 116:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 117:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 118:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }

    break;

  case 119:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 120:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 121:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 122:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 123:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 124:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 125:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 126:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 127:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 128:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 129:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 130:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 131:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 132:

    { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }

    break;

  case 133:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 134:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 135:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 136:

    { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 137:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 138:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 139:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 140:

    { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }

    break;

  case 141:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }

    break;

  case 142:

    { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }

    break;

  case 143:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 144:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 145:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 146:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 147:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 148:

    { (yyval.ast) = (yyvsp[-3].ast); }

    break;

  case 149:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 150:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }

    break;

  case 151:

    { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }

    break;

  case 152:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 153:

    { (yyval.ast) = NULL; }

    break;

  case 154:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 155:

    { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[-1].ast)); }

    break;

  case 156:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }

    break;

  case 157:

    { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }

    break;

  case 158:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }

    break;

  case 159:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }

    break;

  case 160:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 161:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 162:

    { (yyval.ast) = NULL; }

    break;

  case 163:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 164:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 165:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 166:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }

    break;

  case 167:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast)); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 168:

    { (yyval.num) = 0; }

    break;

  case 169:

    { (yyval.num) = ZEND_PARAM_REF; }

    break;

  case 170:

    { (yyval.num) = 0; }

    break;

  case 171:

    { (yyval.num) = ZEND_PARAM_VARIADIC; }

    break;

  case 172:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 173:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL); }

    break;

  case 174:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 175:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL); }

    break;

  case 176:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 177:

    { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 178:

    { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }

    break;

  case 179:

    { (yyval.num) = ZEND_ACC_FINAL; }

    break;

  case 180:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 181:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL); }

    break;

  case 182:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 183:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL); }

    break;

  case 184:

    { (yyval.ast) = NULL; }

    break;

  case 185:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 186:

    { (yyval.ast) = NULL; }

    break;

  case 187:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 188:

    { (yyval.ast) = NULL; }

    break;

  case 189:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 190:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 191:

    { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }

    break;

  case 192:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }

    break;

  case 193:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 194:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 195:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 196:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 197:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 198:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 199:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 200:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 201:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 202:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 203:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 204:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }

    break;

  case 205:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 206:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }

    break;

  case 209:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 210:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 211:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 212:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 213:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 214:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }

    break;

  case 215:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 216:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 217:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 218:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }

    break;

  case 219:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 220:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }

    break;

  case 221:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }

    break;

  case 222:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 223:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-2].num) | (yyvsp[-1].num), (yyvsp[-3].ast), (yyvsp[0].ast), NULL); }

    break;

  case 224:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-4].num) | (yyvsp[-3].num), (yyvsp[-5].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 225:

    { (yyval.ast) = NULL; }

    break;

  case 226:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 227:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 228:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }

    break;

  case 229:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }

    break;

  case 230:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }

    break;

  case 231:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 232:

    { (yyval.ast) = NULL; }

    break;

  case 233:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 234:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 235:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 236:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }

    break;

  case 237:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 238:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 239:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 240:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 241:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 242:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }

    break;

  case 243:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 244:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 245:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }

    break;

  case 246:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 247:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 248:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 249:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast));
			  (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 250:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 251:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 252:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast)); CG(extra_fn_flags) = (yyvsp[-2].num); }

    break;

  case 253:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 254:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 255:

    { (yyval.ast) = NULL; }

    break;

  case 256:

    { (yyval.ast) = NULL; }

    break;

  case 257:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 258:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }

    break;

  case 259:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 260:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 261:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 262:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 263:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 264:

    { zval zv; zend_lex_tstring(&zv); (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }

    break;

  case 265:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[-1].num), (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 266:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[0].num), (yyvsp[-2].ast), NULL); }

    break;

  case 267:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }

    break;

  case 268:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 269:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 270:

    { (yyval.ast) = NULL; }

    break;

  case 271:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 272:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 273:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 274:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 275:

    { (yyval.num) = (yyvsp[0].num); if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }

    break;

  case 276:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 277:

    { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 278:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 279:

    { (yyval.num) = ZEND_ACC_PROTECTED; }

    break;

  case 280:

    { (yyval.num) = ZEND_ACC_PRIVATE; }

    break;

  case 281:

    { (yyval.num) = ZEND_ACC_STATIC; }

    break;

  case 282:

    { (yyval.num) = ZEND_ACC_ABSTRACT; }

    break;

  case 283:

    { (yyval.num) = ZEND_ACC_FINAL; }

    break;

  case 284:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 285:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }

    break;

  case 286:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 287:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 288:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 289:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 290:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 291:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 292:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 293:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 294:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 295:

    { (yyval.ast) = NULL; }

    break;

  case 296:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 297:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 298:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }

    break;

  case 299:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 300:

    {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS, (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}

    break;

  case 301:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 302:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 303:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 304:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 305:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 306:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 307:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 308:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }

    break;

  case 309:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 310:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 311:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 312:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 313:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 314:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 315:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 316:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 317:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 318:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 319:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 320:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 321:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 322:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }

    break;

  case 323:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }

    break;

  case 324:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }

    break;

  case 325:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }

    break;

  case 326:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 327:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 328:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 329:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 330:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 331:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 332:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 333:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 334:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 335:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 336:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 337:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 338:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 339:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 340:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 341:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 342:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 343:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }

    break;

  case 344:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }

    break;

  case 345:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }

    break;

  case 346:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }

    break;

  case 347:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 348:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 349:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 350:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 351:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 352:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 353:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 354:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 355:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 356:

    { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 357:

    {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
			if ((yyval.ast)->kind == ZEND_AST_BINARY_OP && (yyval.ast)->attr == ZEND_CONCAT) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONCAT;
		}

    break;

  case 358:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 359:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 360:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 361:

    { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 362:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 363:

    { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }

    break;

  case 364:

    { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }

    break;

  case 365:

    { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }

    break;

  case 366:

    { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }

    break;

  case 367:

    { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }

    break;

  case 368:

    { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }

    break;

  case 369:

    { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }

    break;

  case 370:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[0].ast)); }

    break;

  case 371:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }

    break;

  case 372:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 373:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }

    break;

  case 374:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }

    break;

  case 375:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 376:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 377:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 378:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 379:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 380:

    { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }

    break;

  case 381:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast)); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 382:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-5].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0), (yyvsp[-8].ast), NULL,
				  zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)), (yyvsp[-6].ast));
				  ((zend_ast_decl *) (yyval.ast))->lex_pos = (yyvsp[-2].ptr);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }

    break;

  case 383:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 384:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 385:

    { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }

    break;

  case 386:

    { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }

    break;

  case 387:

    { (yyval.ptr) = LANG_SCNG(yy_text); }

    break;

  case 388:

    { (yyval.num) = 0; }

    break;

  case 389:

    { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }

    break;

  case 390:

    { (yyval.ast) = NULL; }

    break;

  case 391:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 392:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 393:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }

    break;

  case 394:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 395:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }

    break;

  case 396:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 397:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 398:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 399:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 400:

    { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }

    break;

  case 401:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 402:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 403:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 404:

    { (yyval.ast) = NULL; }

    break;

  case 405:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 406:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 407:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 408:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 409:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 410:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 411:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }

    break;

  case 412:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 413:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 414:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 415:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 416:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }

    break;

  case 417:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }

    break;

  case 418:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }

    break;

  case 419:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }

    break;

  case 420:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }

    break;

  case 421:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }

    break;

  case 422:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }

    break;

  case 423:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }

    break;

  case 424:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 425:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 426:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 427:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 428:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 429:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 430:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }

    break;

  case 431:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 432:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 433:

    { (yyval.ast) = NULL; }

    break;

  case 434:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 435:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 436:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 437:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 438:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 439:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 440:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 441:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 442:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 443:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 444:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 445:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 446:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 447:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 448:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 449:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 450:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 451:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 452:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 453:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 454:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 455:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 456:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 457:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 458:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 459:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 460:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 461:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 462:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 463:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 464:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 465:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 466:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 467:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 468:

    { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }

    break;

  case 469:

    { (yyval.ast) = NULL; }

    break;

  case 470:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 471:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 472:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }

    break;

  case 473:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }

    break;

  case 474:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 475:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }

    break;

  case 476:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }

    break;

  case 477:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 478:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }

    break;

  case 479:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }

    break;

  case 480:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 481:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 482:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }

    break;

  case 483:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 484:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 485:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }

    break;

  case 486:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }

    break;

  case 487:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }

    break;

  case 488:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }

    break;

  case 489:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }

    break;

  case 490:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 491:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 492:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 493:

    { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }

    break;

  case 494:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 495:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 496:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }

    break;

  case 497:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }

    break;

  case 498:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }

    break;

  case 499:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }

    break;

  case 500:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }

    break;

  case 501:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }

    break;

  case 502:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 503:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 504:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ISSET, (yyvsp[0].ast)); }

    break;



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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}



/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	/* CG(parse_error) states:
	 * 0 => yyres = NULL, yystr is the unexpected token
	 * 1 => yyres = NULL, yystr is one of the expected tokens
	 * 2 => yyres != NULL, yystr is the unexpected token
	 * 3 => yyres != NULL, yystr is one of the expected tokens
	 */
	if (yyres && CG(parse_error) < 2) {
		CG(parse_error) = 2;
	}

	if (CG(parse_error) % 2 == 0) {
		/* The unexpected token */
		char buffer[120];
		const unsigned char *end, *str, *tok1 = NULL, *tok2 = NULL;
		unsigned int len = 0, toklen = 0, yystr_len;

		CG(parse_error)++;

		if (LANG_SCNG(yy_text)[0] == 0 &&
			LANG_SCNG(yy_leng) == 1 &&
			strcmp(yystr, "\"end of file\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "end of file");
			}
			return sizeof("end of file")-1;
		}

		str = LANG_SCNG(yy_text);
		end = memchr(str, '\n', LANG_SCNG(yy_leng));
		yystr_len = (unsigned int)yystrlen(yystr);

		if ((tok1 = memchr(yystr, '(', yystr_len)) != NULL
			&& (tok2 = zend_memrchr(yystr, ')', yystr_len)) != NULL) {
			toklen = (tok2 - tok1) + 1;
		} else {
			tok1 = tok2 = NULL;
			toklen = 0;
		}

		if (end == NULL) {
			len = LANG_SCNG(yy_leng) > 30 ? 30 : LANG_SCNG(yy_leng);
		} else {
			len = (end - str) > 30 ? 30 : (end - str);
		}
		if (yyres) {
			if (toklen) {
				snprintf(buffer, sizeof(buffer), "'%.*s' %.*s", len, str, toklen, tok1);
			} else {
				snprintf(buffer, sizeof(buffer), "'%.*s'", len, str);
			}
			yystpcpy(yyres, buffer);
		}
		return len + (toklen ? toklen + 1 : 0) + 2;
	}

	/* One of the expected tokens */
	if (!yyres) {
		return yystrlen(yystr) - (*yystr == '"' ? 2 : 0);
	}

	if (*yystr == '"') {
		YYSIZE_T yyn = 0;
		const char *yyp = yystr;

		for (; *++yyp != '"'; ++yyn) {
			yyres[yyn] = *yyp;
		}
		yyres[yyn] = '\0';
		return yyn;
	}
	yystpcpy(yyres, yystr);
	return strlen(yystr);
}
