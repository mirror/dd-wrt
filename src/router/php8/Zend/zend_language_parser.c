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
    T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 287,
    T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 288,
    T_IS_EQUAL = 289,
    T_IS_NOT_EQUAL = 290,
    T_IS_IDENTICAL = 291,
    T_IS_NOT_IDENTICAL = 292,
    T_SPACESHIP = 293,
    T_IS_SMALLER_OR_EQUAL = 294,
    T_IS_GREATER_OR_EQUAL = 295,
    T_SL = 296,
    T_SR = 297,
    T_INSTANCEOF = 298,
    T_INT_CAST = 299,
    T_DOUBLE_CAST = 300,
    T_STRING_CAST = 301,
    T_ARRAY_CAST = 302,
    T_OBJECT_CAST = 303,
    T_BOOL_CAST = 304,
    T_UNSET_CAST = 305,
    T_POW = 306,
    T_CLONE = 307,
    T_NOELSE = 308,
    T_ELSEIF = 309,
    T_ELSE = 310,
    T_LNUMBER = 311,
    T_DNUMBER = 312,
    T_STRING = 313,
    T_NAME_FULLY_QUALIFIED = 314,
    T_NAME_RELATIVE = 315,
    T_NAME_QUALIFIED = 316,
    T_VARIABLE = 317,
    T_INLINE_HTML = 318,
    T_ENCAPSED_AND_WHITESPACE = 319,
    T_CONSTANT_ENCAPSED_STRING = 320,
    T_STRING_VARNAME = 321,
    T_NUM_STRING = 322,
    T_EVAL = 323,
    T_NEW = 324,
    T_EXIT = 325,
    T_IF = 326,
    T_ENDIF = 327,
    T_ECHO = 328,
    T_DO = 329,
    T_WHILE = 330,
    T_ENDWHILE = 331,
    T_FOR = 332,
    T_ENDFOR = 333,
    T_FOREACH = 334,
    T_ENDFOREACH = 335,
    T_DECLARE = 336,
    T_ENDDECLARE = 337,
    T_AS = 338,
    T_SWITCH = 339,
    T_ENDSWITCH = 340,
    T_CASE = 341,
    T_DEFAULT = 342,
    T_MATCH = 343,
    T_BREAK = 344,
    T_CONTINUE = 345,
    T_GOTO = 346,
    T_FUNCTION = 347,
    T_FN = 348,
    T_CONST = 349,
    T_RETURN = 350,
    T_TRY = 351,
    T_CATCH = 352,
    T_FINALLY = 353,
    T_USE = 354,
    T_INSTEADOF = 355,
    T_GLOBAL = 356,
    T_STATIC = 357,
    T_ABSTRACT = 358,
    T_FINAL = 359,
    T_PRIVATE = 360,
    T_PROTECTED = 361,
    T_PUBLIC = 362,
    T_READONLY = 363,
    T_VAR = 364,
    T_UNSET = 365,
    T_ISSET = 366,
    T_EMPTY = 367,
    T_HALT_COMPILER = 368,
    T_CLASS = 369,
    T_TRAIT = 370,
    T_INTERFACE = 371,
    T_ENUM = 372,
    T_EXTENDS = 373,
    T_IMPLEMENTS = 374,
    T_NAMESPACE = 375,
    T_LIST = 376,
    T_ARRAY = 377,
    T_CALLABLE = 378,
    T_LINE = 379,
    T_FILE = 380,
    T_DIR = 381,
    T_CLASS_C = 382,
    T_TRAIT_C = 383,
    T_METHOD_C = 384,
    T_FUNC_C = 385,
    T_NS_C = 386,
    T_ATTRIBUTE = 387,
    T_INC = 388,
    T_DEC = 389,
    T_OBJECT_OPERATOR = 390,
    T_NULLSAFE_OBJECT_OPERATOR = 391,
    T_COMMENT = 392,
    T_DOC_COMMENT = 393,
    T_OPEN_TAG = 394,
    T_OPEN_TAG_WITH_ECHO = 395,
    T_CLOSE_TAG = 396,
    T_WHITESPACE = 397,
    T_START_HEREDOC = 398,
    T_END_HEREDOC = 399,
    T_DOLLAR_OPEN_CURLY_BRACES = 400,
    T_CURLY_OPEN = 401,
    T_PAAMAYIM_NEKUDOTAYIM = 402,
    T_NS_SEPARATOR = 403,
    T_ELLIPSIS = 404,
    T_BAD_CHARACTER = 405,
    T_ERROR = 406
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
#define YYLAST   8794

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  179
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  177
/* YYNRULES -- Number of rules.  */
#define YYNRULES  596
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1131

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   406

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   177,     2,   178,    55,     2,     2,
     170,   171,    53,    51,   168,    52,    48,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    31,   172,
      44,    16,    46,    30,    66,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   175,     2,   169,    36,     2,   176,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   173,    35,   174,    58,     2,     2,     2,
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
      26,    27,    28,    29,    32,    33,    34,    37,    38,    39,
      40,    41,    42,    43,    45,    47,    49,    50,    57,    59,
      60,    61,    62,    63,    64,    65,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167
};

#if ZENDDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   295,   295,   299,   299,   299,   299,   299,   299,   299,
     299,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   303,   304,   304,   304,   304,   304,   304,
     304,   304,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   309,   310,   310,   310,   310,   310,   310,
     310,   314,   315,   319,   320,   328,   329,   334,   335,   340,
     341,   346,   347,   351,   352,   353,   354,   358,   360,   365,
     367,   372,   376,   377,   381,   382,   383,   384,   385,   389,
     390,   391,   392,   396,   399,   399,   402,   402,   405,   406,
     407,   408,   409,   413,   414,   418,   423,   428,   429,   433,
     435,   440,   442,   447,   449,   454,   455,   459,   461,   466,
     468,   473,   474,   478,   480,   486,   487,   488,   489,   496,
     497,   498,   499,   501,   503,   505,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   518,   522,   521,   525,
     526,   528,   529,   533,   535,   540,   541,   545,   546,   550,
     551,   555,   556,   560,   564,   565,   573,   580,   581,   585,
     586,   590,   590,   593,   593,   599,   600,   605,   607,   612,
     613,   617,   618,   619,   623,   623,   629,   629,   635,   635,
     641,   642,   646,   651,   652,   656,   657,   661,   662,   666,
     667,   671,   672,   673,   674,   678,   679,   683,   684,   688,
     689,   693,   694,   695,   696,   700,   701,   703,   708,   709,
     714,   719,   720,   724,   725,   729,   731,   736,   737,   742,
     743,   748,   751,   757,   758,   763,   766,   772,   773,   779,
     780,   785,   787,   792,   793,   797,   799,   805,   809,   816,
     817,   821,   822,   823,   824,   828,   829,   833,   834,   838,
     840,   845,   846,   853,   854,   855,   856,   860,   861,   862,
     866,   867,   871,   873,   878,   880,   885,   886,   890,   891,
     892,   896,   898,   903,   904,   906,   910,   911,   915,   921,
     922,   926,   927,   931,   933,   939,   942,   945,   948,   952,
     956,   957,   958,   963,   964,   968,   969,   970,   974,   976,
     981,   982,   986,   991,   993,   997,  1002,  1010,  1012,  1016,
    1021,  1022,  1026,  1029,  1034,  1036,  1043,  1045,  1052,  1054,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1069,  1070,  1074,
    1076,  1081,  1082,  1086,  1087,  1095,  1099,  1100,  1103,  1107,
    1108,  1112,  1113,  1117,  1117,  1127,  1129,  1131,  1136,  1138,
    1140,  1142,  1144,  1146,  1147,  1149,  1151,  1153,  1155,  1157,
    1159,  1161,  1163,  1165,  1167,  1169,  1171,  1173,  1174,  1175,
    1176,  1177,  1179,  1181,  1183,  1185,  1187,  1188,  1189,  1190,
    1191,  1192,  1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,
    1201,  1202,  1203,  1204,  1206,  1208,  1210,  1212,  1214,  1216,
    1218,  1220,  1222,  1224,  1228,  1229,  1231,  1233,  1235,  1236,
    1237,  1238,  1239,  1240,  1241,  1242,  1243,  1244,  1245,  1246,
    1247,  1248,  1249,  1250,  1251,  1252,  1253,  1254,  1255,  1256,
    1258,  1263,  1268,  1276,  1280,  1284,  1288,  1292,  1296,  1297,
    1301,  1302,  1306,  1307,  1311,  1312,  1316,  1318,  1323,  1325,
    1327,  1327,  1334,  1337,  1341,  1342,  1343,  1347,  1348,  1352,
    1354,  1355,  1360,  1361,  1366,  1367,  1368,  1369,  1373,  1374,
    1375,  1376,  1378,  1379,  1380,  1381,  1385,  1386,  1387,  1388,
    1389,  1390,  1391,  1392,  1393,  1397,  1399,  1401,  1403,  1408,
    1409,  1413,  1417,  1418,  1419,  1420,  1424,  1425,  1429,  1430,
    1431,  1435,  1437,  1439,  1441,  1443,  1445,  1449,  1451,  1453,
    1455,  1460,  1461,  1462,  1466,  1468,  1473,  1475,  1477,  1479,
    1481,  1483,  1485,  1490,  1491,  1492,  1496,  1497,  1498,  1502,
    1507,  1508,  1512,  1514,  1519,  1521,  1523,  1525,  1527,  1529,
    1532,  1538,  1540,  1542,  1544,  1549,  1551,  1554,  1557,  1560,
    1562,  1564,  1567,  1571,  1572,  1573,  1574,  1579,  1580,  1581,
    1583,  1585,  1587,  1589,  1594,  1595,  1600
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
  "\"'?""?'\"", "\"'||'\"", "\"'&&'\"", "'|'", "'^'", "\"amp\"", "\"'&'\"",
  "\"'=='\"", "\"'!='\"", "\"'==='\"", "\"'!=='\"", "\"'<=>'\"", "'<'",
  "\"'<='\"", "'>'", "\"'>='\"", "'.'", "\"'<<'\"", "\"'>>'\"", "'+'",
  "'-'", "'*'", "'/'", "'%'", "'!'", "\"'instanceof'\"", "'~'",
  "\"'(int)'\"", "\"'(double)'\"", "\"'(string)'\"", "\"'(array)'\"",
  "\"'(object)'\"", "\"'(bool)'\"", "\"'(unset)'\"", "'@'", "\"'**'\"",
  "\"'clone'\"", "T_NOELSE", "\"'elseif'\"", "\"'else'\"", "\"integer\"",
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
  "\"'protected'\"", "\"'public'\"", "\"'readonly'\"", "\"'var'\"",
  "\"'unset'\"", "\"'isset'\"", "\"'empty'\"", "\"'__halt_compiler'\"",
  "\"'class'\"", "\"'trait'\"", "\"'interface'\"", "\"'enum'\"",
  "\"'extends'\"", "\"'implements'\"", "\"'namespace'\"", "\"'list'\"",
  "\"'array'\"", "\"'callable'\"", "\"'__LINE__'\"", "\"'__FILE__'\"",
  "\"'__DIR__'\"", "\"'__CLASS__'\"", "\"'__TRAIT__'\"",
  "\"'__METHOD__'\"", "\"'__FUNCTION__'\"", "\"'__NAMESPACE__'\"",
  "\"'#['\"", "\"'++'\"", "\"'--'\"", "\"'->'\"", "\"'?->'\"",
  "\"comment\"", "\"doc comment\"", "\"open tag\"", "\"'<?='\"",
  "\"'?>'\"", "\"whitespace\"", "\"heredoc start\"", "\"heredoc end\"",
  "\"'${'\"", "\"'{$'\"", "\"'::'\"", "\"'\\\\'\"", "\"'...'\"",
  "\"invalid character\"", "T_ERROR", "','", "']'", "'('", "')'", "';'",
  "'{'", "'}'", "'['", "'`'", "'\"'", "'$'", "$accept", "start",
  "reserved_non_modifiers", "semi_reserved", "ampersand", "identifier",
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
  "function_name", "function_declaration_statement", "is_reference",
  "is_variadic", "class_declaration_statement", "@4", "@5",
  "class_modifiers", "anonymous_class_modifiers",
  "anonymous_class_modifiers_optional", "class_modifier",
  "trait_declaration_statement", "@6", "interface_declaration_statement",
  "@7", "enum_declaration_statement", "@8", "enum_backing_type",
  "enum_case", "enum_case_expr", "extends_from", "interface_extends_list",
  "implements_list", "foreach_variable", "for_statement",
  "foreach_statement", "declare_statement", "switch_case_list",
  "case_list", "case_separator", "match", "match_arm_list",
  "non_empty_match_arm_list", "match_arm", "match_arm_cond_list",
  "while_statement", "if_stmt_without_else", "if_stmt",
  "alt_if_stmt_without_else", "alt_if_stmt", "parameter_list",
  "non_empty_parameter_list", "attributed_parameter",
  "optional_cpp_modifiers", "parameter", "optional_type_without_static",
  "type_expr", "type", "union_type_element", "union_type",
  "intersection_type", "type_expr_without_static", "type_without_static",
  "union_type_without_static_element", "union_type_without_static",
  "intersection_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "attributed_class_statement", "class_statement", "class_name_list",
  "trait_adaptations", "trait_adaptation_list", "trait_adaptation",
  "trait_precedence", "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "property_modifiers",
  "method_modifiers", "class_const_modifiers",
  "non_empty_member_modifiers", "member_modifier", "property_list",
  "property", "class_const_list", "class_const_decl", "const_decl",
  "echo_expr_list", "echo_expr", "for_exprs", "non_empty_for_exprs",
  "anonymous_class", "@9", "new_expr", "expr", "inline_function", "fn",
  "function", "backup_doc_comment", "backup_fn_flags", "backup_lex_pos",
  "returns_ref", "lexical_vars", "lexical_var_list", "lexical_var",
  "function_call", "@10", "class_name", "class_name_reference",
  "exit_expr", "backticks_expr", "ctor_arguments",
  "dereferenceable_scalar", "scalar", "constant", "class_constant",
  "optional_expr", "variable_class_name", "fully_dereferenceable",
  "array_object_dereferenceable", "callable_expr", "callable_variable",
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
      63,    58,   284,   285,   286,   124,    94,   287,   288,   289,
     290,   291,   292,   293,    60,   294,    62,   295,    46,   296,
     297,    43,    45,    42,    47,    37,    33,   298,   126,   299,
     300,   301,   302,   303,   304,   305,    64,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   406,    44,    93,
      40,    41,    59,   123,   125,    91,    96,    34,    36
};
# endif

#define YYPACT_NINF -931

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-931)))

#define YYTABLE_NINF -568

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-568)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -931,    34,  1753,  -931,  6194,  6194,  6194,  6194,  6194,  6194,
    6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,
    6194,  6194,  6194,  6194,  6194,  -931,  -931,    66,  -931,  -931,
    -931,  -931,  -931,  -931,   -67,   231,   -58,   -35,  6194,  4862,
      12,    38,    70,    90,   111,   125,  6194,  6194,    98,  -931,
    -931,   149,  6194,   131,   535,    24,   275,  -931,  -931,   140,
     160,   201,   205,   210,  -931,  -931,  -931,  -931,  7844,   218,
     220,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,   506,
    7696,  7696,   137,  6194,  -931,  -931,  5010,   167,   202,   -21,
     -56,  -931,   882,  -931,  -931,  -931,  -931,  -931,   495,  -931,
    -931,  -931,  -931,  -931,    93,  -931,    89,  -931,  -931,  7170,
    -931,   237,   237,  -931,   156,   327,  -931,   230,   352,   284,
     303,   391,  -931,   272,   919,  -931,  -931,  -931,  -931,   251,
     140,   229,  8637,   237,  8637,  8637,  8637,  8637,  3826,  8727,
    3826,   161,   161,    52,   161,   161,   161,   161,   161,   161,
     161,   161,   161,  -931,  -931,  6194,  -931,  -931,  6194,  -931,
     254,   367,   264,  -931,  -931,   305,   140,  -931,   542,  6194,
    -931,  6194,   -83,  -931,  8637,   309,  6194,  6194,  6194,   149,
    6194,  6194,  8637,   299,   308,   312,   473,   -17,  -931,   321,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,   -48,   333,   323,
       2,  -931,    65,  -931,  -931,   482,    97,  -931,  -931,  1133,
    -931,  7696,  6194,  6194,   330,   442,   454,   458,   465,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,   369,   378,  5010,  5010,  -931,   390,   140,  6194,  5010,
     400,  -931,  -931,   640,   640,   151,   117,  -931,  5454,  7696,
     152,  -931,  1296,  1901,  -931,  -931,   404,  6194,  7696,  8539,
     439,  -931,   422,  -931,    88,   430,   208,    88,    91,  6194,
    -931,  -931,   251,  -931,  -931,  -931,  -931,  -931,   446,  4862,
     450,   600,   466,  6194,  6194,  6194,  5602,  6194,  6194,  6194,
    6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,
    6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,
    6194,   558,  6194,  -931,  -931,  -931,   -16,  3832,  3977,    53,
      53,  6194,  6194,   140,  5158,  6194,  6194,  6194,  6194,  6194,
    6194,  6194,  6194,  6194,  6194,  6194,  6194,  6194,  -931,  -931,
    -931,  6194,  7219,  7268,  -931,  -931,  -931,    24,  -931,  -931,
      53,    53,    24,  6194,  6194,   469,  7317,  6194,  -931,   471,
    7366,   474,   494,  8637,  8440,   224,  7415,  7464,  -931,  -931,
    -931,  6194,   149,  -931,  -931,  2049,   573,   491,   -44,   499,
     119,  -931,   333,  -931,    24,  -931,  6194,   588,  -931,  6194,
    6194,  6194,  6194,  6194,  6194,  5750,  6194,    19,   -67,   999,
      30,   125,   653,   654,   103,   140,   201,   205,   218,   220,
     656,   658,   661,   664,   666,   673,   675,   677,  5898,  -931,
     678,   545,  -931,  8637,   550,  -931,   280,  8637,   552,  -931,
    7513,   538,   587,  -931,   591,   691,  -931,   556,  -931,   561,
     563,   506,   564,  -931,  7562,   566,   663,   665,   313,  -931,
    -931,   332,  6778,   567,  -931,  -931,  -931,   732,   568,  -931,
     882,  -931,  -931,  -931,  5010,  8637,   425,  5306,   724,  5010,
    -931,  -931,  6827,  -931,   670,  6194,  -931,  6194,  -931,  -931,
    8685,  7982,  3826,  6194,  8588,  6341,  6490,  3976,  6634,  8111,
    5354,  5354,  5502,  5502,  5502,  5502,  5502,  1592,  1592,  1592,
    1592,  1036,  1058,  1058,   606,   606,    52,    52,    52,  -931,
     161,   575,  -931,  -931,  -931,   577,  6194,   578,   580,   140,
    6194,   578,   580,   140,  -931,  6194,  -931,   140,   140,  6876,
     582,  -931,  7696,  3826,  3826,  3826,  3826,  3826,  3826,  3826,
    3826,  3826,  3826,  3826,  3826,  3826,  3826,  3826,  -931,  -931,
     140,  -931,  -931,  -931,  -931,  6925,   592,  -931,  4122,  -931,
    6194,  4270,  6194,  6194,  8390,  -931,    16,   597,  8637,  -931,
    -931,  -931,   246,   602,  -931,  -931,   679,  -931,  -931,  8637,
    -931,  -931,  8637,  6194,  1309,   608,  7696,   610,  6194,   613,
    -931,  -931,   506,   650,   614,   506,  -931,   145,   650,  -931,
    1457,   772,  -931,  -931,  -931,   619,  -931,  -931,  -931,   712,
    -931,  -931,  -931,   627,  -931,  6194,  -931,  -931,   626,  -931,
     628,   630,  7696,  8637,  6194,  -931,  -931,   587,  7611,  7660,
    2197,  6341,  6194,   559,   637,   559,  6974,  -931,  7023,  -931,
    7072,  -931,  -931,  -931,  -931,   640,   587,  -931,  -931,  -931,
    -931,  7709,  -931,  -931,  -931,   633,  8637,   642,  5010,  7696,
       9,    15,  4418,   644,   647,  -931,  6046,  -931,   421,   699,
     182,   646,  -931,  -931,   182,  8637,  6194,  -931,  -931,  -931,
     648,  -931,  -931,  -931,   506,  -931,  -931,   655,  -931,   659,
     512,  -931,  -931,  -931,   512,  -931,  -931,    86,   786,   793,
     794,  -931,  -931,  1605,  -931,  6194,  -931,  -931,  7758,   662,
     772,  5010,   431,  3826,   650,  4862,   804,   668,  6341,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,   645,   674,   669,  -931,
      71,  -931,   926,  -931,   559,   684,   671,   671,  -931,   650,
    3677,   672,  2345,  6194,  5010,   687,    17,  8390,  4566,  -931,
    -931,  -931,  -931,   475,  -931,    40,   692,   689,   693,  -931,
     697,  8637,   696,   694,  -931,   799,  -931,   246,   700,   709,
    -931,  -931,   655,   713,  1379,   506,  -931,  -931,   848,     5,
     512,   434,   434,   512,   717,  -931,  3826,   718,  -931,   722,
    -931,  -931,  -931,  -931,  -931,   863,   635,  -931,   479,   479,
     865,  -931,   166,   866,   869,   871,  -931,   738,   796,  -931,
    -931,   741,   749,   750,    25,   752,  -931,  -931,  -931,  2493,
     553,   753,  6194,    21,    51,  -931,  -931,   910,  -931,  6046,
    -931,  6194,   935,   506,  -931,  -931,  -931,  -931,   182,   777,
    -931,  -931,   506,  -931,  -931,   728,  -931,  -931,  -931,    71,
     844,   846,   808,  -931,  4044,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,   772,   788,  3677,   145,   948,  -931,  -931,
     934,     7,  -931,   798,   479,   399,   399,   479,   863,   803,
     863,   801,  -931,  4714,  -931,  4566,  2641,   805,   806,  -931,
    7121,  -931,  -931,  -931,  -931,  6194,  -931,  8637,  6194,    55,
    -931,  2789,  -931,  -931,  4931,  7989,   121,  -931,   897,   237,
    6639,  -931,  4988,  -931,  -931,  -931,  -931,  -931,   901,  -931,
    -931,  -931,  -931,  -931,  -931,   279,  -931,  -931,  -931,  -931,
    -931,  -931,   809,  -931,  -931,  -931,  3677,  8637,  8637,   506,
    -931,   811,  -931,  -931,   969,  -931,  6349,  -931,   971,   159,
    -931,  7989,   973,   976,   978,   979,   980,  8134,   168,  -931,
    -931,  5079,  -931,  -931,   825,  -931,   925,   839,  -931,   835,
    5136,  2937,  -931,  3677,  -931,   836,  6194,   838,   853,  -931,
    -931,  6494,  -931,   845,   847,   912,   904,   858,  6194,  -931,
     897,  -931,  -931,  6194,  6194,   973,   174,  8134,  -931,  -931,
    6194,  1006,  -931,  -931,   279,   860,  -931,  -931,   852,  -931,
    8637,  -931,  -931,  -931,  -931,  -931,  8279,   506,  7989,  8637,
    -931,   862,  8637,  8637,  -931,  -931,  8637,  6194,  3085,  -931,
    -931,  3233,  -931,  3381,  -931,  -931,  7989,   655,  -931,  -931,
     559,  -931,  -931,  -931,  8637,  -931,  -931,  -931,  -931,   864,
    -931,  -931,   863,  -931,   389,  -931,  -931,  -931,  3529,  -931,
    -931
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      86,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     451,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   498,   499,    93,    95,    96,
      94,   541,   162,   496,     0,   199,   487,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   519,   519,     0,   464,
     463,     0,   519,     0,     0,     0,   482,   201,   202,   203,
       0,     0,     0,     0,   193,   204,   206,   208,   116,     0,
       0,   507,   508,   509,   514,   510,   511,   512,   513,     0,
       0,     0,     0,     0,   169,   144,   560,   489,     0,     0,
     506,   102,     0,   110,    85,   109,   104,   105,     0,   195,
     106,   107,   108,   460,   253,   150,     0,   151,   434,     0,
     456,   468,   468,   536,     0,   503,   448,   504,   505,     0,
     526,     0,   480,   537,   378,   531,   538,   438,    93,   482,
       0,     0,   455,   468,   589,   590,   592,   593,   450,   452,
     454,   419,   420,   421,   422,   439,   440,   441,   442,   443,
     444,   445,   447,   383,   172,     0,   482,   203,     0,   483,
     199,   200,     0,   197,   376,   484,   492,   546,   485,   519,
     446,     0,     0,   367,   368,     0,     0,   369,     0,     0,
       0,     0,   520,     0,     0,     0,     0,     0,   142,     0,
     144,    89,    92,    90,   123,   124,    91,   139,     0,     0,
       0,   134,     0,   307,   308,   311,     0,   310,   458,     0,
     477,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       3,     4,     6,     7,     8,     9,    10,    46,    47,    11,
      13,    16,    17,    83,    88,     5,    12,    14,    15,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    49,    50,    51,    52,    71,    53,    41,    42,    43,
      70,    44,    45,    30,    31,    32,    34,    35,    36,    74,
      75,    76,    77,    78,    79,    80,    37,    38,    39,    40,
      61,    59,    60,    72,    56,    57,    58,    48,    54,    55,
      66,    67,    68,    62,    63,    65,    64,    69,    73,    84,
      87,   114,     0,   560,   560,    99,   127,    97,     0,   560,
     524,   527,   525,   398,   400,   575,     0,   501,     0,     0,
       0,   573,     0,     0,    82,    81,     0,     0,     0,   565,
       0,   563,   559,   561,   490,     0,   491,     0,     0,     0,
     543,   476,     0,   103,   111,   457,   191,   196,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   163,   469,   465,   465,     0,     0,     0,
       0,     0,   519,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   397,   399,
     465,     0,     0,     0,   377,   198,   373,     0,   493,   375,
       0,     0,     0,     0,   519,     0,     0,     0,   161,     0,
       0,     0,   370,   372,     0,     0,     0,     0,   156,   157,
     171,     0,     0,   122,   158,     0,     0,     0,   139,     0,
       0,   118,     0,   120,     0,   159,     0,     0,   160,    33,
       3,     4,     6,     7,    46,   451,    13,    93,     5,    12,
     487,    71,   464,   463,    74,    80,    39,    40,    48,    54,
     507,   508,   509,   514,   510,   511,   512,   513,     0,   298,
       0,   127,   301,   303,   127,   181,   522,   596,   127,   594,
       0,     0,   215,   465,   217,   210,   113,     0,    86,     0,
       0,   128,     0,    98,     0,     0,     0,     0,     0,   500,
     574,     0,     0,   522,   572,   502,   571,   433,     0,   149,
       0,   146,   143,   145,   560,   568,   522,     0,   495,   560,
     449,   497,     0,   459,     0,     0,   254,     0,   144,   257,
     403,   405,   404,     0,     0,   437,   401,   402,   406,   409,
     407,   408,   425,   426,   423,   424,   431,   427,   428,   429,
     430,   410,   417,   418,   411,   412,   413,   415,   416,   432,
     414,     0,   184,   185,   465,     0,     0,   515,   544,     0,
       0,   516,   545,     0,   556,     0,   558,   539,   540,     0,
       0,   481,     0,   381,   384,   385,   386,   388,   389,   390,
     391,   392,   393,   394,   395,   387,   396,   453,   591,   486,
     492,   551,   549,   550,   552,     0,     0,   488,     0,   366,
       0,     0,   369,     0,     0,   167,     0,     0,   465,   141,
     173,   140,     0,     0,   119,   121,   139,   133,   306,   312,
     309,   300,   305,     0,   128,     0,   128,     0,   128,     0,
     588,   112,     0,   219,     0,     0,   465,     0,   219,    86,
       0,     0,   494,   100,   101,   523,   495,   577,   578,     0,
     583,   586,   584,     0,   580,     0,   579,   582,     0,   147,
       0,     0,     0,   564,     0,   562,   542,   215,     0,     0,
       0,   436,     0,   265,     0,   265,     0,   478,     0,   479,
       0,   534,   535,   533,   532,   382,   215,   548,   547,   144,
     251,     0,   144,   249,   152,     0,   371,     0,   560,     0,
       0,   522,     0,   235,   235,   155,   241,   365,   179,   137,
       0,   127,   130,   135,     0,   304,     0,   302,   299,   182,
       0,   595,   587,   216,     0,   465,   314,   218,   323,     0,
       0,   276,   287,   288,     0,   289,   211,   271,     0,   273,
     274,   275,   465,     0,   117,     0,   585,   576,     0,     0,
     570,   560,   522,   380,   219,     0,     0,     0,   435,   353,
     354,   355,   352,   351,   350,   356,   265,     0,   127,   261,
     269,   264,   266,   348,   265,     0,   517,   518,   557,   219,
     255,     0,     0,   369,   560,     0,   522,     0,     0,   144,
     229,   168,   235,     0,   235,     0,   127,     0,   127,   243,
     127,   247,     0,     0,   170,     0,   136,   128,     0,   127,
     132,   164,   220,     0,   344,     0,   314,   272,     0,     0,
       0,     0,     0,     0,     0,   115,   379,     0,   148,     0,
     465,   252,   144,   258,   263,   296,   265,   259,     0,     0,
     187,   270,   283,     0,   285,   286,   349,     0,   470,   465,
     153,     0,     0,     0,   495,     0,   144,   227,   165,     0,
       0,     0,     0,     0,     0,   231,   128,     0,   240,   128,
     242,   128,     0,     0,   144,   138,   129,   126,   128,     0,
     314,   465,     0,   343,   205,   344,   319,   320,   313,   269,
       0,     0,   342,   324,   344,   278,   281,   277,   279,   280,
     282,   314,   581,   569,     0,   256,     0,     0,   262,   284,
       0,     0,   188,   189,     0,     0,     0,     0,   296,     0,
     296,     0,   250,     0,   223,     0,     0,     0,     0,   233,
       0,   238,   239,   144,   232,     0,   244,   248,     0,   177,
     175,     0,   131,   125,   344,     0,     0,   321,     0,   468,
       0,   207,   344,   314,   297,   466,   291,   190,     0,   294,
     290,   292,   293,   295,   466,     0,   466,   314,   144,   225,
     154,   166,     0,   230,   234,   144,   237,   246,   245,     0,
     178,     0,   180,   194,   213,   325,     0,   322,   465,     0,
     358,     0,    93,   276,   287,   288,     0,     0,     0,   362,
     209,   344,   467,   465,     0,   474,     0,   127,   473,     0,
     344,     0,   228,   236,   176,     0,     0,     0,    74,   326,
     337,     0,   328,     0,     0,     0,   338,     0,     0,   359,
       0,   315,   465,     0,     0,     0,     0,     0,   316,   192,
       0,   267,   144,   475,   128,     0,   144,   374,     0,   144,
     214,   212,   327,   329,   330,   331,     0,     0,     0,   465,
     357,     0,   465,   465,   317,   361,   466,     0,     0,   472,
     471,     0,   226,     0,   333,   334,   336,   332,   339,   360,
     265,   363,   364,   462,   268,   466,   466,   174,   335,     0,
     186,   461,   296,   466,     0,   340,   144,   466,     0,   318,
     341
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -931,  -931,   -53,  -869,   -98,   -63,  -486,  -931,   -11,  -190,
     115,   525,  -931,   -47,    -2,     0,  -931,  -931,  -931,   983,
    -931,  -931,  -487,  -931,  -931,   854,   216,  -723,   603,   875,
    -162,  -931,    44,  -931,  -931,  -931,  -931,  -931,  -931,   401,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
       1,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -591,  -931,  -619,   239,  -931,   108,  -931,  -931,  -686,   105,
    -931,  -931,  -931,   173,  -931,  -931,  -931,  -931,  -931,  -931,
    -665,  -931,   192,  -931,   270,   148,  -850,  -518,  -284,  -931,
     314,  -931,  -740,  -323,  -931,   211,  -930,   -49,  -931,   427,
    -931,   625,  -931,   641,  -816,   169,  -931,  -739,  -931,  -931,
      32,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -605,
    -796,  -931,    45,    79,    47,   688,  -931,   680,  -616,  -931,
     960,  -931,  -931,    58,     3,  -931,    -1,  -366,  -872,  -931,
    -109,  -931,  -931,    59,  -931,  -931,   -23,   751,  -931,  -931,
     514,   -71,  -931,   -55,   -43,   -20,  -931,  -931,  -931,  -931,
    -931,   -27,   437,  -931,  -931,   747,   -22,  -216,   598,  -931,
    -931,   540,   562,  -931,  -931,  -931,   492
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   298,   299,   328,   490,     2,   301,   739,   197,
      90,   305,   306,    91,   131,   531,    94,   507,   302,   740,
     449,   199,   512,   741,   839,   200,   742,   743,   201,   187,
     323,   532,   533,   732,   738,   969,  1011,   834,   494,   495,
     584,    96,   943,   988,    97,   544,   215,    98,   161,   162,
      99,   100,   216,   101,   217,   102,   218,   668,   916,  1047,
     663,   666,   755,   730,  1000,   888,   821,   735,   823,   963,
     103,   827,   828,   829,   830,   724,   104,   105,   106,   107,
     797,   798,   799,   800,   801,   870,   766,   767,   768,   769,
     770,   871,   771,   873,   874,   875,   937,   210,   491,   492,
     202,   203,   206,   207,   844,   917,   918,   757,  1017,  1051,
    1052,  1053,  1054,  1055,  1056,  1127,   919,   920,   921,   922,
     803,  1019,  1020,  1028,  1029,   188,   172,   173,   431,   432,
     164,   620,   108,   109,   110,   111,   133,   585,  1032,  1070,
     385,   950,  1037,  1038,   113,   393,   114,   166,   170,   335,
     419,   115,   116,   117,   118,   183,   119,   120,   121,   122,
     123,   124,   125,   126,   168,   589,   597,   330,   331,   332,
     333,   320,   321,   683,   127,   498,   499
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      92,   112,    93,   386,   655,   300,   876,   657,   448,   310,
     310,   659,   165,   384,   384,   842,   725,   836,   994,   581,
     996,   840,   670,   817,   410,   311,   311,   184,   445,  -221,
     924,  -222,   189,   160,     3,   384,   163,   312,   312,  -224,
     805,   341,   853,   196,   947,   343,    95,   733,   825,   772,
     -83,   446,   961,   313,   314,   446,   307,    31,   582,   208,
     872,   -14,   132,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   175,   343,   427,   984,   509,   510,   428,
    1009,   112,   344,   515,   974,   345,   174,   154,   802,   347,
     802,   868,    31,   155,   182,   182,   784,  -483,   583,   381,
     182,  1026,   169,   343,   209,   982,   447,   418,   -83,   382,
     643,  -277,  1034,   850,  1039,   809,   876,   594,   939,   940,
    1027,    31,   208,  1010,   345,   171,   890,   664,   894,   877,
     310,   322,   892,   893,   329,   128,    28,    29,    30,   425,
     159,   442,   339,   892,   893,   443,   311,    89,  1026,   350,
     351,   163,   415,   348,   349,   860,   315,  1031,   312,   315,
     452,   524,   185,   976,   453,   760,   925,   352,   986,   872,
     818,  1040,   176,   773,   496,   972,  -221,   196,  -222,   734,
     879,   802,  1123,   962,   159,   315,  -224,   882,  1026,   802,
     169,  -290,    89,   944,   989,   990,   990,   993,   177,   762,
     763,    49,    50,   412,   895,   315,   413,   316,   704,   128,
      28,    29,    30,   186,  1113,   964,   595,   182,   382,   426,
     315,    89,   524,   454,   430,   433,   434,   455,   436,   437,
     178,   869,   847,  1120,  1121,   315,   848,   334,   310,   318,
     319,  1124,   318,   319,   838,  1129,   191,   310,   513,   193,
     179,   802,   646,   761,   311,   457,  -482,   493,   541,   458,
     497,   500,   737,   311,   324,   325,   312,   519,   318,   319,
     315,   180,   337,   762,   763,   312,   315,   452,   524,   845,
    1106,   645,   523,  1015,  1016,   181,   602,   317,   318,   319,
     759,   536,   516,   517,   190,   128,    28,    29,    30,    31,
     209,   867,   525,   318,   319,   764,   324,   325,   690,   387,
     191,   530,   112,   193,   587,   591,   518,  1060,   318,   319,
     211,  1061,   926,   927,   927,   930,  1067,    49,    50,   897,
    1068,   900,  1067,   902,   601,   543,  1094,   342,  1107,   156,
      57,    58,   909,   205,   194,   157,   195,  1035,   165,    49,
      50,   329,   329,   318,   319,   679,   514,   329,   598,   318,
     319,   212,   600,    57,    58,   213,   522,    79,   157,    79,
     214,  -527,  -527,    49,    50,   535,   700,   680,   303,   843,
     304,   681,   442,   546,   416,   635,   682,   542,   622,   623,
     429,   158,    79,  -527,   626,  -527,   854,   191,   192,    89,
     193,   550,   551,   552,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,   564,   565,   566,   567,   568,   569,
     570,   571,   572,   573,   574,   575,   576,   577,   578,   692,
     580,   196,  -528,   530,   112,  1119,   165,   388,  -183,   599,
     182,  -183,   603,   604,   605,   606,   607,   608,   609,   610,
     611,   612,   613,   614,   615,   616,  -521,   160,   417,   617,
     163,   438,   167,   128,    28,    29,    30,   208,  -524,  -524,
     439,   625,   182,   343,   440,   174,    57,    58,   307,   441,
    -524,   157,   204,   444,   934,   451,   159,  -530,   456,   638,
    -524,   501,  -524,  -525,  -525,   802,   684,   685,   128,    28,
      29,    30,   815,   951,   649,  -525,   502,   132,   134,   135,
     136,   137,   138,   139,   153,  -525,   340,  -525,   503,   112,
     689,   310,   504,   345,   832,   833,   729,   762,   763,   505,
     707,   506,   389,   390,   709,   975,   652,   311,   711,   712,
    1075,   508,   761,   128,    28,    29,    30,   810,   511,   312,
     812,  1125,  1126,   310,   391,   859,   392,   928,   929,   869,
    -530,   418,   762,   763,   534,   715,   891,   892,   893,   311,
     128,    28,    29,    30,   159,   310,   128,    28,    29,    30,
     539,   312,   329,  -567,  -567,   693,  -567,   329,   883,  -566,
    -566,   311,  -566,   698,   764,   699,   540,   731,   538,   191,
     192,   701,   193,   312,    57,    58,   545,   762,   763,   157,
     547,   310,   991,   992,   156,   346,   159,   336,   338,   496,
     761,   548,   128,    28,    29,    30,    31,   311,   549,   753,
     627,   630,   758,   194,   706,   195,   632,   641,   708,   312,
     762,   763,  1059,   710,   958,   892,   893,   889,   310,   378,
     379,   380,   633,   381,   642,   782,   205,  1071,    92,   112,
      93,   644,   720,   382,   311,   723,   156,   789,   790,   791,
     792,   793,   794,   795,   -43,   -70,   312,   -66,   721,   -67,
     433,   726,   -68,   420,   421,   -62,  1091,   -63,   530,   112,
     935,   796,   816,   796,   -65,   422,   -64,    79,   -69,   653,
     661,   745,   493,   654,    95,   423,   497,   424,   656,   729,
     658,   662,   667,  1109,   956,   665,  1111,  1112,   158,   669,
    -260,   758,   671,   674,   672,   676,    89,   677,   688,   678,
     694,   687,   971,   778,   697,   703,   310,   705,  -553,   343,
    -555,   714,   783,   789,   790,   791,   792,   793,   794,   795,
     788,   718,   311,   789,   790,   791,   792,   793,   794,   795,
     736,    92,   112,    93,   312,   744,   820,   159,   446,   748,
     159,   750,   765,    79,   752,   754,   329,   756,   775,  -529,
     731,  -522,  -522,    79,   831,   776,   777,   779,   835,   780,
     781,  1006,   796,  -522,   652,   813,  -128,   804,   530,   112,
     530,   112,   814,  -522,   837,  -522,   822,    95,   167,   824,
     841,   851,   923,   845,   588,   592,   596,   596,   852,   861,
     911,   853,   846,   856,   858,   862,  1041,   866,  -346,   329,
     863,  -554,   915,  1043,   880,   865,   789,   790,   791,   792,
     793,   794,   795,   913,   621,   878,   884,   596,   596,   624,
     896,   899,   887,   898,   796,   901,   903,   904,   343,   159,
    1021,   433,   329,   905,   907,   765,    79,   908,   520,   765,
     970,   384,   526,  -523,  -523,   850,   910,   530,   112,   758,
     931,   204,   932,   933,   936,  -523,   520,  1036,   526,   520,
     526,   945,  -529,   942,   946,  -523,   167,  -523,   947,   948,
    1098,   949,  1014,   952,  1101,   765,  -345,  1103,  -347,   979,
     953,   954,   915,   955,   965,   959,   789,   790,   791,   792,
     793,   794,   795,   530,   112,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   968,
     960,   973,    49,  1050,   530,   112,   980,   831,  1062,   967,
     159,   983,   985,   987,  1128,   765,   765,   765,   765,   530,
     112,   944,   915,   995,   997,  1018,  1036,  1003,  1004,  1033,
     915,  1042,  1045,   765,   765,  1046,  1044,  1058,  1050,  1063,
      49,    50,   -74,  1057,   -54,   -55,  1064,   999,  1072,   887,
     342,    57,    58,  1073,   530,   112,   157,  1074,  1076,  1079,
    1081,  1086,    64,    65,    66,    67,  -482,  1084,   159,  1085,
    1087,  1088,  1097,  1007,  1102,  1108,  1008,   159,  1057,   915,
      79,  1100,  1110,  1105,   765,  1122,   673,   198,   915,   530,
     112,   530,   112,  1118,   789,   790,   791,   792,   793,   794,
     795,   765,   450,   906,   435,   647,   885,   749,   938,   765,
     765,   765,   765,  1001,   758,  1005,   864,   978,   408,   409,
    -522,  -522,   966,   128,    28,    29,    30,    31,   849,   648,
     941,   747,  -522,  1083,   977,   374,   375,   376,   377,   378,
     379,   380,  -522,   381,  -522,   765,   530,   112,   650,   530,
     112,   530,   112,   382,  1080,  1090,  1066,   629,   796,   376,
     377,   378,   379,   380,  1095,   381,  1089,   156,    57,    58,
     414,  1092,  1093,   157,   159,   382,   530,   112,  1096,  -199,
     639,   159,   579,  1099,   716,   593,   459,   695,   460,   461,
     462,   463,   224,   225,   226,   464,   465,    79,    11,     0,
     751,     0,     0,     0,     0,  1114,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   159,     0,     0,   158,
       0,     0,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,    14,
     229,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,   466,   159,   231,   232,    25,    26,   467,    28,    29,
      30,    31,     0,     0,    33,     0,     0,   468,   469,   470,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   471,   256,   257,
     258,   472,   473,   261,   262,   263,   264,   265,   266,   267,
     268,   474,   270,   271,   272,   273,   274,   475,   276,   277,
     476,   477,     0,   280,   281,   282,   283,   284,   285,   286,
     478,   479,   289,   480,   481,   482,   483,   484,   485,   486,
     487,    79,    80,    81,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,   488,     0,
       0,     0,     0,    83,   489,   353,   354,   355,    86,    87,
      88,    89,   459,     0,   460,   461,   462,   463,   224,   225,
     226,   464,   465,     0,    11,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,     0,     0,     0,     0,     0,     0,
      12,    13,     0,   382,     0,    14,   229,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,   466,     0,   231,
     232,    25,    26,   467,    28,    29,    30,    31,     0,     0,
      33,     0,     0,   468,   469,   470,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   471,   256,   257,   258,   472,   473,   261,
     262,   263,   264,   265,   266,   267,   268,   474,   270,   271,
     272,   273,   274,   475,   276,   277,   476,   477,     0,   280,
     281,   282,   283,   284,   285,   286,   478,   479,   289,   480,
     481,   482,   483,   484,   485,   486,   487,    79,    80,    81,
       4,     0,     5,     6,     7,     8,     0,   527,    82,     9,
      10,     0,    11,     0,   746,     0,     0,     0,     0,    83,
       0,   911,     0,     0,    86,    87,    88,    89,     0,  -346,
       0,     0,     0,     0,   912,     0,     0,   789,   790,   791,
     792,   793,   794,   795,   913,     0,     0,     0,    12,    13,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,     0,    79,     0,    25,
      26,    27,    28,    29,    30,    31,    32,     0,    33,     0,
       0,    34,    35,    36,    37,     0,    38,    39,    40,     0,
      41,     0,    42,   914,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,     0,    54,     0,    55,    56,    57,    58,     0,     0,
       0,    59,     0,    60,    61,    62,    63,    64,    65,    66,
      67,     0,     0,    68,    69,    70,     0,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     4,     0,
       5,     6,     7,     8,     0,     0,    82,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    83,     0,    84,
      85,   774,    86,    87,    88,    89,  -568,  -568,  -568,  -568,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
       0,     0,     0,     0,     0,     0,    12,    13,     0,   382,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,     0,
      54,     0,    55,    56,    57,    58,     0,     0,     0,    59,
       0,    60,    61,    62,    63,    64,    65,    66,    67,     0,
       0,    68,    69,    70,     0,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     4,     0,     5,     6,
       7,     8,     0,     0,    82,     9,    10,     0,    11,     0,
       0,     0,     0,     0,     0,    83,     0,    84,    85,   855,
      86,    87,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,     0,     0,     0,    25,    26,    27,    28,    29,
      30,    31,    32,     0,    33,     0,     0,    34,    35,    36,
      37,     0,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,     0,    54,     0,
      55,    56,    57,    58,     0,     0,     0,    59,     0,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    68,
      69,    70,     0,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     4,     0,     5,     6,     7,     8,
       0,     0,    82,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    83,     0,    84,    85,     0,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
       0,     0,     0,    25,    26,    27,    28,    29,    30,    31,
      32,     0,    33,     0,     0,    34,    35,    36,    37,     0,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,     0,    52,    53,     0,     0,     0,     0,    55,    56,
      57,    58,     0,     0,     0,    59,     0,    60,    61,    62,
     528,    64,    65,    66,    67,     0,     0,     0,    69,    70,
       0,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     4,     0,     5,     6,     7,     8,     0,     0,
      82,     9,    10,     0,    11,     0,     0,     0,     0,     0,
       0,    83,     0,    84,    85,   529,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,    57,    58,
       0,     0,     0,    59,     0,    60,    61,    62,   528,    64,
      65,    66,    67,     0,     0,     0,    69,    70,     0,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       4,     0,     5,     6,     7,     8,     0,     0,    82,     9,
      10,     0,    11,     0,     0,     0,     0,     0,     0,    83,
       0,    84,    85,   640,    86,    87,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,     0,     0,     0,    25,
      26,    27,    28,    29,    30,    31,    32,     0,    33,     0,
       0,    34,    35,    36,    37,   787,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,    49,    50,     0,    52,    53,
       0,     0,     0,     0,    55,    56,    57,    58,     0,     0,
       0,    59,     0,    60,    61,    62,   528,    64,    65,    66,
      67,     0,     0,     0,    69,    70,     0,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     4,     0,
       5,     6,     7,     8,     0,     0,    82,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    83,     0,    84,
      85,     0,    86,    87,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,   881,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
       0,    60,    61,    62,   528,    64,    65,    66,    67,     0,
       0,     0,    69,    70,     0,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     4,     0,     5,     6,
       7,     8,     0,     0,    82,     9,    10,     0,    11,     0,
       0,     0,     0,     0,     0,    83,     0,    84,    85,     0,
      86,    87,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,     0,     0,     0,    25,    26,    27,    28,    29,
      30,    31,    32,     0,    33,     0,     0,    34,    35,    36,
      37,     0,    38,    39,    40,     0,    41,     0,    42,     0,
      43,   957,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,     0,    52,    53,     0,     0,     0,     0,
      55,    56,    57,    58,     0,     0,     0,    59,     0,    60,
      61,    62,   528,    64,    65,    66,    67,     0,     0,     0,
      69,    70,     0,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     4,     0,     5,     6,     7,     8,
       0,     0,    82,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    83,     0,    84,    85,     0,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
       0,     0,     0,    25,    26,    27,    28,    29,    30,    31,
      32,     0,    33,     0,     0,    34,    35,    36,    37,     0,
      38,    39,    40,     0,    41,     0,    42,  1002,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,     0,    52,    53,     0,     0,     0,     0,    55,    56,
      57,    58,     0,     0,     0,    59,     0,    60,    61,    62,
     528,    64,    65,    66,    67,     0,     0,     0,    69,    70,
       0,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     4,     0,     5,     6,     7,     8,     0,     0,
      82,     9,    10,     0,    11,     0,     0,     0,     0,     0,
       0,    83,     0,    84,    85,     0,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,    57,    58,
       0,     0,     0,    59,     0,    60,    61,    62,   528,    64,
      65,    66,    67,     0,     0,     0,    69,    70,     0,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       4,     0,     5,     6,     7,     8,     0,     0,    82,     9,
      10,     0,    11,     0,     0,     0,     0,     0,     0,    83,
       0,    84,    85,  1012,    86,    87,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,     0,     0,     0,    25,
      26,    27,    28,    29,    30,    31,    32,     0,    33,     0,
       0,    34,    35,    36,    37,     0,    38,    39,    40,     0,
      41,  1078,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,    49,    50,     0,    52,    53,
       0,     0,     0,     0,    55,    56,    57,    58,     0,     0,
       0,    59,     0,    60,    61,    62,   528,    64,    65,    66,
      67,     0,     0,     0,    69,    70,     0,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     4,     0,
       5,     6,     7,     8,     0,     0,    82,     9,    10,     0,
      11,     0,     0,     0,     0,     0,     0,    83,     0,    84,
      85,     0,    86,    87,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,     0,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,     0,    33,     0,     0,    34,
      35,    36,    37,     0,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,     0,    52,    53,     0,     0,
       0,     0,    55,    56,    57,    58,     0,     0,     0,    59,
       0,    60,    61,    62,   528,    64,    65,    66,    67,     0,
       0,     0,    69,    70,     0,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     4,     0,     5,     6,
       7,     8,     0,     0,    82,     9,    10,     0,    11,     0,
       0,     0,     0,     0,     0,    83,     0,    84,    85,  1115,
      86,    87,    88,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,     0,     0,     0,    25,    26,    27,    28,    29,
      30,    31,    32,     0,    33,     0,     0,    34,    35,    36,
      37,     0,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,     0,    52,    53,     0,     0,     0,     0,
      55,    56,    57,    58,     0,     0,     0,    59,     0,    60,
      61,    62,   528,    64,    65,    66,    67,     0,     0,     0,
      69,    70,     0,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     4,     0,     5,     6,     7,     8,
       0,     0,    82,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,    83,     0,    84,    85,  1116,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
       0,     0,     0,    25,    26,    27,    28,    29,    30,    31,
      32,     0,    33,     0,     0,    34,    35,    36,    37,     0,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,     0,    52,    53,     0,     0,     0,     0,    55,    56,
      57,    58,     0,     0,     0,    59,     0,    60,    61,    62,
     528,    64,    65,    66,    67,     0,     0,     0,    69,    70,
       0,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     4,     0,     5,     6,     7,     8,     0,     0,
      82,     9,    10,     0,    11,     0,     0,     0,     0,     0,
       0,    83,     0,    84,    85,  1117,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,     0,    24,     0,     0,
       0,    25,    26,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,    35,    36,    37,     0,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,     0,
      52,    53,     0,     0,     0,     0,    55,    56,    57,    58,
       0,     0,     0,    59,     0,    60,    61,    62,   528,    64,
      65,    66,    67,     0,     0,     0,    69,    70,     0,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       4,     0,     5,     6,     7,     8,     0,     0,    82,     9,
      10,     0,    11,     0,     0,     0,     0,     0,     0,    83,
       0,    84,    85,  1130,    86,    87,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,     0,    24,     0,     0,     0,    25,
      26,    27,    28,    29,    30,    31,    32,     0,    33,     0,
       0,    34,    35,    36,    37,     0,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,    49,    50,     0,    52,    53,
       0,     0,     0,     0,    55,    56,    57,    58,     0,     0,
       0,    59,     0,    60,    61,    62,   528,    64,    65,    66,
      67,     0,     0,     0,    69,    70,     0,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,     0,   219,    82,   220,   221,   222,
     223,   224,   225,   226,   227,   228,     0,    83,     0,    84,
      85,     0,    86,    87,    88,    89,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,     0,     0,     0,     0,     0,   229,
       0,     0,     0,   382,     0,     0,     0,     0,     0,     0,
     230,     0,   231,   232,     0,     0,   233,     0,     0,     0,
      31,     0,     0,     0,     0,     0,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,     0,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     219,     0,   220,   221,   222,   223,   224,   225,   226,   227,
     228,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   586,     0,     0,     0,     0,
      89,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,   229,     0,     0,     0,     0,     0,
       0,     0,     0,   382,     0,   230,     0,   231,   232,     0,
       0,   233,     0,     0,     0,    31,     0,     0,     0,     0,
       0,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,     0,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,     4,     0,     5,     6,     7,
       8,     0,     0,     0,     9,    10,     0,    11,     0,     0,
       0,     0,     0,     0,     0,     0,   911,     0,     0,     0,
     590,     0,     0,   719,  -346,    89,     0,     0,     0,   912,
       0,     0,   789,   790,   791,   792,   793,   794,   795,   913,
       0,     0,     0,    12,    13,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,     0,    79,     0,    25,    26,    27,    28,    29,    30,
      31,    32,     0,    33,     0,     0,    34,    35,    36,    37,
       0,    38,    39,    40,     0,    41,     0,    42,   981,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,     0,    52,    53,     0,     0,     0,     0,    55,
      56,     0,     0,     0,     0,     0,   130,     0,    60,    61,
      62,     0,     0,     0,     0,     0,     0,     0,     0,    69,
      70,     0,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     4,     0,     5,     6,     7,     8,     0,
       0,    82,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    83,     0,    84,    85,     0,    86,    87,    88,
      89,   722,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,    35,    36,    37,     0,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
       0,    52,    53,     0,     0,     0,     0,    55,    56,     0,
       0,     0,     0,     0,   130,     0,    60,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,    69,    70,     0,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     4,     0,     5,     6,     7,     8,     0,     0,    82,
       9,    10,     0,    11,     0,     0,     0,     0,     0,     0,
      83,     0,    84,    85,     0,    86,    87,    88,    89,   819,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,     0,     0,     0,
      25,    26,    27,    28,    29,    30,    31,    32,     0,    33,
       0,     0,    34,    35,    36,    37,     0,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,    49,    50,     0,    52,
      53,     0,     0,     0,     0,    55,    56,     0,     0,     0,
       0,     0,   130,     0,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    70,     0,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     4,
       0,     5,     6,     7,     8,     0,     0,    82,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    83,     0,
      84,    85,     0,    86,    87,    88,    89,   886,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,     0,     0,     0,    25,    26,
      27,    28,    29,    30,    31,    32,     0,    33,     0,     0,
      34,    35,    36,    37,     0,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,    49,    50,     0,    52,    53,     0,
       0,     0,     0,    55,    56,     0,     0,     0,     0,     0,
     130,     0,    60,    61,    62,     0,     0,     0,     0,     0,
       0,     0,     0,    69,    70,     0,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     4,     0,     5,
       6,     7,     8,     0,     0,    82,     9,    10,     0,    11,
       0,     0,     0,     0,     0,     0,    83,     0,    84,    85,
       0,    86,    87,    88,    89,   998,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,    27,    28,
      29,    30,    31,    32,     0,    33,     0,     0,    34,    35,
      36,    37,     0,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,    49,    50,     0,    52,    53,     0,     0,     0,
       0,    55,    56,     0,     0,     0,     0,     0,   130,     0,
      60,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,    69,    70,     0,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     4,     0,     5,     6,     7,
       8,     0,     0,    82,     9,    10,     0,    11,     0,     0,
       0,     0,     0,     0,    83,     0,    84,    85,     0,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,     0,     0,     0,    25,    26,    27,    28,    29,    30,
      31,    32,     0,    33,     0,     0,    34,    35,    36,    37,
       0,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,     0,    52,    53,     0,     0,     0,     0,    55,
      56,     0,     0,     0,     0,     0,   130,     0,    60,    61,
      62,     0,     0,     0,     0,     0,     0,     0,     0,    69,
      70,     0,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     4,     0,     5,     6,     7,     8,     0,
       0,    82,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    83,   911,    84,    85,     0,    86,    87,    88,
      89,  -346,     0,     0,     0,     0,   912,   324,   325,   789,
     790,   791,   792,   793,   794,   795,   913,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,    79,
       0,     0,    25,    26,   128,    28,    29,    30,    31,     0,
     911,    33,     0,     0,    34,    35,    36,     0,  -346,     0,
       0,     0,     0,   912,     0,  1013,   789,   790,   791,   792,
     793,   794,   795,   913,    45,     0,     0,     0,    49,    50,
       0,     0,     0,     0,     0,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   130,     0,    79,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,   326,    70,     0,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     4,  1030,     5,     6,     7,     8,     0,     0,    82,
       9,    10,     0,    11,     0,   327,     0,     0,     0,     0,
      83,   911,     0,     0,     0,    86,    87,    88,    89,  -346,
       0,     0,     0,     0,   912,   324,   325,   789,   790,   791,
     792,   793,   794,   795,   913,     0,     0,     0,     0,    12,
      13,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,    79,     0,     0,
      25,    26,   128,    28,    29,    30,    31,     0,   911,    33,
       0,     0,    34,    35,    36,     0,  -346,     0,     0,     0,
       0,   912,     0,  1069,   789,   790,   791,   792,   793,   794,
     795,   913,    45,     0,     0,     0,    49,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   129,     0,     0,     0,
       0,     0,   130,     0,    79,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    70,     0,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     4,
    1077,     5,     6,     7,     8,     0,     0,    82,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    83,     0,
       0,     0,     0,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,   324,   325,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,     0,     0,     0,    25,    26,
     128,    28,    29,    30,    31,     0,     0,    33,     0,     0,
      34,    35,    36,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
      45,   381,     0,     0,    49,    50,     0,     0,     0,     0,
       0,   382,     0,     0,   129,     0,     0,     0,     0,     0,
     130,     0,     0,    61,    62,     0,     0,     0,     0,     0,
       0,     0,     0,   691,    70,     0,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     4,     0,     5,
       6,     7,     8,     0,     0,    82,     9,    10,     0,    11,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    86,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   128,    28,
      29,    30,    31,     0,     0,    33,   521,     0,    34,    35,
      36,  -568,  -568,  -568,  -568,  -568,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,    45,   381,
       0,     0,    49,    50,     0,     0,     0,     0,     0,   382,
       0,     0,   129,     0,     0,     0,     0,     0,   130,     0,
       0,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,    69,    70,     0,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     4,     0,     5,     6,     7,
       8,     0,     0,    82,     9,    10,     0,    11,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,    86,
      87,    88,    89,   553,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,     0,     0,     0,    25,    26,   128,    28,    29,    30,
      31,     0,     0,    33,     0,     0,    34,    35,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    45,     0,     0,     0,
      49,    50,     0,     0,     0,     0,     0,     0,     0,     0,
     129,     0,     0,     0,     0,     0,   130,     0,     0,    61,
      62,     0,     0,     0,     0,     0,     0,     0,     0,    69,
      70,     0,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     4,     0,     5,     6,     7,     8,     0,
       0,    82,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,     0,    86,    87,    88,
      89,   -47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,     0,
       0,     0,    25,    26,   128,    28,    29,    30,    31,     0,
       0,    33,     0,     0,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    45,     0,     0,     0,    49,    50,
       0,     0,     0,     0,     0,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   130,     0,     0,    61,    62,     0,
       0,     0,     0,     0,     0,     0,     0,    69,    70,     0,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     4,     0,     5,     6,     7,     8,     0,     0,    82,
       9,    10,     0,    11,     0,     0,     0,     0,     0,     0,
      83,     0,     0,     0,     0,    86,    87,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,     0,    24,     0,     0,     0,
      25,    26,   128,    28,    29,    30,    31,     0,     0,    33,
       0,     0,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    45,     0,     0,     0,    49,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   129,     0,     0,     0,
       0,     0,   130,     0,     0,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,    69,    70,     0,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     4,
       0,     5,     6,     7,     8,     0,     0,    82,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,    83,   651,
       0,     0,     0,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,     0,     0,     0,    25,    26,
     128,    28,    29,    30,    31,     0,     0,    33,     0,     0,
      34,    35,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   826,
      45,     0,     0,     0,    49,    50,     0,     0,     0,     0,
       0,     0,     0,     0,   129,     0,     0,     0,     0,     0,
     130,     0,     0,    61,    62,     0,     0,     0,     0,     0,
       0,     0,     0,    69,    70,     0,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     4,     0,     5,
       6,     7,     8,     0,     0,    82,     9,    10,     0,    11,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    86,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,     0,     0,     0,    25,    26,   128,    28,
      29,    30,    31,     0,     0,    33,     0,     0,    34,    35,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    45,     0,
       0,     0,    49,    50,     0,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,     0,     0,   130,     0,
       0,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,    69,    70,     0,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,     0,   219,    82,   220,   221,   222,   223,   224,   225,
     226,   227,   228,     0,    83,     0,     0,     0,     0,    86,
      87,    88,    89,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,     0,   381,     0,
       0,     0,     0,     0,     0,     0,   229,     0,   382,     0,
       0,     0,     0,     0,     0,     0,     0,   230,     0,   231,
     232,     0,     0,   467,    28,    29,    30,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,  1048,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   219,     0,   220,
     221,   222,   223,   224,   225,   226,   227,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1049,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,     0,     0,
       0,   229,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,   230,     0,   231,   232,     0,     0,   467,    28,
      29,    30,     0,     0,     0,     0,     0,     0,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,  1048,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,     0,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   219,     0,   220,   221,   222,   223,   224,   225,
     226,   227,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1082,   760,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,   229,     0,     0,     0,
       0,   382,     0,     0,     0,     0,     0,   230,     0,   231,
     232,     0,     0,  1022,    28,    29,    30,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,  1023,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,     0,   280,
     281,   282,   283,   284,   285,   286,   287,  1024,  1025,   290,
     291,   292,   293,   294,   295,   296,   297,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   356,   764,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,     0,   381,   353,   354,   355,     0,
       0,     0,     0,     0,     0,   382,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,     0,   381,   353,   354,   355,     0,     0,
       0,     0,     0,     0,   382,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,   353,   354,   355,     0,     0,     0,
       0,     0,     0,   382,     0,     0,     0,     0,     0,     0,
       0,     0,   686,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,     0,   381,   353,   354,   355,     0,     0,     0,     0,
       0,     0,   382,     0,     0,     0,     0,     0,     0,     0,
       0,   696,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,   353,   354,   355,     0,     0,     0,     0,     0,
       0,   382,     0,     0,     0,     0,     0,     0,     0,     0,
     713,     0,     0,   356,     0,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,     0,
     381,   353,   354,   355,     0,     0,     0,     0,     0,     0,
     382,     0,     0,     0,     0,     0,     0,     0,     0,   717,
       0,     0,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
     353,   354,   355,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,     0,   806,     0,
       0,   356,   961,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,     0,   381,   353,
     354,   355,     0,     0,     0,     0,     0,     0,   382,     0,
       0,     0,     0,     0,     0,     0,     0,   807,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   353,   354,
     355,     0,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,     0,     0,     0,     0,   808,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,   353,   354,   355,
       0,     0,     0,     0,     0,     0,   382,     0,     0,     0,
       0,     0,     0,   962,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,     0,   381,   353,   354,   355,     0,
       0,     0,     0,     0,     0,   382,     0,     0,     0,     0,
       0,     0,   383,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,     0,   381,   353,   354,   355,     0,     0,
       0,     0,     0,     0,   382,     0,     0,     0,     0,     0,
     618,     0,     0,     0,     0,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,   353,   354,   355,     0,     0,     0,
       0,     0,     0,   382,     0,     0,     0,     0,     0,   619,
       0,     0,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,     0,   381,   353,   354,   355,     0,     0,     0,     0,
       0,     0,   382,     0,     0,     0,     0,     0,   628,     0,
       0,     0,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,   353,   354,   355,     0,     0,     0,     0,     0,
       0,   382,     0,     0,     0,     0,     0,   631,     0,     0,
       0,     0,     0,   356,     0,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,     0,
     381,   353,   354,   355,     0,     0,     0,     0,     0,     0,
     382,     0,     0,     0,     0,     0,   636,     0,     0,     0,
       0,     0,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
     353,   354,   355,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,   637,     0,     0,     0,     0,
       0,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,     0,   381,   353,
     354,   355,     0,     0,     0,     0,     0,     0,   382,     0,
       0,     0,     0,     0,   660,     0,     0,     0,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   353,   354,
     355,     0,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,     0,   675,     0,     0,     0,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,   353,   354,   355,
     128,    28,    29,    30,    31,     0,   382,    33,     0,     0,
       0,     0,   785,     0,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   156,   381,     0,     0,     0,     0,
     130,     0,     0,     0,     0,   382,     0,     0,     0,     0,
       0,   786,     0,     0,    70,     0,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,   219,     0,   220,
     221,   222,   223,   224,   225,   226,   227,   228,     0,     0,
       0,     0,     0,     0,     0,     0,   308,     0,     0,     0,
       0,   309,     0,    88,    89,     0,     0,     0,     0,     0,
     811,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,   231,   232,     0,     0,   233,     0,
       0,   234,     0,     0,     0,     0,     0,   857,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,     0,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   219,   355,   220,   221,   222,   223,   224,   225,
     226,   227,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
       0,     0,     0,     0,     0,     0,   229,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,   230,     0,   231,
     232,     0,     0,   233,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   219,     0,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,     0,   381,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   382,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,   231,   232,     0,     0,  1065,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,     0,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   219,     0,   220,   221,   222,   223,   224,   225,
     226,   227,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   230,     0,   231,
     232,     0,     0,  1104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   789,   790,   791,
     792,   793,   794,   795,   276,   277,   278,   279,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   324,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   128,    28,    29,    30,    31,     0,
     356,    33,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   382,   156,     0,
       0,     0,     0,     0,   130,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   727,    70,     0,
      71,    72,    73,    74,    75,    76,    77,    78,     0,   634,
       0,     0,     0,     0,     0,     0,     0,     0,   353,   354,
     355,     0,     0,   537,     0,     0,     0,     0,     0,     0,
     308,     0,     0,     0,     0,   728,     0,    88,    89,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,   353,   354,   355,
       0,     0,     0,     0,     0,     0,   382,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   356,   702,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,     0,   381,   353,   354,   355,     0,
       0,     0,     0,     0,     0,   382,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,     0,   381,   354,   355,     0,     0,     0,
       0,     0,     0,     0,   382,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   411,   381,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   382,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,     0,   381,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   382
};

static const yytype_int16 yycheck[] =
{
       2,     2,     2,   112,   491,    68,   802,   494,   198,    80,
      81,   498,    35,   111,   112,   754,   632,   740,   948,   385,
     950,   744,   508,    14,   133,    80,    81,    47,   190,    14,
     846,    14,    52,    35,     0,   133,    35,    80,    81,    14,
     705,    90,    37,    54,    37,    92,     2,    31,   734,   668,
      31,    99,    31,    80,    81,    99,    79,    78,    74,    56,
     800,    31,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    39,   131,   168,   936,   303,   304,   172,
      35,    92,    92,   309,   910,    92,    38,    31,   703,    98,
     705,    30,    78,   170,    46,    47,   697,   163,   124,    57,
      52,   980,   170,   160,   170,   931,   164,   166,    99,    67,
     164,    35,   994,    37,   996,   716,   922,    74,   868,   869,
     980,    78,   129,    78,   131,   170,   822,   503,   824,   804,
     211,    83,   102,   103,    86,    74,    75,    76,    77,   169,
      35,   168,   173,   102,   103,   172,   211,   178,  1027,    70,
      71,   160,   161,    70,    71,   784,    78,   983,   211,    78,
     168,    80,    74,   912,   172,    30,   171,    88,   171,   919,
     171,   997,   170,   669,   211,   908,   171,   198,   171,   173,
     809,   796,  1122,   172,    79,    78,   171,   813,  1067,   804,
     170,    35,   178,    37,   944,   945,   946,   947,   170,   138,
     139,   108,   109,   155,   174,    78,   158,    80,   584,    74,
      75,    76,    77,    74,  1096,   174,   173,   169,    67,   171,
      78,   178,    80,   168,   176,   177,   178,   172,   180,   181,
     170,   170,   760,  1115,  1116,    78,   764,    80,   319,   161,
     162,  1123,   161,   162,   741,  1127,    74,   328,   307,    77,
     170,   866,   452,   118,   319,   168,   163,   209,   177,   172,
     212,   213,   638,   328,    37,    38,   319,   160,   161,   162,
      78,   170,    80,   138,   139,   328,    78,   168,    80,   168,
    1086,   172,   319,   172,   173,   170,   394,   160,   161,   162,
     666,   328,   151,   152,   173,    74,    75,    76,    77,    78,
     170,   798,   160,   161,   162,   170,    37,    38,   534,   163,
      74,   323,   323,    77,   387,   388,   175,   168,   161,   162,
     170,   172,   850,   851,   852,   853,   168,   108,   109,   826,
     172,   828,   168,   830,   393,   342,   172,   118,  1087,   118,
     119,   120,   839,    78,   108,   124,   110,    78,   381,   108,
     109,   303,   304,   161,   162,    52,   308,   309,   390,   161,
     162,   170,   392,   119,   120,   170,   318,   148,   124,   148,
     170,   151,   152,   108,   109,   327,   548,    74,   170,   755,
     170,    78,   168,   349,   130,   171,    83,   339,   420,   421,
      91,   170,   148,   173,   424,   175,   772,    74,    75,   178,
      77,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   537,
     382,   452,   170,   445,   445,  1110,   469,   163,   168,   391,
     392,   171,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   404,   405,   406,   407,   163,   469,   163,   411,
     469,   172,    35,    74,    75,    76,    77,   474,   151,   152,
     172,   423,   424,   530,   172,   427,   119,   120,   511,    16,
     163,   124,    55,   172,   860,   172,   381,   170,    16,   441,
     173,   171,   175,   151,   152,  1110,   174,   175,    74,    75,
      76,    77,   728,   879,   456,   163,    74,   459,   460,   461,
     462,   463,   464,   465,   466,   173,    89,   175,    74,   530,
     530,   602,    74,   530,   113,   114,   634,   138,   139,    74,
     589,   172,   151,   152,   593,   911,   488,   602,   597,   598,
    1037,   173,   118,    74,    75,    76,    77,   719,   168,   602,
     722,   172,   173,   634,   173,   781,   175,   851,   852,   170,
     170,   620,   138,   139,   170,   602,   101,   102,   103,   634,
      74,    75,    76,    77,   469,   656,    74,    75,    76,    77,
     168,   634,   534,   168,   169,   537,   171,   539,   814,   168,
     169,   656,   171,   545,   170,   547,   176,   634,   169,    74,
      75,   553,    77,   656,   119,   120,   170,   138,   139,   124,
     170,   692,   945,   946,   118,   130,   511,    87,    88,   656,
     118,    31,    74,    75,    76,    77,    78,   692,   172,   662,
     171,   170,   665,   108,   586,   110,   172,    74,   590,   692,
     138,   139,  1018,   595,   101,   102,   103,   819,   729,    53,
      54,    55,   168,    57,   173,   692,    78,  1033,   670,   670,
     670,   172,   628,    67,   729,   631,   118,   118,   119,   120,
     121,   122,   123,   124,    31,    31,   729,    31,   630,    31,
     632,   633,    31,   151,   152,    31,  1062,    31,   700,   700,
     862,   703,   729,   705,    31,   163,    31,   148,    31,    31,
     172,   653,   654,   168,   670,   173,   658,   175,   168,   817,
     168,   134,    31,  1089,   886,   134,  1092,  1093,   170,   173,
     171,   754,   171,   169,   171,   169,   178,    74,   170,    74,
      16,   174,   904,   685,    74,   170,   817,   170,   170,   796,
     170,   169,   694,   118,   119,   120,   121,   122,   123,   124,
     702,   169,   817,   118,   119,   120,   121,   122,   123,   124,
     173,   773,   773,   773,   817,   173,   732,   662,    99,   171,
     665,   171,   667,   148,   171,   135,   728,   173,    16,   170,
     817,   151,   152,   148,   736,    83,   169,   171,    99,   171,
     170,   963,   804,   163,   746,   172,   171,   170,   810,   810,
     812,   812,   170,   173,   168,   175,   172,   773,   381,   172,
     172,    35,   845,   168,   387,   388,   389,   390,    35,   785,
     102,    37,   173,   775,   172,    31,   998,   168,   110,   781,
     172,   170,   844,  1005,   172,   171,   118,   119,   120,   121,
     122,   123,   124,   125,   417,   171,   169,   420,   421,   422,
     168,   168,   818,   174,   866,   168,   170,   173,   915,   754,
     979,   813,   814,    74,   174,   760,   148,   168,   316,   764,
     903,   979,   320,   151,   152,    37,   173,   889,   889,   912,
     173,   454,   174,   171,    31,   163,   334,   995,   336,   337,
     338,    35,   170,    38,    35,   173,   469,   175,    37,   171,
    1072,   115,   975,   172,  1076,   800,   108,  1079,   110,   920,
     171,   171,   924,   171,    14,   172,   118,   119,   120,   121,
     122,   123,   124,   935,   935,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    14,
     892,   174,   108,  1016,   956,   956,   110,   899,  1021,   901,
     845,   173,    14,   165,  1126,   850,   851,   852,   853,   971,
     971,    37,   974,   170,   173,    78,  1074,   172,   172,    78,
     982,   172,   171,   868,   869,    16,  1009,    16,  1051,    16,
     108,   109,    16,  1016,    16,    16,    16,   953,   173,   955,
     118,   119,   120,    78,  1006,  1006,   124,   168,   173,   173,
     172,    99,   130,   131,   132,   133,   163,   172,   903,   172,
     116,   163,    16,   965,   172,  1088,   968,   912,  1051,  1031,
     148,   171,   170,  1086,   919,   171,   511,    54,  1040,  1041,
    1041,  1043,  1043,  1106,   118,   119,   120,   121,   122,   123,
     124,   936,   198,   837,   179,   452,   817,   656,   866,   944,
     945,   946,   947,   955,  1087,   960,   796,   919,   149,   150,
     151,   152,   899,    74,    75,    76,    77,    78,   764,   454,
     869,   654,   163,  1051,   915,    49,    50,    51,    52,    53,
      54,    55,   173,    57,   175,   980,  1098,  1098,   457,  1101,
    1101,  1103,  1103,    67,  1046,  1060,  1027,   427,  1110,    51,
      52,    53,    54,    55,  1067,    57,  1058,   118,   119,   120,
     160,  1063,  1064,   124,  1009,    67,  1128,  1128,  1070,   130,
     442,  1016,   381,  1074,   620,   388,     3,   539,     5,     6,
       7,     8,     9,    10,    11,    12,    13,   148,    15,    -1,
     658,    -1,    -1,    -1,    -1,  1097,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1051,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,  1087,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    -1,    -1,    81,    -1,    -1,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,   165,    -1,
      -1,    -1,    -1,   170,   171,     9,    10,    11,   175,   176,
     177,   178,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    15,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    67,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    -1,    -1,
      81,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       3,    -1,     5,     6,     7,     8,    -1,   171,   159,    12,
      13,    -1,    15,    -1,   165,    -1,    -1,    -1,    -1,   170,
      -1,   102,    -1,    -1,   175,   176,   177,   178,    -1,   110,
      -1,    -1,    -1,    -1,   115,    -1,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,   148,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    84,    85,    86,    87,    -1,    89,    90,    91,    -1,
      93,    -1,    95,   174,    97,    -1,    -1,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,   115,    -1,   117,   118,   119,   120,    -1,    -1,
      -1,   124,    -1,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,   136,   137,   138,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   159,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,
     173,   174,   175,   176,   177,   178,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    67,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    84,
      85,    86,    87,    -1,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
     115,    -1,   117,   118,   119,   120,    -1,    -1,    -1,   124,
      -1,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
      -1,   136,   137,   138,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     3,    -1,     5,     6,
       7,     8,    -1,    -1,   159,    12,    13,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,   174,
     175,   176,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    84,    85,    86,
      87,    -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,   115,    -1,
     117,   118,   119,   120,    -1,    -1,    -1,   124,    -1,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,   136,
     137,   138,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   172,   173,    -1,   175,   176,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    84,    85,    86,    87,    -1,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,    -1,   111,   112,    -1,    -1,    -1,    -1,   117,   118,
     119,   120,    -1,    -1,    -1,   124,    -1,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,     3,    -1,     5,     6,     7,     8,    -1,    -1,
     159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    -1,   172,   173,   174,   175,   176,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    84,    85,    86,    87,    -1,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,    -1,
     111,   112,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
      -1,    -1,    -1,   124,    -1,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       3,    -1,     5,     6,     7,     8,    -1,    -1,   159,    12,
      13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,
      -1,   172,   173,   174,   175,   176,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,    -1,   111,   112,
      -1,    -1,    -1,    -1,   117,   118,   119,   120,    -1,    -1,
      -1,   124,    -1,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   159,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,
     173,    -1,   175,   176,   177,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    84,
      85,    86,    87,    -1,    89,    90,    91,    92,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,    -1,   111,   112,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,    -1,    -1,    -1,   124,
      -1,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     3,    -1,     5,     6,
       7,     8,    -1,    -1,   159,    12,    13,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,    -1,
     175,   176,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    84,    85,    86,
      87,    -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    98,    -1,   100,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,    -1,   111,   112,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,    -1,    -1,    -1,   124,    -1,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   172,   173,    -1,   175,   176,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    84,    85,    86,    87,    -1,
      89,    90,    91,    -1,    93,    -1,    95,    96,    97,    -1,
      -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,    -1,   111,   112,    -1,    -1,    -1,    -1,   117,   118,
     119,   120,    -1,    -1,    -1,   124,    -1,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,     3,    -1,     5,     6,     7,     8,    -1,    -1,
     159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    -1,   172,   173,    -1,   175,   176,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    84,    85,    86,    87,    -1,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,    -1,
     111,   112,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
      -1,    -1,    -1,   124,    -1,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       3,    -1,     5,     6,     7,     8,    -1,    -1,   159,    12,
      13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,
      -1,   172,   173,   174,   175,   176,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    84,    85,    86,    87,    -1,    89,    90,    91,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,    -1,   111,   112,
      -1,    -1,    -1,    -1,   117,   118,   119,   120,    -1,    -1,
      -1,   124,    -1,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,     3,    -1,
       5,     6,     7,     8,    -1,    -1,   159,    12,    13,    -1,
      15,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,
     173,    -1,   175,   176,   177,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    84,
      85,    86,    87,    -1,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,    -1,   111,   112,    -1,    -1,
      -1,    -1,   117,   118,   119,   120,    -1,    -1,    -1,   124,
      -1,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,     3,    -1,     5,     6,
       7,     8,    -1,    -1,   159,    12,    13,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,   174,
     175,   176,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    84,    85,    86,
      87,    -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,    -1,   111,   112,    -1,    -1,    -1,    -1,
     117,   118,   119,   120,    -1,    -1,    -1,   124,    -1,   126,
     127,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     3,    -1,     5,     6,     7,     8,
      -1,    -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   172,   173,   174,   175,   176,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    84,    85,    86,    87,    -1,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,    -1,   111,   112,    -1,    -1,    -1,    -1,   117,   118,
     119,   120,    -1,    -1,    -1,   124,    -1,   126,   127,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,     3,    -1,     5,     6,     7,     8,    -1,    -1,
     159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    -1,   172,   173,   174,   175,   176,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    84,    85,    86,    87,    -1,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,    -1,
     111,   112,    -1,    -1,    -1,    -1,   117,   118,   119,   120,
      -1,    -1,    -1,   124,    -1,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       3,    -1,     5,     6,     7,     8,    -1,    -1,   159,    12,
      13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,
      -1,   172,   173,   174,   175,   176,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    84,    85,    86,    87,    -1,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,    -1,   111,   112,
      -1,    -1,    -1,    -1,   117,   118,   119,   120,    -1,    -1,
      -1,   124,    -1,   126,   127,   128,   129,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,   159,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,   170,    -1,   172,
     173,    -1,   175,   176,   177,   178,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    70,    71,    -1,    -1,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
       3,    -1,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,
     178,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    68,    -1,    70,    71,    -1,
      -1,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,     3,    -1,     5,     6,     7,
       8,    -1,    -1,    -1,    12,    13,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,
     173,    -1,    -1,    31,   110,   178,    -1,    -1,    -1,   115,
      -1,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,   148,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    84,    85,    86,    87,
      -1,    89,    90,    91,    -1,    93,    -1,    95,   174,    97,
      -1,    -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,
     108,   109,    -1,   111,   112,    -1,    -1,    -1,    -1,   117,
     118,    -1,    -1,    -1,    -1,    -1,   124,    -1,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    -1,   172,   173,    -1,   175,   176,   177,
     178,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    84,    85,    86,    87,    -1,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,
     100,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
      -1,   111,   112,    -1,    -1,    -1,    -1,   117,   118,    -1,
      -1,    -1,    -1,    -1,   124,    -1,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,     3,    -1,     5,     6,     7,     8,    -1,    -1,   159,
      12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,   172,   173,    -1,   175,   176,   177,   178,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    84,    85,    86,    87,    -1,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,    -1,
      -1,    -1,   104,   105,   106,   107,   108,   109,    -1,   111,
     112,    -1,    -1,    -1,    -1,   117,   118,    -1,    -1,    -1,
      -1,    -1,   124,    -1,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   159,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
     172,   173,    -1,   175,   176,   177,   178,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      84,    85,    86,    87,    -1,    89,    90,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,
     104,   105,   106,   107,   108,   109,    -1,   111,   112,    -1,
      -1,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,
     124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,     3,    -1,     5,
       6,     7,     8,    -1,    -1,   159,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,
      -1,   175,   176,   177,   178,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    84,    85,
      86,    87,    -1,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,    -1,    -1,    -1,   104,   105,
     106,   107,   108,   109,    -1,   111,   112,    -1,    -1,    -1,
      -1,   117,   118,    -1,    -1,    -1,    -1,    -1,   124,    -1,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,     3,    -1,     5,     6,     7,
       8,    -1,    -1,   159,    12,    13,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,   172,   173,    -1,   175,
     176,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    84,    85,    86,    87,
      -1,    89,    90,    91,    -1,    93,    -1,    95,    -1,    97,
      -1,    -1,   100,    -1,    -1,    -1,   104,   105,   106,   107,
     108,   109,    -1,   111,   112,    -1,    -1,    -1,    -1,   117,
     118,    -1,    -1,    -1,    -1,    -1,   124,    -1,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   170,   102,   172,   173,    -1,   175,   176,   177,
     178,   110,    -1,    -1,    -1,    -1,   115,    37,    38,   118,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,   148,
      -1,    -1,    72,    73,    74,    75,    76,    77,    78,    -1,
     102,    81,    -1,    -1,    84,    85,    86,    -1,   110,    -1,
      -1,    -1,    -1,   115,    -1,   174,   118,   119,   120,   121,
     122,   123,   124,   125,   104,    -1,    -1,    -1,   108,   109,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,    -1,   124,    -1,   148,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,     3,   174,     5,     6,     7,     8,    -1,    -1,   159,
      12,    13,    -1,    15,    -1,   165,    -1,    -1,    -1,    -1,
     170,   102,    -1,    -1,    -1,   175,   176,   177,   178,   110,
      -1,    -1,    -1,    -1,   115,    37,    38,   118,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,   148,    -1,    -1,
      72,    73,    74,    75,    76,    77,    78,    -1,   102,    81,
      -1,    -1,    84,    85,    86,    -1,   110,    -1,    -1,    -1,
      -1,   115,    -1,   174,   118,   119,   120,   121,   122,   123,
     124,   125,   104,    -1,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,   124,    -1,   148,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     3,
     174,     5,     6,     7,     8,    -1,    -1,   159,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
      -1,    -1,    -1,   175,   176,   177,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    78,    -1,    -1,    81,    -1,    -1,
      84,    85,    86,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     104,    57,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,     3,    -1,     5,
       6,     7,     8,    -1,    -1,   159,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,
      -1,   175,   176,   177,   178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    78,    -1,    -1,    81,    82,    -1,    84,    85,
      86,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   104,    57,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,     3,    -1,     5,     6,     7,
       8,    -1,    -1,   159,    12,    13,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,    -1,   175,
     176,   177,   178,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      78,    -1,    -1,    81,    -1,    -1,    84,    85,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
     108,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,     3,    -1,     5,     6,     7,     8,    -1,
      -1,   159,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    -1,    -1,    -1,    -1,   175,   176,   177,
     178,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    78,    -1,
      -1,    81,    -1,    -1,    84,    85,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,   108,   109,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,     3,    -1,     5,     6,     7,     8,    -1,    -1,   159,
      12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,    -1,    -1,    -1,   175,   176,   177,   178,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    78,    -1,    -1,    81,
      -1,    -1,    84,    85,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,   108,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,     3,
      -1,     5,     6,     7,     8,    -1,    -1,   159,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
      -1,    -1,    -1,   175,   176,   177,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    78,    -1,    -1,    81,    -1,    -1,
      84,    85,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,     3,    -1,     5,
       6,     7,     8,    -1,    -1,   159,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,
      -1,   175,   176,   177,   178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    78,    -1,    -1,    81,    -1,    -1,    84,    85,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,   159,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,   170,    -1,    -1,    -1,    -1,   175,
     176,   177,   178,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    70,
      71,    -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,     3,    -1,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    68,    -1,    70,    71,    -1,    -1,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    30,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    68,    -1,    70,
      71,    -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,   170,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,   172,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,     9,    10,    11,
      74,    75,    76,    77,    78,    -1,    67,    81,    -1,    -1,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   118,    57,    -1,    -1,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,   138,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,    -1,    -1,    -1,     3,    -1,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,
      -1,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    70,    71,    -1,    -1,    74,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,   169,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,     3,    11,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    70,
      71,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,     3,    -1,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    70,    71,    -1,    -1,    74,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    70,
      71,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    -1,
      30,    81,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,   118,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,    -1,    -1,    -1,   175,    -1,   177,   178,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    14,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   180,   185,     0,     3,     5,     6,     7,     8,    12,
      13,    15,    51,    52,    56,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    68,    72,    73,    74,    75,    76,
      77,    78,    79,    81,    84,    85,    86,    87,    89,    90,
      91,    93,    95,    97,   100,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   115,   117,   118,   119,   120,   124,
     126,   127,   128,   129,   130,   131,   132,   133,   136,   137,
     138,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   159,   170,   172,   173,   175,   176,   177,   178,
     189,   192,   193,   194,   195,   211,   220,   223,   226,   229,
     230,   232,   234,   249,   255,   256,   257,   258,   311,   312,
     313,   314,   315,   323,   325,   330,   331,   332,   333,   335,
     336,   337,   338,   339,   340,   341,   342,   353,    74,   118,
     124,   193,   312,   315,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,    31,   170,   118,   124,   170,   189,
     193,   227,   228,   229,   309,   325,   326,   341,   343,   170,
     327,   170,   305,   306,   312,   211,   170,   170,   170,   170,
     170,   170,   312,   334,   334,    74,    74,   208,   304,   334,
     173,    74,    75,    77,   108,   110,   187,   188,   198,   200,
     204,   207,   279,   280,   341,    78,   281,   282,   313,   170,
     276,   170,   170,   170,   170,   225,   231,   233,   235,     3,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    57,
      68,    70,    71,    74,    77,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   181,   182,
     184,   186,   197,   170,   170,   190,   191,   325,   170,   175,
     330,   332,   333,   340,   340,    78,    80,   160,   161,   162,
     350,   351,   312,   209,    37,    38,   137,   165,   183,   312,
     346,   347,   348,   349,    80,   328,   350,    80,   350,   173,
     341,   276,   118,   192,   194,   313,   130,   229,    70,    71,
      70,    71,    88,     9,    10,    11,    30,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    67,   172,   183,   319,   319,   163,   163,   151,
     152,   173,   175,   324,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   149,   150,
     319,    14,   312,   312,   309,   229,   130,   163,   276,   329,
     151,   152,   163,   173,   175,   334,   312,   168,   172,    91,
     312,   307,   308,   312,   312,   208,   312,   312,   172,   172,
     172,    16,   168,   172,   172,   209,    99,   164,   188,   199,
     204,   172,   168,   172,   168,   172,    16,   168,   172,     3,
       5,     6,     7,     8,    12,    13,    68,    74,    84,    85,
      86,   104,   108,   109,   118,   124,   127,   128,   137,   138,
     140,   141,   142,   143,   144,   145,   146,   147,   165,   171,
     184,   277,   278,   312,   217,   218,   340,   312,   354,   355,
     312,   171,    74,    74,    74,    74,   172,   196,   173,   346,
     346,   168,   201,   276,   312,   346,   151,   152,   175,   160,
     351,    82,   312,   340,    80,   160,   351,   171,   129,   174,
     193,   194,   210,   211,   170,   312,   340,    14,   169,   168,
     176,   177,   312,   313,   224,   170,   211,   170,    31,   172,
     312,   312,   312,    31,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   326,
     312,   316,    74,   124,   219,   316,   173,   184,   341,   344,
     173,   184,   341,   344,    74,   173,   341,   345,   345,   312,
     334,   276,   183,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   171,   171,
     310,   341,   345,   345,   341,   312,   334,   171,   171,   306,
     170,   171,   172,   168,    99,   171,   171,   171,   312,   304,
     174,    74,   173,   164,   172,   172,   188,   207,   280,   312,
     282,   171,   312,    31,   168,   201,   168,   201,   168,   201,
     171,   172,   134,   239,   316,   134,   240,    31,   236,   173,
     185,   171,   171,   190,   169,   171,   169,    74,    74,    52,
      74,    78,    83,   352,   174,   175,   174,   174,   170,   194,
     346,   137,   183,   312,    16,   347,   174,    74,   312,   312,
     209,   312,    31,   170,   316,   170,   312,   276,   312,   276,
     312,   276,   276,   174,   169,   340,   329,   174,   169,    31,
     211,   312,    31,   211,   254,   307,   312,   137,   175,   183,
     242,   340,   212,    31,   173,   246,   173,   316,   213,   187,
     198,   202,   205,   206,   173,   312,   165,   278,   171,   218,
     171,   355,   171,   325,   135,   241,   173,   286,   325,   316,
      30,   118,   138,   139,   170,   189,   265,   266,   267,   268,
     269,   271,   241,   185,   174,    16,    83,   169,   312,   171,
     171,   170,   340,   312,   239,   171,   171,    88,   312,   118,
     119,   120,   121,   122,   123,   124,   193,   259,   260,   261,
     262,   263,   298,   299,   170,   259,   174,   174,   174,   239,
     209,   171,   209,   172,   170,   346,   340,    14,   171,    31,
     211,   245,   172,   247,   172,   247,   103,   250,   251,   252,
     253,   312,   113,   114,   216,    99,   206,   168,   201,   203,
     206,   172,   286,   316,   283,   168,   173,   266,   266,   269,
      37,    35,    35,    37,   316,   174,   312,   169,   172,   346,
     241,   211,    31,   172,   263,   171,   168,   201,    30,   170,
     264,   270,   271,   272,   273,   274,   299,   259,   171,   241,
     172,    92,   307,   346,   169,   242,    31,   211,   244,   209,
     247,   101,   102,   103,   247,   174,   168,   201,   174,   168,
     201,   168,   201,   170,   173,    74,   205,   174,   168,   201,
     173,   102,   115,   125,   174,   193,   237,   284,   285,   295,
     296,   297,   298,   325,   283,   171,   266,   266,   267,   267,
     266,   173,   174,   171,   316,   209,    31,   275,   261,   271,
     271,   274,    38,   221,    37,    35,    35,    37,   171,   115,
     320,   316,   172,   171,   171,   171,   209,    98,   101,   172,
     312,    31,   172,   248,   174,    14,   252,   312,    14,   214,
     325,   209,   206,   174,   283,   316,   286,   284,   264,   315,
     110,   174,   283,   173,   265,    14,   171,   165,   222,   271,
     271,   272,   272,   271,   275,   170,   275,   173,    31,   211,
     243,   244,    96,   172,   172,   248,   209,   312,   312,    35,
      78,   215,   174,   174,   184,   172,   173,   287,    78,   300,
     301,   319,    74,   118,   138,   139,   182,   265,   302,   303,
     174,   283,   317,    78,   317,    78,   183,   321,   322,   317,
     283,   209,   172,   209,   325,   171,    16,   238,   118,   174,
     184,   288,   289,   290,   291,   292,   293,   325,    16,   316,
     168,   172,   184,    16,    16,    74,   302,   168,   172,   174,
     318,   316,   173,    78,   168,   201,   173,   174,    94,   173,
     312,   172,   174,   289,   172,   172,    99,   116,   163,   312,
     301,   316,   312,   312,   172,   303,   312,    16,   209,   322,
     171,   209,   172,   209,    74,   181,   299,   286,   184,   316,
     170,   316,   316,   317,   312,   174,   174,   174,   184,   259,
     317,   317,   171,   275,   317,   172,   173,   294,   209,   317,
     174
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   179,   180,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   182,   182,   182,   182,   182,   182,   182,
     182,   183,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   188,   189,   189,   189,   189,   190,   190,   191,
     191,   192,   193,   193,   194,   194,   194,   194,   194,   195,
     195,   195,   195,   195,   196,   195,   197,   195,   195,   195,
     195,   195,   195,   198,   198,   199,   200,   201,   201,   202,
     202,   203,   203,   204,   204,   205,   205,   206,   206,   207,
     207,   208,   208,   209,   209,   210,   210,   210,   210,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   212,   211,   211,
     211,   211,   211,   213,   213,   214,   214,   215,   215,   216,
     216,   217,   217,   218,   219,   219,   220,   221,   221,   222,
     222,   224,   223,   225,   223,   226,   226,   227,   227,   228,
     228,   229,   229,   229,   231,   230,   233,   232,   235,   234,
     236,   236,   237,   238,   238,   239,   239,   240,   240,   241,
     241,   242,   242,   242,   242,   243,   243,   244,   244,   245,
     245,   246,   246,   246,   246,   247,   247,   247,   248,   248,
     249,   250,   250,   251,   251,   252,   252,   253,   253,   254,
     254,   255,   255,   256,   256,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   262,   262,   263,   263,   264,
     264,   265,   265,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   270,   270,   270,   270,   271,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   276,   276,
     276,   277,   277,   278,   278,   278,   279,   279,   280,   281,
     281,   282,   282,   283,   283,   284,   284,   284,   284,   284,
     285,   285,   285,   286,   286,   287,   287,   287,   288,   288,
     289,   289,   290,   291,   291,   291,   291,   292,   292,   293,
     294,   294,   295,   295,   296,   296,   297,   297,   298,   298,
     299,   299,   299,   299,   299,   299,   299,   300,   300,   301,
     301,   302,   302,   303,   303,   304,   305,   305,   306,   307,
     307,   308,   308,   310,   309,   311,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   313,   313,   314,   315,   316,   317,   318,   319,   319,
     320,   320,   321,   321,   322,   322,   323,   323,   323,   323,
     324,   323,   325,   325,   326,   326,   326,   327,   327,   328,
     328,   328,   329,   329,   330,   330,   330,   330,   331,   331,
     331,   331,   331,   331,   331,   331,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   333,   333,   333,   333,   334,
     334,   335,   336,   336,   336,   336,   337,   337,   338,   338,
     338,   339,   339,   339,   339,   339,   339,   340,   340,   340,
     340,   341,   341,   341,   342,   342,   343,   343,   343,   343,
     343,   343,   343,   344,   344,   344,   345,   345,   345,   346,
     347,   347,   348,   348,   349,   349,   349,   349,   349,   349,
     349,   350,   350,   350,   350,   351,   351,   351,   351,   351,
     351,   351,   351,   352,   352,   352,   352,   353,   353,   353,
     353,   353,   353,   353,   354,   354,   355
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
       1,     1,     1,     1,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       3,     4,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     2,     4,     3,     0,     6,     0,     5,     3,     4,
       3,     4,     3,     1,     1,     6,     6,     0,     1,     3,
       1,     3,     1,     3,     1,     1,     2,     1,     3,     1,
       3,     3,     1,     2,     0,     1,     1,     2,     4,     3,
       1,     1,     5,     7,     9,     5,     3,     3,     3,     3,
       3,     3,     1,     2,     6,     7,     9,     0,     6,     1,
       6,     3,     2,     0,     9,     1,     3,     0,     1,     0,
       4,     1,     3,     1,     1,     1,    13,     0,     1,     0,
       1,     0,    10,     0,     9,     1,     2,     1,     2,     0,
       1,     1,     1,     1,     0,     7,     0,     8,     0,     9,
       0,     2,     5,     0,     2,     0,     2,     0,     2,     0,
       2,     1,     2,     4,     3,     1,     4,     1,     4,     1,
       4,     3,     4,     4,     5,     0,     5,     4,     1,     1,
       7,     0,     2,     1,     3,     4,     4,     1,     3,     1,
       4,     5,     6,     1,     3,     6,     7,     3,     6,     2,
       0,     1,     3,     2,     1,     0,     1,     6,     8,     0,
       1,     1,     2,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     1,     2,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     0,     2,     2,     4,
       3,     1,     3,     1,     3,     2,     3,     1,     1,     3,
       1,     1,     3,     2,     0,     4,     4,     5,    12,     1,
       1,     2,     3,     1,     3,     1,     2,     3,     1,     2,
       2,     2,     3,     3,     3,     4,     3,     1,     1,     3,
       1,     3,     1,     1,     0,     1,     0,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     2,
       4,     3,     1,     4,     4,     4,     3,     1,     1,     0,
       1,     3,     1,     0,    10,     3,     2,     3,     1,     6,
       5,     3,     4,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     3,
       2,     1,     2,     4,     2,     2,     1,     2,     2,     3,
       1,    13,    12,     1,     1,     0,     0,     0,     0,     1,
       0,     5,     3,     1,     1,     2,     2,     2,     4,     4,
       0,     3,     1,     1,     1,     1,     3,     0,     3,     0,
       1,     1,     0,     1,     4,     3,     1,     3,     1,     1,
       3,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     5,     5,     0,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     1,     4,     4,     4,     4,     1,     1,     1,     3,
       3,     1,     4,     2,     3,     3,     1,     4,     4,     3,
       3,     3,     3,     1,     3,     1,     1,     3,     1,     1,
       0,     1,     3,     1,     3,     1,     4,     2,     2,     6,
       4,     2,     2,     1,     2,     1,     4,     3,     3,     3,
       3,     6,     3,     1,     1,     2,     1,     5,     4,     2,
       2,     4,     2,     2,     1,     3,     1
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
          case 72: /* "integer"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 73: /* "floating-point number"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 74: /* "identifier"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 75: /* "fully qualified name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 76: /* "namespace-relative name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 77: /* "namespaced name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 78: /* "variable"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 79: /* T_INLINE_HTML  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 80: /* "string content"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 81: /* "quoted string"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 82: /* "variable name"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 83: /* "number"  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 184: /* identifier  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 185: /* top_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 186: /* namespace_declaration_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 187: /* namespace_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 188: /* legacy_namespace_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 189: /* name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 190: /* attribute_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 191: /* attribute_group  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 192: /* attribute  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 193: /* attributes  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 194: /* attributed_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 195: /* top_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 199: /* group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 200: /* mixed_group_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 202: /* inline_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 203: /* unprefixed_use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 204: /* use_declarations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 205: /* inline_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 206: /* unprefixed_use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 207: /* use_declaration  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 208: /* const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 209: /* inner_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 210: /* inner_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 211: /* statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 213: /* catch_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 214: /* catch_name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 215: /* optional_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 216: /* finally_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 217: /* unset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 218: /* unset_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 219: /* function_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 220: /* function_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 223: /* class_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 230: /* trait_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 232: /* interface_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 234: /* enum_declaration_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 236: /* enum_backing_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 237: /* enum_case  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 238: /* enum_case_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 239: /* extends_from  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 240: /* interface_extends_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 241: /* implements_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 242: /* foreach_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 243: /* for_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 244: /* foreach_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 245: /* declare_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 246: /* switch_case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 247: /* case_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 249: /* match  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 250: /* match_arm_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 251: /* non_empty_match_arm_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 252: /* match_arm  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 253: /* match_arm_cond_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 254: /* while_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 255: /* if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 256: /* if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 257: /* alt_if_stmt_without_else  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 258: /* alt_if_stmt  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 259: /* parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 260: /* non_empty_parameter_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 261: /* attributed_parameter  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 263: /* parameter  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 264: /* optional_type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 265: /* type_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 266: /* type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 267: /* union_type_element  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 268: /* union_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 269: /* intersection_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 270: /* type_expr_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 271: /* type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 272: /* union_type_without_static_element  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 273: /* union_type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 274: /* intersection_type_without_static  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 275: /* return_type  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 276: /* argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 277: /* non_empty_argument_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 278: /* argument  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 279: /* global_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 280: /* global_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 281: /* static_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 282: /* static_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 283: /* class_statement_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 284: /* attributed_class_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 285: /* class_statement  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 286: /* class_name_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 287: /* trait_adaptations  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 288: /* trait_adaptation_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 289: /* trait_adaptation  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 290: /* trait_precedence  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 291: /* trait_alias  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 292: /* trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 293: /* absolute_trait_method_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 294: /* method_body  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 298: /* non_empty_member_modifiers  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 300: /* property_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 301: /* property  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 302: /* class_const_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 303: /* class_const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 304: /* const_decl  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 305: /* echo_expr_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 306: /* echo_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 307: /* for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 308: /* non_empty_for_exprs  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 309: /* anonymous_class  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 311: /* new_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 312: /* expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 313: /* inline_function  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 316: /* backup_doc_comment  */

      { if (((*yyvaluep).str)) zend_string_release_ex(((*yyvaluep).str), 0); }

        break;

    case 320: /* lexical_vars  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 321: /* lexical_var_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 322: /* lexical_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 323: /* function_call  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 325: /* class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 326: /* class_name_reference  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 327: /* exit_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 328: /* backticks_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 329: /* ctor_arguments  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 330: /* dereferenceable_scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 331: /* scalar  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 332: /* constant  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 333: /* class_constant  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 334: /* optional_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 335: /* variable_class_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 336: /* fully_dereferenceable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 337: /* array_object_dereferenceable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 338: /* callable_expr  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 339: /* callable_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 340: /* variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 341: /* simple_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 342: /* static_member  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 343: /* new_variable  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 344: /* member_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 345: /* property_name  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 346: /* array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 347: /* possible_array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 348: /* non_empty_array_pair_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 349: /* array_pair  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 350: /* encaps_list  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 351: /* encaps_var  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 352: /* encaps_var_offset  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 353: /* internal_functions_in_yacc  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 354: /* isset_variables  */

      { zend_ast_destroy(((*yyvaluep).ast)); }

        break;

    case 355: /* isset_variable  */

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

    { CG(ast) = (yyvsp[0].ast); (void) zendnerrs; }

    break;

  case 83:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 84:

    {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}

    break;

  case 85:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 86:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

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

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 93:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }

    break;

  case 94:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }

    break;

  case 95:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }

    break;

  case 96:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }

    break;

  case 97:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[0].ast), NULL); }

    break;

  case 98:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 99:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_GROUP, (yyvsp[0].ast)); }

    break;

  case 100:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 101:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 102:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_LIST, (yyvsp[0].ast)); }

    break;

  case 103:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 104:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 105:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 106:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 107:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 108:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 109:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 110:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 111:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 112:

    { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }

    break;

  case 113:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }

    break;

  case 114:

    { RESET_DOC_COMMENT(); }

    break;

  case 115:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 116:

    { RESET_DOC_COMMENT(); }

    break;

  case 117:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }

    break;

  case 118:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 119:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 120:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 121:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }

    break;

  case 122:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 123:

    { (yyval.num) = ZEND_SYMBOL_FUNCTION; }

    break;

  case 124:

    { (yyval.num) = ZEND_SYMBOL_CONST; }

    break;

  case 125:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 126:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}

    break;

  case 129:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 130:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 131:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 132:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 133:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 134:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }

    break;

  case 135:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }

    break;

  case 136:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }

    break;

  case 137:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 138:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 139:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 140:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 141:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 142:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 143:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 144:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 145:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 146:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 147:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 148:

    { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }

    break;

  case 149:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 150:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 151:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 152:

    { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 153:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }

    break;

  case 154:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 155:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 156:

    { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }

    break;

  case 157:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }

    break;

  case 158:

    { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }

    break;

  case 159:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 160:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 161:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 162:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 163:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 164:

    { (yyval.ast) = (yyvsp[-3].ast); }

    break;

  case 165:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 166:

    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }

    break;

  case 167:

    { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }

    break;

  case 168:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 169:

    { (yyval.ast) = NULL; }

    break;

  case 170:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 171:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }

    break;

  case 172:

    { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }

    break;

  case 173:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }

    break;

  case 174:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }

    break;

  case 175:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 176:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 177:

    { (yyval.ast) = NULL; }

    break;

  case 178:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 179:

    { (yyval.ast) = NULL; }

    break;

  case 180:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 181:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 182:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 183:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }

    break;

  case 184:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 185:

    {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}

    break;

  case 186:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 187:

    { (yyval.num) = 0; }

    break;

  case 188:

    { (yyval.num) = ZEND_PARAM_REF; }

    break;

  case 189:

    { (yyval.num) = 0; }

    break;

  case 190:

    { (yyval.num) = ZEND_PARAM_VARIADIC; }

    break;

  case 191:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 192:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 193:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 194:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 195:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 196:

    { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 197:

    { (yyval.num) = zend_add_anonymous_class_modifier(0, (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 198:

    { (yyval.num) = zend_add_anonymous_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }

    break;

  case 199:

    { (yyval.num) = 0; }

    break;

  case 200:

    { (yyval.num) = (yyvsp[0].num); }

    break;

  case 201:

    { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }

    break;

  case 202:

    { (yyval.num) = ZEND_ACC_FINAL; }

    break;

  case 203:

    { (yyval.num) = ZEND_ACC_READONLY_CLASS|ZEND_ACC_NO_DYNAMIC_PROPERTIES; }

    break;

  case 204:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 205:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 206:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 207:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }

    break;

  case 208:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 209:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }

    break;

  case 210:

    { (yyval.ast) = NULL; }

    break;

  case 211:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 212:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }

    break;

  case 213:

    { (yyval.ast) = NULL; }

    break;

  case 214:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 215:

    { (yyval.ast) = NULL; }

    break;

  case 216:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 217:

    { (yyval.ast) = NULL; }

    break;

  case 218:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 219:

    { (yyval.ast) = NULL; }

    break;

  case 220:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 221:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 222:

    { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }

    break;

  case 223:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }

    break;

  case 224:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 225:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 226:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 227:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 228:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 229:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 230:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 231:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 232:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 233:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 234:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 235:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }

    break;

  case 236:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 237:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }

    break;

  case 240:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 241:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }

    break;

  case 242:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 243:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }

    break;

  case 244:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 245:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 246:

    { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }

    break;

  case 247:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }

    break;

  case 248:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 249:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 250:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 251:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 252:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }

    break;

  case 253:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 254:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }

    break;

  case 255:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 256:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }

    break;

  case 257:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 258:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }

    break;

  case 259:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 260:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }

    break;

  case 261:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }

    break;

  case 262:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 263:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 264:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 265:

    { (yyval.num) = 0; }

    break;

  case 266:

    { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CPP, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }

    break;

  case 267:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-5].num) | (yyvsp[-3].num) | (yyvsp[-2].num), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL,
					NULL, (yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL); }

    break;

  case 268:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-7].num) | (yyvsp[-5].num) | (yyvsp[-4].num), (yyvsp[-6].ast), (yyvsp[-3].ast), (yyvsp[0].ast),
					NULL, (yyvsp[-2].str) ? zend_ast_create_zval_from_str((yyvsp[-2].str)) : NULL); }

    break;

  case 269:

    { (yyval.ast) = NULL; }

    break;

  case 270:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 271:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 272:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }

    break;

  case 273:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 274:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 275:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 276:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }

    break;

  case 277:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 278:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 279:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 280:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 281:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 282:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 283:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 284:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }

    break;

  case 285:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 286:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 287:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }

    break;

  case 288:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }

    break;

  case 289:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 290:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 291:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 292:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 293:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 294:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 295:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 296:

    { (yyval.ast) = NULL; }

    break;

  case 297:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 298:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 299:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 300:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALLABLE_CONVERT); }

    break;

  case 301:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }

    break;

  case 302:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 303:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 304:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 305:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 306:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 307:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 308:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }

    break;

  case 309:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 310:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 311:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }

    break;

  case 312:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 313:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 314:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }

    break;

  case 315:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 316:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }

    break;

  case 317:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, (yyvsp[-2].ast));
			  (yyval.ast)->attr = (yyvsp[-4].num); }

    break;

  case 318:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }

    break;

  case 319:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 320:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 321:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 322:

    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 323:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }

    break;

  case 324:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 325:

    { (yyval.ast) = NULL; }

    break;

  case 326:

    { (yyval.ast) = NULL; }

    break;

  case 327:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 328:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }

    break;

  case 329:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 330:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 331:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 332:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 333:

    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 334:

    { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }

    break;

  case 335:

    { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[-1].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-3].ast), (yyvsp[0].ast));
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }

    break;

  case 336:

    { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-2].ast), NULL);
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }

    break;

  case 337:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }

    break;

  case 338:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 339:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 340:

    { (yyval.ast) = NULL; }

    break;

  case 341:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 342:

    { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }

    break;

  case 343:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 344:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 345:

    { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }

    break;

  case 346:

    { (yyval.num) = ZEND_ACC_PUBLIC; }

    break;

  case 347:

    { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CONSTANT, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }

    break;

  case 348:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MODIFIER_LIST, zend_ast_create_zval_from_long((yyvsp[0].num))); }

    break;

  case 349:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), zend_ast_create_zval_from_long((yyvsp[0].num))); }

    break;

  case 350:

    { (yyval.num) = T_PUBLIC; }

    break;

  case 351:

    { (yyval.num) = T_PROTECTED; }

    break;

  case 352:

    { (yyval.num) = T_PRIVATE; }

    break;

  case 353:

    { (yyval.num) = T_STATIC; }

    break;

  case 354:

    { (yyval.num) = T_ABSTRACT; }

    break;

  case 355:

    { (yyval.num) = T_FINAL; }

    break;

  case 356:

    { (yyval.num) = T_READONLY; }

    break;

  case 357:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 358:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }

    break;

  case 359:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 360:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 361:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 362:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }

    break;

  case 363:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 364:

    {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-3].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, zend_ast_create_zval(&zv), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL));
		}

    break;

  case 365:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }

    break;

  case 366:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 367:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }

    break;

  case 368:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }

    break;

  case 369:

    { (yyval.ast) = NULL; }

    break;

  case 370:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 371:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 372:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }

    break;

  case 373:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 374:

    {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS | (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}

    break;

  case 375:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 376:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 377:

    { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 378:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 379:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 380:

    { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 381:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 382:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }

    break;

  case 383:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }

    break;

  case 384:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 385:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 386:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 387:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 388:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 389:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 390:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 391:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 392:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 393:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 394:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 395:

    { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 396:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 397:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }

    break;

  case 398:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }

    break;

  case 399:

    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }

    break;

  case 400:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }

    break;

  case 401:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 402:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 403:

    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 404:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 405:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 406:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 407:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 408:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 409:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 410:

    { (yyval.ast) = zend_ast_create_concat_op((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 411:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 412:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 413:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 414:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 415:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 416:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 417:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 418:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 419:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }

    break;

  case 420:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }

    break;

  case 421:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }

    break;

  case 422:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }

    break;

  case 423:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 424:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 425:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 426:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 427:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 428:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 429:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 430:

    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 431:

    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 432:

    { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 433:

    {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
		}

    break;

  case 434:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 435:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 436:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }

    break;

  case 437:

    { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 438:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 439:

    { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }

    break;

  case 440:

    { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }

    break;

  case 441:

    { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }

    break;

  case 442:

    { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }

    break;

  case 443:

    { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }

    break;

  case 444:

    { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }

    break;

  case 445:

    { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }

    break;

  case 446:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[0].ast)); }

    break;

  case 447:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }

    break;

  case 448:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 449:

    { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }

    break;

  case 450:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }

    break;

  case 451:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 452:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 453:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 454:

    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }

    break;

  case 455:

    { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }

    break;

  case 456:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 457:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }

    break;

  case 458:

    { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }

    break;

  case 459:

    { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }

    break;

  case 460:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 461:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  ZSTR_INIT_LITERAL("{closure}", 0),
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }

    break;

  case 462:

    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  ZSTR_INIT_LITERAL("{closure}", 0), (yyvsp[-7].ast), NULL, (yyvsp[-1].ast), (yyvsp[-5].ast), NULL);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }

    break;

  case 463:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 464:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 465:

    { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }

    break;

  case 466:

    { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }

    break;

  case 467:

    { (yyval.ptr) = LANG_SCNG(yy_text); }

    break;

  case 468:

    { (yyval.num) = 0; }

    break;

  case 469:

    { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }

    break;

  case 470:

    { (yyval.ast) = NULL; }

    break;

  case 471:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 472:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 473:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }

    break;

  case 474:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 475:

    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }

    break;

  case 476:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 477:

    {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-1].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, zend_ast_create_zval(&zv), (yyvsp[0].ast));
		}

    break;

  case 478:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 479:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 480:

    { (yyval.num) = CG(zend_lineno); }

    break;

  case 481:

    {
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-2].ast), (yyvsp[0].ast));
			(yyval.ast)->lineno = (yyvsp[-1].num);
		}

    break;

  case 482:

    { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }

    break;

  case 483:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 484:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 485:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 486:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 487:

    { (yyval.ast) = NULL; }

    break;

  case 488:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 489:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 490:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 491:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 492:

    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }

    break;

  case 493:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 494:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }

    break;

  case 495:

    { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }

    break;

  case 496:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 497:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 498:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 499:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 500:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 501:

    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }

    break;

  case 502:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 503:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 504:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 505:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 506:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }

    break;

  case 507:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }

    break;

  case 508:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }

    break;

  case 509:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }

    break;

  case 510:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }

    break;

  case 511:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }

    break;

  case 512:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }

    break;

  case 513:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }

    break;

  case 514:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }

    break;

  case 515:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 516:

    { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 517:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 518:

    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }

    break;

  case 519:

    { (yyval.ast) = NULL; }

    break;

  case 520:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 521:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 522:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 523:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 524:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 525:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 526:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 527:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 528:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 529:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 530:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 531:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 532:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 533:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 534:

    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 535:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 536:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 537:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 538:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 539:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 540:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 541:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 542:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 543:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 544:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 545:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 546:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 547:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 548:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }

    break;

  case 549:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 550:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 551:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 552:

    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 553:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 554:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 555:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 556:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 557:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 558:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 559:

    { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }

    break;

  case 560:

    { (yyval.ast) = NULL; }

    break;

  case 561:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 562:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 563:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }

    break;

  case 564:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }

    break;

  case 565:

    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }

    break;

  case 566:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }

    break;

  case 567:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }

    break;

  case 568:

    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }

    break;

  case 569:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }

    break;

  case 570:

    { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }

    break;

  case 571:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 572:

    { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 573:

    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }

    break;

  case 574:

    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }

    break;

  case 575:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 576:

    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }

    break;

  case 577:

    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }

    break;

  case 578:

    { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }

    break;

  case 579:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR, (yyvsp[-1].ast)); }

    break;

  case 580:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY, (yyvsp[-1].ast)); }

    break;

  case 581:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_ENCAPS_VAR_DOLLAR_CURLY,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }

    break;

  case 582:

    { (yyval.ast) = (yyvsp[-1].ast); }

    break;

  case 583:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 584:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 585:

    { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }

    break;

  case 586:

    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }

    break;

  case 587:

    { (yyval.ast) = (yyvsp[-2].ast); }

    break;

  case 588:

    { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }

    break;

  case 589:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }

    break;

  case 590:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }

    break;

  case 591:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }

    break;

  case 592:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }

    break;

  case 593:

    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }

    break;

  case 594:

    { (yyval.ast) = (yyvsp[0].ast); }

    break;

  case 595:

    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }

    break;

  case 596:

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

		/* We used "amp" as a dummy label to avoid a duplicate token literal warning. */
		if (strcmp(toktype, "\"amp\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"&\"");
			}
			return sizeof("token \"&\"")-1;
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
