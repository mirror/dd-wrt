/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */


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

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif


/* Substitute the type names.  */
#define YYSTYPE         ZENDSTYPE
/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yydebug         zenddebug
#define yynerrs         zendnerrs


/* Copy the first part of user declarations.  */



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
#ifndef ZENDDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ZENDDEBUG 1
#  else
#   define ZENDDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ZENDDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ZENDDEBUG */
#if ZENDDEBUG
extern int zenddebug;
#endif
/* "%code requires" blocks.  */


#include "zend_compile.h"



/* Token type.  */
#ifndef ZENDTOKENTYPE
# define ZENDTOKENTYPE
  enum zendtokentype
  {
    END = 0,
    T_THROW = 258,
    PREC_ARROW_FUNCTION = 259,
    T_INCLUDE = 260,
    T_INCLUDE_ONCE = 261,
    T_REQUIRE = 262,
    T_REQUIRE_ONCE = 263,
    T_LOGICAL_OR = 264,
    T_LOGICAL_XOR = 265,
    T_LOGICAL_AND = 266,
    T_PRINT = 267,
    T_YIELD = 268,
    T_DOUBLE_ARROW = 269,
    T_YIELD_FROM = 270,
    T_PLUS_EQUAL = 271,
    T_MINUS_EQUAL = 272,
    T_MUL_EQUAL = 273,
    T_DIV_EQUAL = 274,
    T_CONCAT_EQUAL = 275,
    T_MOD_EQUAL = 276,
    T_AND_EQUAL = 277,
    T_OR_EQUAL = 278,
    T_XOR_EQUAL = 279,
    T_SL_EQUAL = 280,
    T_SR_EQUAL = 281,
    T_POW_EQUAL = 282,
    T_COALESCE_EQUAL = 283,
    T_COALESCE = 284,
    T_BOOLEAN_OR = 285,
    T_BOOLEAN_AND = 286,
    T_IS_EQUAL = 287,
    T_IS_NOT_EQUAL = 288,
    T_IS_IDENTICAL = 289,
    T_IS_NOT_IDENTICAL = 290,
    T_SPACESHIP = 291,
    T_IS_SMALLER_OR_EQUAL = 292,
    T_IS_GREATER_OR_EQUAL = 293,
    T_SL = 294,
    T_SR = 295,
    T_INSTANCEOF = 296,
    T_INT_CAST = 297,
    T_DOUBLE_CAST = 298,
    T_STRING_CAST = 299,
    T_ARRAY_CAST = 300,
    T_OBJECT_CAST = 301,
    T_BOOL_CAST = 302,
    T_UNSET_CAST = 303,
    T_POW = 304,
    T_CLONE = 305,
    T_NOELSE = 306,
    T_ELSEIF = 307,
    T_ELSE = 308,
    T_LNUMBER = 309,
    T_DNUMBER = 310,
    T_STRING = 311,
    T_NAME_FULLY_QUALIFIED = 312,
    T_NAME_RELATIVE = 313,
    T_NAME_QUALIFIED = 314,
    T_VARIABLE = 315,
    T_INLINE_HTML = 316,
    T_ENCAPSED_AND_WHITESPACE = 317,
    T_CONSTANT_ENCAPSED_STRING = 318,
    T_STRING_VARNAME = 319,
    T_NUM_STRING = 320,
    T_EVAL = 321,
    T_NEW = 322,
    T_EXIT = 323,
    T_IF = 324,
    T_ENDIF = 325,
    T_ECHO = 326,
    T_DO = 327,
    T_WHILE = 328,
    T_ENDWHILE = 329,
    T_FOR = 330,
    T_ENDFOR = 331,
    T_FOREACH = 332,
    T_ENDFOREACH = 333,
    T_DECLARE = 334,
    T_ENDDECLARE = 335,
    T_AS = 336,
    T_SWITCH = 337,
    T_ENDSWITCH = 338,
    T_CASE = 339,
    T_DEFAULT = 340,
    T_MATCH = 341,
    T_BREAK = 342,
    T_CONTINUE = 343,
    T_GOTO = 344,
    T_FUNCTION = 345,
    T_FN = 346,
    T_CONST = 347,
    T_RETURN = 348,
    T_TRY = 349,
    T_CATCH = 350,
    T_FINALLY = 351,
    T_USE = 352,
    T_INSTEADOF = 353,
    T_GLOBAL = 354,
    T_STATIC = 355,
    T_ABSTRACT = 356,
    T_FINAL = 357,
    T_PRIVATE = 358,
    T_PROTECTED = 359,
    T_PUBLIC = 360,
    T_VAR = 361,
    T_UNSET = 362,
    T_ISSET = 363,
    T_EMPTY = 364,
    T_HALT_COMPILER = 365,
    T_CLASS = 366,
    T_TRAIT = 367,
    T_INTERFACE = 368,
    T_EXTENDS = 369,
    T_IMPLEMENTS = 370,
    T_NAMESPACE = 371,
    T_LIST = 372,
    T_ARRAY = 373,
    T_CALLABLE = 374,
    T_LINE = 375,
    T_FILE = 376,
    T_DIR = 377,
    T_CLASS_C = 378,
    T_TRAIT_C = 379,
    T_METHOD_C = 380,
    T_FUNC_C = 381,
    T_NS_C = 382,
    T_ATTRIBUTE = 383,
    T_INC = 384,
    T_DEC = 385,
    T_OBJECT_OPERATOR = 386,
    T_NULLSAFE_OBJECT_OPERATOR = 387,
    T_COMMENT = 388,
    T_DOC_COMMENT = 389,
    T_OPEN_TAG = 390,
    T_OPEN_TAG_WITH_ECHO = 391,
    T_CLOSE_TAG = 392,
    T_WHITESPACE = 393,
    T_START_HEREDOC = 394,
    T_END_HEREDOC = 395,
    T_DOLLAR_OPEN_CURLY_BRACES = 396,
    T_CURLY_OPEN = 397,
    T_PAAMAYIM_NEKUDOTAYIM = 398,
    T_NS_SEPARATOR = 399,
    T_ELLIPSIS = 400,
    T_BAD_CHARACTER = 401,
    T_ERROR = 402
  };
#endif

/* Value type.  */
#if ! defined ZENDSTYPE && ! defined ZENDSTYPE_IS_DECLARED
typedef zend_parser_stack_elem ZENDSTYPE;
# define ZENDSTYPE_IS_TRIVIAL 1
# define ZENDSTYPE_IS_DECLARED 1
#endif



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
         || (defined ZENDSTYPE_IS_TRIVIAL && ZENDSTYPE_IS_TRIVIAL)))

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
#define YYLAST   7975

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  176
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  162
/* YYNRULES -- Number of rules.  */
#define YYNRULES  557
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1055

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   402

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    55,   174,     2,   175,    54,    37,     2,
     167,   168,    52,    50,   165,    51,    47,    53,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    31,   169,
      43,    16,    45,    30,    65,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   172,     2,   166,    36,     2,   173,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   170,    35,   171,    57,     2,     2,     2,
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
      15,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    32,    33,    34,    38,    39,    40,
      41,    42,    44,    46,    48,    49,    56,    58,    59,    60,
      61,    62,    63,    64,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164
};

#if ZENDDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   284,   284,   288,   288,   288,   288,   288,   288,   288,
     288,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   291,   291,   291,   291,   291,   291,   291,
     291,   291,   291,   292,   292,   292,   292,   292,   292,   292,
     292,   292,   292,   292,   293,   293,   293,   293,   293,   293,
     293,   293,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   298,   299,   299,   299,   299,   299,   299,   303,
     304,   312,   313,   318,   319,   324,   325,   330,   331,   335,
     336,   337,   338,   342,   344,   349,   351,   356,   360,   361,
     365,   366,   367,   368,   372,   373,   374,   375,   379,   382,
     382,   385,   385,   388,   389,   390,   391,   392,   396,   397,
     401,   406,   411,   412,   416,   418,   423,   425,   430,   432,
     437,   438,   442,   444,   449,   451,   456,   457,   461,   463,
     469,   470,   471,   472,   479,   480,   481,   482,   484,   486,
     488,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   501,   505,   504,   508,   509,   511,   512,   516,   518,
     523,   524,   528,   529,   533,   534,   538,   539,   543,   547,
     554,   555,   559,   560,   564,   564,   567,   567,   573,   574,
     579,   580,   584,   584,   590,   590,   596,   597,   601,   602,
     606,   607,   611,   612,   613,   614,   618,   619,   623,   624,
     628,   629,   633,   634,   635,   636,   640,   641,   643,   648,
     649,   654,   659,   660,   664,   665,   669,   671,   676,   677,
     682,   683,   688,   691,   697,   698,   703,   706,   712,   713,
     719,   720,   725,   727,   732,   733,   737,   738,   739,   740,
     744,   748,   756,   757,   761,   762,   763,   767,   768,   772,
     773,   780,   781,   782,   786,   787,   788,   792,   794,   799,
     800,   804,   805,   809,   811,   816,   817,   819,   823,   824,
     828,   834,   835,   839,   840,   844,   846,   852,   855,   858,
     865,   866,   867,   872,   873,   877,   878,   879,   883,   885,
     890,   891,   895,   900,   902,   906,   908,   913,   915,   919,
     924,   925,   929,   930,   934,   935,   940,   941,   946,   947,
     948,   949,   950,   951,   955,   956,   960,   962,   967,   968,
     972,   976,   980,   981,   984,   988,   989,   993,   994,   998,
     998,  1008,  1010,  1012,  1017,  1019,  1021,  1023,  1025,  1027,
    1028,  1030,  1032,  1034,  1036,  1038,  1040,  1042,  1044,  1046,
    1048,  1050,  1052,  1054,  1055,  1056,  1057,  1058,  1060,  1062,
    1064,  1066,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1086,
    1088,  1090,  1092,  1094,  1096,  1098,  1100,  1102,  1104,  1108,
    1109,  1111,  1113,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,  1131,
    1132,  1133,  1134,  1135,  1136,  1138,  1143,  1148,  1158,  1162,
    1166,  1170,  1174,  1178,  1179,  1183,  1184,  1188,  1189,  1193,
    1194,  1198,  1200,  1202,  1204,  1209,  1212,  1216,  1217,  1218,
    1222,  1223,  1227,  1229,  1230,  1235,  1236,  1241,  1242,  1243,
    1244,  1248,  1249,  1250,  1251,  1253,  1254,  1255,  1256,  1260,
    1261,  1262,  1263,  1264,  1265,  1266,  1267,  1268,  1272,  1274,
    1279,  1280,  1284,  1288,  1289,  1290,  1291,  1295,  1296,  1300,
    1301,  1302,  1306,  1308,  1310,  1312,  1314,  1316,  1320,  1322,
    1324,  1326,  1331,  1332,  1333,  1337,  1339,  1344,  1346,  1348,
    1350,  1352,  1354,  1356,  1361,  1362,  1363,  1367,  1368,  1369,
    1373,  1378,  1379,  1383,  1385,  1390,  1392,  1394,  1396,  1398,
    1400,  1403,  1409,  1411,  1413,  1415,  1420,  1422,  1425,  1428,
    1431,  1433,  1435,  1438,  1442,  1443,  1444,  1445,  1450,  1451,
    1452,  1454,  1456,  1458,  1460,  1465,  1466,  1471
};
#endif

#if ZENDDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"'throw'\"",
  "PREC_ARROW_FUNCTION", "\"'include'\"", "\"'include_once'\"",
  "\"'require'\"", "\"'require_once'\"", "\"'or'\"", "\"'xor'\"",
  "\"'and'\"", "\"'print'\"", "\"'yield'\"", "\"'=>'\"",
  "\"'yield from'\"", "'='", "\"'+='\"", "\"'-='\"", "\"'*='\"",
  "\"'/='\"", "\"'.='\"", "\"'%='\"", "\"'&='\"", "\"'|='\"", "\"'^='\"",
  "\"'<<='\"", "\"'>>='\"", "\"'**='\"", "\"'?""?='\"", "'?'", "':'",
  "\"'?""?'\"", "\"'||'\"", "\"'&&'\"", "'|'", "'^'", "'&'", "\"'=='\"",
  "\"'!='\"", "\"'==='\"", "\"'!=='\"", "\"'<=>'\"", "'<'", "\"'<='\"",
  "'>'", "\"'>='\"", "'.'", "\"'<<'\"", "\"'>>'\"", "'+'", "'-'", "'*'",
  "'/'", "'%'", "'!'", "\"'instanceof'\"", "'~'", "\"'(int)'\"",
  "\"'(double)'\"", "\"'(string)'\"", "\"'(array)'\"", "\"'(object)'\"",
  "\"'(bool)'\"", "\"'(unset)'\"", "'@'", "\"'**'\"", "\"'clone'\"",
  "T_NOELSE", "\"'elseif'\"", "\"'else'\"", "\"integer\"",
  "\"floating-point number\"", "\"identifier\"",
  "\"fully qualified name\"", "\"namespace-relative name\"",
  "\"namespaced name\"", "\"variable\"", "T_INLINE_HTML",
  "\"string content\"", "\"quoted string\"", "\"variable name\"",
  "\"number\"", "\"'eval'\"", "\"'new'\"", "\"'exit'\"", "\"'if'\"",
  "\"'endif'\"", "\"'echo'\"", "\"'do'\"", "\"'while'\"", "\"'endwhile'\"",
  "\"'for'\"", "\"'endfor'\"", "\"'foreach'\"", "\"'endforeach'\"",
  "\"'declare'\"", "\"'enddeclare'\"", "\"'as'\"", "\"'switch'\"",
  "\"'endswitch'\"", "\"'case'\"", "\"'default'\"", "\"'match'\"",
  "\"'break'\"", "\"'continue'\"", "\"'goto'\"", "\"'function'\"",
  "\"'fn'\"", "\"'const'\"", "\"'return'\"", "\"'try'\"", "\"'catch'\"",
  "\"'finally'\"", "\"'use'\"", "\"'insteadof'\"", "\"'global'\"",
  "\"'static'\"", "\"'abstract'\"", "\"'final'\"", "\"'private'\"",
  "\"'protected'\"", "\"'public'\"", "\"'var'\"", "\"'unset'\"",
  "\"'isset'\"", "\"'empty'\"", "\"'__halt_compiler'\"", "\"'class'\"",
  "\"'trait'\"", "\"'interface'\"", "\"'extends'\"", "\"'implements'\"",
  "\"'namespace'\"", "\"'list'\"", "\"'array'\"", "\"'callable'\"",
  "\"'__LINE__'\"", "\"'__FILE__'\"", "\"'__DIR__'\"", "\"'__CLASS__'\"",
  "\"'__TRAIT__'\"", "\"'__METHOD__'\"", "\"'__FUNCTION__'\"",
  "\"'__NAMESPACE__'\"", "\"'#['\"", "\"'++'\"", "\"'--'\"", "\"'->'\"",
  "\"'?->'\"", "\"comment\"", "\"doc comment\"", "\"open tag\"",
  "\"'<?='\"", "\"'?>'\"", "\"whitespace\"", "\"heredoc start\"",
  "\"heredoc end\"", "\"'${'\"", "\"'{$'\"", "\"'::'\"", "\"'\\\\'\"",
  "\"'...'\"", "\"invalid character\"", "T_ERROR", "','", "']'", "'('",
  "')'", "';'", "'{'", "'}'", "'['", "'`'", "'\"'", "'$'", "$accept",
  "start", "reserved_non_modifiers", "semi_reserved", "identifier",
  "top_statement_list", "namespace_declaration_name", "namespace_name",
  "legacy_namespace_name", "name", "attribute_decl", "attribute_group",
  "attribute", "attributes", "attributed_statement", "top_statement",
  "$@1", "$@2", "use_type", "group_use_declaration",
  "mixed_group_use_declaration", "possible_comma",
  "inline_use_declarations", "unprefixed_use_declarations",
  "use_declarations", "inline_use_declaration",
  "unprefixed_use_declaration", "use_declaration", "const_list",
  "inner_statement_list", "inner_statement", "statement", "$@3",
  "catch_list", "catch_name_list", "optional_variable",
  "finally_statement", "unset_variables", "unset_variable",
  "function_declaration_statement", "is_reference", "is_variadic",
  "class_declaration_statement", "@4", "@5", "class_modifiers",
  "class_modifier", "trait_declaration_statement", "@6",
  "interface_declaration_statement", "@7", "extends_from",
  "interface_extends_list", "implements_list", "foreach_variable",
  "for_statement", "foreach_statement", "declare_statement",
  "switch_case_list", "case_list", "case_separator", "match",
  "match_arm_list", "non_empty_match_arm_list", "match_arm",
  "match_arm_cond_list", "while_statement", "if_stmt_without_else",
  "if_stmt", "alt_if_stmt_without_else", "alt_if_stmt", "parameter_list",
  "non_empty_parameter_list", "attributed_parameter",
  "optional_visibility_modifier", "parameter",
  "optional_type_without_static", "type_expr", "type", "union_type",
  "type_expr_without_static", "type_without_static",
  "union_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "attributed_class_statement", "class_statement", "class_name_list",
  "trait_adaptations", "trait_adaptation_list", "trait_adaptation",
  "trait_precedence", "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "class_const_list", "class_const_decl",
  "const_decl", "echo_expr_list", "echo_expr", "for_exprs",
  "non_empty_for_exprs", "anonymous_class", "@8", "new_expr", "expr",
  "inline_function", "fn", "function", "backup_doc_comment",
  "backup_fn_flags", "backup_lex_pos", "returns_ref", "lexical_vars",
  "lexical_var_list", "lexical_var", "function_call", "class_name",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "dereferencable_scalar", "scalar", "constant", "class_constant",
  "optional_expr", "variable_class_name", "fully_dereferencable",
  "array_object_dereferencable", "callable_expr", "callable_variable",
  "variable", "simple_variable", "static_member", "new_variable",
  "member_name", "property_name", "array_pair_list", "possible_array_pair",
  "non_empty_array_pair_list", "array_pair", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions_in_yacc", "isset_variables",
  "isset_variable", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    61,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
      63,    58,   284,   285,   286,   124,    94,    38,   287,   288,
     289,   290,   291,    60,   292,    62,   293,    46,   294,   295,
      43,    45,    42,    47,    37,    33,   296,   126,   297,   298,
     299,   300,   301,   302,   303,    64,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,    44,    93,    40,    41,    59,
     123,   125,    91,    96,    34,    36
};
# endif

#define YYPACT_NINF -884

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-884)))

#define YYTABLE_NINF -529

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-529)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -884,    30,  1731,  -884,  5795,  5795,  5795,  5795,  5795,  5795,
    5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,
    5795,  5795,  5795,  5795,  5795,  -884,  -884,    23,  -884,  -884,
    -884,  -884,  -884,  -884,  -108,   674,  -104,   -99,  5795,  4635,
     -79,   -67,     5,    36,    46,   106,  5795,  5795,   213,  -884,
    -884,   227,  5795,   -39,   240,   -53,    92,  -884,  -884,   138,
     163,   181,   187,  -884,  -884,  -884,  7225,   199,   215,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,   325,  7081,  7081,
     -24,  5795,  -884,  -884,  4780,   -13,   -10,   -50,   130,  -884,
     698,  -884,  -884,  -884,  -884,  -884,   -41,  -884,  -884,  -884,
    -884,   355,  -884,   232,  -884,  -884,  6565,  -884,   299,   299,
    -884,   211,   283,  -884,   459,   511,   220,   243,   463,   238,
     252,    85,  -884,  -884,  -884,  -884,   288,   261,  7760,   299,
    7760,  7760,  7760,  7760,  3762,  7848,  3762,   375,   375,     9,
     375,   375,   375,   375,   375,   375,   375,   375,   375,  -884,
    -884,  5795,  -884,  -884,  5795,  -884,   102,  -884,   279,   238,
    -884,   582,  5795,  -884,  5795,   123,  -884,  7760,   354,  5795,
    5795,  5795,   227,  5795,  5795,  7760,   289,   298,   304,   436,
     198,  -884,   307,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
     -25,   178,   313,   218,  -884,   294,  -884,  -884,   446,   303,
    -884,  -884,  7081,  5795,  5795,   316,   414,   424,   429,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,   321,
     335,  4780,  4780,  -884,   371,   238,  5795,  4780,   376,  -884,
    -884,   597,   597,   180,    78,  -884,  4925,  7081,    53,  -884,
    1283,  1876,  7081,   378,  5795,  7712,   374,  -884,   402,  -884,
     -19,   406,   148,   -19,    50,  5795,  -884,  1123,  -884,   288,
    -884,  -884,  -884,  -884,  -884,   431,  4635,   432,   565,   434,
    5795,  5795,  5795,  5070,  5795,  5795,  5795,  5795,  5795,  5795,
    5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,
    5795,  5795,  5795,  5795,  5795,  5795,  5795,   195,  5795,  -884,
    -884,  -884,   528,  3768,  3768,    44,    44,  5795,  5795,  -884,
    5215,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,  5795,
    5795,  5795,  5795,  5795,  -884,  -884,  -884,  5795,  6613,   238,
    6661,  -884,   -53,  -884,  -884,    44,    44,   -53,  5795,  5795,
     441,  6709,  5795,  -884,   437,  6757,   444,   449,  7760,  7645,
      58,  6805,  6853,  -884,  -884,  -884,  5795,   227,  -884,  -884,
    2021,   545,   447,    -7,   458,   305,  -884,   178,  -884,   -53,
    -884,  5795,   551,  -884,   469,  -884,    81,  7760,   472,  -884,
    6901,   473,   507,  -884,   516,  -884,   502,  -884,   509,   512,
     325,   513,  -884,  6949,   521,   602,   615,   353,  -884,  -884,
     262,  6229,   519,  -884,  -884,  -884,   331,   531,  -884,   698,
    -884,  -884,  -884,   373,  4780,  7760,  5360,   679,  4780,  -884,
    -884,  6277,  5795,  5795,  5795,  5795,  5795,  5795,  5505,  5795,
      45,  -108,   674,     0,   106,   669,   671,   -36,   163,   181,
     199,   215,   673,   676,   687,   689,   702,   706,   708,   710,
    5795,  -884,   713,   588,  -884,  7760,  -884,   685,  5795,  -884,
    5795,  -884,  -884,  7807,  7360,  3762,  5795,  1071,  5939,  6085,
    7488,  7879,  7909,  1101,  4828,  4828,  4828,  4828,  4828,  1429,
    1429,  1429,  1429,   661,   807,   807,   640,   640,     9,     9,
       9,  -884,   375,   592,  -884,   593,  5795,   599,   601,   238,
     599,   601,   238,  -884,  5795,  -884,   238,   238,  6325,   604,
    7081,  3762,  3762,  3762,  3762,  3762,  3762,  3762,  3762,  3762,
    3762,  3762,  3762,  3762,  3762,  3762,  -884,   507,  -884,  -884,
    -884,  -884,  -884,  6373,   605,  -884,  3910,  -884,  5795,  4055,
    5795,  5795,   918,  -884,   -12,   613,  7760,  -884,  -884,  -884,
     347,   622,  -884,  -884,   695,  -884,  -884,  7760,  -884,  7081,
     626,  5795,   628,  -884,  -884,   325,   665,   629,   325,  -884,
    -884,  1441,   756,  -884,  -884,  -884,   631,  -884,  -884,  -884,
     719,  -884,  -884,  -884,   637,  -884,  5795,  -884,  -884,   636,
    -884,   639,  7081,   644,  7760,  5795,  -884,  -884,  7760,  5795,
    1296,   650,   507,  6997,  7045,  2166,  5939,  5795,   173,   645,
     173,  6421,  -884,  -884,  6469,  -884,  -884,  -884,  -884,   597,
     665,  -884,  -884,  -884,  -884,  7093,  -884,  -884,  -884,   652,
    7760,  7081,   656,  4780,    26,    27,  4200,   655,   660,  -884,
    5650,  -884,   451,   732,   235,   668,  -884,  -884,   235,  -884,
     666,  -884,  -884,  -884,   325,  -884,  -884,   672,  -884,   675,
    1586,  -884,  5795,  -884,  -884,  7141,   693,   756,   417,  4780,
    3762,  7760,  -884,  -884,   665,  4635,   805,   696,  5939,  -884,
    -884,  -884,   326,   680,   699,  -884,   168,  -884,   173,   700,
    -884,  -884,  -884,  3616,   707,  2311,  5795,    37,  4780,   703,
     918,  4345,  -884,  -884,  -884,  -884,   544,  -884,   -45,   712,
     714,   716,  -884,   718,  7760,   704,   717,  -884,   815,  -884,
     347,   722,   724,  -884,  -884,   672,   720,   144,   325,  -884,
    -884,  3762,   723,  -884,   727,  -884,  -884,  -884,  -884,  -884,
     865,   272,  -884,   209,  -884,  -884,  -884,   861,  -884,   864,
     870,   742,   798,   744,  -884,   746,   748,   749,    38,   750,
    -884,  -884,  -884,  2456,   623,   751,  5795,    -9,   127,  -884,
    -884,   907,  -884,  5650,  -884,  5795,   908,   325,  -884,  -884,
    -884,  -884,   235,   752,  -884,   325,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,   503,  -884,  -884,   168,   228,   667,
    -884,  -884,   293,  -884,   756,   754,  3616,     6,   911,  -884,
    -884,  -884,   764,   209,   209,   865,   760,   865,  -884,  -884,
    4490,  -884,  4345,  2601,   761,   765,  -884,  6517,  -884,  -884,
    -884,  -884,  5795,  -884,  7760,  5795,   204,  -884,  2746,  -884,
    -884,   470,   152,  -884,   852,  7367,   299,  -884,  -884,  -884,
     301,  -884,  -884,   901,   903,  -884,  -884,  -884,   863,  -884,
    -884,  -884,   183,  -884,   733,  -884,  -884,  -884,  -884,   774,
    -884,  -884,  -884,  3616,  7760,  7760,   325,  -884,   776,  -884,
    -884,  -884,  5947,  -884,   930,   324,  -884,   931,   330,  -884,
    7367,   866,  -884,   301,   301,  -884,  -884,   778,   873,  -884,
     787,  -884,   783,  -884,  2891,  -884,  3616,  -884,   784,   796,
    -884,  -884,  6089,  -884,   788,   789,   862,   847,   803,  5795,
    -884,   852,  -884,  5795,  7367,  -884,  -884,  -884,  -884,  -884,
    5795,   948,  -884,  -884,   183,   797,  -884,   800,  -884,  -884,
    -884,  -884,  -884,  7509,   325,  7367,  7760,  -884,  7760,  -884,
     804,  7760,  5795,  3036,  -884,  -884,  3181,  -884,  3326,  -884,
    -884,  7367,   672,  -884,  -884,  -884,   173,  -884,  7760,  -884,
    -884,  -884,  -884,   802,  -884,  -884,   865,  -884,   400,  -884,
    -884,  -884,  3471,  -884,  -884
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      82,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     416,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   461,   462,    89,    91,    92,
      90,   502,   157,   459,     0,     0,   450,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480,   480,     0,   429,
     428,     0,   480,     0,     0,     0,   445,   190,   191,     0,
       0,     0,     0,   186,   192,   194,   111,     0,     0,   470,
     471,   472,   477,   473,   474,   475,   476,     0,     0,     0,
       0,     0,   164,   139,   521,   452,     0,     0,   469,    98,
       0,   105,    81,   104,   100,   101,     0,   188,   102,   103,
     425,   234,   145,     0,   146,   399,     0,   421,   433,   433,
     497,     0,   466,   413,   467,   468,     0,   487,     0,     0,
     498,   344,   492,   499,   403,    89,   445,     0,   420,   433,
     550,   551,   553,   554,   415,   417,   419,   384,   385,   386,
     387,   404,   405,   406,   407,   408,   409,   410,   412,   349,
     167,     0,   445,   339,     0,   446,     0,   342,   447,   455,
     507,   448,   480,   411,     0,     0,   333,   334,     0,     0,
     335,     0,     0,     0,     0,   481,     0,     0,     0,     0,
       0,   137,     0,   139,    85,    88,    86,   118,   119,    87,
     134,     0,     0,     0,   129,     0,   279,   280,   283,     0,
     282,   423,     0,     0,     0,     0,     0,     0,     0,    33,
       3,     4,     6,     7,     8,     9,    10,    46,    47,    11,
      13,    16,    17,    79,    84,     5,    12,    14,    15,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    49,    50,    51,    52,    71,    53,    41,    42,    43,
      70,    44,    45,    30,    31,    32,    34,    35,    36,    73,
      74,    75,    76,    77,    78,    37,    38,    39,    40,    61,
      59,    60,    56,    57,    58,    48,    54,    55,    66,    67,
      68,    62,    63,    65,    64,    69,    72,    80,    83,   109,
       0,   521,   521,    95,   122,    93,     0,   521,   485,   488,
     486,   364,   366,   536,     0,   464,     0,     0,     0,   534,
       0,     0,     0,     0,     0,   526,     0,   524,   520,   522,
     453,     0,   454,     0,     0,     0,   504,     0,   441,     0,
      99,   106,   422,   184,   189,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   158,
     434,   430,   430,     0,     0,     0,     0,     0,   480,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   363,   365,   430,     0,     0,   455,
       0,   343,     0,   456,   341,     0,     0,     0,     0,   480,
       0,     0,     0,   156,     0,     0,     0,   336,   338,     0,
       0,     0,     0,   151,   152,   166,     0,     0,   117,   153,
       0,     0,     0,   134,     0,     0,   113,     0,   115,     0,
     154,     0,     0,   155,   122,   176,   483,   557,   122,   555,
       0,     0,   196,   430,   198,   108,     0,    82,     0,     0,
     123,     0,    94,     0,     0,     0,     0,     0,   463,   535,
       0,     0,   483,   533,   465,   532,   398,     0,   144,     0,
     141,   138,   140,   483,   521,   529,     0,   458,   521,   414,
     460,     0,    33,     3,     4,     6,     7,    46,   416,    13,
      89,     5,    12,   450,    71,   429,   428,    73,    39,    40,
      48,    54,   470,   471,   472,   477,   473,   474,   475,   476,
       0,   271,     0,   122,   273,   275,   424,     0,     0,   235,
       0,   139,   238,   369,   371,   370,     0,     0,   402,   367,
     368,   372,   374,   373,   390,   391,   388,   389,   396,   392,
     393,   394,   395,   375,   382,   383,   376,   377,   378,   380,
     381,   397,   379,     0,   430,     0,     0,   478,   505,     0,
     479,   506,     0,   517,     0,   519,   500,   501,     0,     0,
       0,   347,   350,   351,   352,   354,   355,   356,   357,   358,
     359,   360,   361,   353,   362,   418,   552,   196,   449,   512,
     510,   511,   513,     0,     0,   451,     0,   332,     0,     0,
     335,     0,     0,   162,     0,     0,   430,   136,   168,   135,
       0,     0,   114,   116,   134,   128,   278,   284,   281,   123,
       0,   123,     0,   549,   107,     0,   200,     0,     0,   430,
      82,     0,     0,   457,    96,    97,   484,   458,   538,   539,
       0,   544,   547,   545,     0,   541,     0,   540,   543,     0,
     142,     0,     0,     0,   525,     0,   523,   503,   277,     0,
     123,     0,   196,     0,     0,     0,   401,     0,   246,     0,
     246,     0,   442,   443,     0,   495,   496,   494,   493,   348,
     200,   509,   508,   139,   232,     0,   139,   230,   147,     0,
     337,     0,     0,   521,     0,   483,     0,   216,   216,   150,
     222,   331,   174,   132,     0,   122,   125,   130,     0,   177,
       0,   556,   548,   197,     0,   430,   286,   199,   293,     0,
       0,   112,     0,   546,   537,     0,     0,   531,   483,   521,
     346,   276,   274,   272,   200,     0,     0,     0,   400,   249,
     248,   247,   246,     0,   122,   242,   252,   245,   246,     0,
     515,   518,   430,   236,     0,     0,   335,   483,   521,     0,
       0,     0,   139,   210,   163,   216,     0,   216,     0,   122,
       0,   122,   224,   122,   228,     0,     0,   165,     0,   131,
     123,     0,   122,   127,   159,   201,     0,   314,     0,   286,
     110,   345,     0,   143,     0,   430,   233,   139,   239,   244,
     269,   246,   240,     0,   264,   265,   266,   180,   253,   261,
     263,     0,   435,     0,   148,     0,     0,     0,   458,     0,
     139,   208,   160,     0,     0,     0,     0,     0,     0,   212,
     123,     0,   221,   123,   223,   123,     0,     0,   139,   133,
     124,   121,   123,     0,   286,     0,   321,   322,   323,   320,
     319,   318,   313,   193,   314,   290,   285,   252,     0,   312,
     316,   294,   314,   542,   530,     0,   237,     0,     0,   243,
     262,   181,   182,     0,     0,   269,     0,   269,   286,   231,
       0,   204,     0,     0,     0,     0,   214,     0,   219,   220,
     139,   213,     0,   225,   229,     0,   172,   170,     0,   126,
     120,   314,     0,   291,     0,     0,   433,   317,   195,   286,
       0,   258,   270,   254,   256,   257,   431,   183,     0,   267,
     268,   431,     0,   431,   314,   139,   206,   149,   161,     0,
     211,   215,   139,   218,   227,   226,     0,   173,     0,   175,
     187,   295,     0,   292,   430,     0,   325,     0,     0,   329,
       0,   314,   255,     0,     0,   432,   430,     0,     0,   439,
     122,   438,     0,   340,     0,   209,   217,   171,     0,    73,
     296,   307,     0,   298,     0,     0,     0,   308,     0,     0,
     326,     0,   287,     0,     0,   288,   430,   185,   259,   260,
       0,   250,   139,   440,   123,     0,   139,     0,   139,   297,
     299,   300,   301,     0,     0,     0,   430,   324,   430,   328,
       0,   431,     0,     0,   437,   436,     0,   207,     0,   303,
     304,   306,   302,   309,   327,   330,   246,   427,   251,   431,
     431,   169,   305,     0,   179,   426,   269,   431,     0,   310,
     139,   431,     0,   289,   311
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -884,  -884,   -37,  -884,   -49,  -443,  -884,   -29,  -178,    84,
     517,  -884,   -82,    -2,     2,  -884,  -884,  -884,   924,  -884,
    -884,  -428,  -884,  -884,   799,   189,  -686,   559,   825,  -122,
    -884,    10,  -884,  -884,  -884,  -884,  -884,  -884,   370,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,   904,  -884,  -884,  -884,
    -884,  -554,  -884,  -628,   231,  -884,   110,  -884,  -884,  -537,
     109,  -884,  -884,  -884,   160,  -884,  -884,  -884,  -884,  -884,
    -884,  -675,  -884,   196,  -884,   256,   142,  -884,  -878,  -884,
    -884,  -666,  -884,  -850,    -4,  -884,   340,  -884,   574,  -884,
     573,  -760,   153,  -884,  -718,  -884,  -884,    34,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -848,  -884,    28,  -884,
      24,   596,  -884,   608,  -603,  -884,   872,  -884,  -884,   169,
     -30,  -884,    -1,  -360,  -883,  -884,  -106,  -884,  -884,    25,
    -884,   -33,   664,  -884,  -884,   633,   -32,  -884,    20,    72,
     -18,  -884,  -884,  -884,  -884,  -884,    74,   -17,  -884,  -884,
     659,  -128,  -282,   550,  -884,  -884,   490,   184,  -884,  -884,
    -884,   408
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   286,   287,   522,     2,   289,   713,   190,    88,
     293,   294,    89,   127,   480,    92,   456,   290,   714,   434,
     192,   461,   715,   792,   193,   716,   717,   194,   180,   311,
     481,   482,   706,   712,   906,   948,   787,   444,   445,    94,
     882,   928,    95,   527,   206,    96,    97,    98,   207,    99,
     208,   636,   639,   725,   704,   937,   832,   774,   709,   776,
     900,   100,   780,   781,   782,   783,   698,   101,   102,   103,
     104,   753,   754,   755,   756,   757,   817,   922,   923,   924,
     818,   925,   820,   878,   403,   523,   524,   195,   196,   199,
     200,   797,   865,   866,   727,   953,   982,   983,   984,   985,
     986,   987,  1051,   867,   868,   869,   870,   955,   956,   958,
     959,   181,   165,   166,   416,   417,   157,   399,   105,   106,
     107,   108,   129,   565,   965,  1000,   371,   887,   970,   971,
     110,   111,   159,   163,   321,   404,   112,   113,   114,   115,
     176,   116,   117,   118,   119,   120,   121,   122,   123,   161,
     569,   576,   316,   317,   318,   319,   308,   309,   654,   124,
     448,   449
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      90,   109,   158,   372,    91,   759,   795,   699,   330,   458,
     459,   563,    93,   433,   641,   464,   630,   288,   160,   707,
     632,   917,   898,   396,    31,   189,   201,    31,   789,   177,
       3,   -14,   793,   156,   182,   931,   920,   933,   197,   872,
     770,  -202,   962,   690,   295,   330,   298,   298,   967,   168,
     972,  -203,  -205,   303,   150,   304,   836,   837,   303,   151,
     332,   430,   762,   162,   303,   367,   320,   303,   164,   323,
     326,    49,    50,   431,   330,   368,   -79,    57,    58,   125,
      28,    29,    30,   821,   328,   998,   999,   333,   169,   109,
     819,   431,   331,   637,   911,   671,   201,   332,   299,   299,
     170,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   379,   805,   573,   744,   155,
     325,    31,    87,   921,  -445,    87,   839,   303,   934,   473,
     303,   183,   473,   305,   306,   307,   432,   912,  1037,   306,
     307,   814,   815,   -79,   410,   306,   307,   880,   306,   307,
     300,   300,   301,   302,   621,   303,  1044,  1045,   708,   961,
     899,   155,   189,   826,  1048,  1031,   909,   162,  1053,   198,
     298,   778,   171,   128,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   771,  -202,  1047,   730,   813,    49,
      50,   819,   661,   172,   679,  -203,  -205,   167,   306,   307,
     474,   306,   307,   173,   574,   175,   175,   929,   930,    87,
     968,   175,   299,   427,   490,   303,   613,   473,   836,   837,
     153,   394,   395,  -483,  -483,   468,   306,   307,   834,   946,
     838,   125,    28,    29,    30,  -483,  -178,    77,   577,  -178,
     310,   184,   185,   315,   186,  -483,   711,  -483,   855,   624,
     969,   856,   857,   858,   859,   860,   861,   862,   125,    28,
      29,    30,    31,   174,   300,   298,   446,   600,   601,   729,
     298,   947,   125,    28,    29,    30,   178,   791,   412,    77,
    -446,   462,   413,   749,   750,   751,  1032,   327,   901,   526,
     179,   337,   338,   814,   815,   202,   306,   307,   184,   479,
     109,   186,   152,   184,   185,   863,   186,   798,    77,   339,
     398,   951,   952,   400,   567,   570,   812,   299,   465,   466,
     203,   175,   299,   411,   158,    49,   370,   915,   415,   418,
     419,  -241,   421,   422,   814,   815,   529,   187,   204,   188,
     160,   841,   467,   844,   205,   846,   568,   571,   575,   575,
     579,  1043,   154,   427,   853,   796,   291,   428,    49,    50,
      87,   373,   447,   450,   125,    28,    29,    30,   329,   300,
     374,   472,   292,   437,   300,   599,   483,   438,   575,   575,
     602,   604,   749,   750,   751,    49,    50,   330,   125,    28,
      29,    30,   823,  -482,   650,   327,    77,   855,   189,   675,
     856,   857,   858,   859,   860,   861,   862,    77,   921,  -489,
     184,   769,   197,   186,   335,   336,   651,   295,   479,   109,
     652,  -485,  -485,   655,   656,   653,   814,   815,    77,   402,
    -123,   368,   152,  -485,   414,   875,   749,   750,   751,   332,
    -491,   155,   426,  -485,   187,  -485,   188,   804,   423,   439,
     315,   315,   441,   440,   918,   463,   315,   424,   442,   158,
     437,    77,   443,   425,   623,   471,   429,   201,   109,  -484,
    -484,   660,   436,   485,   451,   160,   827,   452,   469,   991,
     455,  -484,   475,   992,   491,   994,   525,   453,  -490,   995,
     156,  -484,   454,  -484,   469,   457,   475,   469,   475,   533,
     534,   535,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   460,   562,  -528,  -528,
     487,  -528,  1005,  -491,   155,   484,   578,   175,   298,   581,
     582,   583,   584,   585,   586,   587,   588,   589,   590,   591,
     592,   593,   594,   785,   786,   682,   595,   488,   683,  1049,
    1050,   763,   685,   686,   765,   322,   324,   603,   175,   489,
     298,   167,  -527,  -527,   855,  -527,   155,   856,   857,   858,
     859,   860,   861,   862,   990,   616,   531,   298,   528,   530,
     299,   564,   723,   532,   608,   728,  1001,  -488,  -488,   605,
     627,   375,   376,   610,   611,    77,   694,   620,   619,   697,
     856,   857,   858,   859,   860,   861,   862,   622,   198,  -488,
     298,  -488,   299,   377,   629,   378,  1020,   631,   635,    90,
     109,   950,   634,    91,   835,   836,   837,   638,    77,   299,
     833,    93,   300,   315,   689,   664,  1034,   315,  1035,  -486,
    -486,   128,   130,   131,   132,   133,   134,   135,   149,   298,
     330,  -486,   640,   479,   109,   648,   752,   642,   752,   645,
     643,  -486,   299,  -486,   300,   876,   705,   647,   649,   668,
     658,   728,   364,   365,   366,   665,   367,   673,   659,   674,
     -43,   300,   -70,   446,   -66,   676,   368,   -67,   893,   360,
     361,   362,   363,   364,   365,   366,   773,   367,   -68,   155,
     -62,   299,   155,   895,   836,   837,   908,   368,    90,   109,
     405,   406,    91,   -63,   300,   681,   738,   -65,   298,   -64,
      93,   -69,   407,   684,   669,  -483,  -483,   125,    28,    29,
      30,    31,   408,   670,   409,   806,   752,  -483,   672,   678,
     680,   479,   109,   479,   109,   871,  -514,  -483,  -516,  -483,
     688,   692,   732,   300,  -315,   767,  -315,   695,   943,   418,
     700,   831,   330,   710,   856,   857,   858,   859,   860,   861,
     299,   152,   718,   431,   720,   864,   722,   724,  -490,   726,
     447,   733,   153,   734,   736,    49,    50,   737,   155,   752,
     960,   739,   758,   974,   907,   329,    57,    58,   743,    77,
     976,   766,   728,   768,   775,   735,    63,    64,    65,   777,
     788,   479,   109,   790,   740,   794,   807,   798,   741,   525,
     816,   154,   300,    77,   705,   799,   748,   855,   810,    87,
     856,   857,   858,   859,   860,   861,   862,   362,   363,   364,
     365,   366,   803,   367,   811,   808,   957,   916,   822,   828,
     864,   847,   315,   368,   479,   109,   824,   840,    77,   784,
    1023,   843,   155,   845,  1026,   842,  1028,   848,   849,   852,
     854,   479,   109,   851,   873,   874,   877,   816,   881,   883,
     936,   801,   831,   981,   973,   884,   479,   109,   315,   864,
     885,   996,   886,   977,   888,   889,   890,   891,   892,   988,
     896,   902,   905,   910,   919,   926,   927,   932,  1052,   954,
     940,   155,   864,   981,   941,   418,   963,   315,   964,   155,
     966,   479,   109,   975,   978,   957,   989,   993,  1002,   988,
    1003,   816,  1004,  1006,  1008,   701,  -445,  1011,  1012,   864,
    1013,   816,  1014,  1015,  1022,  1025,  1033,   816,   816,  1027,
    1046,  1036,   479,   109,   479,   109,  1030,   644,   191,   850,
     855,   728,  1042,   856,   857,   858,   859,   860,   861,   862,
     435,   125,    28,    29,    30,    31,   625,   420,    33,   719,
     334,   829,   938,   903,   816,   897,   942,   879,   809,   914,
     742,    77,   784,   626,   904,   628,  1010,   913,  1019,  1017,
     607,   479,   109,   617,   479,   109,   479,   109,   401,  1024,
     155,   561,   597,   572,   752,   152,   155,   997,   666,   721,
       0,     0,     0,     0,     0,     0,     0,   816,   816,     0,
     479,   109,   702,    68,     0,    69,    70,    71,    72,    73,
      74,    75,    76,     0,     0,     0,   155,     0,     0,     0,
       0,   944,     0,     0,   945,     0,     0,     0,     0,     0,
     340,   341,   342,     0,     0,   296,     0,     0,     0,     0,
     703,     0,    86,    87,     0,     0,     0,     0,   155,     0,
       0,   343,   677,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   492,   367,   493,   494,
     495,   496,   214,   215,   216,   497,   498,   368,    11,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,     0,   367,  1016,     0,
       0,     0,  1018,     0,     0,     0,     0,   368,     0,  1021,
       0,     0,     0,    12,    13,     0,     0,     0,    14,   219,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
     499,  1038,   221,   222,    25,    26,   500,    28,    29,    30,
      31,     0,     0,    33,     0,     0,   501,   502,   503,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   504,   246,   247,   248,
     505,   506,   251,   252,   253,   254,   255,   256,   257,   258,
     507,   260,   261,   262,   263,   264,   265,   266,   508,   509,
       0,   269,   270,   271,   272,   273,   274,   510,   511,   277,
     512,   513,   514,   515,   516,   517,   518,   519,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,     0,   520,     0,     0,     0,     0,
      81,   521,   340,   341,   342,    84,    85,    86,    87,   492,
       0,   493,   494,   495,   496,   214,   215,   216,   497,   498,
       0,    11,     0,   343,     0,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,     0,   367,
       0,     0,     0,     0,     0,     0,    12,    13,     0,   368,
       0,    14,   219,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,   499,     0,   221,   222,    25,    26,   500,
      28,    29,    30,    31,     0,     0,    33,     0,     0,   501,
     502,   503,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   504,
     246,   247,   248,   505,   506,   251,   252,   253,   254,   255,
     256,   257,   258,   507,   260,   261,   262,   263,   264,   265,
     266,   508,   509,     0,   269,   270,   271,   272,   273,   274,
     510,   511,   277,   512,   513,   514,   515,   516,   517,   518,
     519,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,   476,    80,     9,    10,     0,    11,     0,   520,     0,
       0,     0,     0,    81,     0,     0,     0,     0,    84,    85,
      86,    87,  -529,  -529,  -529,  -529,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,     0,     0,     0,     0,
       0,    12,    13,     0,     0,   368,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,     0,    54,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,    62,    63,
      64,    65,     0,     0,    66,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,   731,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,     0,
      54,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,    62,    63,    64,    65,     0,     0,    66,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,   800,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,     0,    54,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,    62,    63,
      64,    65,     0,     0,    66,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,   478,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,   618,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,   747,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,   825,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,   894,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,   939,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,   949,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,  1007,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,  1039,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,  1040,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     4,     0,     5,     6,     7,     8,
       0,     0,    80,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,  1041,    84,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,    57,
      58,     0,     0,     0,     0,    59,    60,    61,   477,    63,
      64,    65,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     4,
       0,     5,     6,     7,     8,     0,     0,    80,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,  1054,    84,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,     0,
      59,    60,    61,   477,    63,    64,    65,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     0,     0,     0,     0,     0,     0,
       0,   209,    80,   210,   211,   212,   213,   214,   215,   216,
     217,   218,     0,    81,     0,    82,    83,     0,    84,    85,
      86,    87,   343,     0,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,     0,   367,     0,
       0,     0,     0,     0,   219,     0,     0,     0,   368,     0,
       0,     0,     0,     0,     0,   220,     0,   221,   222,     0,
       0,   223,     0,     0,     0,    31,     0,     0,     0,     0,
       0,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,     4,     0,     5,     6,     7,     8,     0,
       0,     0,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   566,     0,
       0,   693,     0,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,     0,     0,
       0,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,    82,
      83,     0,    84,    85,    86,    87,   696,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,    27,    28,
      29,    30,    31,    32,     0,    33,     0,     0,    34,    35,
      36,    37,     0,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,    49,    50,     0,    52,    53,     0,     0,     0,
       0,    55,    56,     0,     0,     0,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,    82,    83,     0,    84,    85,    86,
      87,   772,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,     0,     0,
       0,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,    82,
      83,     0,    84,    85,    86,    87,   830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,    27,    28,
      29,    30,    31,    32,     0,    33,     0,     0,    34,    35,
      36,    37,     0,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,    49,    50,     0,    52,    53,     0,     0,     0,
       0,    55,    56,     0,     0,     0,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,    82,    83,     0,    84,    85,    86,
      87,   935,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,     0,     0,
       0,     0,     0,     0,    59,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,    27,    28,
      29,    30,    31,    32,     0,    33,     0,     0,    34,    35,
      36,    37,     0,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,    49,    50,     0,    52,    53,     0,     0,     0,
       0,    55,    56,     0,     0,     0,     0,     0,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,     0,     0,     0,     0,   312,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,   125,    28,    29,    30,    31,     0,     0,
      33,     0,     0,    34,    35,    36,  -529,  -529,  -529,  -529,
    -529,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,    45,   367,     0,     0,    49,    50,     0,
       0,     0,     0,     0,   368,     0,     0,   126,     0,     0,
       0,     0,     0,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,   313,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,   314,     0,     0,     0,     0,    81,     0,     0,
       0,     0,    84,    85,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   125,    28,
      29,    30,    31,     0,     0,    33,   470,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    49,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   126,     0,     0,     0,     0,     0,     0,     0,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,    84,    85,    86,
      87,   536,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,   125,    28,    29,    30,    31,     0,     0,
      33,     0,     0,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    45,     0,     0,     0,    49,    50,     0,
       0,     0,     0,     0,     0,     0,     0,   126,     0,     0,
       0,     0,     0,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,    84,    85,    86,    87,     0,     0,     0,     0,
       0,     0,   580,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   125,    28,
      29,    30,    31,     0,     0,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    49,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   126,     0,     0,     0,     0,     0,     0,     0,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,    84,    85,    86,
      87,     0,     0,     0,     0,     0,     0,   662,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,   125,    28,    29,    30,    31,     0,     0,
      33,     0,     0,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    45,     0,     0,     0,    49,    50,     0,
       0,     0,     0,     0,     0,     0,     0,   126,     0,     0,
       0,     0,     0,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,   663,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,    84,    85,    86,    87,   -47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   125,    28,
      29,    30,    31,     0,     0,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    49,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   126,     0,     0,     0,     0,     0,     0,     0,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     4,     0,     5,     6,     7,     8,     0,
       0,    80,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,    84,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,   125,    28,    29,    30,    31,     0,     0,
      33,     0,     0,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   779,    45,     0,     0,     0,    49,    50,     0,
       0,     0,     0,     0,     0,     0,     0,   126,     0,     0,
       0,     0,     0,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,     4,     0,
       5,     6,     7,     8,     0,     0,    80,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,    84,    85,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   125,    28,
      29,    30,    31,     0,     0,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    49,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   126,     0,     0,     0,     0,     0,     0,     0,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
     209,    80,   210,   211,   212,   213,   214,   215,   216,   217,
     218,     0,    81,     0,     0,     0,     0,    84,    85,    86,
      87,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,     0,     0,     0,     0,
       0,     0,     0,   219,     0,   368,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,   221,   222,     0,     0,
     500,    28,    29,    30,     0,     0,     0,     0,     0,     0,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   979,   260,   261,   262,   263,   264,
     265,   266,   267,   268,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   209,     0,   210,   211,   212,   213,   214,   215,
     216,   217,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   980,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,     0,     0,     0,   219,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,   220,     0,   221,   222,
       0,     0,   500,    28,    29,    30,     0,     0,     0,     0,
       0,     0,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   979,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,     0,     0,     0,     0,   340,   341,
     342,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   343,
    1009,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,   340,   341,   342,     0,
       0,     0,     0,     0,     0,   368,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   343,     0,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,     0,   367,   340,   341,   342,     0,     0,     0,
       0,     0,     0,   368,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   343,     0,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,   340,   341,   342,     0,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,     0,     0,     0,     0,
     657,     0,     0,   343,     0,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,     0,   367,
     340,   341,   342,     0,     0,     0,     0,     0,     0,   368,
       0,     0,     0,     0,     0,     0,     0,     0,   667,     0,
       0,   343,     0,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,     0,   367,   340,   341,
     342,     0,     0,     0,     0,     0,     0,   368,     0,     0,
       0,     0,     0,     0,     0,     0,   687,     0,     0,   343,
       0,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,   340,   341,   342,     0,
       0,     0,     0,     0,     0,   368,     0,     0,     0,     0,
       0,     0,     0,     0,   691,     0,     0,   343,   898,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,     0,   367,   340,   341,   342,     0,     0,     0,
       0,     0,     0,   368,     0,     0,     0,     0,     0,     0,
       0,     0,   760,     0,     0,   343,     0,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,   340,   341,   342,     0,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,     0,     0,     0,     0,
     761,     0,     0,   343,     0,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,     0,   367,
     340,   341,   342,     0,     0,     0,     0,     0,     0,   368,
       0,     0,     0,     0,     0,     0,   899,     0,     0,     0,
       0,   343,     0,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,     0,   367,   340,   341,
     342,     0,     0,     0,     0,     0,     0,   368,     0,     0,
       0,     0,     0,     0,   369,     0,     0,     0,     0,   343,
       0,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,   340,   341,   342,     0,
       0,     0,     0,     0,     0,   368,     0,     0,     0,     0,
       0,   596,     0,     0,     0,     0,     0,   343,     0,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,     0,   367,   340,   341,   342,     0,     0,     0,
       0,     0,     0,   368,     0,     0,     0,     0,     0,   598,
       0,     0,     0,     0,     0,   343,     0,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,   340,   341,   342,     0,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,     0,   606,     0,     0,
       0,     0,     0,   343,     0,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,     0,   367,
     340,   341,   342,     0,     0,     0,     0,     0,     0,   368,
       0,     0,     0,     0,     0,   609,     0,     0,     0,     0,
       0,   343,     0,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,     0,   367,   340,   341,
     342,     0,     0,     0,     0,     0,     0,   368,     0,     0,
       0,     0,     0,   614,     0,     0,     0,     0,     0,   343,
       0,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,   340,   341,   342,     0,
       0,     0,     0,     0,     0,   368,     0,     0,     0,     0,
       0,   615,     0,     0,     0,     0,     0,   343,     0,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,     0,   367,   340,   341,   342,     0,     0,     0,
       0,     0,     0,   368,     0,     0,     0,     0,     0,   633,
       0,     0,     0,     0,     0,   343,     0,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,   340,   341,   342,     0,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,     0,   646,     0,     0,
       0,     0,     0,   343,     0,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,     0,   367,
     340,   341,   342,     0,   125,    28,    29,    30,    31,   368,
       0,    33,     0,     0,     0,   745,     0,     0,     0,     0,
       0,   343,     0,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,     0,   367,   152,     0,
       0,     0,     0,     0,     0,     0,     0,   368,     0,     0,
       0,     0,     0,   746,     0,     0,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,     0,     0,   209,     0,
     210,   211,   212,   213,   214,   215,   216,   217,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   296,     0,
       0,     0,     0,   297,     0,    86,    87,     0,     0,     0,
       0,   764,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,     0,   221,   222,     0,     0,   223,     0,
       0,   224,     0,     0,     0,     0,     0,   802,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     209,   342,   210,   211,   212,   213,   214,   215,   216,   217,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     343,     0,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,     0,   367,     0,     0,     0,
       0,     0,     0,   219,     0,     0,   368,     0,     0,     0,
       0,     0,     0,     0,   220,     0,   221,   222,     0,     0,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   209,     0,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,     0,   367,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   368,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   220,     0,   221,   222,
       0,     0,  1029,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   856,   857,   858,   859,
     860,   861,   265,   266,   267,   268,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   340,   341,   342,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   343,     0,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
       0,   367,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   368,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,   341,   342,     0,     0,   486,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   343,   612,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,     0,   367,   340,
     341,   342,     0,     0,     0,     0,     0,     0,   368,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     343,     0,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,     0,   367,   341,   342,     0,
       0,     0,     0,     0,     0,     0,   368,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   343,     0,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   397,   367,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   368,     0,     0,     0,     0,   343,     0,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,     0,   367,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   368,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   368,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,     0,   367,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   368
};

static const yytype_int16 yycheck[] =
{
       2,     2,    35,   109,     2,   680,   724,   610,    90,   291,
     292,   371,     2,   191,   457,   297,   444,    66,    35,    31,
     448,   869,    31,   129,    77,    54,    56,    77,   714,    47,
       0,    31,   718,    35,    52,   885,    30,   887,    55,   799,
      14,    14,   920,   597,    77,   127,    78,    79,   931,    39,
     933,    14,    14,    77,    31,    79,   101,   102,    77,   167,
      90,   183,   690,   167,    77,    56,    79,    77,   167,    79,
      87,   107,   108,    98,   156,    66,    31,   118,   119,    73,
      74,    75,    76,   758,    88,   963,   964,   128,   167,    90,
     756,    98,    90,   453,   854,   523,   126,   127,    78,    79,
     167,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   119,   744,    73,   672,    35,
     170,    77,   175,   117,   160,   175,   171,    77,   888,    79,
      77,   170,    79,   157,   158,   159,   161,   855,  1021,   158,
     159,   135,   136,    98,   162,   158,   159,   813,   158,   159,
      78,    79,    78,    79,   161,    77,  1039,  1040,   170,   919,
     169,    77,   191,   766,  1047,  1013,   852,   167,  1051,    77,
     202,   708,   167,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,   168,   168,  1046,   640,    30,   107,
     108,   867,   484,   167,   564,   168,   168,    38,   158,   159,
     157,   158,   159,   167,   170,    46,    47,   883,   884,   175,
      37,    52,   202,   165,   174,    77,   168,    79,   101,   102,
     128,   146,   147,   148,   149,   157,   158,   159,   775,    35,
     777,    73,    74,    75,    76,   160,   165,   145,   376,   168,
      81,    73,    74,    84,    76,   170,   616,   172,   114,   437,
      77,   117,   118,   119,   120,   121,   122,   123,    73,    74,
      75,    76,    77,   167,   202,   307,   202,   405,   406,   639,
     312,    77,    73,    74,    75,    76,    73,   715,   165,   145,
     160,   295,   169,   120,   121,   122,  1014,   167,   171,   329,
      73,    69,    70,   135,   136,   167,   158,   159,    73,   311,
     311,    76,   117,    73,    74,   171,    76,   165,   145,    87,
     151,   169,   170,   154,   373,   374,   754,   307,   148,   149,
     167,   162,   312,   164,   367,   107,    37,   109,   169,   170,
     171,   168,   173,   174,   135,   136,   336,   107,   167,   109,
     367,   779,   172,   781,   167,   783,   373,   374,   375,   376,
     378,  1036,   167,   165,   792,   725,   167,   169,   107,   108,
     175,   160,   203,   204,    73,    74,    75,    76,   117,   307,
     160,   307,   167,   165,   312,   402,   312,   169,   405,   406,
     407,   409,   120,   121,   122,   107,   108,   479,    73,    74,
      75,    76,   762,   160,    51,   167,   145,   114,   437,   531,
     117,   118,   119,   120,   121,   122,   123,   145,   117,   167,
      73,   703,   439,    76,    69,    70,    73,   460,   430,   430,
      77,   148,   149,   171,   172,    82,   135,   136,   145,   160,
     168,    66,   117,   160,    90,   805,   120,   121,   122,   479,
     167,   367,    16,   170,   107,   172,   109,   739,   169,   165,
     291,   292,    16,   169,   171,   296,   297,   169,   165,   502,
     165,   145,   169,   169,   169,   306,   169,   507,   479,   148,
     149,   479,   169,   314,   168,   502,   768,    73,   304,   165,
     169,   160,   308,   169,   325,   165,   327,    73,   167,   169,
     502,   170,    73,   172,   320,   170,   322,   323,   324,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   165,   368,   165,   166,
     166,   168,   970,   167,   460,   167,   377,   378,   580,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   112,   113,   569,   397,   165,   572,   169,
     170,   693,   576,   577,   696,    85,    86,   408,   409,   173,
     612,   412,   165,   166,   114,   168,   502,   117,   118,   119,
     120,   121,   122,   123,   954,   426,    31,   629,   167,   167,
     580,    73,   635,   169,   167,   638,   966,   148,   149,   168,
     441,   148,   149,   169,   165,   145,   606,   170,    73,   609,
     117,   118,   119,   120,   121,   122,   123,   169,    77,   170,
     662,   172,   612,   170,   165,   172,   996,   165,   131,   641,
     641,   171,   169,   641,   100,   101,   102,   131,   145,   629,
     772,   641,   580,   484,   580,   486,  1016,   488,  1018,   148,
     149,   492,   493,   494,   495,   496,   497,   498,   499,   701,
     752,   160,   170,   675,   675,    73,   678,   168,   680,   166,
     168,   170,   662,   172,   612,   807,   612,   166,    73,   520,
     171,   724,    52,    53,    54,    16,    56,   528,   167,   530,
      31,   629,    31,   629,    31,   536,    66,    31,   830,    48,
      49,    50,    51,    52,    53,    54,   706,    56,    31,   635,
      31,   701,   638,   100,   101,   102,   848,    66,   730,   730,
     148,   149,   730,    31,   662,   566,   662,    31,   770,    31,
     730,    31,   160,   574,    31,   148,   149,    73,    74,    75,
      76,    77,   170,   165,   172,   745,   758,   160,    73,   167,
     167,   763,   763,   765,   765,   798,   167,   170,   167,   172,
     166,   166,    16,   701,   107,   701,   109,   608,   900,   610,
     611,   771,   864,   170,   117,   118,   119,   120,   121,   122,
     770,   117,   170,    98,   168,   797,   168,   132,   167,   170,
     631,    82,   128,   166,   168,   107,   108,   168,   724,   811,
     916,   167,   167,   935,   847,   117,   118,   119,   168,   145,
     942,   169,   855,   167,   169,   656,   128,   129,   130,   169,
      98,   833,   833,   165,   665,   169,    31,   165,   669,   670,
     756,   167,   770,   145,   770,   170,   677,   114,   168,   175,
     117,   118,   119,   120,   121,   122,   123,    50,    51,    52,
      53,    54,   169,    56,   165,   169,   915,   868,   168,   166,
     872,   167,   703,    66,   876,   876,   169,   165,   145,   710,
    1002,   165,   798,   165,  1006,   171,  1008,   170,    73,   165,
     170,   893,   893,   171,   171,   168,    31,   813,    37,    35,
     890,   732,   892,   952,   171,    35,   908,   908,   739,   911,
     168,   960,   114,   946,   170,   169,   168,   168,   168,   952,
     169,    14,    14,   171,   170,    14,   162,   167,  1050,    77,
     169,   847,   934,   982,   169,   766,    35,   768,    35,   855,
      77,   943,   943,   169,   168,   994,    16,    16,   170,   982,
      77,   867,   165,   170,   170,    37,   160,   169,   169,   961,
      98,   877,   115,   160,    16,   168,  1015,   883,   884,   169,
     168,   167,   974,   974,   976,   976,  1013,   460,    54,   790,
     114,  1014,  1031,   117,   118,   119,   120,   121,   122,   123,
     191,    73,    74,    75,    76,    77,   437,   172,    80,   629,
      96,   770,   892,   843,   920,   836,   897,   811,   752,   867,
     670,   145,   843,   439,   845,   442,   982,   864,   994,   991,
     412,  1023,  1023,   427,  1026,  1026,  1028,  1028,   156,  1004,
     946,   367,   399,   374,  1036,   117,   952,   171,   488,   631,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   963,   964,    -1,
    1052,  1052,   134,   135,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,   982,    -1,    -1,    -1,
      -1,   902,    -1,    -1,   905,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,   167,    -1,    -1,    -1,    -1,
     172,    -1,   174,   175,    -1,    -1,    -1,    -1,  1014,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,     3,    56,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    66,    15,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,   989,    -1,
      -1,    -1,   993,    -1,    -1,    -1,    -1,    66,    -1,  1000,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,  1022,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,     9,    10,    11,   172,   173,   174,   175,     3,
      -1,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    15,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    66,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,   168,   156,    12,    13,    -1,    15,    -1,   162,    -1,
      -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    66,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,   133,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,   133,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,   133,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    97,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    95,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    93,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   167,    -1,   169,   170,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
      -1,   110,   111,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   156,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
     169,   170,   171,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,   126,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,   156,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    -1,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    69,    70,    -1,
      -1,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,     3,    -1,     5,     6,     7,     8,    -1,
      -1,    -1,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
      -1,    31,    -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
      -1,    -1,    -1,   103,   104,   105,   106,   107,   108,    -1,
     110,   111,    -1,    -1,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,    -1,   110,   111,    -1,    -1,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
      -1,    -1,    -1,   103,   104,   105,   106,   107,   108,    -1,
     110,   111,    -1,    -1,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,    -1,   110,   111,    -1,    -1,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    83,    84,    85,    86,    -1,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
      -1,    -1,    -1,   103,   104,   105,   106,   107,   108,    -1,
     110,   111,    -1,    -1,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    83,    84,
      85,    86,    -1,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,    -1,   110,   111,    -1,    -1,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      80,    -1,    -1,    83,    84,    85,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,   103,    56,    -1,    -1,   107,   108,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,   162,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    80,    81,    -1,    83,    84,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      80,    -1,    -1,    83,    84,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    80,    -1,    -1,    83,    84,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      80,    -1,    -1,    83,    84,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    80,    -1,    -1,    83,    84,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   156,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    55,    -1,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      80,    -1,    -1,    83,    84,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,   103,    -1,    -1,    -1,   107,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   156,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    80,    -1,    -1,    83,    84,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,   156,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,   167,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    69,    70,    -1,    -1,
      73,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    67,    -1,    69,    70,
      -1,    -1,    73,    74,    75,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
     171,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   171,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    56,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    56,
       9,    10,    11,    -1,    73,    74,    75,    76,    77,    66,
      -1,    80,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    56,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,   135,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,     3,    -1,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,
      -1,    -1,    -1,   172,    -1,   174,   175,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    69,    70,    -1,    -1,    73,    -1,
      -1,    76,    -1,    -1,    -1,    -1,    -1,   166,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
       3,    11,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    56,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    69,    70,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    69,    70,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    14,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    98,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    56,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    56,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    14,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   177,   181,     0,     3,     5,     6,     7,     8,    12,
      13,    15,    50,    51,    55,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    67,    71,    72,    73,    74,    75,
      76,    77,    78,    80,    83,    84,    85,    86,    88,    89,
      90,    92,    94,    96,    99,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   114,   116,   117,   118,   119,   124,
     125,   126,   127,   128,   129,   130,   133,   134,   135,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     156,   167,   169,   170,   172,   173,   174,   175,   185,   188,
     189,   190,   191,   207,   215,   218,   221,   222,   223,   225,
     237,   243,   244,   245,   246,   294,   295,   296,   297,   298,
     306,   307,   312,   313,   314,   315,   317,   318,   319,   320,
     321,   322,   323,   324,   335,    73,   117,   189,   295,   298,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
      31,   167,   117,   128,   167,   185,   189,   292,   307,   308,
     323,   325,   167,   309,   167,   288,   289,   295,   207,   167,
     167,   167,   167,   167,   167,   295,   316,   316,    73,    73,
     204,   287,   316,   170,    73,    74,    76,   107,   109,   183,
     184,   194,   196,   200,   203,   263,   264,   323,    77,   265,
     266,   296,   167,   167,   167,   167,   220,   224,   226,     3,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    56,
      67,    69,    70,    73,    76,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   178,   179,   180,   182,
     193,   167,   167,   186,   187,   307,   167,   172,   312,   314,
     315,   322,   322,    77,    79,   157,   158,   159,   332,   333,
     295,   205,    37,   134,   162,   295,   328,   329,   330,   331,
      79,   310,   332,    79,   332,   170,   323,   167,   260,   117,
     188,   190,   296,   128,   222,    69,    70,    69,    70,    87,
       9,    10,    11,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    56,    66,   169,
      37,   302,   302,   160,   160,   148,   149,   170,   172,   260,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,   146,   147,   302,    14,   295,   293,
     295,   292,   160,   260,   311,   148,   149,   160,   170,   172,
     316,   295,   165,   169,    90,   295,   290,   291,   295,   295,
     204,   295,   295,   169,   169,   169,    16,   165,   169,   169,
     205,    98,   161,   184,   195,   200,   169,   165,   169,   165,
     169,    16,   165,   169,   213,   214,   322,   295,   336,   337,
     295,   168,    73,    73,    73,   169,   192,   170,   328,   328,
     165,   197,   260,   295,   328,   148,   149,   172,   157,   333,
      81,   295,   322,    79,   157,   333,   168,   127,   171,   189,
     190,   206,   207,   322,   167,   295,    14,   166,   165,   173,
     174,   295,     3,     5,     6,     7,     8,    12,    13,    67,
      73,    83,    84,    85,   103,   107,   108,   117,   125,   126,
     134,   135,   137,   138,   139,   140,   141,   142,   143,   144,
     162,   168,   180,   261,   262,   295,   296,   219,   167,   207,
     167,    31,   169,   295,   295,   295,    31,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   308,   295,   299,    73,   299,   170,   180,   323,   326,
     180,   323,   326,    73,   170,   323,   327,   327,   295,   316,
      37,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   168,   311,   168,   323,
     327,   327,   323,   295,   316,   168,   168,   289,   167,   168,
     169,   165,    98,   168,   168,   168,   295,   287,   171,    73,
     170,   161,   169,   169,   184,   203,   264,   295,   266,   165,
     197,   165,   197,   168,   169,   131,   227,   299,   131,   228,
     170,   181,   168,   168,   186,   166,   168,   166,    73,    73,
      51,    73,    77,    82,   334,   171,   172,   171,   171,   167,
     190,   328,    37,   134,   295,    16,   329,   171,   295,    31,
     165,   197,    73,   295,   295,   205,   295,    31,   167,   299,
     167,   295,   260,   260,   295,   260,   260,   171,   166,   322,
     227,   171,   166,    31,   207,   295,    31,   207,   242,   290,
     295,    37,   134,   172,   230,   322,   208,    31,   170,   234,
     170,   299,   209,   183,   194,   198,   201,   202,   170,   214,
     168,   337,   168,   307,   132,   229,   170,   270,   307,   299,
     181,   171,    16,    82,   166,   295,   168,   168,   322,   167,
     295,   295,   262,   168,   227,   168,   168,    87,   295,   120,
     121,   122,   189,   247,   248,   249,   250,   251,   167,   247,
     171,   171,   229,   205,   168,   205,   169,   322,   167,   328,
      14,   168,    31,   207,   233,   169,   235,   169,   235,   102,
     238,   239,   240,   241,   295,   112,   113,   212,    98,   202,
     165,   197,   199,   202,   169,   270,   299,   267,   165,   170,
     171,   295,   166,   169,   328,   229,   207,    31,   169,   251,
     168,   165,   197,    30,   135,   136,   185,   252,   256,   257,
     258,   247,   168,   299,   169,    91,   290,   328,   166,   230,
      31,   207,   232,   205,   235,   100,   101,   102,   235,   171,
     165,   197,   171,   165,   197,   165,   197,   167,   170,    73,
     201,   171,   165,   197,   170,   114,   117,   118,   119,   120,
     121,   122,   123,   171,   189,   268,   269,   279,   280,   281,
     282,   307,   267,   171,   168,   299,   205,    31,   259,   249,
     257,    37,   216,    35,    35,   168,   114,   303,   170,   169,
     168,   168,   168,   205,    97,   100,   169,   295,    31,   169,
     236,   171,    14,   240,   295,    14,   210,   307,   205,   202,
     171,   267,   270,   268,   252,   109,   298,   282,   171,   170,
      30,   117,   253,   254,   255,   257,    14,   162,   217,   257,
     257,   259,   167,   259,   267,    31,   207,   231,   232,    95,
     169,   169,   236,   205,   295,   295,    35,    77,   211,   171,
     171,   169,   170,   271,    77,   283,   284,   180,   285,   286,
     302,   267,   254,    35,    35,   300,    77,   300,    37,    77,
     304,   305,   300,   171,   205,   169,   205,   307,   168,   117,
     171,   180,   272,   273,   274,   275,   276,   277,   307,    16,
     299,   165,   169,    16,   165,   169,   180,   171,   254,   254,
     301,   299,   170,    77,   165,   197,   170,    93,   170,   171,
     273,   169,   169,    98,   115,   160,   295,   284,   295,   286,
     299,   295,    16,   205,   305,   168,   205,   169,   205,    73,
     178,   282,   270,   180,   299,   299,   167,   300,   295,   171,
     171,   171,   180,   247,   300,   300,   168,   259,   300,   169,
     170,   278,   205,   300,   171
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   176,   177,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   179,   179,   179,   179,   179,   179,   179,   180,
     180,   181,   181,   182,   182,   183,   183,   184,   184,   185,
     185,   185,   185,   186,   186,   187,   187,   188,   189,   189,
     190,   190,   190,   190,   191,   191,   191,   191,   191,   192,
     191,   193,   191,   191,   191,   191,   191,   191,   194,   194,
     195,   196,   197,   197,   198,   198,   199,   199,   200,   200,
     201,   201,   202,   202,   203,   203,   204,   204,   205,   205,
     206,   206,   206,   206,   207,   207,   207,   207,   207,   207,
     207,   207,   207,   207,   207,   207,   207,   207,   207,   207,
     207,   207,   208,   207,   207,   207,   207,   207,   209,   209,
     210,   210,   211,   211,   212,   212,   213,   213,   214,   215,
     216,   216,   217,   217,   219,   218,   220,   218,   221,   221,
     222,   222,   224,   223,   226,   225,   227,   227,   228,   228,
     229,   229,   230,   230,   230,   230,   231,   231,   232,   232,
     233,   233,   234,   234,   234,   234,   235,   235,   235,   236,
     236,   237,   238,   238,   239,   239,   240,   240,   241,   241,
     242,   242,   243,   243,   244,   244,   245,   245,   246,   246,
     247,   247,   248,   248,   249,   249,   250,   250,   250,   250,
     251,   251,   252,   252,   253,   253,   253,   254,   254,   255,
     255,   256,   256,   256,   257,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   262,   262,   262,   263,   263,
     264,   265,   265,   266,   266,   267,   267,   268,   268,   268,
     269,   269,   269,   270,   270,   271,   271,   271,   272,   272,
     273,   273,   274,   275,   275,   275,   275,   276,   276,   277,
     278,   278,   279,   279,   280,   280,   281,   281,   282,   282,
     282,   282,   282,   282,   283,   283,   284,   284,   285,   285,
     286,   287,   288,   288,   289,   290,   290,   291,   291,   293,
     292,   294,   294,   294,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   296,   296,   297,   298,
     299,   300,   301,   302,   302,   303,   303,   304,   304,   305,
     305,   306,   306,   306,   306,   307,   307,   308,   308,   308,
     309,   309,   310,   310,   310,   311,   311,   312,   312,   312,
     312,   313,   313,   313,   313,   313,   313,   313,   313,   314,
     314,   314,   314,   314,   314,   314,   314,   314,   315,   315,
     316,   316,   317,   318,   318,   318,   318,   319,   319,   320,
     320,   320,   321,   321,   321,   321,   321,   321,   322,   322,
     322,   322,   323,   323,   323,   324,   324,   325,   325,   325,
     325,   325,   325,   325,   326,   326,   326,   327,   327,   327,
     328,   329,   329,   330,   330,   331,   331,   331,   331,   331,
     331,   331,   332,   332,   332,   332,   333,   333,   333,   333,
     333,   333,   333,   333,   334,   334,   334,   334,   335,   335,
     335,   335,   335,   335,   335,   336,   336,   337
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
       1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     3,     4,     1,     2,
       1,     1,     1,     1,     1,     1,     2,     4,     3,     0,
       6,     0,     5,     3,     4,     3,     4,     3,     1,     1,
       6,     6,     0,     1,     3,     1,     3,     1,     3,     1,
       1,     2,     1,     3,     1,     3,     3,     1,     2,     0,
       1,     1,     2,     4,     3,     1,     1,     5,     7,     9,
       5,     3,     3,     3,     3,     3,     3,     1,     2,     6,
       7,     9,     0,     6,     1,     6,     3,     2,     0,     9,
       1,     3,     0,     1,     0,     4,     1,     3,     1,    13,
       0,     1,     0,     1,     0,    10,     0,     9,     1,     2,
       1,     1,     0,     7,     0,     8,     0,     2,     0,     2,
       0,     2,     1,     2,     4,     3,     1,     4,     1,     4,
       1,     4,     3,     4,     4,     5,     0,     5,     4,     1,
       1,     7,     0,     2,     1,     3,     4,     4,     1,     3,
       1,     4,     5,     6,     1,     3,     6,     7,     3,     6,
       2,     0,     1,     3,     2,     1,     0,     1,     1,     1,
       6,     8,     0,     1,     1,     2,     1,     1,     1,     3,
       3,     1,     2,     1,     1,     1,     1,     3,     3,     0,
       2,     2,     4,     1,     3,     1,     3,     2,     3,     1,
       1,     3,     1,     1,     3,     2,     0,     4,     4,    12,
       1,     2,     3,     1,     3,     1,     2,     3,     1,     2,
       2,     2,     3,     3,     3,     4,     3,     1,     1,     3,
       1,     3,     1,     1,     0,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     4,     3,     1,
       4,     4,     3,     1,     1,     0,     1,     3,     1,     0,
       9,     3,     2,     3,     1,     6,     5,     3,     4,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       5,     4,     3,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     3,     2,     1,     2,     4,     2,
       2,     1,     2,     2,     3,     1,    13,    12,     1,     1,
       0,     0,     0,     0,     1,     0,     5,     3,     1,     1,
       2,     2,     4,     4,     2,     1,     1,     1,     1,     3,
       0,     3,     0,     1,     1,     0,     1,     4,     3,     1,
       3,     1,     1,     3,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       0,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       3,     1,     1,     4,     4,     4,     4,     1,     1,     1,
       3,     3,     1,     4,     2,     3,     3,     1,     4,     4,
       3,     3,     3,     3,     1,     3,     1,     1,     3,     1,
       1,     0,     1,     3,     1,     3,     1,     4,     2,     2,
       6,     4,     2,     2,     1,     2,     1,     4,     3,     3,
       3,     3,     6,     3,     1,     1,     2,     1,     5,     4,
       2,     2,     4,     2,     2,     1,     3,     1
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
#if ZENDDEBUG

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
#else /* !ZENDDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ZENDDEBUG */


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
          case 71: /* "integer"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 72: /* "floating-point number"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 73: /* "identifier"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 74: /* "fully qualified name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 75: /* "namespace-relative name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 76: /* "namespaced name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 77: /* "variable"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 78: /* T_INLINE_HTML  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 79: /* "string content"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 80: /* "quoted string"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 81: /* "variable name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 82: /* "number"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 180: /* identifier  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 181: /* top_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 182: /* namespace_declaration_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 183: /* namespace_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 184: /* legacy_namespace_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 185: /* name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 186: /* attribute_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 187: /* attribute_group  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 188: /* attribute  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 189: /* attributes  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 190: /* attributed_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 191: /* top_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 195: /* group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 196: /* mixed_group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 198: /* inline_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 199: /* unprefixed_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 200: /* use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 201: /* inline_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 202: /* unprefixed_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 203: /* use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 204: /* const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 205: /* inner_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 206: /* inner_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 207: /* statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 209: /* catch_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 210: /* catch_name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 211: /* optional_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 212: /* finally_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 213: /* unset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 214: /* unset_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 215: /* function_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 218: /* class_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 223: /* trait_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 225: /* interface_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 227: /* extends_from  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 228: /* interface_extends_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 229: /* implements_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 230: /* foreach_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 231: /* for_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 232: /* foreach_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 233: /* declare_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 234: /* switch_case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 235: /* case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 237: /* match  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 238: /* match_arm_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 239: /* non_empty_match_arm_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 240: /* match_arm  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 241: /* match_arm_cond_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 242: /* while_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 243: /* if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 244: /* if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 245: /* alt_if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 246: /* alt_if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 247: /* parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 248: /* non_empty_parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 249: /* attributed_parameter  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 251: /* parameter  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 252: /* optional_type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 253: /* type_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 254: /* type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 255: /* union_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 256: /* type_expr_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 257: /* type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 258: /* union_type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 259: /* return_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 260: /* argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 261: /* non_empty_argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 262: /* argument  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 263: /* global_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 264: /* global_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 265: /* static_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 266: /* static_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 267: /* class_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 268: /* attributed_class_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 269: /* class_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 270: /* class_name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 271: /* trait_adaptations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 272: /* trait_adaptation_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 273: /* trait_adaptation  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 274: /* trait_precedence  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 275: /* trait_alias  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 276: /* trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 277: /* absolute_trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 278: /* method_body  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 283: /* property_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 284: /* property  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 285: /* class_const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 286: /* class_const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 287: /* const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 288: /* echo_expr_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 289: /* echo_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 290: /* for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 291: /* non_empty_for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 292: /* anonymous_class  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 294: /* new_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 295: /* expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 296: /* inline_function  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 299: /* backup_doc_comment  */

      { if (((*yyvaluep).str)) zend_string_release_ex(((*yyvaluep).str), 0); }

        break;

    case 303: /* lexical_vars  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 304: /* lexical_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 305: /* lexical_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 306: /* function_call  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 307: /* class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 308: /* class_name_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 309: /* exit_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 310: /* backticks_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 311: /* ctor_arguments  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 312: /* dereferencable_scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 313: /* scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 314: /* constant  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 315: /* class_constant  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 316: /* optional_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 317: /* variable_class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 318: /* fully_dereferencable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 319: /* array_object_dereferencable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 320: /* callable_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 321: /* callable_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 322: /* variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 323: /* simple_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 324: /* static_member  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 325: /* new_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 326: /* member_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 327: /* property_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 328: /* array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 329: /* possible_array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 330: /* non_empty_array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 331: /* array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 332: /* encaps_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 333: /* encaps_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 334: /* encaps_var_offset  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 335: /* internal_functions_in_yacc  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 336: /* isset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 337: /* isset_variable  */

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

  case 79:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 80:

    {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}

    break;

  case 81:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 82:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 83:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 84:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 85:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 86:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 87:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 88:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 89:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }

    break;

  case 90:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }

    break;

  case 91:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }

    break;

  case 92:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }

    break;

  case 93:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[0].ast), NULL); }

    break;

  case 94:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 95:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_GROUP, (yyvsp[0].ast)); }

    break;

  case 96:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 97:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 98:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_LIST, (yyvsp[0].ast)); }

    break;

  case 99:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 100:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 101:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 102:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 103:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 104:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 105:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 106:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 107:

    { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }

    break;

  case 108:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }

    break;

  case 109:

    { RESET_DOC_COMMENT(); }

    break;

  case 110:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 111:

    { RESET_DOC_COMMENT(); }

    break;

  case 112:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }

    break;

  case 113:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 114:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 115:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 116:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 117:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 118:

    { (yyval.num) = ZEND_SYMBOL_FUNCTION; }

    break;

  case 119:

    { (yyval.num) = ZEND_SYMBOL_CONST; }

    break;

  case 120:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 121:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}

    break;

  case 124:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 125:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 126:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 127:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 128:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 129:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 130:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 131:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }

    break;

  case 132:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 133:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 134:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 135:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 136:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 137:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 138:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 139:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 140:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 141:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 142:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 143:

    { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }

    break;

  case 144:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 145:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 146:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 147:

    { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 148:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 149:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 150:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 151:

    { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }

    break;

  case 152:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }

    break;

  case 153:

    { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }

    break;

  case 154:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 155:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 156:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 157:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 158:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 159:

    { (yyval.ast) = (yyvsp[-3].ast); }

    break;

  case 160:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 161:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }

    break;

  case 162:

    { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }

    break;

  case 163:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 164:

    { (yyval.ast) = NULL; }

    break;

  case 165:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 166:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }

    break;

  case 167:

    { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }

    break;

  case 168:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }

    break;

  case 169:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }

    break;

  case 170:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 171:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 172:

    { (yyval.ast) = NULL; }

    break;

  case 173:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 174:

    { (yyval.ast) = NULL; }

    break;

  case 175:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 176:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 177:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 178:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }

    break;

  case 179:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 180:

    { (yyval.num) = 0; }

    break;

  case 181:

    { (yyval.num) = ZEND_PARAM_REF; }

    break;

  case 182:

    { (yyval.num) = 0; }

    break;

  case 183:

    { (yyval.num) = ZEND_PARAM_VARIADIC; }

    break;

  case 184:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 185:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 186:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 187:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 188:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 189:

    { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 190:

    { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }

    break;

  case 191:

    { (yyval.num) = ZEND_ACC_FINAL; }

    break;

  case 192:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 193:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 194:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 195:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 196:

    { (yyval.ast) = NULL; }

    break;

  case 197:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 198:

    { (yyval.ast) = NULL; }

    break;

  case 199:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 200:

    { (yyval.ast) = NULL; }

    break;

  case 201:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 202:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 203:

    { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }

    break;

  case 204:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }

    break;

  case 205:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 206:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 207:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 208:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 209:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 210:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 211:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 212:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 213:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 214:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 215:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 216:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }

    break;

  case 217:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 218:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }

    break;

  case 221:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 222:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }

    break;

  case 223:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 224:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }

    break;

  case 225:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 226:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 227:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }

    break;

  case 228:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }

    break;

  case 229:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 230:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 231:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 232:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 233:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 234:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 235:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }

    break;

  case 236:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 237:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 238:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 239:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }

    break;

  case 240:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 241:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }

    break;

  case 242:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }

    break;

  case 243:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 244:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 245:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 246:

    { (yyval.num) = 0; }

    break;

  case 247:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 248:

    { (yyval.num) = ZEND_ACC_PROTECTED; }

    break;

  case 249:

    { (yyval.num) = ZEND_ACC_PRIVATE; }

    break;

  case 250:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-5].num) | (yyvsp[-3].num) | (yyvsp[-2].num), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL,
					NULL, (yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL); }

    break;

  case 251:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-7].num) | (yyvsp[-5].num) | (yyvsp[-4].num), (yyvsp[-6].ast), (yyvsp[-3].ast), (yyvsp[0].ast),
					NULL, (yyvsp[-2].str) ? zend_ast_create_zval_from_str((yyvsp[-2].str)) : NULL); }

    break;

  case 252:

    { (yyval.ast) = NULL; }

    break;

  case 253:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 254:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 255:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }

    break;

  case 256:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 257:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 258:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }

    break;

  case 259:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 260:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 261:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 262:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }

    break;

  case 263:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 264:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }

    break;

  case 265:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }

    break;

  case 266:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 267:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 268:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 269:

    { (yyval.ast) = NULL; }

    break;

  case 270:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 271:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 272:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 273:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }

    break;

  case 274:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 275:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 276:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 277:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 278:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 279:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 280:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }

    break;

  case 281:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 282:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 283:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }

    break;

  case 284:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 285:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 286:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 287:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 288:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 289:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }

    break;

  case 290:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 291:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 292:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 293:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 294:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 295:

    { (yyval.ast) = NULL; }

    break;

  case 296:

    { (yyval.ast) = NULL; }

    break;

  case 297:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 298:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }

    break;

  case 299:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 300:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 301:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 302:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 303:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 304:

    { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }

    break;

  case 305:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[-1].num), (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 306:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[0].num), (yyvsp[-2].ast), NULL); }

    break;

  case 307:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }

    break;

  case 308:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 309:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 310:

    { (yyval.ast) = NULL; }

    break;

  case 311:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 312:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 313:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 314:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 315:

    { (yyval.num) = (yyvsp[0].num); if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }

    break;

  case 316:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 317:

    { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 318:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 319:

    { (yyval.num) = ZEND_ACC_PROTECTED; }

    break;

  case 320:

    { (yyval.num) = ZEND_ACC_PRIVATE; }

    break;

  case 321:

    { (yyval.num) = ZEND_ACC_STATIC; }

    break;

  case 322:

    { (yyval.num) = ZEND_ACC_ABSTRACT; }

    break;

  case 323:

    { (yyval.num) = ZEND_ACC_FINAL; }

    break;

  case 324:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 325:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }

    break;

  case 326:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 327:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 328:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 329:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 330:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 331:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 332:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 333:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 334:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 335:

    { (yyval.ast) = NULL; }

    break;

  case 336:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 337:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 338:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }

    break;

  case 339:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 340:

    {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS, (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}

    break;

  case 341:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 342:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 343:

    { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 344:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 345:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 346:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 347:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 348:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 349:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }

    break;

  case 350:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 351:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 352:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 353:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 354:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 355:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 356:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 357:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 358:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 359:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 360:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 361:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 362:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 363:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }

    break;

  case 364:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }

    break;

  case 365:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }

    break;

  case 366:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }

    break;

  case 367:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 368:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 369:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 370:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 371:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 372:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 373:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 374:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 375:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 376:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 377:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 378:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 379:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 380:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 381:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 382:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 383:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 384:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }

    break;

  case 385:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }

    break;

  case 386:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }

    break;

  case 387:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }

    break;

  case 388:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 389:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 390:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 391:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 392:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 393:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 394:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 395:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 396:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 397:

    { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 398:

    {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
		}

    break;

  case 399:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 400:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 401:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 402:

    { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 403:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 404:

    { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }

    break;

  case 405:

    { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }

    break;

  case 406:

    { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }

    break;

  case 407:

    { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }

    break;

  case 408:

    { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }

    break;

  case 409:

    { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }

    break;

  case 410:

    { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }

    break;

  case 411:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[0].ast)); }

    break;

  case 412:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }

    break;

  case 413:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 414:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }

    break;

  case 415:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }

    break;

  case 416:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 417:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 418:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 419:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 420:

    { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }

    break;

  case 421:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 422:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 423:

    { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }

    break;

  case 424:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }

    break;

  case 425:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 426:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 427:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0), (yyvsp[-7].ast), NULL,
				  zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)), (yyvsp[-5].ast), NULL);
				  ((zend_ast_decl *) (yyval.ast))->lex_pos = (yyvsp[-2].ptr);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }

    break;

  case 428:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 429:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 430:

    { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }

    break;

  case 431:

    { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }

    break;

  case 432:

    { (yyval.ptr) = LANG_SCNG(yy_text); }

    break;

  case 433:

    { (yyval.num) = 0; }

    break;

  case 434:

    { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }

    break;

  case 435:

    { (yyval.ast) = NULL; }

    break;

  case 436:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 437:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 438:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }

    break;

  case 439:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 440:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }

    break;

  case 441:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 442:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 443:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 444:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 445:

    { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }

    break;

  case 446:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 447:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 448:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 449:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 450:

    { (yyval.ast) = NULL; }

    break;

  case 451:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 452:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 453:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 454:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 455:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 456:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 457:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }

    break;

  case 458:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 459:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 460:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 461:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 462:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 463:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 464:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 465:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 466:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 467:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 468:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 469:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }

    break;

  case 470:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }

    break;

  case 471:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }

    break;

  case 472:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }

    break;

  case 473:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }

    break;

  case 474:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }

    break;

  case 475:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }

    break;

  case 476:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }

    break;

  case 477:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }

    break;

  case 478:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 479:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 480:

    { (yyval.ast) = NULL; }

    break;

  case 481:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 482:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 483:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 484:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 485:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 486:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 487:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 488:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 489:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 490:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 491:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 492:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 493:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 494:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 495:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 496:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 497:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 498:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 499:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 500:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 501:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 502:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 503:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 504:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 505:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 506:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 507:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 508:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 509:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 510:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 511:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 512:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 513:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 514:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 515:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 516:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 517:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 518:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 519:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 520:

    { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }

    break;

  case 521:

    { (yyval.ast) = NULL; }

    break;

  case 522:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 523:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 524:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }

    break;

  case 525:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }

    break;

  case 526:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 527:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }

    break;

  case 528:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }

    break;

  case 529:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 530:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }

    break;

  case 531:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }

    break;

  case 532:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 533:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 534:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }

    break;

  case 535:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 536:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 537:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }

    break;

  case 538:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }

    break;

  case 539:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }

    break;

  case 540:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }

    break;

  case 541:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }

    break;

  case 542:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }

    break;

  case 543:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 544:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 545:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 546:

    { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }

    break;

  case 547:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 548:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 549:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }

    break;

  case 550:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }

    break;

  case 551:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }

    break;

  case 552:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }

    break;

  case 553:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }

    break;

  case 554:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }

    break;

  case 555:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 556:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 557:

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



/* Over-ride Bison formatting routine to give better token descriptions.
   Copy to YYRES the contents of YYSTR for use in yyerror.
   YYSTR is taken from yytname, from the %token declaration.
   If YYRES is null, do not copy; instead, return the length of what
   the result would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	const char *toktype = yystr;
	size_t toktype_len = strlen(toktype);

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
		const unsigned char *tokcontent, *tokcontent_end;
		size_t tokcontent_len;

		CG(parse_error)++;

		if (LANG_SCNG(yy_text)[0] == 0 &&
			LANG_SCNG(yy_leng) == 1 &&
			strcmp(toktype, "\"end of file\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "end of file");
			}
			return sizeof("end of file")-1;
		}

		/* Prevent the backslash getting doubled in the output (eugh) */
		if (strcmp(toktype, "\"'\\\\'\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"\\\"");
			}
			return sizeof("token \"\\\"")-1;
		}

		/* Avoid unreadable """ */
		/* "'" would theoretically be just as bad, but is never currently parsed as a separate token */
		if (strcmp(toktype, "'\"'") == 0) {
			if (yyres) {
				yystpcpy(yyres, "double-quote mark");
			}
			return sizeof("double-quote mark")-1;
		}

		/* Strip off the outer quote marks */
		if (toktype_len >= 2 && *toktype == '"') {
			toktype++;
			toktype_len -= 2;
		}

		/* If the token always has one form, the %token line should have a single-quoted name */
		/* The parser rules also include single-character un-named tokens which will be single-quoted here */
		/* We re-format this with double quotes here to ensure everything's consistent */
		if (toktype_len > 0 && *toktype == '\'') {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "token \"%.*s\"", (int)toktype_len-2, toktype+1);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + sizeof("token ")-1;
		}

		/* Fetch the content of the last seen token from global lexer state */
		tokcontent = LANG_SCNG(yy_text);
		tokcontent_len = LANG_SCNG(yy_leng);

		/* For T_BAD_CHARACTER, the content probably won't be a printable char */
		/* Also, "unexpected invalid character" sounds a bit redundant */
		if (tokcontent_len == 1 && strcmp(yystr, "\"invalid character\"") == 0) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "character 0x%02hhX", *tokcontent);
				yystpcpy(yyres, buffer);
			}
			return sizeof("character 0x00")-1;
		}

		/* Truncate at line end to avoid messing up log formats */
		tokcontent_end = memchr(tokcontent, '\n', tokcontent_len);
		if (tokcontent_end != NULL) {
			tokcontent_len = (tokcontent_end - tokcontent);
		}

		/* Try to be helpful about what kind of string was found, before stripping the quotes */
		if (tokcontent_len > 0 && strcmp(yystr, "\"quoted string\"") == 0) {
			if (*tokcontent == '"') {
				toktype = "double-quoted string";
				toktype_len = sizeof("double-quoted string")-1;
			}
			else if (*tokcontent == '\'') {
				toktype = "single-quoted string";
				toktype_len = sizeof("single-quoted string")-1;
			}
		}

		/* For quoted strings, strip off another layer of quotes to avoid putting quotes inside quotes */
		if (tokcontent_len > 0 && (*tokcontent == '\'' || *tokcontent=='"'))  {
			tokcontent++;
			tokcontent_len--;
		}
		if (tokcontent_len > 0 && (tokcontent[tokcontent_len-1] == '\'' || tokcontent[tokcontent_len-1] == '"'))  {
			tokcontent_len--;
		}

		/* Truncate to 30 characters and add a ... */
		if (tokcontent_len > 30 + sizeof("...")-1) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "%.*s \"%.*s...\"", (int)toktype_len, toktype, 30, tokcontent);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + 30 + sizeof(" \"...\"")-1;
		}

		if (yyres) {
			snprintf(buffer, sizeof(buffer), "%.*s \"%.*s\"", (int)toktype_len, toktype, (int)tokcontent_len, tokcontent);
			yystpcpy(yyres, buffer);
		}
		return toktype_len + tokcontent_len + sizeof(" \"\"")-1;
	}

	/* One of the expected tokens */

	/* Prevent the backslash getting doubled in the output (eugh) */
	if (strcmp(toktype, "\"'\\\\'\"") == 0) {
		if (yyres) {
			yystpcpy(yyres, "\"\\\"");
		}
		return sizeof("\"\\\"")-1;
	}

	/* Strip off the outer quote marks */
	if (toktype_len >= 2 && *toktype == '"') {
		toktype++;
		toktype_len -= 2;
	}

	if (yyres) {
		YYSIZE_T yyn = 0;

		for (; yyn < toktype_len; ++yyn) {
			/* Replace single quotes with double for consistency */
			if (toktype[yyn] == '\'') {
				yyres[yyn] = '"';
			}
			else {
				yyres[yyn] = toktype[yyn];
			}
		}
		yyres[toktype_len] = '\0';
	}

	return toktype_len;
}
