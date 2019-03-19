/* A Bison parser, made by GNU Bison 3.0.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "libyang.h"
#include "common.h"
#include "context.h"
#include "resolve.h"
#include "parser_yang.h"
#include "parser_yang_lex.h"
#include "parser.h"

#define YANG_ADDELEM(current_ptr, size)                                                  \
    if ((size) == LY_ARRAY_MAX(size)) {                                                    \
         LOGERR(trg->ctx, LY_EINT, "Reached limit (%"PRIu64") for storing typedefs.", LY_ARRAY_MAX(trg->tpdf_size));\
         free(s);                                                                        \
         YYABORT;                                                                        \
    } else if (!((size) % LY_YANG_ARRAY_SIZE)) {                                           \
        void *tmp;                                                                       \
                                                                                         \
        tmp = realloc((current_ptr), (sizeof *(current_ptr)) * ((size) + LY_YANG_ARRAY_SIZE)); \
        if (!tmp) {                                                                      \
            LOGMEM(trg->ctx);                                                            \
            free(s);                                                                     \
            YYABORT;                                                                     \
        }                                                                                \
        memset(tmp + (sizeof *(current_ptr)) * (size), 0, (sizeof *(current_ptr)) * LY_YANG_ARRAY_SIZE); \
        (current_ptr) = tmp;                                                               \
    }                                                                                    \
    actual = &(current_ptr)[(size)++];                                                       \

void yyerror(YYLTYPE *yylloc, void *scanner, struct yang_parameter *param, ...);
/* pointer on the current parsed element 'actual' */



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
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser_yang_bis.h".  */
#ifndef YY_YY_PARSER_YANG_BIS_H_INCLUDED
# define YY_YY_PARSER_YANG_BIS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNION_KEYWORD = 258,
    ANYXML_KEYWORD = 259,
    WHITESPACE = 260,
    ERROR = 261,
    EOL = 262,
    STRING = 263,
    STRINGS = 264,
    IDENTIFIER = 265,
    IDENTIFIERPREFIX = 266,
    REVISION_DATE = 267,
    TAB = 268,
    DOUBLEDOT = 269,
    URI = 270,
    INTEGER = 271,
    NON_NEGATIVE_INTEGER = 272,
    ZERO = 273,
    DECIMAL = 274,
    ARGUMENT_KEYWORD = 275,
    AUGMENT_KEYWORD = 276,
    BASE_KEYWORD = 277,
    BELONGS_TO_KEYWORD = 278,
    BIT_KEYWORD = 279,
    CASE_KEYWORD = 280,
    CHOICE_KEYWORD = 281,
    CONFIG_KEYWORD = 282,
    CONTACT_KEYWORD = 283,
    CONTAINER_KEYWORD = 284,
    DEFAULT_KEYWORD = 285,
    DESCRIPTION_KEYWORD = 286,
    ENUM_KEYWORD = 287,
    ERROR_APP_TAG_KEYWORD = 288,
    ERROR_MESSAGE_KEYWORD = 289,
    EXTENSION_KEYWORD = 290,
    DEVIATION_KEYWORD = 291,
    DEVIATE_KEYWORD = 292,
    FEATURE_KEYWORD = 293,
    FRACTION_DIGITS_KEYWORD = 294,
    GROUPING_KEYWORD = 295,
    IDENTITY_KEYWORD = 296,
    IF_FEATURE_KEYWORD = 297,
    IMPORT_KEYWORD = 298,
    INCLUDE_KEYWORD = 299,
    INPUT_KEYWORD = 300,
    KEY_KEYWORD = 301,
    LEAF_KEYWORD = 302,
    LEAF_LIST_KEYWORD = 303,
    LENGTH_KEYWORD = 304,
    LIST_KEYWORD = 305,
    MANDATORY_KEYWORD = 306,
    MAX_ELEMENTS_KEYWORD = 307,
    MIN_ELEMENTS_KEYWORD = 308,
    MODULE_KEYWORD = 309,
    MUST_KEYWORD = 310,
    NAMESPACE_KEYWORD = 311,
    NOTIFICATION_KEYWORD = 312,
    ORDERED_BY_KEYWORD = 313,
    ORGANIZATION_KEYWORD = 314,
    OUTPUT_KEYWORD = 315,
    PATH_KEYWORD = 316,
    PATTERN_KEYWORD = 317,
    POSITION_KEYWORD = 318,
    PREFIX_KEYWORD = 319,
    PRESENCE_KEYWORD = 320,
    RANGE_KEYWORD = 321,
    REFERENCE_KEYWORD = 322,
    REFINE_KEYWORD = 323,
    REQUIRE_INSTANCE_KEYWORD = 324,
    REVISION_KEYWORD = 325,
    REVISION_DATE_KEYWORD = 326,
    RPC_KEYWORD = 327,
    STATUS_KEYWORD = 328,
    SUBMODULE_KEYWORD = 329,
    TYPE_KEYWORD = 330,
    TYPEDEF_KEYWORD = 331,
    UNIQUE_KEYWORD = 332,
    UNITS_KEYWORD = 333,
    USES_KEYWORD = 334,
    VALUE_KEYWORD = 335,
    WHEN_KEYWORD = 336,
    YANG_VERSION_KEYWORD = 337,
    YIN_ELEMENT_KEYWORD = 338,
    ADD_KEYWORD = 339,
    CURRENT_KEYWORD = 340,
    DELETE_KEYWORD = 341,
    DEPRECATED_KEYWORD = 342,
    FALSE_KEYWORD = 343,
    NOT_SUPPORTED_KEYWORD = 344,
    OBSOLETE_KEYWORD = 345,
    REPLACE_KEYWORD = 346,
    SYSTEM_KEYWORD = 347,
    TRUE_KEYWORD = 348,
    UNBOUNDED_KEYWORD = 349,
    USER_KEYWORD = 350,
    ACTION_KEYWORD = 351,
    MODIFIER_KEYWORD = 352,
    ANYDATA_KEYWORD = 353,
    NODE = 354,
    NODE_PRINT = 355,
    EXTENSION_INSTANCE = 356,
    SUBMODULE_EXT_KEYWORD = 357
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


  int32_t i;
  uint32_t uint;
  char *str;
  char **p_str;
  void *v;
  char ch;
  struct yang_type *type;
  struct lys_deviation *dev;
  struct lys_deviate *deviate;
  union {
    uint32_t index;
    struct lys_node_container *container;
    struct lys_node_anydata *anydata;
    struct type_node node;
    struct lys_node_case *cs;
    struct lys_node_grp *grouping;
    struct lys_refine *refine;
    struct lys_node_notif *notif;
    struct lys_node_uses *uses;
    struct lys_node_inout *inout;
    struct lys_node_augment *augment;
  } nodes;
  enum yytokentype token;
  struct {
    void *actual;
    enum yytokentype token;
  } backup_token;
  struct {
    struct lys_revision **revision;
    int index;
  } revisions;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void *scanner, struct yang_parameter *param);

#endif /* !YY_YY_PARSER_YANG_BIS_H_INCLUDED  */

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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3466

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  113
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  329
/* YYNRULES -- Number of rules.  */
#define YYNRULES  827
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1318

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   357

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     111,   112,     2,   103,     2,     2,     2,   107,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   106,
       2,   110,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   108,     2,   109,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   104,     2,   105,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   338,   338,   339,   340,   342,   365,   368,   370,   369,
     393,   404,   414,   424,   425,   431,   436,   442,   453,   463,
     476,   477,   483,   485,   489,   491,   495,   497,   498,   499,
     501,   509,   517,   518,   523,   534,   545,   556,   564,   569,
     570,   574,   575,   586,   597,   608,   612,   614,   637,   654,
     658,   660,   661,   666,   671,   676,   682,   686,   688,   692,
     694,   698,   700,   704,   706,   719,   730,   731,   743,   747,
     748,   752,   753,   758,   765,   765,   776,   782,   830,   849,
     852,   853,   854,   855,   856,   857,   858,   859,   860,   861,
     864,   879,   886,   887,   891,   892,   893,   899,   904,   910,
     928,   930,   931,   935,   940,   941,   963,   964,   965,   978,
     983,   985,   986,   987,   988,  1003,  1017,  1022,  1023,  1038,
    1039,  1040,  1046,  1051,  1057,  1114,  1119,  1120,  1122,  1138,
    1143,  1144,  1169,  1170,  1184,  1185,  1191,  1196,  1202,  1206,
    1208,  1261,  1272,  1275,  1278,  1283,  1288,  1294,  1299,  1305,
    1310,  1319,  1320,  1324,  1371,  1372,  1374,  1375,  1379,  1385,
    1398,  1399,  1400,  1404,  1405,  1407,  1411,  1429,  1434,  1436,
    1437,  1453,  1458,  1467,  1468,  1472,  1488,  1493,  1498,  1503,
    1509,  1513,  1529,  1544,  1545,  1549,  1550,  1560,  1565,  1570,
    1575,  1581,  1585,  1596,  1608,  1609,  1612,  1620,  1631,  1632,
    1647,  1648,  1649,  1661,  1667,  1672,  1678,  1683,  1685,  1686,
    1701,  1706,  1707,  1712,  1716,  1718,  1723,  1725,  1726,  1727,
    1740,  1752,  1753,  1755,  1763,  1775,  1776,  1791,  1792,  1793,
    1805,  1811,  1816,  1822,  1827,  1829,  1830,  1846,  1850,  1852,
    1856,  1858,  1862,  1864,  1868,  1870,  1880,  1887,  1888,  1892,
    1893,  1899,  1904,  1909,  1910,  1911,  1912,  1913,  1919,  1920,
    1921,  1922,  1923,  1924,  1925,  1926,  1929,  1939,  1946,  1947,
    1970,  1971,  1972,  1973,  1974,  1979,  1985,  1991,  1996,  2001,
    2002,  2003,  2008,  2009,  2011,  2051,  2061,  2064,  2065,  2066,
    2069,  2074,  2075,  2080,  2086,  2092,  2098,  2103,  2109,  2119,
    2174,  2177,  2178,  2179,  2182,  2193,  2198,  2199,  2205,  2218,
    2231,  2241,  2247,  2252,  2258,  2268,  2315,  2318,  2319,  2320,
    2321,  2330,  2336,  2342,  2355,  2368,  2378,  2384,  2389,  2394,
    2395,  2396,  2397,  2402,  2404,  2414,  2421,  2422,  2442,  2445,
    2446,  2447,  2457,  2464,  2471,  2478,  2484,  2490,  2492,  2493,
    2495,  2496,  2497,  2498,  2499,  2500,  2501,  2507,  2517,  2524,
    2525,  2539,  2540,  2541,  2542,  2548,  2553,  2558,  2561,  2571,
    2578,  2588,  2595,  2596,  2619,  2622,  2623,  2624,  2625,  2632,
    2639,  2646,  2651,  2657,  2667,  2674,  2675,  2707,  2708,  2709,
    2710,  2716,  2721,  2726,  2727,  2729,  2730,  2732,  2745,  2750,
    2751,  2783,  2786,  2800,  2816,  2838,  2889,  2908,  2927,  2948,
    2969,  2974,  2980,  2981,  2984,  2999,  3008,  3009,  3011,  3022,
    3031,  3032,  3033,  3034,  3040,  3045,  3050,  3051,  3052,  3057,
    3059,  3074,  3081,  3091,  3098,  3099,  3123,  3126,  3127,  3133,
    3138,  3143,  3144,  3145,  3152,  3160,  3175,  3205,  3206,  3207,
    3208,  3209,  3211,  3226,  3256,  3265,  3272,  3273,  3305,  3306,
    3307,  3308,  3314,  3319,  3324,  3325,  3326,  3328,  3340,  3360,
    3361,  3367,  3373,  3375,  3376,  3378,  3379,  3382,  3390,  3395,
    3396,  3398,  3399,  3400,  3402,  3410,  3415,  3416,  3448,  3449,
    3455,  3456,  3462,  3468,  3475,  3482,  3490,  3499,  3507,  3512,
    3513,  3545,  3546,  3552,  3553,  3559,  3566,  3574,  3579,  3580,
    3594,  3595,  3596,  3602,  3608,  3615,  3622,  3630,  3639,  3648,
    3653,  3654,  3658,  3659,  3664,  3670,  3675,  3677,  3678,  3679,
    3692,  3697,  3699,  3700,  3701,  3714,  3718,  3720,  3725,  3727,
    3728,  3748,  3753,  3755,  3756,  3757,  3777,  3782,  3784,  3785,
    3786,  3798,  3867,  3872,  3873,  3877,  3881,  3883,  3884,  3886,
    3890,  3892,  3892,  3899,  3902,  3911,  3930,  3932,  3933,  3936,
    3936,  3953,  3953,  3960,  3960,  3967,  3970,  3972,  3974,  3975,
    3977,  3979,  3981,  3982,  3984,  3986,  3987,  3989,  3990,  3992,
    3994,  3997,  4000,  4002,  4003,  4005,  4006,  4008,  4010,  4021,
    4022,  4025,  4026,  4038,  4039,  4041,  4042,  4044,  4045,  4051,
    4052,  4055,  4056,  4057,  4081,  4082,  4085,  4091,  4095,  4100,
    4101,  4102,  4105,  4110,  4120,  4122,  4123,  4125,  4126,  4128,
    4129,  4130,  4132,  4133,  4135,  4136,  4138,  4139,  4143,  4144,
    4171,  4209,  4210,  4212,  4214,  4216,  4217,  4219,  4220,  4222,
    4223,  4226,  4227,  4230,  4232,  4233,  4236,  4236,  4243,  4245,
    4246,  4247,  4248,  4249,  4250,  4251,  4253,  4254,  4255,  4257,
    4258,  4259,  4260,  4261,  4262,  4263,  4264,  4265,  4266,  4269,
    4270,  4271,  4272,  4273,  4274,  4275,  4276,  4277,  4278,  4279,
    4280,  4281,  4282,  4283,  4284,  4285,  4286,  4287,  4288,  4289,
    4290,  4291,  4292,  4293,  4294,  4295,  4296,  4297,  4298,  4299,
    4300,  4301,  4302,  4303,  4304,  4305,  4306,  4307,  4308,  4309,
    4310,  4311,  4312,  4313,  4314,  4315,  4316,  4317,  4318,  4319,
    4320,  4321,  4322,  4323,  4324,  4325,  4326,  4327,  4328,  4329,
    4330,  4331,  4332,  4333,  4334,  4335,  4336,  4337,  4338,  4340,
    4347,  4354,  4374,  4392,  4408,  4435,  4442,  4460,  4500,  4502,
    4503,  4504,  4505,  4506,  4507,  4508,  4509,  4510,  4511,  4512,
    4513,  4514,  4516,  4517,  4518,  4519,  4520,  4521,  4522,  4523,
    4524,  4525,  4526,  4527,  4528,  4529,  4531,  4532,  4533,  4534,
    4536,  4544,  4545,  4550,  4555,  4560,  4565,  4570,  4575,  4580,
    4585,  4590,  4595,  4600,  4605,  4610,  4615,  4620,  4634,  4654,
    4659,  4664,  4669,  4682,  4687,  4691,  4701,  4716,  4731,  4746,
    4761,  4781,  4796,  4797,  4803,  4810,  4825,  4828
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNION_KEYWORD", "ANYXML_KEYWORD",
  "WHITESPACE", "ERROR", "EOL", "STRING", "STRINGS", "IDENTIFIER",
  "IDENTIFIERPREFIX", "REVISION_DATE", "TAB", "DOUBLEDOT", "URI",
  "INTEGER", "NON_NEGATIVE_INTEGER", "ZERO", "DECIMAL", "ARGUMENT_KEYWORD",
  "AUGMENT_KEYWORD", "BASE_KEYWORD", "BELONGS_TO_KEYWORD", "BIT_KEYWORD",
  "CASE_KEYWORD", "CHOICE_KEYWORD", "CONFIG_KEYWORD", "CONTACT_KEYWORD",
  "CONTAINER_KEYWORD", "DEFAULT_KEYWORD", "DESCRIPTION_KEYWORD",
  "ENUM_KEYWORD", "ERROR_APP_TAG_KEYWORD", "ERROR_MESSAGE_KEYWORD",
  "EXTENSION_KEYWORD", "DEVIATION_KEYWORD", "DEVIATE_KEYWORD",
  "FEATURE_KEYWORD", "FRACTION_DIGITS_KEYWORD", "GROUPING_KEYWORD",
  "IDENTITY_KEYWORD", "IF_FEATURE_KEYWORD", "IMPORT_KEYWORD",
  "INCLUDE_KEYWORD", "INPUT_KEYWORD", "KEY_KEYWORD", "LEAF_KEYWORD",
  "LEAF_LIST_KEYWORD", "LENGTH_KEYWORD", "LIST_KEYWORD",
  "MANDATORY_KEYWORD", "MAX_ELEMENTS_KEYWORD", "MIN_ELEMENTS_KEYWORD",
  "MODULE_KEYWORD", "MUST_KEYWORD", "NAMESPACE_KEYWORD",
  "NOTIFICATION_KEYWORD", "ORDERED_BY_KEYWORD", "ORGANIZATION_KEYWORD",
  "OUTPUT_KEYWORD", "PATH_KEYWORD", "PATTERN_KEYWORD", "POSITION_KEYWORD",
  "PREFIX_KEYWORD", "PRESENCE_KEYWORD", "RANGE_KEYWORD",
  "REFERENCE_KEYWORD", "REFINE_KEYWORD", "REQUIRE_INSTANCE_KEYWORD",
  "REVISION_KEYWORD", "REVISION_DATE_KEYWORD", "RPC_KEYWORD",
  "STATUS_KEYWORD", "SUBMODULE_KEYWORD", "TYPE_KEYWORD", "TYPEDEF_KEYWORD",
  "UNIQUE_KEYWORD", "UNITS_KEYWORD", "USES_KEYWORD", "VALUE_KEYWORD",
  "WHEN_KEYWORD", "YANG_VERSION_KEYWORD", "YIN_ELEMENT_KEYWORD",
  "ADD_KEYWORD", "CURRENT_KEYWORD", "DELETE_KEYWORD", "DEPRECATED_KEYWORD",
  "FALSE_KEYWORD", "NOT_SUPPORTED_KEYWORD", "OBSOLETE_KEYWORD",
  "REPLACE_KEYWORD", "SYSTEM_KEYWORD", "TRUE_KEYWORD", "UNBOUNDED_KEYWORD",
  "USER_KEYWORD", "ACTION_KEYWORD", "MODIFIER_KEYWORD", "ANYDATA_KEYWORD",
  "NODE", "NODE_PRINT", "EXTENSION_INSTANCE", "SUBMODULE_EXT_KEYWORD",
  "'+'", "'{'", "'}'", "';'", "'/'", "'['", "']'", "'='", "'('", "')'",
  "$accept", "start", "tmp_string", "string_1", "string_2", "$@1",
  "module_arg_str", "module_stmt", "module_header_stmts",
  "module_header_stmt", "submodule_arg_str", "submodule_stmt",
  "submodule_header_stmts", "submodule_header_stmt", "yang_version_arg",
  "yang_version_stmt", "namespace_arg_str", "namespace_stmt",
  "linkage_stmts", "import_stmt", "import_arg_str", "import_opt_stmt",
  "include_arg_str", "include_stmt", "include_end", "include_opt_stmt",
  "revision_date_arg", "revision_date_stmt", "belongs_to_arg_str",
  "belongs_to_stmt", "prefix_arg", "prefix_stmt", "meta_stmts",
  "organization_arg", "organization_stmt", "contact_arg", "contact_stmt",
  "description_arg", "description_stmt", "reference_arg", "reference_stmt",
  "revision_stmts", "revision_arg_stmt", "revision_stmts_opt",
  "revision_stmt", "revision_end", "revision_opt_stmt", "date_arg_str",
  "$@2", "body_stmts_end", "body_stmts", "body_stmt", "extension_arg_str",
  "extension_stmt", "extension_end", "extension_opt_stmt", "argument_str",
  "argument_stmt", "argument_end", "yin_element_arg", "yin_element_stmt",
  "yin_element_arg_str", "status_arg", "status_stmt", "status_arg_str",
  "feature_arg_str", "feature_stmt", "feature_end", "feature_opt_stmt",
  "if_feature_arg", "if_feature_stmt", "if_feature_end",
  "identity_arg_str", "identity_stmt", "identity_end", "identity_opt_stmt",
  "base_arg", "base_stmt", "typedef_arg_str", "typedef_stmt",
  "type_opt_stmt", "type_stmt", "type_arg_str", "type_end",
  "type_body_stmts", "some_restrictions", "union_stmt", "union_spec",
  "fraction_digits_arg", "fraction_digits_stmt", "fraction_digits_arg_str",
  "length_stmt", "length_arg_str", "length_end", "message_opt_stmt",
  "pattern_sep", "pattern_stmt", "pattern_arg_str", "pattern_end",
  "pattern_opt_stmt", "modifier_arg", "modifier_stmt",
  "enum_specification", "enum_stmts", "enum_stmt", "enum_arg_str",
  "enum_end", "enum_opt_stmt", "value_arg", "value_stmt",
  "integer_value_arg_str", "range_stmt", "range_end", "path_arg",
  "path_stmt", "require_instance_arg", "require_instance_stmt",
  "require_instance_arg_str", "bits_specification", "bit_stmts",
  "bit_stmt", "bit_arg_str", "bit_end", "bit_opt_stmt",
  "position_value_arg", "position_stmt", "position_value_arg_str",
  "error_message_arg", "error_message_stmt", "error_app_tag_arg",
  "error_app_tag_stmt", "units_arg", "units_stmt", "default_arg",
  "default_stmt", "grouping_arg_str", "grouping_stmt", "grouping_end",
  "grouping_opt_stmt", "data_def_stmt", "container_arg_str",
  "container_stmt", "container_end", "container_opt_stmt", "leaf_stmt",
  "leaf_arg_str", "leaf_opt_stmt", "leaf_list_arg_str", "leaf_list_stmt",
  "leaf_list_opt_stmt", "list_arg_str", "list_stmt", "list_opt_stmt",
  "choice_arg_str", "choice_stmt", "choice_end", "choice_opt_stmt",
  "short_case_case_stmt", "short_case_stmt", "case_arg_str", "case_stmt",
  "case_end", "case_opt_stmt", "anyxml_arg_str", "anyxml_stmt",
  "anydata_arg_str", "anydata_stmt", "anyxml_end", "anyxml_opt_stmt",
  "uses_arg_str", "uses_stmt", "uses_end", "uses_opt_stmt",
  "refine_args_str", "refine_arg_str", "refine_stmt", "refine_end",
  "refine_body_opt_stmts", "uses_augment_arg_str", "uses_augment_arg",
  "uses_augment_stmt", "augment_arg_str", "augment_arg", "augment_stmt",
  "augment_opt_stmt", "action_arg_str", "action_stmt", "rpc_arg_str",
  "rpc_stmt", "rpc_end", "rpc_opt_stmt", "input_arg", "input_stmt",
  "input_output_opt_stmt", "output_arg", "output_stmt",
  "notification_arg_str", "notification_stmt", "notification_end",
  "notification_opt_stmt", "deviation_arg", "deviation_stmt",
  "deviation_opt_stmt", "deviation_arg_str", "deviate_body_stmt",
  "deviate_not_supported", "deviate_not_supported_stmt",
  "deviate_not_supported_end", "deviate_stmts", "deviate_add",
  "deviate_add_stmt", "deviate_add_end", "deviate_add_opt_stmt",
  "deviate_delete", "deviate_delete_stmt", "deviate_delete_end",
  "deviate_delete_opt_stmt", "deviate_replace", "deviate_replace_stmt",
  "deviate_replace_end", "deviate_replace_opt_stmt", "when_arg_str",
  "when_stmt", "when_end", "when_opt_stmt", "config_arg", "config_stmt",
  "config_arg_str", "mandatory_arg", "mandatory_stmt", "mandatory_arg_str",
  "presence_arg", "presence_stmt", "min_value_arg", "min_elements_stmt",
  "min_value_arg_str", "max_value_arg", "max_elements_stmt",
  "max_value_arg_str", "ordered_by_arg", "ordered_by_stmt",
  "ordered_by_arg_str", "must_agr_str", "must_stmt", "must_end",
  "unique_arg", "unique_stmt", "unique_arg_str", "key_arg", "key_stmt",
  "key_arg_str", "$@3", "range_arg_str", "absolute_schema_nodeid",
  "absolute_schema_nodeids", "absolute_schema_nodeid_opt",
  "descendant_schema_nodeid", "$@4", "path_arg_str", "$@5", "$@6",
  "absolute_path", "absolute_paths", "absolute_path_opt", "relative_path",
  "relative_path_part1", "relative_path_part1_opt", "descendant_path",
  "descendant_path_opt", "path_predicate", "path_equality_expr",
  "path_key_expr", "rel_path_keyexpr", "rel_path_keyexpr_part1",
  "rel_path_keyexpr_part1_opt", "rel_path_keyexpr_part2",
  "current_function_invocation", "positive_integer_value",
  "non_negative_integer_value", "integer_value", "integer_value_convert",
  "prefix_arg_str", "identifier_arg_str", "node_identifier",
  "identifier_ref_arg_str", "stmtend", "semicolom", "curly_bracket_close",
  "curly_bracket_open", "stmtsep", "unknown_statement", "string_opt",
  "string_opt_part1", "string_opt_part2", "unknown_string",
  "unknown_string_part1", "unknown_string_part2", "unknown_statement_end",
  "unknown_statement2_opt", "unknown_statement2", "unknown_statement2_end",
  "unknown_statement2_yang_stmt", "unknown_statement2_module_stmt",
  "unknown_statement3_opt", "unknown_statement3_opt_end", "sep_stmt",
  "optsep", "sep", "whitespace_opt", "string", "$@7", "strings",
  "identifier", "identifier1", "yang_stmt", "identifiers",
  "identifiers_ref", "type_ext_alloc", "typedef_ext_alloc",
  "iffeature_ext_alloc", "restriction_ext_alloc", "when_ext_alloc",
  "revision_ext_alloc", "datadef_ext_check", "not_supported_ext_check",
  "not_supported_ext", "datadef_ext_stmt", "restriction_ext_stmt",
  "ext_substatements", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,    43,   123,   125,    59,    47,    91,    93,
      61,    40,    41
};
# endif

#define YYPACT_NINF -1012

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1012)))

#define YYTABLE_NINF -757

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     440,   100, -1012, -1012,   566,  1894, -1012, -1012, -1012,   266,
     266, -1012,   266, -1012,   266,   266, -1012,   266,   266,   266,
     266, -1012,   266,   266, -1012, -1012, -1012, -1012,   266, -1012,
   -1012, -1012,   266,   266,   266,   266,   266,   266,   266,   266,
     266,   266,   266,   266,   266,   266,   266,   266,   266,   266,
   -1012, -1012,   266, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,    -8,    31,    79,   868,    70,   125,   481,
     266, -1012, -1012,  3273,  3273,  3273,  2893,  3273,    71,  2703,
    2703,  2703,  2703,  2703,    98,  2988,    94,    52,   287,  2703,
      77,  2703,   104,   287,  3273,  2703,  2703,   182,    58,   246,
    2988,  2703,   321,  2703,   279,   279,   266, -1012,   266, -1012,
     266, -1012,   266,   266,   266,   266, -1012, -1012, -1012, -1012,
   -1012,   266, -1012,   266, -1012,   266,   266,   266,   266,   266,
   -1012,   266,   266,   266,   266, -1012,   266,   266,   266, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
      29, -1012,   134, -1012, -1012, -1012,   -22,  2798,   266, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   151, -1012, -1012, -1012, -1012, -1012,   156,
   -1012,    67, -1012, -1012, -1012,   224, -1012, -1012, -1012,   161,
   -1012, -1012, -1012, -1012,   224, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   224, -1012, -1012, -1012,   224, -1012,   224,
   -1012,   224, -1012,   224, -1012, -1012, -1012,   224, -1012, -1012,
   -1012, -1012,   224, -1012, -1012, -1012, -1012, -1012, -1012,   224,
   -1012, -1012, -1012,   224, -1012, -1012, -1012, -1012,   224, -1012,
   -1012, -1012,   224, -1012, -1012, -1012, -1012,   224, -1012,   224,
   -1012, -1012,   224, -1012,   262,   202, -1012,   224, -1012, -1012,
   -1012,   224, -1012, -1012,   224, -1012,   224, -1012, -1012, -1012,
   -1012,   224, -1012, -1012, -1012,   224, -1012, -1012, -1012, -1012,
   -1012,   224, -1012, -1012,   224, -1012, -1012, -1012,   224, -1012,
   -1012, -1012, -1012, -1012,   224, -1012, -1012, -1012,   224, -1012,
   -1012, -1012, -1012,  2893,   279,  3273,   279,  2703,   279,  2703,
    2703,  2703, -1012,  2703,   279,  2703,   279,    58,   279,  3273,
    3273,  3273,  3273,  3273,   266,  3273,  3273,  3273,  3273,   266,
    2893,  3273,  3273, -1012, -1012,   279, -1012, -1012, -1012, -1012,
   -1012, -1012,   266, -1012,   266, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,   266,   266, -1012,   266,   266, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   266, -1012, -1012,
     266,   266, -1012,   266, -1012,   266, -1012,   266, -1012,   266,
     266, -1012, -1012, -1012,  3368, -1012, -1012,   291, -1012, -1012,
   -1012,   266, -1012,   266, -1012, -1012,   266,   266, -1012, -1012,
   -1012,   266,   266,   266, -1012, -1012,   266, -1012, -1012, -1012,
     266, -1012,   228,  2703,   266,   274, -1012,   189, -1012,   288,
   -1012,   298, -1012,   303, -1012,   370, -1012,   380, -1012,   389,
   -1012,   393, -1012,   407, -1012,   411, -1012,   419, -1012,   426,
   -1012,   463, -1012,   238, -1012,   314, -1012,   317, -1012,   505,
   -1012,   506, -1012,   521, -1012,   407, -1012,   279,   279,   266,
     266,   266,   266,   326,   279,   279,   109,   279,   112,   608,
     266,   266, -1012,   262, -1012,  3083,   266,   332, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,  1964,  1994,  2191,   347,
   -1012, -1012,    19, -1012,    54,   266,   355, -1012, -1012,   368,
     373, -1012, -1012, -1012,    48,  3368, -1012,   266,   831,   279,
     186,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   266, -1012,   127,   515,   266, -1012, -1012,
   -1012,   515, -1012, -1012,   188, -1012,   279, -1012,   473, -1012,
     306, -1012,  2552,   266,   266,   397,  1000, -1012, -1012, -1012,
   -1012,   783, -1012,   230,   503,   404,   887,   359,   438,   929,
    1958,  1645,   817,   947,   344,  2074,  1547,  1768,   235,   852,
     279,   279,   279,   279,   266,   -22,   280, -1012,   266,   266,
   -1012, -1012,   375,  2703,   375,   279, -1012, -1012, -1012,   224,
   -1012, -1012,  3368, -1012, -1012, -1012, -1012, -1012,   266,   266,
   -1012,  3273,  2703, -1012, -1012, -1012,    -8, -1012, -1012, -1012,
   -1012, -1012, -1012,   279,   474, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012,   266,   266, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,  3273,  3273,   279,   279, -1012, -1012,
   -1012, -1012, -1012,   125,   224, -1012, -1012,   266,   266, -1012,
     399,   473,   266,   528,   528,   545, -1012,   567, -1012,   279,
   -1012,   279,   279,   279,   480, -1012,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
    2988,  2988,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   266,   266,   423, -1012,   570, -1012,   428,  2046, -1012,
   -1012,   434, -1012,   465, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   479, -1012, -1012,
   -1012,   571, -1012, -1012, -1012, -1012, -1012, -1012,   266,   266,
     266,   266,   266,   266, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   279, -1012,   473,   266,   279,
     279,   279,   279, -1012,   266, -1012, -1012, -1012,   266,   279,
     279,   266,    51,  3273,    51,  3273,  3273,  3273,   279,   266,
     459,  2367,   385,   509,   279,   279,   341,   365, -1012, -1012,
     483, -1012, -1012,   574, -1012, -1012,   486, -1012, -1012,   591,
   -1012,   592, -1012,   521, -1012,   473, -1012,   473, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012,   885,  1126, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012,   332,   266, -1012, -1012, -1012, -1012,   266,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,   467,   492,   279,
     279, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012,   279,   279,   279,   279,   279,   473,   473,   279,
     279,   279,   279,   279,   279,   279,   279,  1460,   205,   524,
     216,   201,   493,   579, -1012, -1012, -1012, -1012, -1012, -1012,
     266, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012,   473, -1012, -1012,   279,
     293,   279,   279,   497,  3178, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,   473,
   -1012, -1012,   279,    73,   120,   133,   163, -1012,    50, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012,   510,   222,   279,   279,   279,   473, -1012,   824,   346,
     985,  3368, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   279,   279,   279
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     790,     0,     2,     3,     0,   757,     1,   649,   650,     0,
       0,   652,     0,   763,     0,     0,   761,     0,     0,     0,
       0,   762,     0,     0,   766,   764,   765,   767,     0,   768,
     769,   770,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     759,   760,     0,   771,   802,   806,   619,   792,   803,   797,
     793,   794,   619,   809,   796,   815,   814,   819,   804,   813,
     818,   799,   800,   795,   798,   810,   811,   805,   816,   817,
     812,   820,   801,     0,     0,     0,     0,     0,     0,     0,
     627,   758,   651,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   571,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   791,   822,     0,   619,     0,   619,
       0,   619,     0,     0,     0,     0,   789,   787,   788,   786,
     619,     0,   619,     0,   619,     0,     0,     0,     0,     0,
     651,     0,     0,     0,     0,   651,     0,     0,     0,   778,
     777,   780,   781,   782,   776,   775,   774,   773,   785,   772,
       0,   779,     0,   784,   783,   619,     0,   629,   653,   679,
       5,   669,   680,   681,   682,   683,   684,   685,   686,   687,
     688,   689,   690,   691,   692,   693,   694,   695,   696,   697,
     698,   699,   700,   701,   702,   703,   704,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   666,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   723,   724,   725,   726,
     727,   728,   729,   730,   731,   732,   733,   734,   735,   736,
     737,   738,   739,   740,   741,   742,   743,   670,   744,   671,
     672,   673,   674,   745,   675,   676,   677,   678,   746,   747,
     748,   651,   608,     0,    10,   749,   667,   668,   651,     0,
      17,     0,    99,   750,   613,     0,   138,   651,   651,     0,
      47,   651,   651,   529,     0,   525,   659,   662,   660,   664,
     665,   663,   658,     0,    58,   656,   661,     0,   243,     0,
      60,     0,   239,     0,   237,   598,   170,     0,   167,   651,
     610,   563,     0,   559,   561,   609,   651,   651,   534,     0,
     530,   651,   545,     0,   541,   651,   599,   540,     0,   537,
     600,   651,     0,    25,   651,   651,   550,     0,   546,     0,
      56,   575,     0,   213,     0,     0,   236,     0,   233,   651,
     605,     0,    49,   651,     0,   535,     0,    62,   651,   651,
     219,     0,   215,    74,    76,     0,    45,   651,   651,   651,
     114,     0,   109,   558,     0,   555,   651,   569,     0,   241,
     603,   604,   601,   209,     0,   206,   651,   602,     0,   191,
     621,   620,   651,     0,   807,     0,   808,     0,   821,     0,
       0,     0,   180,     0,   823,     0,   824,     0,   825,     0,
       0,     0,     0,     0,   445,     0,     0,     0,     0,   452,
       0,     0,     0,   619,   619,   826,   651,   651,   827,   651,
     628,   651,     7,   619,   607,   619,   619,   101,   100,   618,
     616,   139,   619,   619,   611,   612,   619,   528,   527,   526,
      59,   651,   244,    61,   240,   238,   168,   169,   560,   651,
     533,   532,   531,   543,   542,   544,   538,   539,    26,   549,
     548,   547,    57,   214,     0,   578,   572,     0,   574,   582,
     234,   235,    50,   606,   536,    63,   218,   217,   216,   651,
      46,   111,   113,   112,   110,   556,   557,   567,   242,   207,
     208,   192,     0,   625,   624,     0,   150,     0,   140,     0,
     124,     0,   172,     0,   551,     0,   182,     0,   564,     0,
     518,     0,    65,     0,   368,     0,   357,     0,   334,     0,
     266,     0,   245,     0,   285,     0,   298,     0,   314,     0,
     454,     0,   383,     0,   430,     0,   370,   447,   447,   645,
     647,   632,   630,     6,    13,    20,   104,   614,     0,     0,
     657,   562,   587,   577,   581,     0,    75,   570,   651,   634,
     622,   623,   626,   619,   151,   149,   619,   619,   126,   125,
     619,   173,   171,   619,   553,   552,   619,   183,   181,   619,
     211,   210,   619,   520,   519,   619,    69,    68,   619,   372,
     369,   619,   359,   358,   619,   336,   335,   619,   268,   267,
     619,   247,   246,   619,   619,   619,   619,   456,   455,   619,
     385,   384,   619,   434,   431,   371,     0,     0,     0,   631,
     651,    27,    12,    27,    19,     0,     0,   617,   619,     0,
     576,   579,   583,   580,   585,     0,   568,   636,   156,   142,
       0,   175,   175,   185,   175,   522,    71,   374,   361,   338,
     270,   249,   286,   300,   316,   458,   387,   436,   446,   619,
     619,   619,   258,   259,   260,   261,   262,   263,   264,   265,
     619,   453,   651,   627,   651,     0,    51,     0,    14,    15,
      16,    51,    21,   619,     0,   102,   615,    48,   654,   584,
       0,   565,     0,     0,     0,     0,   153,   154,   619,   155,
     221,     0,   127,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     449,   450,   451,   448,   648,     0,     0,     8,     0,     0,
     619,   619,    66,     0,    66,    22,   651,   651,   108,     0,
     103,   655,     0,   586,   644,   635,   638,   651,   627,   627,
     643,     0,     0,   152,   159,   619,     0,   162,   619,   619,
     619,   158,   157,   194,   220,   141,   147,   148,   146,   619,
     144,   145,   174,   178,   179,   176,   177,   554,   184,   189,
     190,   186,   187,   188,   212,   521,   523,   524,    70,    72,
      73,   373,   381,   382,   380,   619,   619,   378,   379,   619,
     360,   365,   366,   364,   619,   619,   619,   337,   345,   346,
     344,   619,   341,   350,   351,   352,   353,   356,   619,   348,
     349,   354,   355,   619,   342,   343,   269,   277,   278,   276,
     619,   619,   619,   619,   619,   619,   619,   275,   274,   619,
     248,   251,   252,   250,   619,   619,   619,   619,   619,   284,
     296,   297,   295,   619,   619,   290,   292,   619,   293,   294,
     619,   299,   312,   313,   311,   619,   619,   305,   304,   619,
     307,   308,   309,   310,   619,   315,   327,   328,   326,   619,
     619,   619,   619,   619,   619,   619,   322,   323,   324,   325,
     619,   321,   320,   457,   462,   463,   461,   619,   619,   619,
     619,   619,     0,     0,   386,   391,   392,   390,   619,   619,
     619,   619,   435,   439,   440,   438,   619,   619,   619,   619,
     619,   646,   651,   651,     0,     0,    28,    29,    52,    53,
      54,    55,    78,    64,     0,    23,    78,   107,   106,   105,
       0,   654,   637,     0,     0,     0,   224,     0,   197,   164,
     165,   160,   161,   163,   193,   222,   143,   376,   375,   377,
     363,   367,   362,   340,   347,   339,   272,   282,   279,   283,
     280,   281,   271,   273,   254,   253,   255,   256,   257,   288,
     289,   287,   291,   302,   303,   301,   306,   318,   329,   330,
     333,   331,   332,   317,   319,   460,   464,   465,   466,   459,
       0,     0,   389,   393,   394,   388,   437,   441,   442,   443,
     444,   633,     9,     0,    31,     0,    37,     0,    77,   619,
      24,     0,   588,     0,   651,   641,   639,   640,   619,   225,
     619,   619,   198,   196,   619,   413,   414,     0,   651,   396,
     397,     0,   651,   619,   619,    39,    38,   651,     0,     0,
       0,     0,     0,     0,   619,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    67,   651,   654,   645,   227,
     223,   200,   195,   619,   412,   619,   399,   398,   395,    32,
      41,    11,     0,     0,     0,     0,     0,     0,    79,    18,
       0,     0,     0,     0,   420,   401,     0,     0,   417,   418,
       0,   567,   651,     0,    90,   474,     0,   467,   651,     0,
     115,     0,   128,     0,   432,   654,   589,   654,   642,   226,
     231,   232,   230,   619,   229,   199,   204,   205,   203,   619,
     202,     0,     0,    30,    36,    33,    34,    35,    40,    44,
      42,    43,   619,   566,   416,   619,    92,    91,   619,   473,
     619,   117,   116,   619,   130,   129,   433,     0,     0,   228,
     201,   415,   424,   425,   423,   619,   619,   619,   619,   619,
     619,   400,   410,   411,   619,   405,   406,   407,   404,   408,
     409,   619,   420,    94,   469,   119,   132,   654,   654,   422,
     426,   429,   427,   428,   421,   403,   402,     0,     0,     0,
       0,     0,     0,     0,   419,    93,    97,    98,   619,    96,
       0,   468,   470,   471,   118,   122,   123,   121,   619,   131,
     136,   137,   135,   619,   133,   597,   654,   590,   593,    95,
       0,   120,   134,     0,     0,   484,   497,   477,   506,   619,
     651,   475,   476,   651,   481,   651,   483,   651,   482,   654,
     594,   595,   472,     0,     0,     0,     0,   592,   591,   619,
     479,   478,   619,   486,   485,   619,   499,   498,   619,   508,
     507,     0,     0,   488,   501,   510,   654,   480,     0,     0,
       0,     0,   487,   489,   492,   493,   494,   495,   496,   619,
     491,   500,   502,   505,   619,   504,   509,   619,   512,   513,
     514,   515,   516,   517,   596,   490,   503,   511
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1012, -1012, -1012,   245, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012,   -16, -1012,    -2,    -9, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1011, -1012,    22,
   -1012,  -534,   -24, -1012,   658, -1012,   681, -1012,    63, -1012,
     105,   -41, -1012, -1012,  -237, -1012, -1012,   305, -1012,  -225,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012,  -486, -1012, -1012,
   -1012, -1012, -1012,    41, -1012, -1012, -1012, -1012, -1012, -1012,
     -11, -1012, -1012, -1012, -1012, -1012, -1012,  -657, -1012,    -3,
   -1012,  -653, -1012, -1012, -1012, -1012, -1012, -1012, -1012,    23,
   -1012,    30, -1012, -1012,    53, -1012,    35, -1012, -1012, -1012,
   -1012,    10, -1012, -1012,  -236, -1012, -1012, -1012, -1012,  -366,
   -1012,    38, -1012, -1012,    40, -1012,    43, -1012, -1012, -1012,
     -21, -1012, -1012, -1012, -1012,  -355, -1012, -1012,    12, -1012,
      18, -1012,  -704, -1012,  -480, -1012,    16, -1012, -1012,  -560,
   -1012,   -80, -1012, -1012,   -67, -1012, -1012, -1012,   -63, -1012,
   -1012,   -39, -1012, -1012,   -36, -1012, -1012, -1012, -1012, -1012,
     -33, -1012, -1012, -1012,   -28, -1012,   -27,   206, -1012, -1012,
     667, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012,  -435, -1012,   -62, -1012, -1012,  -365,
   -1012, -1012,    42,   211, -1012,    44, -1012,   -87, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012,
   -1012, -1012,    74, -1012, -1012, -1012,  -473, -1012, -1012,  -667,
   -1012, -1012,  -675, -1012,  -722, -1012, -1012,  -662, -1012, -1012,
    -271, -1012, -1012,   -35, -1012, -1012,  -725, -1012, -1012,    46,
   -1012, -1012, -1012,  -390,  -319,  -335,  -461, -1012, -1012, -1012,
   -1012,   214,    97, -1012, -1012,   239, -1012, -1012, -1012,   135,
   -1012, -1012, -1012,  -441, -1012, -1012, -1012,   155,   693, -1012,
   -1012, -1012,   185,   -93,  -328,  1164, -1012, -1012, -1012,   526,
     110, -1012, -1012, -1012,  -433, -1012, -1012, -1012, -1012, -1012,
    -143, -1012, -1012,  -260,    84,    -4,  1453,   166,  -694,   119,
   -1012,   660,   -12, -1012,   137,   -20,   -23, -1012, -1012, -1012,
   -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012, -1012
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   261,   262,   553,   933,   263,     2,   631,   632,
     269,     3,   633,   634,   944,   688,   332,    54,   686,   740,
    1023,  1106,  1025,   741,  1056,  1107,   365,    55,   279,    56,
     351,    57,   742,   339,   938,   293,   939,   299,   783,   356,
     784,   942,   521,   943,   144,   597,   718,   366,   489,  1027,
    1028,  1064,  1113,  1065,  1157,  1208,   271,    62,   438,   749,
     636,   750,   371,  1174,   372,  1119,  1066,  1162,  1210,   509,
    1175,   579,  1121,  1067,  1165,  1211,   275,    64,   507,   669,
     711,   127,   505,   575,   705,   706,   765,   766,   307,    65,
     308,   136,   511,   582,   713,   401,   137,   515,   588,   715,
     388,    66,   707,   964,   708,   957,  1043,  1103,   384,    67,
     385,   138,   591,   342,    68,   361,    69,   362,   709,   774,
     710,   955,  1040,  1102,   347,    70,   348,   303,   785,   301,
     786,   378,    73,   297,    74,   531,   670,   612,   723,   671,
     529,   672,   609,   722,   673,   533,   724,   535,   674,   725,
     537,   675,   726,   527,   676,   606,   721,   828,   829,   525,
    1177,   603,   720,   523,   677,   545,   678,   600,   719,   541,
     679,   621,   728,  1050,  1051,   919,  1087,  1142,  1046,  1047,
     920,  1109,  1110,  1071,  1141,   543,  1178,  1123,  1072,   624,
     729,   170,   171,   626,   172,   173,   539,  1179,   618,   727,
    1116,  1074,  1209,  1117,  1249,  1250,  1251,  1271,  1252,  1253,
    1254,  1274,  1288,  1255,  1256,  1277,  1289,  1257,  1258,  1280,
    1290,   519,  1180,   594,   717,   284,    75,   285,   319,    76,
     320,   354,    77,   328,    78,   329,   323,    79,   324,   337,
      80,   338,   513,   680,   585,   374,    81,   375,   312,    82,
     313,   459,   517,   646,  1112,   567,   376,   497,   343,   344,
     345,   475,   476,   563,   478,   479,   565,   643,   699,   640,
     950,  1126,  1237,  1238,  1244,  1268,  1127,   330,   331,   386,
     387,   352,   264,   377,   276,   441,   442,   638,   443,   124,
     390,   502,   503,   571,   176,   430,   629,   570,   702,   757,
    1036,   758,   759,   628,   428,   391,     4,   177,   752,   294,
     451,   295,   265,   266,   267,   268,   392,    83,    84,    85,
      86,    87,    88,    89,    90,    91,   175,   140,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      11,   901,   174,   881,   897,    92,    92,   780,    92,   160,
      92,    92,   314,    92,    92,    92,    92,    71,    92,    92,
     865,   877,   161,    72,    92,   639,   162,   169,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    63,   848,    92,   764,
     163,   139,   808,   164,   835,   751,   165,   869,   779,   180,
     180,   166,   167,   882,   898,   506,   180,   126,    60,   305,
     363,   864,   876,   278,   131,    36,   277,    15,     7,   180,
       8,   129,   426,    41,   427,   180,    92,   296,   296,   296,
     296,   296,   542,   315,   353,  1144,  1149,   296,   690,   296,
       6,   687,   180,   296,   296,   159,   180,   128,   315,   296,
      61,   296,   180,   960,     7,   305,     8,     7,  -573,     8,
     273,   130,    92,   273,    92,     7,    92,     8,    92,    92,
      92,    92,     7,   423,     8,   737,   687,    92,     7,    92,
       8,    92,    92,    92,    92,    92,   321,    92,    92,    92,
      92,   141,    92,    92,    92,  -587,  -587,  -654,   645,   281,
     815,   142,   843,   856,   282,   296,   892,   910,     7,   334,
       8,   436,   335,   437,    11,    93,    94,  1269,    95,  1270,
      96,    97,   316,    98,    99,   100,   101,   317,   102,   103,
     180,     7,   635,     8,   104,   143,   180,   273,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   477,   637,   123,   298,
     300,   302,   304,    14,  1272,    12,  1273,     7,   333,     8,
     340,   781,    20,   273,   355,   357,    20,  1275,   424,  1276,
     379,   822,   389,   130,   866,   878,   807,    20,   834,   847,
     735,   868,   880,   896,   180,   433,   912,  1033,   130,   309,
     435,    20,   325,    22,    23,   446,    20,  1278,    43,  1279,
     358,     7,    43,     8,    46,   359,   746,   130,    46,   270,
     272,   747,   280,    43,     7,     7,     8,     8,   932,    46,
     273,   712,   393,   576,   395,   180,   397,    43,   399,   400,
     402,   403,    43,   913,   305,   326,  1229,   405,    46,   407,
    1215,   409,   410,   411,   412,   413,   141,   415,   416,   417,
     418,  1224,   420,   421,   422,   953,   954,  1287,   439,   180,
     440,   367,   568,   368,   569,   782,   369,   380,   381,   382,
     914,   274,   613,   283,   292,   292,   292,   292,   292,   306,
     311,   318,   322,   327,   292,   336,   292,   341,   346,   350,
     292,   292,   360,   364,   370,   373,   292,   383,   292,   474,
     278,    17,    20,   277,    19,    20,    19,  1245,   573,  1246,
     574,   562,  1247,  1100,  1248,   296,   130,   296,   296,   296,
      20,   296,   577,   296,   578,    33,    20,   278,   564,   133,
     277,   133,   580,    18,   581,    41,    20,   583,    43,   584,
      11,    43,    45,   474,   698,    11,    20,    46,   614,   126,
    1189,   615,    48,    47,    48,   141,    43,   130,    11,   630,
      11,  1167,    43,  1168,    38,    20,    45,    22,    23,   645,
      11,    11,    43,    11,    11,  -651,  1143,  -651,    40,   859,
     684,  1301,    43,    11,   883,   899,    11,    11,    46,    11,
     695,    11,   315,    11,   795,    11,    11,  1188,  1070,    20,
    1148,    43,   644,   697,   586,  1187,   587,    11,   751,    11,
    1190,   698,    11,    11,   589,   145,   590,    11,    11,    11,
    1129,   296,    11,   592,  -651,   593,    11,   595,   703,   596,
      11,    52,   763,  1212,  1213,    43,   146,   147,  1032,   788,
     148,   598,   704,   599,  -651,   601,   510,   602,   512,   514,
     516,   149,   518,   604,   520,   605,   150,  1053,   151,   152,
     607,   153,   608,  1057,    20,   683,    22,    23,   154,  1076,
      20,   155,  1243,   798,  1125,    11,    11,    11,    11,  1048,
    1052,   130,   701,   315,  1234,    20,    11,    11,   738,   739,
     156,  1220,    11,  1300,  1305,  1267,  1297,   610,  1312,   611,
      43,     7,  1145,     8,  1281,  1077,    43,   157,  1197,   158,
     508,  1176,    46,  1083,  1293,  1302,  1308,  1152,   125,    49,
    1158,    43,  1291,  1236,   524,   526,   528,   530,   532,  1198,
     534,   536,   538,   540,  1259,  1235,   544,   546,   787,   616,
     619,   617,   620,     7,  1135,     8,   315,  1286,   692,   273,
       9,  1296,   572,  1311,   691,   622,  1298,   623,  1313,  1221,
     689,    92,  1034,   315,  1035,   845,   858,  1307,   274,   894,
      10,   823,   292,    11,   292,   292,   292,  1176,   292,  1038,
     292,  1039,   364,   394,   824,   396,   693,   398,   825,   951,
     844,   857,  1185,    58,   893,   274,   404,   744,   406,  1186,
     408,  1041,    41,  1042,  1054,  1085,  1055,  1086,  1155,    92,
    1156,    11,   826,    92,   809,   827,    59,   849,   830,   870,
     884,   900,   911,   831,   832,  1160,  1163,  1161,  1164,    92,
      92,   425,  1111,   946,  1111,   714,  1029,   716,   805,   814,
     821,   840,   522,   863,   875,   889,   907,   918,   926,   841,
     854,  1031,  1218,   890,   908,   791,   927,   792,  1044,   767,
      11,   296,    11,   793,    92,    92,   768,  1140,   842,   855,
     315,   769,   891,   909,   770,   928,   771,  1134,   292,   772,
     296,   625,   778,   965,    92,    92,   168,  1207,  1166,   627,
     804,   813,   820,   839,   853,   862,   874,   888,   906,   917,
     925,   929,   902,   930,   776,  1118,  1153,   641,   789,   700,
     796,   799,   802,   811,   818,   837,   851,   860,   872,   886,
     904,   915,   923,   806,   816,   833,   846,   753,   867,   879,
     895,   694,   921,  1260,   642,   940,   349,   940,  1294,  1303,
    1309,  1037,   756,    19,    20,  1295,   777,  1310,  1101,   931,
     790,   145,   797,   800,   803,   812,   819,   838,   852,   861,
     873,   887,   905,   916,   924,     0,     7,   431,     8,   760,
       0,     0,   273,   147,    17,     0,   148,   941,    20,   941,
      43,    17,     0,   743,    19,   703,    46,   149,   126,   130,
       0,    48,   945,   704,   151,   152,     0,   153,     0,   761,
     762,     0,   133,     0,   154,    33,    34,    35,     0,   133,
       0,   958,    42,    20,    43,     0,     0,     0,   775,   145,
      46,     0,   149,   128,   130,     0,   156,   150,   141,     0,
       0,    47,    48,     0,   934,   935,     0,     0,    92,    92,
     146,   147,   155,   157,   148,   158,    20,   132,    20,    43,
      22,    23,   836,   133,     0,    46,     0,   130,   128,  1292,
     134,     0,   151,   152,   135,   153,     0,     0,     0,   748,
       0,  1073,   154,    11,    11,     0,   956,     0,    11,   547,
     548,   145,    43,     0,    43,     0,    17,   922,    46,   554,
      20,   555,   556,     0,   156,     0,   141,     0,   557,   558,
       0,   130,   559,   147,     0,     0,   148,     0,    20,     0,
      33,   157,     0,   158,   133,     0,     0,   149,   292,     0,
    1171,     0,   794,     0,   151,   152,    43,   153,   315,   315,
       0,     0,    46,     0,   154,     0,     0,   292,   683,     0,
     141,     0,    17,     0,    43,    19,     0,    11,    11,     0,
      46,     0,    14,   128,     0,  1068,   156,     0,     0,     0,
       0,     0,     0,     0,   801,     0,    33,    34,    35,    28,
       0,     0,     0,   157,  1069,   158,     0,     0,     0,   132,
       0,     0,   850,     0,    92,    92,    92,    92,    92,    92,
     126,    39,   134,    48,     0,     0,   135,     0,     0,    44,
       0,     0,     0,     0,    11,  -166,     0,     0,  1010,  1011,
      11,     0,     0,     0,    11,     0,     0,    11,     0,   315,
    1306,  1133,  1139,     0,     0,    11,     0,     0,     0,   648,
       0,     0,   649,   650,     0,     0,   651,  1191,     0,   652,
       0,     0,   653,     0,     0,   654,     0,     0,   655,  1024,
    1026,   656,     0,     0,   657,     0,     0,   658,     0,     0,
     659,  1184,     0,   660,     0,     0,   661,     0,     0,   662,
     663,   664,   665,  1132,  1138,   666,     0,     0,   667,     0,
      11,  1261,     0,    17,     0,    11,    19,    20,     0,     0,
       0,     0,     0,     0,   696,  1130,  1136,     0,   130,  1146,
    1150,     0,     0,     0,     0,     0,     0,    33,    34,    35,
       0,   133,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    42,     0,    43,     0,   730,   731,   732,  1314,  1228,
    1233,     0,     0,     0,  1172,  1182,   733,  1131,  1137,     0,
       0,  1147,  1151,     0,     0,     0,    92,     0,     0,   745,
       0,     0,     0,     0,  1092,  1093,  1094,  1095,  1096,  1097,
       0,  1181,   315,     0,   773,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1173,  1183,     0,  1219,
       0,  1227,  1232,  1299,  1304,  1045,  1049,     0,     0,    11,
      11,    11,    11,     0,     0,     0,   936,   937,     0,     0,
    1172,  1216,  1222,  1225,  1230,     0,     0,     0,  1114,   315,
    1120,  1122,  1124,     0,     0,     0,     0,     0,     0,     0,
       0,   959,     0,     0,   961,   962,   963,     0,     0,     0,
       0,     0,     0,     0,     0,   966,     0,     0,     0,     0,
       0,     0,  1173,  1217,  1223,  1226,  1231,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   967,   968,     0,     0,   969,     0,  1108,     0,  1115,
     970,   971,   972,     0,     0,     0,     0,   973,     0,     0,
       0,     0,     0,     0,   974,     0,     0,     0,     0,   975,
       0,     0,     0,     0,     0,     0,   976,   977,   978,   979,
     980,   981,   982,     0,     0,   983,     0,     0,     0,     0,
     984,   985,   986,   987,   988,     0,  1240,     0,     0,   989,
     990,     0,     0,   991,     0,     0,   992,     0,     0,     0,
       0,   993,   994,     0,     0,   995,     0,     0,     0,     0,
     996,     0,     0,     0,     0,   997,   998,   999,  1000,  1001,
    1002,  1003,     0,     0,     0,     0,  1004,     0,     0,     0,
       0,     0,     0,  1005,  1006,  1007,  1008,  1009,     0,     0,
       0,     0,     0,     0,  1012,  1013,  1014,  1015,   449,     0,
       0,     0,  1016,  1017,  1018,  1019,  1020,   450,     0,     0,
       0,   452,     0,   453,   145,   454,     0,   455,     0,     0,
       0,   456,     0,     0,     0,     0,   458,     0,     0,     0,
       0,     0,     0,   462,     0,   146,   147,   464,     0,   148,
       0,    20,   466,     0,     0,     0,   468,     0,     0,     0,
       0,   471,   130,   472,     0,     0,   473,   151,   152,     0,
     153,   480,     0,     0,     0,   482,     0,   154,   484,     0,
     485,     0,     0,     0,     0,   488,     0,    43,     0,   490,
       0,     0,     0,    46,     0,   494,     0,     0,   495,   156,
       0,   141,   498,     0,     0,   178,     0,     0,   499,     0,
       0,   145,   501,     0,     0,  1075,   157,     0,   158,     0,
       0,     0,     0,     0,  1079,  1214,  1080,  1081,     0,     0,
    1082,     0,     0,   147,    17,     0,   148,     0,    20,  1089,
    1090,     0,     0,     0,     0,     0,     0,   149,     0,   130,
    1098,     0,     0,    32,   151,   152,     0,   153,     0,    34,
      35,     0,   133,   414,   154,    37,     0,     0,   419,  1104,
       0,  1105,     0,     0,    43,     0,     0,     0,     0,     0,
      46,     0,     0,   128,    47,     0,   156,     0,   141,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   157,     0,   158,     0,     0,     0,   145,
       0,     0,   885,     0,     0,     0,     0,     0,     0,  1169,
       0,     0,     0,     0,     0,  1170,     0,     0,     0,     0,
     146,   147,    17,     0,   148,    19,    20,     0,  1192,     0,
       0,  1193,     0,     0,  1194,     0,  1195,   130,     0,  1196,
       0,     0,   151,   152,     0,   153,    33,     0,     0,     0,
       0,  1199,  1200,  1201,  1202,  1203,  1204,     0,     0,     0,
    1205,     0,    43,     0,   432,     0,     0,  1206,    46,     0,
       0,   434,     0,     0,     0,     0,   141,     0,     0,     0,
     444,   445,     0,     0,   447,   448,     0,     0,     0,     0,
       0,     0,     0,   158,  1239,     0,     0,     0,     0,     0,
     817,     0,     0,     0,  1241,     0,     0,     0,     0,  1242,
       0,     0,   457,     0,     0,     0,     0,     0,     0,   460,
     461,     0,   145,     0,   463,  1262,     0,     0,   465,     0,
       0,     0,     0,     0,   467,     0,     0,   469,   470,     0,
       0,     0,     0,     0,   147,  1282,     0,   148,  1283,    20,
       0,  1284,   481,     0,  1285,     0,   483,     0,   149,     0,
     130,   486,   487,     0,     0,   151,   152,     0,   153,     0,
     491,   492,   493,   133,     0,  1315,     0,     0,     0,   496,
    1316,     0,     0,  1317,     0,    43,     0,     0,     0,   500,
       0,    46,     0,     0,   128,   504,     0,   156,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   158,     0,     0,     0,
       0,     0,     0,   903,     0,     0,     0,     0,     0,   549,
     550,     0,   551,     0,   552,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    -4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   560,     0,     0,     0,     0,     0,
       0,     0,   561,   949,    12,    13,    14,    15,    16,     0,
       0,    17,    18,     0,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,  -753,    30,    31,     0,
      32,     0,   566,  -754,     0,    33,    34,    35,     0,  -754,
      36,     0,    37,    38,     0,    39,  -754,    40,    41,    42,
    -754,    43,   145,    44,  -756,    45,     0,    46,   145,  -751,
    -752,    47,    48,     0,    49,  -755,    50,    51,     0,     0,
       0,     0,     0,     0,   147,     0,     0,   148,     0,    20,
     147,    52,     0,   148,     0,     0,    53,     0,   145,     0,
     130,     0,     0,     0,   149,   151,   152,     0,   153,     0,
       0,   151,   152,     0,   153,     0,     0,     0,     0,   133,
     147,   647,     0,   148,     0,    43,     0,     0,     0,     0,
       0,    46,     0,     0,   149,     0,     0,   156,     0,   141,
     128,   151,   152,   156,   153,     0,     0,     0,     0,   133,
     145,     0,     0,     0,     0,     0,   158,     0,     0,     0,
       0,     0,   158,   810,     0,     0,     0,  1058,     0,   668,
     128,     0,   147,   156,     0,   148,     0,     0,     0,     0,
       0,  1059,  1060,   685,  1061,     0,   149,  1062,     0,     0,
       0,     0,   158,   151,   152,     0,   153,     0,     0,   681,
       0,    17,     0,   154,    19,    20,     0,     0,  1030,     0,
       0,     0,     0,     0,     0,     0,   130,     0,  1063,     0,
       0,     0,   128,     0,     0,   156,    34,    35,     0,   133,
       0,     0,    37,     0,     0,   734,     0,   736,     0,     0,
       0,    43,     0,     0,   158,     0,     0,    46,     0,   126,
       0,     0,    48,     0,     0,   141,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   871,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   179,     0,     0,     0,   947,
     948,   181,   310,     0,     0,     0,     0,     0,     0,     0,
     952,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,     0,     0,   682,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   179,     0,     0,     0,     0,     0,   181,   310,     0,
       0,     0,     0,     0,     0,  1021,  1022,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,     0,     0,     0,     0,
       0,     0,  1128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1078,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1084,     0,     0,     0,  1088,     0,     0,     0,     0,
    1091,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1099,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   179,     0,     0,     0,
       0,     0,     0,   273,     0,  1154,     0,     0,     0,     0,
       0,  1159,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   754,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,     0,   248,     0,
       0,     0,     0,   253,     0,     0,     0,     0,   258,   259,
     260,     0,     0,     0,     0,     0,     0,   755,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1263,     0,     0,  1264,   179,  1265,     0,
    1266,   180,   286,   181,   287,   288,     0,     0,     0,   289,
     290,   291,     0,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   179,     0,     0,     0,   429,   286,   181,   287,
     288,     0,     0,     0,   289,   290,   291,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   179,     0,     0,
       0,   180,     0,   181,   273,     0,     0,     0,     0,     0,
       0,     0,     0,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   179,     0,     0,     0,   180,     0,   181,   310,
       0,     0,     0,     0,     0,     0,     0,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   179,     0,     0,
       0,     0,     0,   181,   310,     0,     0,   477,     0,     0,
       0,     0,     0,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   179,     0,     0,     0,     0,     0,   181,   310,
       0,     0,  1236,     0,     0,     0,     0,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   179,     0,     0,
       0,   180,     0,   181,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   179,     0,     0,     0,     0,     0,   181,   310,
       0,     0,     0,     0,     0,     0,     0,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260
};

static const yytype_int16 yycheck[] =
{
       4,   726,    89,   725,   726,     9,    10,   711,    12,    89,
      14,    15,   105,    17,    18,    19,    20,     5,    22,    23,
     724,   725,    89,     5,    28,   559,    89,    89,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     5,   722,    52,   706,
      89,    86,   719,    89,   721,     5,    89,   724,   711,     8,
       8,    89,    89,   725,   726,   393,     8,    75,     5,    17,
      12,   724,   725,    96,    85,    56,    96,    23,     5,     8,
       7,    84,   104,    64,   106,     8,    90,    99,   100,   101,
     102,   103,   420,   105,   114,  1106,  1107,   109,   632,   111,
       0,    82,     8,   115,   116,    89,     8,    76,   120,   121,
       5,   123,     8,   766,     5,    17,     7,     5,    14,     7,
      11,    42,   126,    11,   128,     5,   130,     7,   132,   133,
     134,   135,     5,   104,     7,     8,    82,   141,     5,   143,
       7,   145,   146,   147,   148,   149,    94,   151,   152,   153,
     154,    81,   156,   157,   158,   107,   108,   107,   107,    88,
     720,    87,   722,   723,    93,   177,   726,   727,     5,    92,
       7,   104,    95,   106,   178,     9,    10,   104,    12,   106,
      14,    15,    88,    17,    18,    19,    20,    93,    22,    23,
       8,     5,    83,     7,    28,    70,     8,    11,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    14,   105,    52,   100,
     101,   102,   103,    22,   104,    20,   106,     5,   109,     7,
     111,   711,    31,    11,   115,   116,    31,   104,   104,   106,
     121,   721,   123,    42,   724,   725,   719,    31,   721,   722,
     683,   724,   725,   726,     8,   104,    21,   951,    42,   104,
     104,    31,   107,    33,    34,   104,    31,   104,    67,   106,
      88,     5,    67,     7,    73,    93,    88,    42,    73,    94,
      95,    93,    97,    67,     5,     5,     7,     7,     8,    73,
      11,   105,   126,   104,   128,     8,   130,    67,   132,   133,
     134,   135,    67,    68,    17,    18,   105,   141,    73,   143,
     105,   145,   146,   147,   148,   149,    81,   151,   152,   153,
     154,   105,   156,   157,   158,   758,   759,   105,   104,     8,
     106,    85,   104,    87,   106,   105,    90,    16,    17,    18,
     105,    96,   104,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   107,
     393,    27,    31,   393,    30,    31,    30,    84,   104,    86,
     106,   474,    89,  1077,    91,   397,    42,   399,   400,   401,
      31,   403,   104,   405,   106,    51,    31,   420,   107,    55,
     420,    55,   104,    28,   106,    64,    31,   104,    67,   106,
     414,    67,    71,   107,   108,   419,    31,    73,   104,    75,
    1142,   104,    78,    77,    78,    81,    67,    42,   432,   103,
     434,  1125,    67,  1127,    59,    31,    71,    33,    34,   107,
     444,   445,    67,   447,   448,     5,   105,     7,    63,   105,
     103,   105,    67,   457,   725,   726,   460,   461,    73,   463,
     105,   465,   474,   467,   105,   469,   470,  1142,  1028,    31,
     105,    67,   565,   105,   104,  1142,   106,   481,     5,   483,
    1142,   108,   486,   487,   104,     4,   106,   491,   492,   493,
     105,   503,   496,   104,    54,   106,   500,   104,    24,   106,
     504,    97,   105,  1197,  1198,    67,    25,    26,   109,   105,
      29,   104,    32,   106,    74,   104,   397,   106,   399,   400,
     401,    40,   403,   104,   405,   106,    45,   104,    47,    48,
     104,    50,   106,   105,    31,   628,    33,    34,    57,   105,
      31,    60,  1236,   105,    85,   549,   550,   551,   552,  1010,
    1011,    42,   645,   565,  1211,    31,   560,   561,    43,    44,
      79,    37,   566,  1288,  1289,  1259,  1288,   104,  1290,   106,
      67,     5,  1106,     7,  1268,   110,    67,    96,   111,    98,
     395,  1141,    73,   104,  1288,  1289,  1290,   104,    62,    80,
     104,    67,  1286,    14,   409,   410,   411,   412,   413,   107,
     415,   416,   417,   418,   107,   112,   421,   422,   105,   104,
     104,   106,   106,     5,   105,     7,   628,   107,   634,    11,
      54,  1288,   503,  1290,   633,   104,  1288,   106,  1290,   105,
     632,   635,   104,   645,   106,   722,   723,  1290,   393,   726,
      74,   721,   397,   647,   399,   400,   401,  1207,   403,   104,
     405,   106,   407,   127,   721,   129,   634,   131,   721,   752,
     722,   723,  1142,     5,   726,   420,   140,   691,   142,  1142,
     144,   104,    64,   106,   104,   104,   106,   106,   104,   683,
     106,   685,   721,   687,   719,   721,     5,   722,   721,   724,
     725,   726,   727,   721,   721,   104,   104,   106,   106,   703,
     704,   175,  1092,   744,  1094,   652,   943,   654,   719,   720,
     721,   722,   407,   724,   725,   726,   727,   728,   729,   722,
     723,   946,  1208,   726,   727,   715,   729,   715,   964,   706,
     734,   743,   736,   715,   738,   739,   706,  1103,   722,   723,
     752,   706,   726,   727,   706,   729,   706,  1102,   503,   706,
     762,   545,   711,   774,   758,   759,    89,  1192,  1123,   548,
     719,   720,   721,   722,   723,   724,   725,   726,   727,   728,
     729,   729,   726,   729,   711,  1094,  1111,   563,   715,   644,
     717,   718,   719,   720,   721,   722,   723,   724,   725,   726,
     727,   728,   729,   719,   720,   721,   722,   700,   724,   725,
     726,   635,   728,  1244,   565,   742,   113,   744,  1288,  1289,
    1290,   954,   702,    30,    31,  1288,   711,  1290,  1078,   735,
     715,     4,   717,   718,   719,   720,   721,   722,   723,   724,
     725,   726,   727,   728,   729,    -1,     5,   177,     7,   702,
      -1,    -1,    11,    26,    27,    -1,    29,   742,    31,   744,
      67,    27,    -1,   687,    30,    24,    73,    40,    75,    42,
      -1,    78,   743,    32,    47,    48,    -1,    50,    -1,   703,
     704,    -1,    55,    -1,    57,    51,    52,    53,    -1,    55,
      -1,   762,    65,    31,    67,    -1,    -1,    -1,   105,     4,
      73,    -1,    40,    76,    42,    -1,    79,    45,    81,    -1,
      -1,    77,    78,    -1,   738,   739,    -1,    -1,   912,   913,
      25,    26,    60,    96,    29,    98,    31,    49,    31,    67,
      33,    34,   105,    55,    -1,    73,    -1,    42,    76,   105,
      62,    -1,    47,    48,    66,    50,    -1,    -1,    -1,   694,
      -1,  1028,    57,   947,   948,    -1,   761,    -1,   952,   423,
     424,     4,    67,    -1,    67,    -1,    27,   105,    73,   433,
      31,   435,   436,    -1,    79,    -1,    81,    -1,   442,   443,
      -1,    42,   446,    26,    -1,    -1,    29,    -1,    31,    -1,
      51,    96,    -1,    98,    55,    -1,    -1,    40,   743,    -1,
     105,    -1,   105,    -1,    47,    48,    67,    50,  1010,  1011,
      -1,    -1,    73,    -1,    57,    -1,    -1,   762,  1101,    -1,
      81,    -1,    27,    -1,    67,    30,    -1,  1021,  1022,    -1,
      73,    -1,    22,    76,    -1,  1028,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    51,    52,    53,    39,
      -1,    -1,    -1,    96,  1028,    98,    -1,    -1,    -1,    49,
      -1,    -1,   105,    -1,  1058,  1059,  1060,  1061,  1062,  1063,
      75,    61,    62,    78,    -1,    -1,    66,    -1,    -1,    69,
      -1,    -1,    -1,    -1,  1078,    75,    -1,    -1,   912,   913,
    1084,    -1,    -1,    -1,  1088,    -1,    -1,  1091,    -1,  1101,
     105,  1102,  1103,    -1,    -1,  1099,    -1,    -1,    -1,   573,
      -1,    -1,   576,   577,    -1,    -1,   580,  1142,    -1,   583,
      -1,    -1,   586,    -1,    -1,   589,    -1,    -1,   592,   934,
     935,   595,    -1,    -1,   598,    -1,    -1,   601,    -1,    -1,
     604,  1142,    -1,   607,    -1,    -1,   610,    -1,    -1,   613,
     614,   615,   616,  1102,  1103,   619,    -1,    -1,   622,    -1,
    1154,  1244,    -1,    27,    -1,  1159,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,   638,  1102,  1103,    -1,    42,  1106,
    1107,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    67,    -1,   669,   670,   671,  1291,  1210,
    1211,    -1,    -1,    -1,  1141,  1142,   680,  1102,  1103,    -1,
      -1,  1106,  1107,    -1,    -1,    -1,  1220,    -1,    -1,   693,
      -1,    -1,    -1,    -1,  1058,  1059,  1060,  1061,  1062,  1063,
      -1,   105,  1244,    -1,   708,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,  1142,    -1,  1208,
      -1,  1210,  1211,  1288,  1289,  1010,  1011,    -1,    -1,  1263,
    1264,  1265,  1266,    -1,    -1,    -1,   740,   741,    -1,    -1,
    1207,  1208,  1209,  1210,  1211,    -1,    -1,    -1,  1093,  1291,
    1095,  1096,  1097,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   765,    -1,    -1,   768,   769,   770,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   779,    -1,    -1,    -1,    -1,
      -1,    -1,  1207,  1208,  1209,  1210,  1211,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   805,   806,    -1,    -1,   809,    -1,  1092,    -1,  1094,
     814,   815,   816,    -1,    -1,    -1,    -1,   821,    -1,    -1,
      -1,    -1,    -1,    -1,   828,    -1,    -1,    -1,    -1,   833,
      -1,    -1,    -1,    -1,    -1,    -1,   840,   841,   842,   843,
     844,   845,   846,    -1,    -1,   849,    -1,    -1,    -1,    -1,
     854,   855,   856,   857,   858,    -1,  1220,    -1,    -1,   863,
     864,    -1,    -1,   867,    -1,    -1,   870,    -1,    -1,    -1,
      -1,   875,   876,    -1,    -1,   879,    -1,    -1,    -1,    -1,
     884,    -1,    -1,    -1,    -1,   889,   890,   891,   892,   893,
     894,   895,    -1,    -1,    -1,    -1,   900,    -1,    -1,    -1,
      -1,    -1,    -1,   907,   908,   909,   910,   911,    -1,    -1,
      -1,    -1,    -1,    -1,   918,   919,   920,   921,   284,    -1,
      -1,    -1,   926,   927,   928,   929,   930,   293,    -1,    -1,
      -1,   297,    -1,   299,     4,   301,    -1,   303,    -1,    -1,
      -1,   307,    -1,    -1,    -1,    -1,   312,    -1,    -1,    -1,
      -1,    -1,    -1,   319,    -1,    25,    26,   323,    -1,    29,
      -1,    31,   328,    -1,    -1,    -1,   332,    -1,    -1,    -1,
      -1,   337,    42,   339,    -1,    -1,   342,    47,    48,    -1,
      50,   347,    -1,    -1,    -1,   351,    -1,    57,   354,    -1,
     356,    -1,    -1,    -1,    -1,   361,    -1,    67,    -1,   365,
      -1,    -1,    -1,    73,    -1,   371,    -1,    -1,   374,    79,
      -1,    81,   378,    -1,    -1,    92,    -1,    -1,   384,    -1,
      -1,     4,   388,    -1,    -1,  1029,    96,    -1,    98,    -1,
      -1,    -1,    -1,    -1,  1038,   105,  1040,  1041,    -1,    -1,
    1044,    -1,    -1,    26,    27,    -1,    29,    -1,    31,  1053,
    1054,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    42,
    1064,    -1,    -1,    46,    47,    48,    -1,    50,    -1,    52,
      53,    -1,    55,   150,    57,    58,    -1,    -1,   155,  1083,
      -1,  1085,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    76,    77,    -1,    79,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    96,    -1,    98,    -1,    -1,    -1,     4,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,  1133,
      -1,    -1,    -1,    -1,    -1,  1139,    -1,    -1,    -1,    -1,
      25,    26,    27,    -1,    29,    30,    31,    -1,  1152,    -1,
      -1,  1155,    -1,    -1,  1158,    -1,  1160,    42,    -1,  1163,
      -1,    -1,    47,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,  1175,  1176,  1177,  1178,  1179,  1180,    -1,    -1,    -1,
    1184,    -1,    67,    -1,   261,    -1,    -1,  1191,    73,    -1,
      -1,   268,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
     277,   278,    -1,    -1,   281,   282,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,  1218,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,  1228,    -1,    -1,    -1,    -1,  1233,
      -1,    -1,   309,    -1,    -1,    -1,    -1,    -1,    -1,   316,
     317,    -1,     4,    -1,   321,  1249,    -1,    -1,   325,    -1,
      -1,    -1,    -1,    -1,   331,    -1,    -1,   334,   335,    -1,
      -1,    -1,    -1,    -1,    26,  1269,    -1,    29,  1272,    31,
      -1,  1275,   349,    -1,  1278,    -1,   353,    -1,    40,    -1,
      42,   358,   359,    -1,    -1,    47,    48,    -1,    50,    -1,
     367,   368,   369,    55,    -1,  1299,    -1,    -1,    -1,   376,
    1304,    -1,    -1,  1307,    -1,    67,    -1,    -1,    -1,   386,
      -1,    73,    -1,    -1,    76,   392,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,   426,
     427,    -1,   429,    -1,   431,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     0,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   451,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   459,   749,    20,    21,    22,    23,    24,    -1,
      -1,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    41,    42,    43,    44,    -1,
      46,    -1,   489,    49,    -1,    51,    52,    53,    -1,    55,
      56,    -1,    58,    59,    -1,    61,    62,    63,    64,    65,
      66,    67,     4,    69,    70,    71,    -1,    73,     4,    75,
      76,    77,    78,    -1,    80,    81,    82,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    31,
      26,    97,    -1,    29,    -1,    -1,   102,    -1,     4,    -1,
      42,    -1,    -1,    -1,    40,    47,    48,    -1,    50,    -1,
      -1,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    55,
      26,   568,    -1,    29,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    40,    -1,    -1,    79,    -1,    81,
      76,    47,    48,    79,    50,    -1,    -1,    -1,    -1,    55,
       4,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    98,   105,    -1,    -1,    -1,    21,    -1,   105,
      76,    -1,    26,    79,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    35,    36,   630,    38,    -1,    40,    41,    -1,    -1,
      -1,    -1,    98,    47,    48,    -1,    50,    -1,    -1,   105,
      -1,    27,    -1,    57,    30,    31,    -1,    -1,   944,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    72,    -1,
      -1,    -1,    76,    -1,    -1,    79,    52,    53,    -1,    55,
      -1,    -1,    58,    -1,    -1,   682,    -1,   684,    -1,    -1,
      -1,    67,    -1,    -1,    98,    -1,    -1,    73,    -1,    75,
      -1,    -1,    78,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,   746,
     747,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     757,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,   932,   933,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1034,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1048,    -1,    -1,    -1,  1052,    -1,    -1,    -1,    -1,
    1057,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1076,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    11,    -1,  1112,    -1,    -1,    -1,    -1,
      -1,  1118,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    96,    97,
      98,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1250,    -1,    -1,  1253,     4,  1255,    -1,
    1257,     8,     9,    10,    11,    12,    -1,    -1,    -1,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,     8,     9,    10,    11,
      12,    -1,    -1,    -1,    16,    17,    18,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,     8,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     4,    -1,    -1,
      -1,     8,    -1,    10,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,     4,    -1,    -1,    -1,    -1,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   114,   120,   124,   419,   441,     0,     5,     7,    54,
      74,   418,    20,    21,    22,    23,    24,    27,    28,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    41,
      43,    44,    46,    51,    52,    53,    56,    58,    59,    61,
      63,    64,    65,    67,    69,    71,    73,    77,    78,    80,
      82,    83,    97,   102,   130,   140,   142,   144,   147,   149,
     151,   153,   170,   176,   190,   202,   214,   222,   227,   229,
     238,   241,   243,   245,   247,   339,   342,   345,   347,   350,
     353,   359,   362,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   418,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   402,   402,    75,   194,    76,   192,
      42,   183,    49,    55,    62,    66,   204,   209,   224,   356,
     440,    81,   335,    70,   157,     4,    25,    26,    29,    40,
      45,    47,    48,    50,    57,    60,    79,    96,    98,   249,
     254,   257,   261,   264,   267,   273,   277,   279,   283,   299,
     304,   305,   307,   308,   310,   439,   407,   420,   419,     4,
       8,    10,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   115,   116,   119,   395,   425,   426,   427,   428,   123,
     395,   169,   395,    11,   116,   189,   397,   428,   429,   141,
     395,    88,    93,   116,   338,   340,     9,    11,    12,    16,
      17,    18,   116,   148,   422,   424,   425,   246,   422,   150,
     422,   242,   422,   240,   422,    17,   116,   201,   203,   390,
      11,   116,   361,   363,   396,   425,    88,    93,   116,   341,
     343,    94,   116,   349,   351,   390,    18,   116,   346,   348,
     390,   391,   129,   422,    92,    95,   116,   352,   354,   146,
     422,   116,   226,   371,   372,   373,   116,   237,   239,   391,
     116,   143,   394,   428,   344,   422,   152,   422,    88,    93,
     116,   228,   230,    12,   116,   139,   160,    85,    87,    90,
     116,   175,   177,   116,   358,   360,   369,   396,   244,   422,
      16,    17,    18,   116,   221,   223,   392,   393,   213,   422,
     403,   418,   429,   420,   402,   420,   402,   420,   402,   420,
     420,   208,   420,   420,   402,   420,   402,   420,   402,   420,
     420,   420,   420,   420,   419,   420,   420,   420,   420,   419,
     420,   420,   420,   104,   104,   402,   104,   106,   417,     8,
     408,   424,   419,   104,   419,   104,   104,   106,   171,   104,
     106,   398,   399,   401,   419,   419,   104,   419,   419,   398,
     398,   423,   398,   398,   398,   398,   398,   419,   398,   364,
     419,   419,   398,   419,   398,   419,   398,   419,   398,   419,
     419,   398,   398,   398,   107,   374,   375,    14,   377,   378,
     398,   419,   398,   419,   398,   398,   419,   419,   398,   161,
     398,   419,   419,   419,   398,   398,   419,   370,   398,   398,
     419,   398,   404,   405,   419,   195,   397,   191,   395,   182,
     422,   205,   422,   355,   422,   210,   422,   365,   422,   334,
     422,   155,   160,   276,   395,   272,   395,   266,   395,   253,
     395,   248,   395,   258,   395,   260,   395,   263,   395,   309,
     395,   282,   397,   298,   395,   278,   395,   402,   402,   419,
     419,   419,   419,   117,   402,   402,   402,   402,   402,   402,
     419,   419,   396,   376,   107,   379,   419,   368,   104,   106,
     410,   406,   422,   104,   106,   196,   104,   104,   106,   184,
     104,   106,   206,   104,   106,   357,   104,   106,   211,   104,
     106,   225,   104,   106,   336,   104,   106,   158,   104,   106,
     280,   104,   106,   274,   104,   106,   268,   104,   106,   255,
     104,   106,   250,   104,   104,   104,   104,   106,   311,   104,
     106,   284,   104,   106,   302,   280,   306,   306,   416,   409,
     103,   121,   122,   125,   126,    83,   173,   105,   400,   144,
     382,   374,   378,   380,   396,   107,   366,   419,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   105,   192,
     249,   252,   254,   257,   261,   264,   267,   277,   279,   283,
     356,   105,   105,   396,   103,   419,   131,    82,   128,   130,
     144,   131,   128,   142,   420,   105,   402,   105,   108,   381,
     382,   396,   411,    24,    32,   197,   198,   215,   217,   231,
     233,   193,   105,   207,   207,   212,   207,   337,   159,   281,
     275,   269,   256,   251,   259,   262,   265,   312,   285,   303,
     402,   402,   402,   402,   419,   407,   419,     8,    43,    44,
     132,   136,   145,   420,   145,   402,    88,    93,   116,   172,
     174,     5,   421,   375,    54,   105,   403,   412,   414,   415,
     427,   420,   420,   105,   190,   199,   200,   202,   204,   209,
     224,   227,   229,   402,   232,   105,   151,   153,   176,   194,
     245,   247,   105,   151,   153,   241,   243,   105,   105,   151,
     153,   214,   241,   243,   105,   105,   151,   153,   105,   151,
     153,   105,   151,   153,   176,   183,   335,   339,   342,   356,
     105,   151,   153,   176,   183,   252,   335,   105,   151,   153,
     176,   183,   247,   254,   257,   261,   264,   267,   270,   271,
     273,   277,   279,   335,   339,   342,   105,   151,   153,   176,
     183,   192,   249,   252,   299,   310,   335,   339,   345,   356,
     105,   151,   153,   176,   192,   249,   252,   299,   310,   105,
     151,   153,   176,   183,   194,   245,   247,   335,   339,   342,
     356,   105,   151,   153,   176,   183,   194,   245,   247,   335,
     339,   347,   350,   353,   356,   105,   151,   153,   176,   183,
     192,   249,   252,   299,   310,   335,   339,   347,   350,   353,
     356,   359,   362,   105,   151,   153,   176,   183,   192,   249,
     252,   356,    21,    68,   105,   151,   153,   176,   183,   288,
     293,   335,   105,   151,   153,   176,   183,   192,   249,   305,
     308,   417,     8,   118,   420,   420,   402,   402,   147,   149,
     151,   153,   154,   156,   127,   422,   154,   419,   419,   398,
     383,   396,   419,   407,   407,   234,   395,   218,   422,   402,
     194,   402,   402,   402,   216,   233,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     420,   420,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   419,   419,   133,   395,   135,   395,   162,   163,   157,
     398,   162,   109,   421,   104,   106,   413,   413,   104,   106,
     235,   104,   106,   219,   217,   116,   291,   292,   369,   116,
     286,   287,   369,   104,   104,   106,   137,   105,    21,    35,
      36,    38,    41,    72,   164,   166,   179,   186,   192,   249,
     252,   296,   301,   310,   314,   402,   105,   110,   419,   402,
     402,   402,   402,   104,   419,   104,   106,   289,   419,   402,
     402,   419,   420,   420,   420,   420,   420,   420,   402,   419,
     421,   416,   236,   220,   402,   402,   134,   138,   116,   294,
     295,   366,   367,   165,   395,   116,   313,   316,   367,   178,
     395,   185,   395,   300,   395,    85,   384,   389,   105,   105,
     151,   153,   176,   183,   238,   105,   151,   153,   176,   183,
     222,   297,   290,   105,   140,   144,   151,   153,   105,   140,
     151,   153,   104,   368,   419,   104,   106,   167,   104,   419,
     104,   106,   180,   104,   106,   187,   302,   421,   421,   402,
     402,   105,   151,   153,   176,   183,   252,   273,   299,   310,
     335,   105,   151,   153,   183,   247,   339,   342,   345,   347,
     350,   356,   402,   402,   402,   402,   402,   111,   107,   402,
     402,   402,   402,   402,   402,   402,   402,   297,   168,   315,
     181,   188,   421,   421,   105,   105,   151,   153,   170,   176,
      37,   105,   151,   153,   105,   151,   153,   176,   183,   105,
     151,   153,   176,   183,   190,   112,    14,   385,   386,   402,
     420,   402,   402,   421,   387,    84,    86,    89,    91,   317,
     318,   319,   321,   322,   323,   326,   327,   330,   331,   107,
     386,   396,   402,   419,   419,   419,   419,   421,   388,   104,
     106,   320,   104,   106,   324,   104,   106,   328,   104,   106,
     332,   421,   402,   402,   402,   402,   107,   105,   325,   329,
     333,   421,   105,   245,   247,   339,   342,   347,   350,   356,
     359,   105,   245,   247,   356,   359,   105,   194,   245,   247,
     339,   342,   347,   350,   396,   402,   402,   402
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   113,   114,   114,   114,   115,   116,   117,   118,   117,
     119,   120,   121,   122,   122,   122,   122,   123,   124,   125,
     126,   126,   126,   127,   128,   129,   130,   131,   131,   131,
     132,   133,   134,   134,   134,   134,   134,   135,   136,   137,
     137,   138,   138,   138,   138,   139,   140,   141,   142,   143,
     144,   145,   145,   145,   145,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   156,   157,   158,
     158,   159,   159,   159,   161,   160,   160,   162,   163,   163,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     165,   166,   167,   167,   168,   168,   168,   168,   168,   169,
     170,   171,   171,   172,   173,   173,   174,   174,   174,   175,
     176,   177,   177,   177,   177,   178,   179,   180,   180,   181,
     181,   181,   181,   181,   182,   183,   184,   184,   185,   186,
     187,   187,   188,   188,   188,   188,   188,   188,   189,   190,
     191,   192,   193,   193,   193,   193,   193,   193,   193,   194,
     195,   196,   196,   197,   197,   197,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   199,   200,   201,   202,   203,
     203,   204,   205,   206,   206,   207,   207,   207,   207,   207,
     208,   209,   210,   211,   211,   212,   212,   212,   212,   212,
     212,   213,   214,   215,   216,   216,   217,   218,   219,   219,
     220,   220,   220,   220,   220,   220,   221,   222,   223,   223,
     224,   225,   225,   226,   227,   228,   229,   230,   230,   230,
     231,   232,   232,   233,   234,   235,   235,   236,   236,   236,
     236,   236,   236,   237,   238,   239,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   250,   251,
     251,   251,   251,   251,   251,   251,   251,   251,   252,   252,
     252,   252,   252,   252,   252,   252,   253,   254,   255,   255,
     256,   256,   256,   256,   256,   256,   256,   256,   256,   256,
     256,   256,   256,   256,   257,   258,   259,   259,   259,   259,
     259,   259,   259,   259,   259,   259,   259,   259,   260,   261,
     262,   262,   262,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   263,   264,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   266,   267,   268,   268,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   270,   270,
     271,   271,   271,   271,   271,   271,   271,   272,   273,   274,
     274,   275,   275,   275,   275,   275,   275,   275,   276,   277,
     278,   279,   280,   280,   281,   281,   281,   281,   281,   281,
     281,   281,   281,   282,   283,   284,   284,   285,   285,   285,
     285,   285,   285,   285,   285,   286,   286,   287,   288,   289,
     289,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   291,   291,   292,   293,   294,   294,   295,   296,
     297,   297,   297,   297,   297,   297,   297,   297,   297,   297,
     298,   299,   300,   301,   302,   302,   303,   303,   303,   303,
     303,   303,   303,   303,   303,   304,   305,   306,   306,   306,
     306,   306,   307,   308,   309,   310,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   313,   314,   315,
     315,   315,   315,   316,   316,   317,   317,   318,   319,   320,
     320,   321,   321,   321,   322,   323,   324,   324,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   326,   327,   328,
     328,   329,   329,   329,   329,   329,   330,   331,   332,   332,
     333,   333,   333,   333,   333,   333,   333,   333,   334,   335,
     336,   336,   337,   337,   337,   338,   339,   340,   340,   340,
     341,   342,   343,   343,   343,   344,   345,   346,   347,   348,
     348,   349,   350,   351,   351,   351,   352,   353,   354,   354,
     354,   355,   356,   357,   357,   358,   359,   360,   360,   361,
     362,   364,   363,   363,   365,   366,   367,   368,   368,   370,
     369,   372,   371,   373,   371,   371,   374,   375,   376,   376,
     377,   378,   379,   379,   380,   381,   381,   382,   382,   383,
     384,   385,   386,   387,   387,   388,   388,   389,   390,   391,
     391,   392,   392,   393,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   397,   398,   398,   399,   400,   401,   402,
     402,   402,   403,   404,   405,   406,   406,   407,   407,   408,
     408,   408,   409,   409,   410,   410,   411,   411,   412,   412,
     412,   413,   413,   414,   415,   416,   416,   417,   417,   418,
     418,   419,   419,   420,   421,   421,   423,   422,   422,   424,
     424,   424,   424,   424,   424,   424,   425,   425,   425,   426,
     426,   426,   426,   426,   426,   426,   426,   426,   426,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   439,   439,   439,   439,   439,   440,   440,   440,   440,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     0,     0,     6,
       1,    13,     1,     0,     2,     2,     2,     1,    13,     1,
       0,     2,     3,     1,     4,     1,     4,     0,     3,     3,
       7,     1,     0,     2,     2,     2,     2,     1,     4,     1,
       4,     0,     2,     2,     2,     1,     4,     1,     7,     1,
       4,     0,     2,     2,     2,     2,     1,     4,     1,     4,
       1,     4,     1,     4,     1,     1,     0,     3,     4,     1,
       4,     0,     2,     2,     0,     3,     1,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     4,     0,     3,     2,     2,     2,     1,
       4,     1,     4,     1,     0,     4,     2,     2,     1,     1,
       4,     2,     2,     2,     1,     1,     4,     1,     4,     0,
       3,     2,     2,     2,     1,     4,     1,     3,     1,     4,
       1,     4,     0,     2,     3,     2,     2,     2,     1,     4,
       1,     7,     0,     3,     2,     2,     2,     2,     2,     4,
       1,     1,     4,     1,     1,     1,     0,     2,     2,     2,
       3,     3,     2,     3,     3,     2,     0,     1,     4,     2,
       1,     4,     1,     1,     4,     0,     2,     2,     2,     2,
       1,     4,     1,     1,     4,     0,     2,     2,     2,     2,
       2,     1,     4,     3,     0,     3,     4,     1,     1,     4,
       0,     3,     2,     2,     2,     2,     1,     4,     2,     1,
       4,     1,     4,     1,     4,     1,     4,     2,     2,     1,
       2,     0,     2,     5,     1,     1,     4,     0,     3,     2,
       2,     2,     2,     1,     4,     2,     1,     1,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     0,
       2,     2,     2,     3,     3,     3,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     4,
       0,     3,     3,     3,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     7,     1,     0,     3,     3,     3,
       2,     3,     2,     2,     2,     2,     2,     2,     1,     7,
       0,     3,     3,     3,     2,     2,     3,     2,     2,     2,
       2,     2,     2,     2,     1,     7,     0,     3,     3,     3,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     1,     4,     1,     4,     0,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       4,     0,     3,     3,     2,     2,     2,     3,     1,     4,
       1,     4,     1,     4,     0,     3,     3,     3,     2,     2,
       2,     2,     2,     1,     4,     1,     4,     0,     3,     3,
       2,     2,     2,     3,     3,     2,     1,     1,     4,     1,
       4,     0,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     7,     2,     1,     1,     7,
       0,     3,     3,     2,     2,     2,     3,     3,     3,     3,
       1,     4,     1,     4,     1,     4,     0,     3,     2,     2,
       2,     3,     3,     3,     3,     2,     5,     0,     3,     3,
       3,     3,     2,     5,     1,     4,     1,     4,     0,     3,
       3,     2,     2,     2,     3,     3,     3,     1,     7,     0,
       2,     2,     5,     2,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     1,     3,     1,     4,     0,     2,
       3,     2,     2,     2,     2,     2,     2,     1,     3,     1,
       4,     0,     2,     3,     2,     2,     1,     3,     1,     4,
       0,     3,     2,     2,     2,     2,     2,     2,     1,     4,
       1,     4,     0,     2,     2,     1,     4,     2,     2,     1,
       1,     4,     2,     2,     1,     1,     4,     1,     4,     2,
       1,     1,     4,     2,     2,     1,     1,     4,     2,     2,
       1,     1,     4,     1,     4,     1,     4,     2,     1,     1,
       4,     0,     3,     1,     1,     2,     2,     0,     2,     0,
       3,     0,     2,     0,     2,     1,     3,     2,     0,     2,
       3,     2,     0,     2,     2,     0,     2,     0,     5,     5,
       5,     4,     4,     0,     2,     0,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     2,     4,     1,     1,     1,     0,
       2,     2,     3,     2,     1,     0,     1,     0,     2,     0,
       2,     3,     0,     5,     1,     4,     0,     3,     1,     3,
       3,     1,     4,     1,     1,     0,     4,     2,     5,     1,
       1,     0,     2,     2,     0,     1,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     0,     0,     0,     0,     0,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     4,     4,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     4,     3,     4,     4,     4,     4,     4
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
      yyerror (&yylloc, scanner, param, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, scanner, param); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct yang_parameter *param)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (param);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct yang_parameter *param)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, param);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void *scanner, struct yang_parameter *param)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, param);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, param); \
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
    default: /* Avoid compiler warnings. */
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner, struct yang_parameter *param)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (param);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 115: /* tmp_string  */

      { free((((*yyvaluep).p_str)) ? *((*yyvaluep).p_str) : NULL); }

        break;

    case 210: /* pattern_arg_str  */

      { free(((*yyvaluep).str)); }

        break;

    case 399: /* semicolom  */

      { free(((*yyvaluep).str)); }

        break;

    case 401: /* curly_bracket_open  */

      { free(((*yyvaluep).str)); }

        break;

    case 405: /* string_opt_part1  */

      { free(((*yyvaluep).str)); }

        break;

    case 430: /* type_ext_alloc  */

      { yang_type_free(param->module->ctx, ((*yyvaluep).v)); }

        break;

    case 431: /* typedef_ext_alloc  */

      { yang_type_free(param->module->ctx, &((struct lys_tpdf *)((*yyvaluep).v))->type); }

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
yyparse (void *scanner, struct yang_parameter *param)
{
/* The lookahead symbol.  */
int yychar;
char *s = NULL, *tmp_s = NULL, *ext_name = NULL;
struct lys_module *trg = NULL;
struct lys_node *tpdf_parent = NULL, *data_node = NULL;
struct lys_ext_instance_complex *ext_instance = NULL;
int is_ext_instance;
void *actual = NULL;
enum yytokentype backup_type, actual_type = MODULE_KEYWORD;
int64_t cnt_val = 0;
int is_value = 0;
void *yang_type = NULL;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */

{ yylloc.last_column = 0;
                  if (param->flags & EXT_INSTANCE_SUBSTMT) {
                    is_ext_instance = 1;
                    ext_instance = (struct lys_ext_instance_complex *)param->actual_node;
                    ext_name = (char *)param->data_node;
                  } else {
                    is_ext_instance = 0;
                  }
                  yylloc.last_line = is_ext_instance;     /* HACK for flex - return SUBMODULE_KEYWORD or SUBMODULE_EXT_KEYWORD */
                  param->value = &s;
                  param->data_node = (void **)&data_node;
                  param->actual_node = &actual;
                  backup_type = NODE;
                  trg = (param->submodule) ? (struct lys_module *)param->submodule : param->module;
                }


  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc, scanner);
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
  *++yylsp = yylloc;
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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:

    { if (yyget_text(scanner)[0] == '"') {
                      char *tmp;

                      s = malloc(yyget_leng(scanner) - 1 + 7 * yylval.i);
                      if (!s) {
                        LOGMEM(trg->ctx);
                        YYABORT;
                      }
                      if (!(tmp = yang_read_string(trg->ctx, yyget_text(scanner) + 1, s, yyget_leng(scanner) - 2, 0, yylloc.first_column))) {
                        YYABORT;
                      }
                      s = tmp;
                    } else {
                      s = calloc(1, yyget_leng(scanner) - 1);
                      if (!s) {
                        LOGMEM(trg->ctx);
                        YYABORT;
                      }
                      memcpy(s, yyget_text(scanner) + 1, yyget_leng(scanner) - 2);
                    }
                    (yyval.p_str) = &s;
                  }

    break;

  case 8:

    { if (yyget_leng(scanner) > 2) {
                int length_s = strlen(s), length_tmp = yyget_leng(scanner);
                char *tmp;

                tmp = realloc(s, length_s + length_tmp - 1);
                if (!tmp) {
                  LOGMEM(trg->ctx);
                  YYABORT;
                }
                s = tmp;
                if (yyget_text(scanner)[0] == '"') {
                  if (!(tmp = yang_read_string(trg->ctx, yyget_text(scanner) + 1, s, length_tmp - 2, length_s, yylloc.first_column))) {
                    YYABORT;
                  }
                  s = tmp;
                } else {
                  memcpy(s + length_s, yyget_text(scanner) + 1, length_tmp - 2);
                  s[length_s + length_tmp - 2] = '\0';
                }
              }
            }

    break;

  case 10:

    { if (param->submodule) {
                                       free(s);
                                       LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "module");
                                       YYABORT;
                                     }
                                     trg = param->module;
                                     yang_read_common(trg,s,MODULE_KEYWORD);
                                     s = NULL;
                                     actual_type = MODULE_KEYWORD;
                                   }

    break;

  case 12:

    { if (!param->module->ns) {
                                            LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "namespace", "module");
                                            YYABORT;
                                          }
                                          if (!param->module->prefix) {
                                            LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "module");
                                            YYABORT;
                                          }
                                        }

    break;

  case 13:

    { (yyval.i) = 0; }

    break;

  case 14:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                              YYABORT;
                                            }
                                            (yyval.i) = 1;
                                            s = NULL;
                                          }

    break;

  case 15:

    { if (yang_read_common(param->module, s, NAMESPACE_KEYWORD)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 16:

    { if (yang_read_prefix(trg, NULL, s)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 17:

    { if (!param->submodule) {
                                          free(s);
                                          LOGVAL(trg->ctx, LYE_SUBMODULE, LY_VLOG_NONE, NULL);
                                          YYABORT;
                                        }
                                        trg = (struct lys_module *)param->submodule;
                                        yang_read_common(trg,s,MODULE_KEYWORD);
                                        s = NULL;
                                        actual_type = SUBMODULE_KEYWORD;
                                      }

    break;

  case 19:

    { if (!param->submodule->prefix) {
                                                  LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                                  YYABORT;
                                                }
                                                if (!(yyvsp[0].i)) {
                                                  /* check version compatibility with the main module */
                                                  if (param->module->version > 1) {
                                                      LOGVAL(trg->ctx, LYE_INVER, LY_VLOG_NONE, NULL);
                                                      YYABORT;
                                                  }
                                                }
                                              }

    break;

  case 20:

    { (yyval.i) = 0; }

    break;

  case 21:

    { if (yang_check_version(param->module, param->submodule, s, (yyvsp[-1].i))) {
                                                 YYABORT;
                                               }
                                               (yyval.i) = 1;
                                               s = NULL;
                                             }

    break;

  case 23:

    { backup_type = actual_type;
                           actual_type = YANG_VERSION_KEYWORD;
                         }

    break;

  case 25:

    { backup_type = actual_type;
                            actual_type = NAMESPACE_KEYWORD;
                          }

    break;

  case 30:

    { actual_type = (yyvsp[-4].token);
                   backup_type = NODE;
                   actual = NULL;
                 }

    break;

  case 31:

    { YANG_ADDELEM(trg->imp, trg->imp_size);
                                     /* HACK for unres */
                                     ((struct lys_import *)actual)->module = (struct lys_module *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = IMPORT_KEYWORD;
                                   }

    break;

  case 32:

    { (yyval.i) = 0; }

    break;

  case 33:

    { if (yang_read_prefix(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 34:

    { if (trg->version != 2) {
                                          LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                          free(s);
                                          YYABORT;
                                        }
                                        if (yang_read_description(trg, actual, s, "import", IMPORT_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.i) = (yyvsp[-1].i);
                                      }

    break;

  case 35:

    { if (trg->version != 2) {
                                        LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                        free(s);
                                        YYABORT;
                                      }
                                      if (yang_read_reference(trg, actual, s, "import", IMPORT_KEYWORD)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.i) = (yyvsp[-1].i);
                                    }

    break;

  case 36:

    { if ((yyvsp[-1].i)) {
                                            LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "import");
                                            free(s);
                                            YYABORT;
                                          }
                                          memcpy(((struct lys_import *)actual)->rev, s, LY_REV_SIZE-1);
                                          free(s);
                                          s = NULL;
                                          (yyval.i) = 1;
                                        }

    break;

  case 37:

    { YANG_ADDELEM(trg->inc, trg->inc_size);
                                     /* HACK for unres */
                                     ((struct lys_include *)actual)->submodule = (struct lys_submodule *)s;
                                     s = NULL;
                                     (yyval.token) = actual_type;
                                     actual_type = INCLUDE_KEYWORD;
                                   }

    break;

  case 38:

    { actual_type = (yyvsp[-1].token);
                                                                backup_type = NODE;
                                                                actual = NULL;
                                                              }

    break;

  case 41:

    { (yyval.i) = 0; }

    break;

  case 42:

    { if (trg->version != 2) {
                                           LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "description");
                                           free(s);
                                           YYABORT;
                                         }
                                         if (yang_read_description(trg, actual, s, "include", INCLUDE_KEYWORD)) {
                                            YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.i) = (yyvsp[-1].i);
                                       }

    break;

  case 43:

    { if (trg->version != 2) {
                                         LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "reference");
                                         free(s);
                                         YYABORT;
                                       }
                                       if (yang_read_reference(trg, actual, s, "include", INCLUDE_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.i) = (yyvsp[-1].i);
                                     }

    break;

  case 44:

    { if ((yyvsp[-1].i)) {
                                             LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "revision-date", "include");
                                             free(s);
                                             YYABORT;
                                           }
                                           memcpy(((struct lys_include *)actual)->rev, s, LY_REV_SIZE-1);
                                           free(s);
                                           s = NULL;
                                           (yyval.i) = 1;
                                         }

    break;

  case 45:

    { backup_type = actual_type;
                                  actual_type = REVISION_DATE_KEYWORD;
                                }

    break;

  case 47:

    { (yyval.token) = actual_type;
                                         if (is_ext_instance) {
                                           if (yang_read_extcomplex_str(trg, ext_instance, "belongs-to", ext_name, s,
                                                                        0, LY_STMT_BELONGSTO)) {
                                             YYABORT;
                                           }
                                         } else {
                                           if (param->submodule->prefix) {
                                             LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "belongs-to", "submodule");
                                             free(s);
                                             YYABORT;
                                           }
                                           if (!ly_strequal(s, param->submodule->belongsto->name, 0)) {
                                             LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "belongs-to");
                                             free(s);
                                             YYABORT;
                                           }
                                           free(s);
                                         }
                                         s = NULL;
                                         actual_type = BELONGS_TO_KEYWORD;
                                       }

    break;

  case 48:

    { if (is_ext_instance) {
                         if (yang_read_extcomplex_str(trg, ext_instance, "prefix", "belongs-to", s,
                                                      LY_STMT_BELONGSTO, LY_STMT_PREFIX)) {
                           YYABORT;
                         }
                       } else {
                         if (yang_read_prefix(trg, NULL, s)) {
                           YYABORT;
                         }
                       }
                       s = NULL;
                       actual_type = (yyvsp[-4].token);
                     }

    break;

  case 49:

    { backup_type = actual_type;
                             actual_type = PREFIX_KEYWORD;
                           }

    break;

  case 52:

    { if (yang_read_common(trg, s, ORGANIZATION_KEYWORD)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 53:

    { if (yang_read_common(trg, s, CONTACT_KEYWORD)) {
                                 YYABORT;
                               }
                               s = NULL;
                             }

    break;

  case 54:

    { if (yang_read_description(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 55:

    { if (yang_read_reference(trg, NULL, s, NULL, MODULE_KEYWORD)) {
                                   YYABORT;
                                 }
                                 s=NULL;
                               }

    break;

  case 56:

    { backup_type = actual_type;
                           actual_type = ORGANIZATION_KEYWORD;
                         }

    break;

  case 58:

    { backup_type = actual_type;
                      actual_type = CONTACT_KEYWORD;
                    }

    break;

  case 60:

    { backup_type = actual_type;
                          actual_type = DESCRIPTION_KEYWORD;
                        }

    break;

  case 62:

    { backup_type = actual_type;
                        actual_type = REFERENCE_KEYWORD;
                      }

    break;

  case 64:

    { if (trg->rev_size) {
                                      struct lys_revision *tmp;

                                      tmp = realloc(trg->rev, trg->rev_size * sizeof *trg->rev);
                                      if (!tmp) {
                                        LOGMEM(trg->ctx);
                                        YYABORT;
                                      }
                                      trg->rev = tmp;
                                    }
                                  }

    break;

  case 65:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  if (!is_ext_instance) {
                                    YANG_ADDELEM(trg->rev, trg->rev_size);
                                  }
                                  memcpy(((struct lys_revision *)actual)->date, s, LY_REV_SIZE);
                                  free(s);
                                  s = NULL;
                                  actual_type = REVISION_KEYWORD;
                                }

    break;

  case 67:

    { int i;

                                                /* check uniqueness of the revision date - not required by RFC */
                                                for (i = 0; i < (trg->rev_size - 1); i++) {
                                                  if (!strcmp(trg->rev[i].date, trg->rev[trg->rev_size - 1].date)) {
                                                    LOGWRN(trg->ctx, "Module's revisions are not unique (%s).",
                                                           trg->rev[trg->rev_size - 1].date);
                                                    break;
                                                  }
                                                }
                                              }

    break;

  case 68:

    { actual_type = (yyvsp[-1].backup_token).token;
                                                                     actual = (yyvsp[-1].backup_token).actual;
                                                                   }

    break;

  case 72:

    { if (yang_read_description(trg, actual, s, "revision",REVISION_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 73:

    { if (yang_read_reference(trg, actual, s, "revision", REVISION_KEYWORD)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 74:

    { s = strdup(yyget_text(scanner));
                              if (!s) {
                                LOGMEM(trg->ctx);
                                YYABORT;
                              }
                              if (lyp_check_date(trg->ctx, s)) {
                                  free(s);
                                  YYABORT;
                              }
                            }

    break;

  case 76:

    { if (lyp_check_date(trg->ctx, s)) {
                   free(s);
                   YYABORT;
               }
             }

    break;

  case 77:

    { void *tmp;

                             if (trg->tpdf_size) {
                               tmp = realloc(trg->tpdf, trg->tpdf_size * sizeof *trg->tpdf);
                               if (!tmp) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                               trg->tpdf = tmp;
                             }

                             if (trg->features_size) {
                               tmp = realloc(trg->features, trg->features_size * sizeof *trg->features);
                               if (!tmp) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                               trg->features = tmp;
                             }

                             if (trg->ident_size) {
                               tmp = realloc(trg->ident, trg->ident_size * sizeof *trg->ident);
                               if (!tmp) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                               trg->ident = tmp;
                             }

                             if (trg->augment_size) {
                               tmp = realloc(trg->augment, trg->augment_size * sizeof *trg->augment);
                               if (!tmp) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                               trg->augment = tmp;
                             }

                             if (trg->extensions_size) {
                               tmp = realloc(trg->extensions, trg->extensions_size * sizeof *trg->extensions);
                               if (!tmp) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                               trg->extensions = tmp;
                             }
                           }

    break;

  case 78:

    { /* check the module with respect to the context now */
                         if (!param->submodule) {
                           switch (lyp_ctx_check_module(trg)) {
                           case -1:
                             YYABORT;
                           case 0:
                             break;
                           case 1:
                             /* it's already there */
                             param->flags |= YANG_EXIST_MODULE;
                             YYABORT;
                           }
                         }
                         param->flags &= (~YANG_REMOVE_IMPORT);
                         if (yang_check_imports(trg, param->unres)) {
                           YYABORT;
                         }
                         actual = NULL;
                       }

    break;

  case 79:

    { actual = NULL; }

    break;

  case 90:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        YANG_ADDELEM(trg->extensions, trg->extensions_size);
                                        trg->extensions_size--;
                                        ((struct lys_ext *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                        ((struct lys_ext *)actual)->module = trg;
                                        if (lyp_check_identifier(trg->ctx, ((struct lys_ext *)actual)->name, LY_IDENT_EXTENSION, trg, NULL)) {
                                          trg->extensions_size++;
                                          YYABORT;
                                        }
                                        trg->extensions_size++;
                                        s = NULL;
                                        actual_type = EXTENSION_KEYWORD;
                                      }

    break;

  case 91:

    { struct lys_ext *ext = actual;
                  ext->plugin = ext_get_plugin(ext->name, ext->module->name, ext->module->rev ? ext->module->rev[0].date : NULL);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 96:

    { if (((struct lys_ext *)actual)->flags & LYS_STATUS_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "extension");
                                        YYABORT;
                                      }
                                      ((struct lys_ext *)actual)->flags |= (yyvsp[0].i);
                                    }

    break;

  case 97:

    { if (yang_read_description(trg, actual, s, "extension", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 98:

    { if (yang_read_reference(trg, actual, s, "extension", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 99:

    { (yyval.token) = actual_type;
                                   if (is_ext_instance) {
                                     if (yang_read_extcomplex_str(trg, ext_instance, "argument", ext_name, s,
                                                                  0, LY_STMT_ARGUMENT)) {
                                       YYABORT;
                                     }
                                   } else {
                                     if (((struct lys_ext *)actual)->argument) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "argument", "extension");
                                        free(s);
                                        YYABORT;
                                     }
                                     ((struct lys_ext *)actual)->argument = lydict_insert_zc(param->module->ctx, s);
                                   }
                                   s = NULL;
                                   actual_type = ARGUMENT_KEYWORD;
                                 }

    break;

  case 100:

    { actual_type = (yyvsp[-1].token); }

    break;

  case 103:

    { (yyval.uint) = (yyvsp[0].uint);
                                       backup_type = actual_type;
                                       actual_type = YIN_ELEMENT_KEYWORD;
                                     }

    break;

  case 105:

    { if (is_ext_instance) {
         int c;
         const char ***p;
         uint8_t *val;
         struct lyext_substmt *info;

         c = 0;
         p = lys_ext_complex_get_substmt(LY_STMT_ARGUMENT, ext_instance, &info);
         if (info->cardinality >= LY_STMT_CARD_SOME) {
           /* get the index in the array to add new item */
           for (c = 0; p[0][c + 1]; c++);
           val = (uint8_t *)p[1];
         } else {
           val = (uint8_t *)(p + 1);
         }
         val[c] = ((yyvsp[-1].uint) == LYS_YINELEM) ? 1 : 2;
       } else {
         ((struct lys_ext *)actual)->flags |= (yyvsp[-1].uint);
       }
     }

    break;

  case 106:

    { (yyval.uint) = LYS_YINELEM; }

    break;

  case 107:

    { (yyval.uint) = 0; }

    break;

  case 108:

    { if (!strcmp(s, "true")) {
                 (yyval.uint) = LYS_YINELEM;
               } else if (!strcmp(s, "false")) {
                 (yyval.uint) = 0;
               } else {
                 LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                 free(s);
                 YYABORT;
               }
               free(s);
               s = NULL;
             }

    break;

  case 109:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = STATUS_KEYWORD;
                           }

    break;

  case 110:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 111:

    { (yyval.i) = LYS_STATUS_CURR; }

    break;

  case 112:

    { (yyval.i) = LYS_STATUS_OBSLT; }

    break;

  case 113:

    { (yyval.i) = LYS_STATUS_DEPRC; }

    break;

  case 114:

    { if (!strcmp(s, "current")) {
                 (yyval.i) = LYS_STATUS_CURR;
               } else if (!strcmp(s, "obsolete")) {
                 (yyval.i) = LYS_STATUS_OBSLT;
               } else if (!strcmp(s, "deprecated")) {
                 (yyval.i) = LYS_STATUS_DEPRC;
               } else {
                 LOGVAL(trg->ctx,LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                 free(s);
                 YYABORT;
               }
               free(s);
               s = NULL;
             }

    break;

  case 115:

    { /* check uniqueness of feature's names */
                                      if (lyp_check_identifier(trg->ctx, s, LY_IDENT_FEATURE, trg, NULL)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      YANG_ADDELEM(trg->features, trg->features_size);
                                      ((struct lys_feature *)actual)->name = lydict_insert_zc(trg->ctx, s);
                                      ((struct lys_feature *)actual)->module = trg;
                                      s = NULL;
                                      actual_type = FEATURE_KEYWORD;
                                    }

    break;

  case 116:

    { actual = (yyvsp[-1].backup_token).actual;
                actual_type = (yyvsp[-1].backup_token).token;
              }

    break;

  case 118:

    { struct lys_iffeature *tmp;

          if (((struct lys_feature *)actual)->iffeature_size) {
            tmp = realloc(((struct lys_feature *)actual)->iffeature,
                          ((struct lys_feature *)actual)->iffeature_size * sizeof *tmp);
            if (!tmp) {
              LOGMEM(trg->ctx);
              YYABORT;
            }
            ((struct lys_feature *)actual)->iffeature = tmp;
          }
        }

    break;

  case 121:

    { if (((struct lys_feature *)actual)->flags & LYS_STATUS_MASK) {
                                      LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "feature");
                                      YYABORT;
                                    }
                                    ((struct lys_feature *)actual)->flags |= (yyvsp[0].i);
                                  }

    break;

  case 122:

    { if (yang_read_description(trg, actual, s, "feature", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 123:

    { if (yang_read_reference(trg, actual, s, "feature", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 124:

    { (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         switch (actual_type) {
                         case FEATURE_KEYWORD:
                           YANG_ADDELEM(((struct lys_feature *)actual)->iffeature,
                                        ((struct lys_feature *)actual)->iffeature_size);
                           break;
                         case IDENTITY_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "identity");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_ident *)actual)->iffeature,
                                        ((struct lys_ident *)actual)->iffeature_size);
                           break;
                         case ENUM_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_type_enum *)actual)->iffeature,
                                        ((struct lys_type_enum *)actual)->iffeature_size);
                           break;
                         case BIT_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature", "bit");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_type_bit *)actual)->iffeature,
                                        ((struct lys_type_bit *)actual)->iffeature_size);
                           break;
                         case REFINE_KEYWORD:
                           if (trg->version < 2) {
                             LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "if-feature");
                             free(s);
                             YYABORT;
                           }
                           YANG_ADDELEM(((struct lys_refine *)actual)->iffeature,
                                        ((struct lys_refine *)actual)->iffeature_size);
                           break;
                         case EXTENSION_INSTANCE:
                           /* nothing change */
                           break;
                         default:
                           /* lys_node_* */
                           YANG_ADDELEM(((struct lys_node *)actual)->iffeature,
                                        ((struct lys_node *)actual)->iffeature_size);
                           break;
                         }
                         ((struct lys_iffeature *)actual)->features = (struct lys_feature **)s;
                         s = NULL;
                         actual_type = IF_FEATURE_KEYWORD;
                       }

    break;

  case 125:

    { actual = (yyvsp[-1].backup_token).actual;
                   actual_type = (yyvsp[-1].backup_token).token;
                 }

    break;

  case 128:

    { const char *tmp;

                                       tmp = lydict_insert_zc(trg->ctx, s);
                                       s = NULL;
                                       if (dup_identities_check(tmp, trg)) {
                                         lydict_remove(trg->ctx, tmp);
                                         YYABORT;
                                       }
                                       (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       YANG_ADDELEM(trg->ident, trg->ident_size);
                                       ((struct lys_ident *)actual)->name = tmp;
                                       ((struct lys_ident *)actual)->module = trg;
                                       actual_type = IDENTITY_KEYWORD;
                                     }

    break;

  case 129:

    { actual = (yyvsp[-1].backup_token).actual;
                 actual_type = (yyvsp[-1].backup_token).token;
               }

    break;

  case 131:

    { void *tmp;

           if (((struct lys_ident *)actual)->base_size) {
             tmp = realloc(((struct lys_ident *)actual)->base,
                           ((struct lys_ident *)actual)->base_size * sizeof *((struct lys_ident *)actual)->base);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             ((struct lys_ident *)actual)->base = tmp;
           }

           if (((struct lys_ident *)actual)->iffeature_size) {
             tmp = realloc(((struct lys_ident *)actual)->iffeature,
                           ((struct lys_ident *)actual)->iffeature_size * sizeof *((struct lys_ident *)actual)->iffeature);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             ((struct lys_ident *)actual)->iffeature = tmp;
           }
         }

    break;

  case 133:

    { void *identity;

                                   if ((trg->version < 2) && ((struct lys_ident *)actual)->base_size) {
                                     free(s);
                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "base", "identity");
                                     YYABORT;
                                   }
                                   identity = actual;
                                   YANG_ADDELEM(((struct lys_ident *)actual)->base,
                                                ((struct lys_ident *)actual)->base_size);
                                   *((struct lys_ident **)actual) = (struct lys_ident *)s;
                                   s = NULL;
                                   actual = identity;
                                 }

    break;

  case 135:

    { if (((struct lys_ident *)actual)->flags & LYS_STATUS_MASK) {
                                       LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "identity");
                                       YYABORT;
                                     }
                                     ((struct lys_ident *)actual)->flags |= (yyvsp[0].i);
                                   }

    break;

  case 136:

    { if (yang_read_description(trg, actual, s, "identity", NODE)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 137:

    { if (yang_read_reference(trg, actual, s, "identity", NODE)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 138:

    { backup_type = actual_type;
                                   actual_type = BASE_KEYWORD;
                                 }

    break;

  case 140:

    { tpdf_parent = (actual_type == EXTENSION_INSTANCE) ? ext_instance : actual;
                                      (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      if (lyp_check_identifier(trg->ctx, s, LY_IDENT_TYPE, trg, tpdf_parent)) {
                                        free(s);
                                        YYABORT;
                                      }
                                      switch (actual_type) {
                                      case MODULE_KEYWORD:
                                      case SUBMODULE_KEYWORD:
                                        YANG_ADDELEM(trg->tpdf, trg->tpdf_size);
                                        break;
                                      case GROUPING_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_grp *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_grp *)tpdf_parent)->tpdf_size);
                                        break;
                                      case CONTAINER_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_container *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_container *)tpdf_parent)->tpdf_size);
                                        break;
                                      case LIST_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_list *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_list *)tpdf_parent)->tpdf_size);
                                        break;
                                      case RPC_KEYWORD:
                                      case ACTION_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_rpc_action *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_rpc_action *)tpdf_parent)->tpdf_size);
                                        break;
                                      case INPUT_KEYWORD:
                                      case OUTPUT_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_inout *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_inout *)tpdf_parent)->tpdf_size);
                                        break;
                                      case NOTIFICATION_KEYWORD:
                                        YANG_ADDELEM(((struct lys_node_notif *)tpdf_parent)->tpdf,
                                                     ((struct lys_node_notif *)tpdf_parent)->tpdf_size);
                                        break;
                                      case EXTENSION_INSTANCE:
                                        /* typedef is already allocated */
                                        break;
                                      default:
                                        /* another type of nodetype is error*/
                                        LOGINT(trg->ctx);
                                        free(s);
                                        YYABORT;
                                      }
                                      ((struct lys_tpdf *)actual)->name = lydict_insert_zc(param->module->ctx, s);
                                      ((struct lys_tpdf *)actual)->module = trg;
                                      s = NULL;
                                      actual_type = TYPEDEF_KEYWORD;
                                    }

    break;

  case 141:

    { if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                      LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "typedef");
                      YYABORT;
                    }
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 142:

    { (yyval.nodes).node.ptr_tpdf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 143:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 144:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 145:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, TYPEDEF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 146:

    { if ((yyvsp[-1].nodes).node.ptr_tpdf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "typedef");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_tpdf->flags |= (yyvsp[0].i);
                               }

    break;

  case 147:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 148:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_tpdf, s, "typedef", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 149:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 150:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_type(trg->ctx, actual, s, actual_type))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       actual_type = TYPE_KEYWORD;
                                     }

    break;

  case 153:

    { if (((struct yang_type *)actual)->base == LY_TYPE_STRING &&
                                         ((struct yang_type *)actual)->type->info.str.pat_count) {
                                       void *tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.str.patterns,
                                                     ((struct yang_type *)actual)->type->info.str.pat_count * sizeof *((struct yang_type *)actual)->type->info.str.patterns);
                                       if (!tmp) {
                                         LOGMEM(trg->ctx);
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.str.patterns = tmp;

#ifdef LY_ENABLED_CACHE
                                       if (!(trg->ctx->models.flags & LY_CTX_TRUSTED) && ((struct yang_type *)actual)->type->info.str.patterns_pcre) {
                                         tmp = realloc(((struct yang_type *)actual)->type->info.str.patterns_pcre,
                                                       2 * ((struct yang_type *)actual)->type->info.str.pat_count * sizeof *((struct yang_type *)actual)->type->info.str.patterns_pcre);
                                         if (!tmp) {
                                           LOGMEM(trg->ctx);
                                           YYABORT;
                                         }
                                         ((struct yang_type *)actual)->type->info.str.patterns_pcre = tmp;
                                       }
#endif
                                     }
                                     if (((struct yang_type *)actual)->base == LY_TYPE_UNION) {
                                       struct lys_type *tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.uni.types,
                                                     ((struct yang_type *)actual)->type->info.uni.count * sizeof *tmp);
                                       if (!tmp) {
                                         LOGMEM(trg->ctx);
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.uni.types = tmp;
                                     }
                                     if (((struct yang_type *)actual)->base == LY_TYPE_IDENT) {
                                       struct lys_ident **tmp;

                                       tmp = realloc(((struct yang_type *)actual)->type->info.ident.ref,
                                                     ((struct yang_type *)actual)->type->info.ident.count* sizeof *tmp);
                                       if (!tmp) {
                                         LOGMEM(trg->ctx);
                                         YYABORT;
                                       }
                                       ((struct yang_type *)actual)->type->info.ident.ref = tmp;
                                     }
                                   }

    break;

  case 157:

    { if (yang_read_require_instance(trg->ctx, actual, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 158:

    { /* leafref_specification */
                                   if (yang_read_leafref_path(trg, actual, s)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 159:

    { /* identityref_specification */
                                   if (((struct yang_type *)actual)->base && ((struct yang_type *)actual)->base != LY_TYPE_IDENT) {
                                     LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
                                     return EXIT_FAILURE;
                                   }
                                   ((struct yang_type *)actual)->base = LY_TYPE_IDENT;
                                   yang_type = actual;
                                   YANG_ADDELEM(((struct yang_type *)actual)->type->info.ident.ref,
                                                ((struct yang_type *)actual)->type->info.ident.count);
                                   *((struct lys_ident **)actual) = (struct lys_ident *)s;
                                   actual = yang_type;
                                   s = NULL;
                                 }

    break;

  case 162:

    { if (yang_read_fraction(trg->ctx, actual, (yyvsp[0].uint))) {
                                                YYABORT;
                                              }
                                            }

    break;

  case 165:

    { actual_type = (yyvsp[-1].backup_token).token;
                                   actual = (yyvsp[-1].backup_token).actual;
                                 }

    break;

  case 166:

    { struct yang_type *stype = (struct yang_type *)actual;

                         (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         if (stype->base != 0 && stype->base != LY_TYPE_UNION) {
                           LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected type statement.");
                           YYABORT;
                         }
                         stype->base = LY_TYPE_UNION;
                         if (strcmp(stype->name, "union")) {
                           /* type can be a substatement only in "union" type, not in derived types */
                           LOGVAL(trg->ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "type", "derived type");
                           YYABORT;
                         }
                         YANG_ADDELEM(stype->type->info.uni.types, stype->type->info.uni.count)
                         actual_type = UNION_KEYWORD;
                       }

    break;

  case 167:

    { (yyval.uint) = (yyvsp[0].uint);
                                               backup_type = actual_type;
                                               actual_type = FRACTION_DIGITS_KEYWORD;
                                             }

    break;

  case 168:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 169:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 170:

    { char *endptr = NULL;
               unsigned long val;
               errno = 0;

               val = strtoul(s, &endptr, 10);
               if (*endptr || s[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                 LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "fraction-digits");
                 free(s);
                 s = NULL;
                 YYABORT;
               }
               (yyval.uint) = (uint32_t) val;
               free(s);
               s =NULL;
             }

    break;

  case 171:

    { actual = (yyvsp[-1].backup_token).actual;
               actual_type = (yyvsp[-1].backup_token).token;
             }

    break;

  case 172:

    { (yyval.backup_token).token = actual_type;
                         (yyval.backup_token).actual = actual;
                         if (!(actual = yang_read_length(trg->ctx, actual, s, is_ext_instance))) {
                           YYABORT;
                         }
                         actual_type = LENGTH_KEYWORD;
                         s = NULL;
                       }

    break;

  case 175:

    { switch (actual_type) {
                               case MUST_KEYWORD:
                                 (yyval.str) = "must";
                                 break;
                               case LENGTH_KEYWORD:
                                 (yyval.str) = "length";
                                 break;
                               case RANGE_KEYWORD:
                                 (yyval.str) = "range";
                                 break;
                               default:
                                 LOGINT(trg->ctx);
                                 YYABORT;
                                 break;
                               }
                             }

    break;

  case 176:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 177:

    { if (yang_read_message(trg, actual, s, (yyvsp[-1].str), ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 178:

    { if (yang_read_description(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 179:

    { if (yang_read_reference(trg, actual, s, (yyvsp[-1].str), NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 180:

    { (yyval.backup_token).token = actual_type;
                   (yyval.backup_token).actual = actual;
                 }

    break;

  case 181:

    {struct lys_restr *pattern = actual;
                                                                        actual = NULL;
#ifdef LY_ENABLED_CACHE
                                                                        if ((yyvsp[-2].backup_token).token != EXTENSION_INSTANCE &&
                                                                            !(data_node && data_node->nodetype != LYS_GROUPING && lys_ingrouping(data_node))) {
                                                                          unsigned int c = 2 * (((struct yang_type *)(yyvsp[-2].backup_token).actual)->type->info.str.pat_count - 1);
                                                                          YANG_ADDELEM(((struct yang_type *)(yyvsp[-2].backup_token).actual)->type->info.str.patterns_pcre, c);
                                                                        }
#endif
                                                                        if (yang_read_pattern(trg->ctx, pattern, actual, (yyvsp[-1].str), (yyvsp[0].ch))) {
                                                                          YYABORT;
                                                                        }
                                                                        actual_type = (yyvsp[-2].backup_token).token;
                                                                        actual = (yyvsp[-2].backup_token).actual;
                                                                      }

    break;

  case 182:

    { if (actual_type != EXTENSION_INSTANCE) {
                            if (((struct yang_type *)actual)->base != 0 && ((struct yang_type *)actual)->base != LY_TYPE_STRING) {
                              free(s);
                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected pattern statement.");
                              YYABORT;
                            }
                            ((struct yang_type *)actual)->base = LY_TYPE_STRING;
                            YANG_ADDELEM(((struct yang_type *)actual)->type->info.str.patterns,
                                         ((struct yang_type *)actual)->type->info.str.pat_count);
                          }
                          (yyval.str) = s;
                          s = NULL;
                          actual_type = PATTERN_KEYWORD;
                        }

    break;

  case 183:

    { (yyval.ch) = 0x06; }

    break;

  case 184:

    { (yyval.ch) = (yyvsp[-1].ch); }

    break;

  case 185:

    { (yyval.ch) = 0x06; /* ACK */ }

    break;

  case 186:

    { if (trg->version < 2) {
                                        LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "modifier");
                                        YYABORT;
                                      }
                                      if ((yyvsp[-1].ch) != 0x06) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "modifier", "pattern");
                                        YYABORT;
                                      }
                                      (yyval.ch) = (yyvsp[0].ch);
                                    }

    break;

  case 187:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_MESSAGE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 188:

    { if (yang_read_message(trg, actual, s, "pattern", ERROR_APP_TAG_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 189:

    { if (yang_read_description(trg, actual, s, "pattern", NODE)) {
                                           YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 190:

    { if (yang_read_reference(trg, actual, s, "pattern", NODE)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 191:

    { backup_type = actual_type;
                       actual_type = MODIFIER_KEYWORD;
                     }

    break;

  case 192:

    { if (!strcmp(s, "invert-match")) {
                                                             (yyval.ch) = 0x15;
                                                             free(s);
                                                             s = NULL;
                                                           } else {
                                                             LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, s);
                                                             free(s);
                                                             YYABORT;
                                                           }
                                                         }

    break;

  case 193:

    { struct lys_type_enum * tmp;

                                                   cnt_val = 0;
                                                   tmp = realloc(((struct yang_type *)actual)->type->info.enums.enm,
                                                                 ((struct yang_type *)actual)->type->info.enums.count * sizeof *tmp);
                                                   if (!tmp) {
                                                     LOGMEM(trg->ctx);
                                                     YYABORT;
                                                   }
                                                   ((struct yang_type *)actual)->type->info.enums.enm = tmp;
                                                 }

    break;

  case 196:

    { if (yang_check_enum(trg->ctx, yang_type, actual, &cnt_val, is_value)) {
               YYABORT;
             }
             actual = (yyvsp[-1].backup_token).actual;
             actual_type = (yyvsp[-1].backup_token).token;
           }

    break;

  case 197:

    { (yyval.backup_token).token = actual_type;
                       (yyval.backup_token).actual = yang_type = actual;
                       YANG_ADDELEM(((struct yang_type *)actual)->type->info.enums.enm, ((struct yang_type *)actual)->type->info.enums.count);
                       if (yang_read_enum(trg->ctx, yang_type, actual, s)) {
                         YYABORT;
                       }
                       s = NULL;
                       is_value = 0;
                       actual_type = ENUM_KEYWORD;
                     }

    break;

  case 199:

    { if (((struct lys_type_enum *)actual)->iffeature_size) {
             struct lys_iffeature *tmp;

             tmp = realloc(((struct lys_type_enum *)actual)->iffeature,
                           ((struct lys_type_enum *)actual)->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             ((struct lys_type_enum *)actual)->iffeature = tmp;
           }
         }

    break;

  case 202:

    { if (is_value) {
                                  LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "value", "enum");
                                  YYABORT;
                                }
                                ((struct lys_type_enum *)actual)->value = (yyvsp[0].i);

                                /* keep the highest enum value for automatic increment */
                                if ((yyvsp[0].i) >= cnt_val) {
                                  cnt_val = (yyvsp[0].i) + 1;
                                }
                                is_value = 1;
                              }

    break;

  case 203:

    { if (((struct lys_type_enum *)actual)->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "enum");
                                   YYABORT;
                                 }
                                 ((struct lys_type_enum *)actual)->flags |= (yyvsp[0].i);
                               }

    break;

  case 204:

    { if (yang_read_description(trg, actual, s, "enum", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 205:

    { if (yang_read_reference(trg, actual, s, "enum", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 206:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = VALUE_KEYWORD;
                                 }

    break;

  case 207:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 208:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 209:

    { /* convert it to int32_t */
                int64_t val;
                char *endptr;

                val = strtoll(s, &endptr, 10);
                if (val < INT32_MIN || val > INT32_MAX || *endptr) {
                    LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "value");
                    free(s);
                    YYABORT;
                }
                free(s);
                s = NULL;
                (yyval.i) = (int32_t) val;
             }

    break;

  case 210:

    { actual_type = (yyvsp[-1].backup_token).token;
                                                        actual = (yyvsp[-1].backup_token).actual;
                                                      }

    break;

  case 213:

    { backup_type = actual_type;
                         actual_type = PATH_KEYWORD;
                       }

    break;

  case 215:

    { (yyval.i) = (yyvsp[0].i);
                                                 backup_type = actual_type;
                                                 actual_type = REQUIRE_INSTANCE_KEYWORD;
                                               }

    break;

  case 216:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 217:

    { (yyval.i) = 1; }

    break;

  case 218:

    { (yyval.i) = -1; }

    break;

  case 219:

    { if (!strcmp(s,"true")) {
                  (yyval.i) = 1;
                } else if (!strcmp(s,"false")) {
                  (yyval.i) = -1;
                } else {
                  LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "require-instance");
                  free(s);
                  YYABORT;
                }
                free(s);
                s = NULL;
              }

    break;

  case 220:

    { struct lys_type_bit * tmp;

                                         cnt_val = 0;
                                         tmp = realloc(((struct yang_type *)actual)->type->info.bits.bit,
                                                       ((struct yang_type *)actual)->type->info.bits.count * sizeof *tmp);
                                         if (!tmp) {
                                           LOGMEM(trg->ctx);
                                           YYABORT;
                                         }
                                         ((struct yang_type *)actual)->type->info.bits.bit = tmp;
                                       }

    break;

  case 223:

    { if (yang_check_bit(trg->ctx, yang_type, actual, &cnt_val, is_value)) {
                      YYABORT;
                    }
                    actual = (yyvsp[-2].backup_token).actual;
                    actual_type = (yyvsp[-2].backup_token).token;
                  }

    break;

  case 224:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = yang_type = actual;
                                  YANG_ADDELEM(((struct yang_type *)actual)->type->info.bits.bit,
                                               ((struct yang_type *)actual)->type->info.bits.count);
                                  if (yang_read_bit(trg->ctx, yang_type, actual, s)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                  is_value = 0;
                                  actual_type = BIT_KEYWORD;
                                }

    break;

  case 226:

    { if (((struct lys_type_bit *)actual)->iffeature_size) {
             struct lys_iffeature *tmp;

             tmp = realloc(((struct lys_type_bit *)actual)->iffeature,
                           ((struct lys_type_bit *)actual)->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             ((struct lys_type_bit *)actual)->iffeature = tmp;
           }
         }

    break;

  case 229:

    { if (is_value) {
                                    LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "position", "bit");
                                    YYABORT;
                                  }
                                  ((struct lys_type_bit *)actual)->pos = (yyvsp[0].uint);

                                  /* keep the highest position value for automatic increment */
                                  if ((yyvsp[0].uint) >= cnt_val) {
                                    cnt_val = (yyvsp[0].uint) + 1;
                                  }
                                  is_value = 1;
                                }

    break;

  case 230:

    { if (((struct lys_type_bit *)actual)->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "status", "bit");
                                   YYABORT;
                                 }
                                 ((struct lys_type_bit *)actual)->flags |= (yyvsp[0].i);
                              }

    break;

  case 231:

    { if (yang_read_description(trg, actual, s, "bit", NODE)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 232:

    { if (yang_read_reference(trg, actual, s, "bit", NODE)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 233:

    { (yyval.uint) = (yyvsp[0].uint);
                                             backup_type = actual_type;
                                             actual_type = POSITION_KEYWORD;
                                           }

    break;

  case 234:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 235:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 236:

    { /* convert it to uint32_t */
                unsigned long val;
                char *endptr = NULL;
                errno = 0;

                val = strtoul(s, &endptr, 10);
                if (s[0] == '-' || *endptr || errno || val > UINT32_MAX) {
                  LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "position");
                  free(s);
                  YYABORT;
                }
                free(s);
                s = NULL;
                (yyval.uint) = (uint32_t) val;
              }

    break;

  case 237:

    { backup_type = actual_type;
                            actual_type = ERROR_MESSAGE_KEYWORD;
                          }

    break;

  case 239:

    { backup_type = actual_type;
                            actual_type = ERROR_APP_TAG_KEYWORD;
                          }

    break;

  case 241:

    { backup_type = actual_type;
                    actual_type = UNITS_KEYWORD;
                  }

    break;

  case 243:

    { backup_type = actual_type;
                      actual_type = DEFAULT_KEYWORD;
                    }

    break;

  case 245:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_GROUPING, sizeof(struct lys_node_grp)))) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       data_node = actual;
                                       actual_type = GROUPING_KEYWORD;
                                     }

    break;

  case 246:

    { LOGDBG(LY_LDGYANG, "finished parsing grouping statement \"%s\"", data_node->name);
                 actual_type = (yyvsp[-1].backup_token).token;
                 actual = (yyvsp[-1].backup_token).actual;
                 data_node = (yyvsp[-1].backup_token).actual;
               }

    break;

  case 249:

    { (yyval.nodes).grouping = actual; }

    break;

  case 250:

    { if ((yyvsp[-1].nodes).grouping->flags & LYS_STATUS_MASK) {
                                       LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).grouping, "status", "grouping");
                                       YYABORT;
                                     }
                                     (yyvsp[-1].nodes).grouping->flags |= (yyvsp[0].i);
                                   }

    break;

  case 251:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                        }

    break;

  case 252:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).grouping, s, "grouping", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 257:

    { if (trg->version < 2) {
                                                     LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).grouping, "notification");
                                                     YYABORT;
                                                   }
                                                 }

    break;

  case 266:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CONTAINER, sizeof(struct lys_node_container)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                        actual_type = CONTAINER_KEYWORD;
                                      }

    break;

  case 267:

    { LOGDBG(LY_LDGYANG, "finished parsing container statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-1].backup_token).token;
                  actual = (yyvsp[-1].backup_token).actual;
                  data_node = (yyvsp[-1].backup_token).actual;
                }

    break;

  case 269:

    { void *tmp;

            if ((yyvsp[-1].nodes).container->iffeature_size) {
              tmp = realloc((yyvsp[-1].nodes).container->iffeature, (yyvsp[-1].nodes).container->iffeature_size * sizeof *(yyvsp[-1].nodes).container->iffeature);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].nodes).container->iffeature = tmp;
            }

            if ((yyvsp[-1].nodes).container->must_size) {
              tmp = realloc((yyvsp[-1].nodes).container->must, (yyvsp[-1].nodes).container->must_size * sizeof *(yyvsp[-1].nodes).container->must);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].nodes).container->must = tmp;
            }
          }

    break;

  case 270:

    { (yyval.nodes).container = actual; }

    break;

  case 274:

    { if (yang_read_presence(trg, (yyvsp[-1].nodes).container, s)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 275:

    { if ((yyvsp[-1].nodes).container->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "config", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 276:

    { if ((yyvsp[-1].nodes).container->flags & LYS_STATUS_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).container, "status", "container");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).container->flags |= (yyvsp[0].i);
                                    }

    break;

  case 277:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 278:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).container, s, "container", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 281:

    { if (trg->version < 2) {
                                                      LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).container, "notification");
                                                      YYABORT;
                                                    }
                                                  }

    break;

  case 284:

    { void *tmp;

                  if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                    LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "type", "leaf");
                    YYABORT;
                  }
                  if ((yyvsp[-1].nodes).node.ptr_leaf->dflt && ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_TRUE)) {
                    /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                    LOGVAL(trg->ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                    LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                    YYABORT;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_leaf->iffeature_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_leaf->iffeature, (yyvsp[-1].nodes).node.ptr_leaf->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaf->iffeature);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_leaf->iffeature = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_leaf->must_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_leaf->must, (yyvsp[-1].nodes).node.ptr_leaf->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaf->must);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_leaf->must = tmp;
                  }

                  LOGDBG(LY_LDGYANG, "finished parsing leaf statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 285:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAF, sizeof(struct lys_node_leaf)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = LEAF_KEYWORD;
                                 }

    break;

  case 286:

    { (yyval.nodes).node.ptr_leaf = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 289:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 290:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                  YYABORT;
                                }
                                s = NULL;
                              }

    break;

  case 292:

    { if (yang_read_default(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, LEAF_KEYWORD)) {
                                    YYABORT;
                                  }
                                  s = NULL;
                                }

    break;

  case 293:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "config", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 294:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_MAND_MASK) {
                                      LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "mandatory", "leaf");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                                  }

    break;

  case 295:

    { if ((yyvsp[-1].nodes).node.ptr_leaf->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaf, "status", "leaf");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_leaf->flags |= (yyvsp[0].i);
                               }

    break;

  case 296:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 297:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaf, s, "leaf", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 298:

    { (yyval.backup_token).token = actual_type;
                                        (yyval.backup_token).actual = actual;
                                        if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LEAFLIST, sizeof(struct lys_node_leaflist)))) {
                                          YYABORT;
                                        }
                                        data_node = actual;
                                        s = NULL;
                                        actual_type = LEAF_LIST_KEYWORD;
                                      }

    break;

  case 299:

    { void *tmp;

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_R) {
                          /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
                           * ignore oredering MASK - 0x7F
                           */
                          (yyvsp[-1].nodes).node.ptr_leaflist->flags &= 0x7F;
                        }
                        if (!((yyvsp[-1].nodes).node.flag & LYS_TYPE_DEF)) {
                          LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "type", "leaf-list");
                          YYABORT;
                        }
                        if ((yyvsp[-1].nodes).node.ptr_leaflist->dflt_size && (yyvsp[-1].nodes).node.ptr_leaflist->min) {
                          LOGVAL(trg->ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                          LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist,
                                 "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                          YYABORT;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->iffeature, (yyvsp[-1].nodes).node.ptr_leaflist->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->iffeature);
                          if (!tmp) {
                            LOGMEM(trg->ctx);
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->iffeature = tmp;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->must_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->must, (yyvsp[-1].nodes).node.ptr_leaflist->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->must);
                          if (!tmp) {
                            LOGMEM(trg->ctx);
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->must = tmp;
                        }

                        if ((yyvsp[-1].nodes).node.ptr_leaflist->dflt_size) {
                          tmp = realloc((yyvsp[-1].nodes).node.ptr_leaflist->dflt, (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size * sizeof *(yyvsp[-1].nodes).node.ptr_leaflist->dflt);
                          if (!tmp) {
                            LOGMEM(trg->ctx);
                            YYABORT;
                          }
                          (yyvsp[-1].nodes).node.ptr_leaflist->dflt = tmp;
                        }

                        LOGDBG(LY_LDGYANG, "finished parsing leaf-list statement \"%s\"", data_node->name);
                        actual_type = (yyvsp[-4].backup_token).token;
                        actual = (yyvsp[-4].backup_token).actual;
                        data_node = (yyvsp[-4].backup_token).actual;
                      }

    break;

  case 300:

    { (yyval.nodes).node.ptr_leaflist = actual;
                                 (yyval.nodes).node.flag = 0;
                               }

    break;

  case 303:

    { (yyvsp[-2].nodes).node.flag |= LYS_TYPE_DEF;
                                            (yyval.nodes) = (yyvsp[-2].nodes);
                                          }

    break;

  case 304:

    { if (trg->version < 2) {
                                         free(s);
                                         LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "default");
                                         YYABORT;
                                       }
                                       YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_leaflist->dflt,
                                                    (yyvsp[-1].nodes).node.ptr_leaflist->dflt_size);
                                       (*(const char **)actual) = lydict_insert_zc(param->module->ctx, s);
                                       s = NULL;
                                       actual = (yyvsp[-1].nodes).node.ptr_leaflist;
                                     }

    break;

  case 305:

    { if (yang_read_units(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, LEAF_LIST_KEYWORD)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 307:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_CONFIG_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "config", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 308:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                              LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "min-elements", "leaf-list");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].nodes).node.ptr_leaflist->min = (yyvsp[0].uint);
                                            (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                            if ((yyvsp[-1].nodes).node.ptr_leaflist->max && ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max)) {
                                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "\"min-elements\" is bigger than \"max-elements\".");
                                              YYABORT;
                                            }
                                          }

    break;

  case 309:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                              LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "max-elements", "leaf-list");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].nodes).node.ptr_leaflist->max = (yyvsp[0].uint);
                                            (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                            if ((yyvsp[-1].nodes).node.ptr_leaflist->min > (yyvsp[-1].nodes).node.ptr_leaflist->max) {
                                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "max-elements");
                                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "\"max-elements\" is smaller than \"min-elements\".");
                                              YYABORT;
                                            }
                                          }

    break;

  case 310:

    { if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                            LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "ordered by", "leaf-list");
                                            YYABORT;
                                          }
                                          if ((yyvsp[0].i) & LYS_USERORDERED) {
                                            (yyvsp[-1].nodes).node.ptr_leaflist->flags |= LYS_USERORDERED;
                                          }
                                          (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }

    break;

  case 311:

    { if ((yyvsp[-1].nodes).node.ptr_leaflist->flags & LYS_STATUS_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_leaflist, "status", "leaf-list");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_leaflist->flags |= (yyvsp[0].i);
                                    }

    break;

  case 312:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                         }

    break;

  case 313:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_leaflist, s, "leaf-list", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 314:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_LIST, sizeof(struct lys_node_list)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = LIST_KEYWORD;
                                 }

    break;

  case 315:

    { void *tmp;

                  if ((yyvsp[-1].nodes).node.ptr_list->iffeature_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->iffeature, (yyvsp[-1].nodes).node.ptr_list->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->iffeature);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->iffeature = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->must_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->must, (yyvsp[-1].nodes).node.ptr_list->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->must);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->must = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->tpdf_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->tpdf, (yyvsp[-1].nodes).node.ptr_list->tpdf_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->tpdf);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->tpdf = tmp;
                  }

                  if ((yyvsp[-1].nodes).node.ptr_list->unique_size) {
                    tmp = realloc((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size * sizeof *(yyvsp[-1].nodes).node.ptr_list->unique);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    (yyvsp[-1].nodes).node.ptr_list->unique = tmp;
                  }

                  LOGDBG(LY_LDGYANG, "finished parsing list statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 316:

    { (yyval.nodes).node.ptr_list = actual;
                            (yyval.nodes).node.flag = 0;
                          }

    break;

  case 320:

    { if ((yyvsp[-1].nodes).node.ptr_list->keys) {
                                  LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "key", "list");
                                  free(s);
                                  YYABORT;
                              }
                              (yyvsp[-1].nodes).node.ptr_list->keys = (struct lys_node_leaf **)s;
                              (yyval.nodes) = (yyvsp[-1].nodes);
                              s = NULL;
                            }

    break;

  case 321:

    { YANG_ADDELEM((yyvsp[-1].nodes).node.ptr_list->unique, (yyvsp[-1].nodes).node.ptr_list->unique_size);
                                 ((struct lys_unique *)actual)->expr = (const char **)s;
                                 (yyval.nodes) = (yyvsp[-1].nodes);
                                 s = NULL;
                                 actual = (yyvsp[-1].nodes).node.ptr_list;
                               }

    break;

  case 322:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_CONFIG_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "config", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 323:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MIN_ELEMENTS) {
                                         LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "min-elements", "list");
                                         YYABORT;
                                       }
                                       (yyvsp[-1].nodes).node.ptr_list->min = (yyvsp[0].uint);
                                       (yyvsp[-1].nodes).node.flag |= LYS_MIN_ELEMENTS;
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                       if ((yyvsp[-1].nodes).node.ptr_list->max && ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max)) {
                                         LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                         LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "\"min-elements\" is bigger than \"max-elements\".");
                                         YYABORT;
                                       }
                                     }

    break;

  case 324:

    { if ((yyvsp[-1].nodes).node.flag & LYS_MAX_ELEMENTS) {
                                         LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "max-elements", "list");
                                         YYABORT;
                                       }
                                       (yyvsp[-1].nodes).node.ptr_list->max = (yyvsp[0].uint);
                                       (yyvsp[-1].nodes).node.flag |= LYS_MAX_ELEMENTS;
                                       (yyval.nodes) = (yyvsp[-1].nodes);
                                       if ((yyvsp[-1].nodes).node.ptr_list->min > (yyvsp[-1].nodes).node.ptr_list->max) {
                                         LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "min-elements");
                                         LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "\"max-elements\" is smaller than \"min-elements\".");
                                         YYABORT;
                                       }
                                     }

    break;

  case 325:

    { if ((yyvsp[-1].nodes).node.flag & LYS_ORDERED_MASK) {
                                       LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "ordered by", "list");
                                       YYABORT;
                                     }
                                     if ((yyvsp[0].i) & LYS_USERORDERED) {
                                       (yyvsp[-1].nodes).node.ptr_list->flags |= LYS_USERORDERED;
                                     }
                                     (yyvsp[-1].nodes).node.flag |= (yyvsp[0].i);
                                     (yyval.nodes) = (yyvsp[-1].nodes);
                                   }

    break;

  case 326:

    { if ((yyvsp[-1].nodes).node.ptr_list->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_list, "status", "list");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).node.ptr_list->flags |= (yyvsp[0].i);
                               }

    break;

  case 327:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 328:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_list, s, "list", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 332:

    { if (trg->version < 2) {
                                                 LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_list, "notification");
                                                 YYABORT;
                                               }
                                             }

    break;

  case 334:

    { (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CHOICE, sizeof(struct lys_node_choice)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = CHOICE_KEYWORD;
                                   }

    break;

  case 335:

    { LOGDBG(LY_LDGYANG, "finished parsing choice statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 337:

    { struct lys_iffeature *tmp;

           if (((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_TRUE) && (yyvsp[-1].nodes).node.ptr_choice->dflt) {
              LOGVAL(trg->ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "default", "choice");
              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "The \"default\" statement is forbidden on choices with \"mandatory\".");
              YYABORT;
            }

           if ((yyvsp[-1].nodes).node.ptr_choice->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_choice->iffeature, (yyvsp[-1].nodes).node.ptr_choice->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_choice->iffeature = tmp;
           }
         }

    break;

  case 338:

    { (yyval.nodes).node.ptr_choice = actual;
                              (yyval.nodes).node.flag = 0;
                            }

    break;

  case 341:

    { if ((yyvsp[-1].nodes).node.flag & LYS_CHOICE_DEFAULT) {
                                      LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "default", "choice");
                                      free(s);
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->dflt = (struct lys_node *) s;
                                    s = NULL;
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                    (yyval.nodes).node.flag |= LYS_CHOICE_DEFAULT;
                                  }

    break;

  case 342:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "config", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 343:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_MAND_MASK) {
                                      LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "mandatory", "choice");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                    (yyval.nodes) = (yyvsp[-1].nodes);
                                  }

    break;

  case 344:

    { if ((yyvsp[-1].nodes).node.ptr_choice->flags & LYS_STATUS_MASK) {
                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_choice, "status", "choice");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_choice->flags |= (yyvsp[0].i);
                                   (yyval.nodes) = (yyvsp[-1].nodes);
                                 }

    break;

  case 345:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                        (yyval.nodes) = (yyvsp[-1].nodes);
                                      }

    break;

  case 346:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_choice, s, "choice", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                      (yyval.nodes) = (yyvsp[-1].nodes);
                                    }

    break;

  case 356:

    { if (trg->version < 2 ) {
                     LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, actual, "choice");
                     YYABORT;
                   }
                 }

    break;

  case 357:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_CASE, sizeof(struct lys_node_case)))) {
                                     YYABORT;
                                   }
                                   data_node = actual;
                                   s = NULL;
                                   actual_type = CASE_KEYWORD;
                                 }

    break;

  case 358:

    { LOGDBG(LY_LDGYANG, "finished parsing case statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 360:

    { struct lys_iffeature *tmp;

           if ((yyvsp[-1].nodes).cs->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).cs->iffeature, (yyvsp[-1].nodes).cs->iffeature_size * sizeof *tmp);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).cs->iffeature = tmp;
           }
          }

    break;

  case 361:

    { (yyval.nodes).cs = actual; }

    break;

  case 364:

    { if ((yyvsp[-1].nodes).cs->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).cs, "status", "case");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).cs->flags |= (yyvsp[0].i);
                               }

    break;

  case 365:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 366:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).cs, s, "case", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 368:

    { (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYXML, sizeof(struct lys_node_anydata)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ANYXML_KEYWORD;
                                   }

    break;

  case 369:

    { LOGDBG(LY_LDGYANG, "finished parsing anyxml statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 370:

    { (yyval.backup_token).token = actual_type;
                                      (yyval.backup_token).actual = actual;
                                      if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ANYDATA, sizeof(struct lys_node_anydata)))) {
                                        YYABORT;
                                      }
                                      data_node = actual;
                                      s = NULL;
                                      actual_type = ANYDATA_KEYWORD;
                                    }

    break;

  case 371:

    { LOGDBG(LY_LDGYANG, "finished parsing anydata statement \"%s\"", data_node->name);
                actual_type = (yyvsp[-1].backup_token).token;
                actual = (yyvsp[-1].backup_token).actual;
                data_node = (yyvsp[-1].backup_token).actual;
              }

    break;

  case 373:

    { void *tmp;

           if ((yyvsp[-1].nodes).node.ptr_anydata->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_anydata->iffeature, (yyvsp[-1].nodes).node.ptr_anydata->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_anydata->iffeature);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_anydata->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).node.ptr_anydata->must_size) {
             tmp = realloc((yyvsp[-1].nodes).node.ptr_anydata->must, (yyvsp[-1].nodes).node.ptr_anydata->must_size * sizeof *(yyvsp[-1].nodes).node.ptr_anydata->must);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).node.ptr_anydata->must = tmp;
           }
         }

    break;

  case 374:

    { (yyval.nodes).node.ptr_anydata = actual;
                              (yyval.nodes).node.flag = actual_type;
                            }

    break;

  case 378:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_CONFIG_MASK) {
                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "config",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 379:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_MAND_MASK) {
                                        LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "mandatory",
                                               ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                        YYABORT;
                                      }
                                      (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                    }

    break;

  case 380:

    { if ((yyvsp[-1].nodes).node.ptr_anydata->flags & LYS_STATUS_MASK) {
                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_anydata, "status",
                                            ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata");
                                     YYABORT;
                                   }
                                   (yyvsp[-1].nodes).node.ptr_anydata->flags |= (yyvsp[0].i);
                                 }

    break;

  case 381:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                          YYABORT;
                                        }
                                        s = NULL;
                                      }

    break;

  case 382:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_anydata, s, ((yyvsp[-1].nodes).node.flag == ANYXML_KEYWORD) ? "anyxml" : "anydata", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 383:

    { (yyval.backup_token).token = actual_type;
                                       (yyval.backup_token).actual = actual;
                                       if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_USES, sizeof(struct lys_node_uses)))) {
                                         YYABORT;
                                       }
                                       data_node = actual;
                                       s = NULL;
                                       actual_type = USES_KEYWORD;
                                     }

    break;

  case 384:

    { LOGDBG(LY_LDGYANG, "finished parsing uses statement \"%s\"", data_node->name);
             actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
             data_node = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 386:

    { void *tmp;

           if ((yyvsp[-1].nodes).uses->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->iffeature, (yyvsp[-1].nodes).uses->iffeature_size * sizeof *(yyvsp[-1].nodes).uses->iffeature);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).uses->refine_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->refine, (yyvsp[-1].nodes).uses->refine_size * sizeof *(yyvsp[-1].nodes).uses->refine);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->refine = tmp;
           }

           if ((yyvsp[-1].nodes).uses->augment_size) {
             tmp = realloc((yyvsp[-1].nodes).uses->augment, (yyvsp[-1].nodes).uses->augment_size * sizeof *(yyvsp[-1].nodes).uses->augment);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).uses->augment = tmp;
           }
         }

    break;

  case 387:

    { (yyval.nodes).uses = actual; }

    break;

  case 390:

    { if ((yyvsp[-1].nodes).uses->flags & LYS_STATUS_MASK) {
                                   LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).uses, "status", "uses");
                                   YYABORT;
                                 }
                                 (yyvsp[-1].nodes).uses->flags |= (yyvsp[0].i);
                               }

    break;

  case 391:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 392:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).uses, s, "uses", NODE_PRINT)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 397:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  YANG_ADDELEM(((struct lys_node_uses *)actual)->refine,
                                               ((struct lys_node_uses *)actual)->refine_size);
                                  ((struct lys_refine *)actual)->target_name = transform_schema2json(trg, s);
                                  free(s);
                                  s = NULL;
                                  if (!((struct lys_refine *)actual)->target_name) {
                                    YYABORT;
                                  }
                                  actual_type = REFINE_KEYWORD;
                                }

    break;

  case 398:

    { actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 400:

    { void *tmp;

           if ((yyvsp[-1].nodes).refine->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->iffeature, (yyvsp[-1].nodes).refine->iffeature_size * sizeof *(yyvsp[-1].nodes).refine->iffeature);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).refine->must_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->must, (yyvsp[-1].nodes).refine->must_size * sizeof *(yyvsp[-1].nodes).refine->must);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->must = tmp;
           }

           if ((yyvsp[-1].nodes).refine->dflt_size) {
             tmp = realloc((yyvsp[-1].nodes).refine->dflt, (yyvsp[-1].nodes).refine->dflt_size * sizeof *(yyvsp[-1].nodes).refine->dflt);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).refine->dflt = tmp;
           }
         }

    break;

  case 401:

    { (yyval.nodes).refine = actual;
                                    actual_type = REFINE_KEYWORD;
                                  }

    break;

  case 402:

    { actual = (yyvsp[-2].nodes).refine;
                                               actual_type = REFINE_KEYWORD;
                                               if ((yyvsp[-2].nodes).refine->target_type) {
                                                 if ((yyvsp[-2].nodes).refine->target_type & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML)) {
                                                   (yyvsp[-2].nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML);
                                                 } else {
                                                   LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "must", "refine");
                                                   LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                   YYABORT;
                                                 }
                                               } else {
                                                 (yyvsp[-2].nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYXML;
                                               }
                                             }

    break;

  case 403:

    { /* leaf, leaf-list, list, container or anyxml */
               /* check possibility of statements combination */
               if ((yyvsp[-2].nodes).refine->target_type) {
                 if ((yyvsp[-2].nodes).refine->target_type & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA)) {
                   (yyvsp[-2].nodes).refine->target_type &= (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA);
                 } else {
                   free(s);
                   LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "if-feature", "refine");
                   LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                   YYABORT;
                 }
               } else {
                 (yyvsp[-2].nodes).refine->target_type = LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER | LYS_ANYDATA;
               }
             }

    break;

  case 404:

    { if ((yyvsp[-1].nodes).refine->target_type) {
                                             if ((yyvsp[-1].nodes).refine->target_type & LYS_CONTAINER) {
                                               if ((yyvsp[-1].nodes).refine->mod.presence) {
                                                 LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "presence", "refine");
                                                 free(s);
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                               (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                             } else {
                                               free(s);
                                               LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "presence", "refine");
                                               LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                               YYABORT;
                                             }
                                           } else {
                                             (yyvsp[-1].nodes).refine->target_type = LYS_CONTAINER;
                                             (yyvsp[-1].nodes).refine->mod.presence = lydict_insert_zc(trg->ctx, s);
                                           }
                                           s = NULL;
                                           (yyval.nodes) = (yyvsp[-1].nodes);
                                         }

    break;

  case 405:

    { int i;

                                          if ((yyvsp[-1].nodes).refine->dflt_size) {
                                            if (trg->version < 2) {
                                              LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "default", "refine");
                                              YYABORT;
                                            }
                                            if ((yyvsp[-1].nodes).refine->target_type & LYS_LEAFLIST) {
                                              (yyvsp[-1].nodes).refine->target_type = LYS_LEAFLIST;
                                            } else {
                                              free(s);
                                              LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                              LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                              YYABORT;
                                            }
                                          } else {
                                            if ((yyvsp[-1].nodes).refine->target_type) {
                                              if (trg->version < 2 && ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE))) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE);
                                              } if (trg->version > 1 && ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE))) {
                                                /* YANG 1.1 */
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE);
                                              } else {
                                                free(s);
                                                LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "default", "refine");
                                                LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYABORT;
                                              }
                                            } else {
                                              if (trg->version < 2) {
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE;
                                              } else {
                                                /* YANG 1.1 */
                                                (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE;
                                              }
                                            }
                                          }
                                          /* check for duplicity */
                                          for (i = 0; i < (yyvsp[-1].nodes).refine->dflt_size; ++i) {
                                              if (ly_strequal((yyvsp[-1].nodes).refine->dflt[i], s, 0)) {
                                                  LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "default");
                                                  LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", s);
                                                  YYABORT;
                                              }
                                          }
                                          YANG_ADDELEM((yyvsp[-1].nodes).refine->dflt, (yyvsp[-1].nodes).refine->dflt_size);
                                          *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                          actual = (yyvsp[-1].nodes).refine;
                                          s = NULL;
                                          (yyval.nodes) = (yyvsp[-1].nodes);
                                        }

    break;

  case 406:

    { if ((yyvsp[-1].nodes).refine->target_type) {
                                           if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST)) {
                                             (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST);
                                             if ((yyvsp[-1].nodes).refine->flags & LYS_CONFIG_MASK) {
                                               LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "refine");
                                               YYABORT;
                                             }
                                             (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                           } else {
                                             LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "config", "refine");
                                             LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                             YYABORT;
                                           }
                                         } else {
                                           (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_LIST | LYS_CONTAINER | LYS_LEAFLIST;
                                           (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                         }
                                         (yyval.nodes) = (yyvsp[-1].nodes);
                                       }

    break;

  case 407:

    { if ((yyvsp[-1].nodes).refine->target_type) {
                                              if ((yyvsp[-1].nodes).refine->target_type & (LYS_LEAF | LYS_CHOICE | LYS_ANYXML)) {
                                                (yyvsp[-1].nodes).refine->target_type &= (LYS_LEAF | LYS_CHOICE | LYS_ANYXML);
                                                if ((yyvsp[-1].nodes).refine->flags & LYS_MAND_MASK) {
                                                  LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "refine");
                                                  YYABORT;
                                                }
                                                (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                              } else {
                                                LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "refine");
                                                LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                YYABORT;
                                              }
                                            } else {
                                              (yyvsp[-1].nodes).refine->target_type = LYS_LEAF | LYS_CHOICE | LYS_ANYXML;
                                              (yyvsp[-1].nodes).refine->flags |= (yyvsp[0].i);
                                            }
                                            (yyval.nodes) = (yyvsp[-1].nodes);
                                          }

    break;

  case 408:

    { if ((yyvsp[-1].nodes).refine->target_type) {
                                                 if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                   (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                   if ((yyvsp[-1].nodes).refine->flags & LYS_RFN_MINSET) {
                                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                     YYABORT;
                                                   }
                                                   (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MINSET;
                                                   (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                                 } else {
                                                   LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "min-elements", "refine");
                                                   LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                   YYABORT;
                                                 }
                                               } else {
                                                 (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                 (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MINSET;
                                                 (yyvsp[-1].nodes).refine->mod.list.min = (yyvsp[0].uint);
                                               }
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }

    break;

  case 409:

    { if ((yyvsp[-1].nodes).refine->target_type) {
                                                 if ((yyvsp[-1].nodes).refine->target_type & (LYS_LIST | LYS_LEAFLIST)) {
                                                   (yyvsp[-1].nodes).refine->target_type &= (LYS_LIST | LYS_LEAFLIST);
                                                   if ((yyvsp[-1].nodes).refine->flags & LYS_RFN_MAXSET) {
                                                     LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                     YYABORT;
                                                   }
                                                   (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MAXSET;
                                                   (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                                 } else {
                                                   LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "max-elements", "refine");
                                                   LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid refine target nodetype for the substatements.");
                                                   YYABORT;
                                                 }
                                               } else {
                                                 (yyvsp[-1].nodes).refine->target_type = LYS_LIST | LYS_LEAFLIST;
                                                 (yyvsp[-1].nodes).refine->flags |= LYS_RFN_MAXSET;
                                                 (yyvsp[-1].nodes).refine->mod.list.max = (yyvsp[0].uint);
                                               }
                                               (yyval.nodes) = (yyvsp[-1].nodes);
                                             }

    break;

  case 410:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 411:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).refine, s, "refine", NODE)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 414:

    { void *parent;

                                         (yyval.backup_token).token = actual_type;
                                         (yyval.backup_token).actual = actual;
                                         parent = actual;
                                         YANG_ADDELEM(((struct lys_node_uses *)actual)->augment,
                                                      ((struct lys_node_uses *)actual)->augment_size);
                                         if (yang_read_augment(trg, parent, actual, s)) {
                                           YYABORT;
                                         }
                                         data_node = actual;
                                         s = NULL;
                                         actual_type = AUGMENT_KEYWORD;
                                       }

    break;

  case 415:

    { LOGDBG(LY_LDGYANG, "finished parsing augment statement \"%s\"", data_node->name);
                         actual_type = (yyvsp[-4].backup_token).token;
                         actual = (yyvsp[-4].backup_token).actual;
                         data_node = (yyvsp[-4].backup_token).actual;
                       }

    break;

  case 418:

    { (yyval.backup_token).token = actual_type;
                               (yyval.backup_token).actual = actual;
                               YANG_ADDELEM(trg->augment, trg->augment_size);
                               if (yang_read_augment(trg, NULL, actual, s)) {
                                 YYABORT;
                               }
                               data_node = actual;
                               s = NULL;
                               actual_type = AUGMENT_KEYWORD;
                             }

    break;

  case 419:

    { LOGDBG(LY_LDGYANG, "finished parsing augment statement \"%s\"", data_node->name);
                    actual_type = (yyvsp[-4].backup_token).token;
                    actual = (yyvsp[-4].backup_token).actual;
                    data_node = (yyvsp[-4].backup_token).actual;
                  }

    break;

  case 420:

    { (yyval.nodes).augment = actual; }

    break;

  case 423:

    { if ((yyvsp[-1].nodes).augment->flags & LYS_STATUS_MASK) {
                                      LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).augment, "status", "augment");
                                      YYABORT;
                                    }
                                    (yyvsp[-1].nodes).augment->flags |= (yyvsp[0].i);
                                  }

    break;

  case 424:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                       }

    break;

  case 425:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).augment, s, "augment", NODE_PRINT)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                     }

    break;

  case 428:

    { if (trg->version < 2) {
                                                    LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, (yyvsp[-2].nodes).augment, "notification");
                                                    YYABORT;
                                                  }
                                                }

    break;

  case 430:

    { if (param->module->version != 2) {
                                       LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, actual, "action");
                                       free(s);
                                       YYABORT;
                                     }
                                     (yyval.backup_token).token = actual_type;
                                     (yyval.backup_token).actual = actual;
                                     if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_ACTION, sizeof(struct lys_node_rpc_action)))) {
                                       YYABORT;
                                     }
                                     data_node = actual;
                                     s = NULL;
                                     actual_type = ACTION_KEYWORD;
                                   }

    break;

  case 431:

    { LOGDBG(LY_LDGYANG, "finished parsing action statement \"%s\"", data_node->name);
               actual_type = (yyvsp[-1].backup_token).token;
               actual = (yyvsp[-1].backup_token).actual;
               data_node = (yyvsp[-1].backup_token).actual;
             }

    break;

  case 432:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  if (!(actual = yang_read_node(trg, NULL, param->node, s, LYS_RPC, sizeof(struct lys_node_rpc_action)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  actual_type = RPC_KEYWORD;
                                }

    break;

  case 433:

    { LOGDBG(LY_LDGYANG, "finished parsing rpc statement \"%s\"", data_node->name);
            actual_type = (yyvsp[-1].backup_token).token;
            actual = (yyvsp[-1].backup_token).actual;
            data_node = (yyvsp[-1].backup_token).actual;
          }

    break;

  case 435:

    { void *tmp;

            if ((yyvsp[-1].nodes).node.ptr_rpc->iffeature_size) {
              tmp = realloc((yyvsp[-1].nodes).node.ptr_rpc->iffeature, (yyvsp[-1].nodes).node.ptr_rpc->iffeature_size * sizeof *(yyvsp[-1].nodes).node.ptr_rpc->iffeature);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].nodes).node.ptr_rpc->iffeature = tmp;
            }

            if ((yyvsp[-1].nodes).node.ptr_rpc->tpdf_size) {
              tmp = realloc((yyvsp[-1].nodes).node.ptr_rpc->tpdf, (yyvsp[-1].nodes).node.ptr_rpc->tpdf_size * sizeof *(yyvsp[-1].nodes).node.ptr_rpc->tpdf);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].nodes).node.ptr_rpc->tpdf = tmp;
            }
          }

    break;

  case 436:

    { (yyval.nodes).node.ptr_rpc = actual;
                           (yyval.nodes).node.flag = 0;
                         }

    break;

  case 438:

    { if ((yyvsp[-1].nodes).node.ptr_rpc->flags & LYS_STATUS_MASK) {
                                  LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).node.ptr_rpc, "status", "rpc");
                                  YYABORT;
                                }
                                (yyvsp[-1].nodes).node.ptr_rpc->flags |= (yyvsp[0].i);
                             }

    break;

  case 439:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                       YYABORT;
                                     }
                                     s = NULL;
                                   }

    break;

  case 440:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).node.ptr_rpc, s, "rpc", NODE_PRINT)) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                 }

    break;

  case 443:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_INPUT) {
                                         LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "input", "rpc");
                                         YYABORT;
                                       }
                                       (yyvsp[-2].nodes).node.flag |= LYS_RPC_INPUT;
                                       (yyval.nodes) = (yyvsp[-2].nodes);
                                     }

    break;

  case 444:

    { if ((yyvsp[-2].nodes).node.flag & LYS_RPC_OUTPUT) {
                                          LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-2].nodes).node.ptr_rpc, "output", "rpc");
                                          YYABORT;
                                        }
                                        (yyvsp[-2].nodes).node.flag |= LYS_RPC_OUTPUT;
                                        (yyval.nodes) = (yyvsp[-2].nodes);
                                      }

    break;

  case 445:

    { (yyval.backup_token).token = actual_type;
                                  (yyval.backup_token).actual = actual;
                                  s = strdup("input");
                                  if (!s) {
                                    LOGMEM(trg->ctx);
                                    YYABORT;
                                  }
                                  if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_INPUT, sizeof(struct lys_node_inout)))) {
                                    YYABORT;
                                  }
                                  data_node = actual;
                                  s = NULL;
                                  actual_type = INPUT_KEYWORD;
                                }

    break;

  case 446:

    { void *tmp;
                  struct lys_node_inout *input = actual;

                  if (input->must_size) {
                    tmp = realloc(input->must, input->must_size * sizeof *input->must);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    input->must = tmp;
                  }

                  if (input->tpdf_size) {
                    tmp = realloc(input->tpdf, input->tpdf_size * sizeof *input->tpdf);
                    if (!tmp) {
                      LOGMEM(trg->ctx);
                      YYABORT;
                    }
                    input->tpdf = tmp;
                  }

                  LOGDBG(LY_LDGYANG, "finished parsing input statement \"%s\"", data_node->name);
                  actual_type = (yyvsp[-4].backup_token).token;
                  actual = (yyvsp[-4].backup_token).actual;
                  data_node = (yyvsp[-4].backup_token).actual;
                }

    break;

  case 452:

    { (yyval.backup_token).token = actual_type;
                                    (yyval.backup_token).actual = actual;
                                    s = strdup("output");
                                    if (!s) {
                                      LOGMEM(trg->ctx);
                                      YYABORT;
                                    }
                                    if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_OUTPUT, sizeof(struct lys_node_inout)))) {
                                      YYABORT;
                                    }
                                    data_node = actual;
                                    s = NULL;
                                    actual_type = OUTPUT_KEYWORD;
                                  }

    break;

  case 453:

    { void *tmp;
                   struct lys_node_inout *output = actual;

                   if (output->must_size) {
                     tmp = realloc(output->must, output->must_size * sizeof *output->must);
                     if (!tmp) {
                       LOGMEM(trg->ctx);
                       YYABORT;
                     }
                     output->must = tmp;
                   }

                   if (output->tpdf_size) {
                     tmp = realloc(output->tpdf, output->tpdf_size * sizeof *output->tpdf);
                     if (!tmp) {
                       LOGMEM(trg->ctx);
                       YYABORT;
                     }
                     output->tpdf = tmp;
                   }

                   LOGDBG(LY_LDGYANG, "finished parsing output statement \"%s\"", data_node->name);
                   actual_type = (yyvsp[-4].backup_token).token;
                   actual = (yyvsp[-4].backup_token).actual;
                   data_node = (yyvsp[-4].backup_token).actual;
                 }

    break;

  case 454:

    { (yyval.backup_token).token = actual_type;
                                           (yyval.backup_token).actual = actual;
                                           if (!(actual = yang_read_node(trg, actual, param->node, s, LYS_NOTIF, sizeof(struct lys_node_notif)))) {
                                             YYABORT;
                                           }
                                           data_node = actual;
                                           actual_type = NOTIFICATION_KEYWORD;
                                         }

    break;

  case 455:

    { LOGDBG(LY_LDGYANG, "finished parsing notification statement \"%s\"", data_node->name);
                     actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                     data_node = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 457:

    { void *tmp;

            if ((yyvsp[-1].nodes).notif->must_size) {
              tmp = realloc((yyvsp[-1].nodes).notif->must, (yyvsp[-1].nodes).notif->must_size * sizeof *(yyvsp[-1].nodes).notif->must);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].nodes).notif->must = tmp;
            }

           if ((yyvsp[-1].nodes).notif->iffeature_size) {
             tmp = realloc((yyvsp[-1].nodes).notif->iffeature, (yyvsp[-1].nodes).notif->iffeature_size * sizeof *(yyvsp[-1].nodes).notif->iffeature);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).notif->iffeature = tmp;
           }

           if ((yyvsp[-1].nodes).notif->tpdf_size) {
             tmp = realloc((yyvsp[-1].nodes).notif->tpdf, (yyvsp[-1].nodes).notif->tpdf_size * sizeof *(yyvsp[-1].nodes).notif->tpdf);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].nodes).notif->tpdf = tmp;
           }
          }

    break;

  case 458:

    { (yyval.nodes).notif = actual; }

    break;

  case 461:

    { if ((yyvsp[-1].nodes).notif->flags & LYS_STATUS_MASK) {
                                           LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_LYS, (yyvsp[-1].nodes).notif, "status", "notification");
                                           YYABORT;
                                         }
                                         (yyvsp[-1].nodes).notif->flags |= (yyvsp[0].i);
                                       }

    break;

  case 462:

    { if (yang_read_description(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                                YYABORT;
                                              }
                                              s = NULL;
                                            }

    break;

  case 463:

    { if (yang_read_reference(trg, (yyvsp[-1].nodes).notif, s, "notification", NODE_PRINT)) {
                                              YYABORT;
                                            }
                                            s = NULL;
                                          }

    break;

  case 467:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   YANG_ADDELEM(trg->deviation, trg->deviation_size);
                                   ((struct lys_deviation *)actual)->target_name = transform_schema2json(trg, s);
                                   free(s);
                                   if (!((struct lys_deviation *)actual)->target_name) {
                                     YYABORT;
                                   }
                                   s = NULL;
                                   actual_type = DEVIATION_KEYWORD;
                                 }

    break;

  case 468:

    { void *tmp;

                      if ((yyvsp[-1].dev)->deviate_size) {
                        tmp = realloc((yyvsp[-1].dev)->deviate, (yyvsp[-1].dev)->deviate_size * sizeof *(yyvsp[-1].dev)->deviate);
                        if (!tmp) {
                          LOGINT(trg->ctx);
                          YYABORT;
                        }
                        (yyvsp[-1].dev)->deviate = tmp;
                      } else {
                        LOGVAL(trg->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "deviate", "deviation");
                        YYABORT;
                      }
                      actual_type = (yyvsp[-4].backup_token).token;
                      actual = (yyvsp[-4].backup_token).actual;
                    }

    break;

  case 469:

    { (yyval.dev) = actual; }

    break;

  case 470:

    { if (yang_read_description(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.dev) = (yyvsp[-1].dev);
                                         }

    break;

  case 471:

    { if (yang_read_reference(trg, (yyvsp[-1].dev), s, "deviation", NODE)) {
                                           YYABORT;
                                         }
                                         s = NULL;
                                         (yyval.dev) = (yyvsp[-1].dev);
                                       }

    break;

  case 477:

    { (yyval.backup_token).token = actual_type;
                                               (yyval.backup_token).actual = actual;
                                               if (!(actual = yang_read_deviate_unsupported(trg->ctx, actual))) {
                                                 YYABORT;
                                               }
                                               actual_type = NOT_SUPPORTED_KEYWORD;
                                             }

    break;

  case 478:

    { actual_type = (yyvsp[-2].backup_token).token;
                              actual = (yyvsp[-2].backup_token).actual;
                            }

    break;

  case 484:

    { (yyval.backup_token).token = actual_type;
                           (yyval.backup_token).actual = actual;
                           if (!(actual = yang_read_deviate(trg->ctx, actual, LY_DEVIATE_ADD))) {
                             YYABORT;
                           }
                           actual_type = ADD_KEYWORD;
                         }

    break;

  case 485:

    { actual_type = (yyvsp[-2].backup_token).token;
                    actual = (yyvsp[-2].backup_token).actual;
                  }

    break;

  case 487:

    { void *tmp;

           if ((yyvsp[-1].deviate)->must_size) {
             tmp = realloc((yyvsp[-1].deviate)->must, (yyvsp[-1].deviate)->must_size * sizeof *(yyvsp[-1].deviate)->must);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].deviate)->must = tmp;
           }

           if ((yyvsp[-1].deviate)->unique_size) {
             tmp = realloc((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size * sizeof *(yyvsp[-1].deviate)->unique);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].deviate)->unique = tmp;
           }

           if ((yyvsp[-1].deviate)->dflt_size) {
             tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].deviate)->dflt = tmp;
           }
         }

    break;

  case 488:

    { (yyval.deviate) = actual; }

    break;

  case 489:

    { if (yang_read_units(trg, actual, s, ADD_KEYWORD)) {
                                         YYABORT;
                                       }
                                       s = NULL;
                                       (yyval.deviate) = (yyvsp[-1].deviate);
                                     }

    break;

  case 491:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                        ((struct lys_unique *)actual)->expr = (const char **)s;
                                        s = NULL;
                                        actual = (yyvsp[-1].deviate);
                                        (yyval.deviate)= (yyvsp[-1].deviate);
                                      }

    break;

  case 492:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                         *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                         s = NULL;
                                         actual = (yyvsp[-1].deviate);
                                         (yyval.deviate) = (yyvsp[-1].deviate);
                                       }

    break;

  case 493:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                          LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                          YYABORT;
                                        }
                                        (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                        (yyval.deviate) = (yyvsp[-1].deviate);
                                      }

    break;

  case 494:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                             LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                             YYABORT;
                                           }
                                           (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 495:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->min_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 496:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                YYABORT;
                                              }
                                              (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                              (yyvsp[-1].deviate)->max_set = 1;
                                              (yyval.deviate) =  (yyvsp[-1].deviate);
                                            }

    break;

  case 497:

    { (yyval.backup_token).token = actual_type;
                                 (yyval.backup_token).actual = actual;
                                 if (!(actual = yang_read_deviate(trg->ctx, actual, LY_DEVIATE_DEL))) {
                                   YYABORT;
                                 }
                                 actual_type = DELETE_KEYWORD;
                               }

    break;

  case 498:

    { actual_type = (yyvsp[-2].backup_token).token;
                       actual = (yyvsp[-2].backup_token).actual;
                     }

    break;

  case 500:

    { void *tmp;

            if ((yyvsp[-1].deviate)->must_size) {
              tmp = realloc((yyvsp[-1].deviate)->must, (yyvsp[-1].deviate)->must_size * sizeof *(yyvsp[-1].deviate)->must);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].deviate)->must = tmp;
            }

            if ((yyvsp[-1].deviate)->unique_size) {
              tmp = realloc((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size * sizeof *(yyvsp[-1].deviate)->unique);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].deviate)->unique = tmp;
            }

            if ((yyvsp[-1].deviate)->dflt_size) {
              tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
              if (!tmp) {
                LOGMEM(trg->ctx);
                YYABORT;
              }
              (yyvsp[-1].deviate)->dflt = tmp;
            }
          }

    break;

  case 501:

    { (yyval.deviate) = actual; }

    break;

  case 502:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                            YYABORT;
                                          }
                                          s = NULL;
                                          (yyval.deviate) = (yyvsp[-1].deviate);
                                        }

    break;

  case 504:

    { YANG_ADDELEM((yyvsp[-1].deviate)->unique, (yyvsp[-1].deviate)->unique_size);
                                           ((struct lys_unique *)actual)->expr = (const char **)s;
                                           s = NULL;
                                           actual = (yyvsp[-1].deviate);
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 505:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                            *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                            s = NULL;
                                            actual = (yyvsp[-1].deviate);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 506:

    { (yyval.backup_token).token = actual_type;
                                   (yyval.backup_token).actual = actual;
                                   if (!(actual = yang_read_deviate(trg->ctx, actual, LY_DEVIATE_RPL))) {
                                     YYABORT;
                                   }
                                   actual_type = REPLACE_KEYWORD;
                                 }

    break;

  case 507:

    { actual_type = (yyvsp[-2].backup_token).token;
                        actual = (yyvsp[-2].backup_token).actual;
                      }

    break;

  case 509:

    { void *tmp;

           if ((yyvsp[-1].deviate)->dflt_size) {
             tmp = realloc((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size * sizeof *(yyvsp[-1].deviate)->dflt);
             if (!tmp) {
               LOGMEM(trg->ctx);
               YYABORT;
             }
             (yyvsp[-1].deviate)->dflt = tmp;
           }
         }

    break;

  case 510:

    { (yyval.deviate) = actual; }

    break;

  case 512:

    { if (yang_read_units(trg, actual, s, DELETE_KEYWORD)) {
                                             YYABORT;
                                           }
                                           s = NULL;
                                           (yyval.deviate) = (yyvsp[-1].deviate);
                                         }

    break;

  case 513:

    { YANG_ADDELEM((yyvsp[-1].deviate)->dflt, (yyvsp[-1].deviate)->dflt_size);
                                             *((const char **)actual) = lydict_insert_zc(trg->ctx, s);
                                             s = NULL;
                                             actual = (yyvsp[-1].deviate);
                                             (yyval.deviate) = (yyvsp[-1].deviate);
                                           }

    break;

  case 514:

    { if ((yyvsp[-1].deviate)->flags & LYS_CONFIG_MASK) {
                                              LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "config", "deviate");
                                              YYABORT;
                                            }
                                            (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                            (yyval.deviate) = (yyvsp[-1].deviate);
                                          }

    break;

  case 515:

    { if ((yyvsp[-1].deviate)->flags & LYS_MAND_MASK) {
                                                 LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "mandatory", "deviate");
                                                 YYABORT;
                                               }
                                               (yyvsp[-1].deviate)->flags = (yyvsp[0].i);
                                               (yyval.deviate) = (yyvsp[-1].deviate);
                                             }

    break;

  case 516:

    { if ((yyvsp[-1].deviate)->min_set) {
                                                    LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "min-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->min = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->min_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 517:

    { if ((yyvsp[-1].deviate)->max_set) {
                                                    LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "max-elements", "deviation");
                                                    YYABORT;
                                                  }
                                                  (yyvsp[-1].deviate)->max = (yyvsp[0].uint);
                                                  (yyvsp[-1].deviate)->max_set = 1;
                                                  (yyval.deviate) =  (yyvsp[-1].deviate);
                                                }

    break;

  case 518:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_when(trg, actual, actual_type, s))) {
                          YYABORT;
                        }
                        s = NULL;
                        actual_type = WHEN_KEYWORD;
                      }

    break;

  case 519:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 523:

    { if (yang_read_description(trg, actual, s, "when", NODE)) {
                                        YYABORT;
                                      }
                                      s = NULL;
                                    }

    break;

  case 524:

    { if (yang_read_reference(trg, actual, s, "when", NODE)) {
                                      YYABORT;
                                    }
                                    s = NULL;
                                  }

    break;

  case 525:

    { (yyval.i) = (yyvsp[0].i);
                             backup_type = actual_type;
                             actual_type = CONFIG_KEYWORD;
                           }

    break;

  case 526:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 527:

    { (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET; }

    break;

  case 528:

    { (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET; }

    break;

  case 529:

    { if (!strcmp(s, "true")) {
                  (yyval.i) = LYS_CONFIG_W | LYS_CONFIG_SET;
                } else if (!strcmp(s, "false")) {
                  (yyval.i) = LYS_CONFIG_R | LYS_CONFIG_SET;
                } else {
                  LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "config");
                  free(s);
                  YYABORT;
                }
                free(s);
                s = NULL;
              }

    break;

  case 530:

    { (yyval.i) = (yyvsp[0].i);
                                   backup_type = actual_type;
                                   actual_type = MANDATORY_KEYWORD;
                                 }

    break;

  case 531:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 532:

    { (yyval.i) = LYS_MAND_TRUE; }

    break;

  case 533:

    { (yyval.i) = LYS_MAND_FALSE; }

    break;

  case 534:

    { if (!strcmp(s, "true")) {
                  (yyval.i) = LYS_MAND_TRUE;
                } else if (!strcmp(s, "false")) {
                  (yyval.i) = LYS_MAND_FALSE;
                } else {
                  LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "mandatory");
                  free(s);
                  YYABORT;
                }
                free(s);
                s = NULL;
              }

    break;

  case 535:

    { backup_type = actual_type;
                       actual_type = PRESENCE_KEYWORD;
                     }

    break;

  case 537:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MIN_ELEMENTS_KEYWORD;
                                 }

    break;

  case 538:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 539:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 540:

    { if (strlen(s) == 1 && s[0] == '0') {
                  (yyval.uint) = 0;
                } else {
                  /* convert it to uint32_t */
                  uint64_t val;
                  char *endptr = NULL;
                  errno = 0;

                  val = strtoul(s, &endptr, 10);
                  if (*endptr || s[0] == '-' || errno || val > UINT32_MAX) {
                      LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "min-elements");
                      free(s);
                      YYABORT;
                  }
                  (yyval.uint) = (uint32_t) val;
                }
                free(s);
                s = NULL;
              }

    break;

  case 541:

    { (yyval.uint) = (yyvsp[0].uint);
                                   backup_type = actual_type;
                                   actual_type = MAX_ELEMENTS_KEYWORD;
                                 }

    break;

  case 542:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 543:

    { (yyval.uint) = 0; }

    break;

  case 544:

    { (yyval.uint) = (yyvsp[-1].uint); }

    break;

  case 545:

    { if (!strcmp(s, "unbounded")) {
                  (yyval.uint) = 0;
                } else {
                  /* convert it to uint32_t */
                  uint64_t val;
                  char *endptr = NULL;
                  errno = 0;

                  val = strtoul(s, &endptr, 10);
                  if (*endptr || s[0] == '-' || errno || val == 0 || val > UINT32_MAX) {
                      LOGVAL(trg->ctx, LYE_INARG, LY_VLOG_NONE, NULL, s, "max-elements");
                      free(s);
                      YYABORT;
                  }
                  (yyval.uint) = (uint32_t) val;
                }
                free(s);
                s = NULL;
              }

    break;

  case 546:

    { (yyval.i) = (yyvsp[0].i);
                                     backup_type = actual_type;
                                     actual_type = ORDERED_BY_KEYWORD;
                                   }

    break;

  case 547:

    { (yyval.i) = (yyvsp[-1].i); }

    break;

  case 548:

    { (yyval.i) = LYS_USERORDERED; }

    break;

  case 549:

    { (yyval.i) = LYS_SYSTEMORDERED; }

    break;

  case 550:

    { if (!strcmp(s, "user")) {
                  (yyval.i) = LYS_USERORDERED;
                } else if (!strcmp(s, "system")) {
                  (yyval.i) = LYS_SYSTEMORDERED;
                } else {
                  free(s);
                  YYABORT;
                }
                free(s);
                s=NULL;
              }

    break;

  case 551:

    { (yyval.backup_token).token = actual_type;
                       (yyval.backup_token).actual = actual;
                       switch (actual_type) {
                       case CONTAINER_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_container *)actual)->must,
                                     ((struct lys_node_container *)actual)->must_size);
                         break;
                       case ANYDATA_KEYWORD:
                       case ANYXML_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_anydata *)actual)->must,
                                     ((struct lys_node_anydata *)actual)->must_size);
                         break;
                       case LEAF_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_leaf *)actual)->must,
                                     ((struct lys_node_leaf *)actual)->must_size);
                         break;
                       case LEAF_LIST_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_leaflist *)actual)->must,
                                     ((struct lys_node_leaflist *)actual)->must_size);
                         break;
                       case LIST_KEYWORD:
                         YANG_ADDELEM(((struct lys_node_list *)actual)->must,
                                     ((struct lys_node_list *)actual)->must_size);
                         break;
                       case REFINE_KEYWORD:
                         YANG_ADDELEM(((struct lys_refine *)actual)->must,
                                     ((struct lys_refine *)actual)->must_size);
                         break;
                       case ADD_KEYWORD:
                       case DELETE_KEYWORD:
                         YANG_ADDELEM(((struct lys_deviate *)actual)->must,
                                      ((struct lys_deviate *)actual)->must_size);
                         break;
                       case NOTIFICATION_KEYWORD:
                         if (trg->version < 2) {
                           free(s);
                           LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, actual, "must");
                           YYABORT;
                         }
                         YANG_ADDELEM(((struct lys_node_notif *)actual)->must,
                                     ((struct lys_node_notif *)actual)->must_size);
                         break;
                       case INPUT_KEYWORD:
                       case OUTPUT_KEYWORD:
                         if (trg->version < 2) {
                           free(s);
                           LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_LYS, actual, "must");
                           YYABORT;
                         }
                         YANG_ADDELEM(((struct lys_node_inout *)actual)->must,
                                     ((struct lys_node_inout *)actual)->must_size);
                         break;
                       case EXTENSION_INSTANCE:
                         /* must is already allocated */
                         break;
                       default:
                         free(s);
                         LOGINT(trg->ctx);
                         YYABORT;
                       }
                       ((struct lys_restr *)actual)->expr = transform_schema2json(trg, s);
                       free(s);
                       if (!((struct lys_restr *)actual)->expr) {
                         YYABORT;
                       }
                       s = NULL;
                       actual_type = MUST_KEYWORD;
                     }

    break;

  case 552:

    { actual_type = (yyvsp[-1].backup_token).token;
             actual = (yyvsp[-1].backup_token).actual;
           }

    break;

  case 555:

    { backup_type = actual_type;
                             actual_type = UNIQUE_KEYWORD;
                           }

    break;

  case 559:

    { backup_type = actual_type;
                       actual_type = KEY_KEYWORD;
                     }

    break;

  case 561:

    { s = strdup(yyget_text(scanner));
                               if (!s) {
                                 LOGMEM(trg->ctx);
                                 YYABORT;
                               }
                             }

    break;

  case 564:

    { (yyval.backup_token).token = actual_type;
                        (yyval.backup_token).actual = actual;
                        if (!(actual = yang_read_range(trg->ctx, actual, s, is_ext_instance))) {
                          YYABORT;
                        }
                        actual_type = RANGE_KEYWORD;
                        s = NULL;
                      }

    break;

  case 565:

    { if (s) {
                                                s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 2);
                                                if (!s) {
                                                  LOGMEM(trg->ctx);
                                                  YYABORT;
                                                }
                                                strcat(s,"/");
                                                strcat(s, yyget_text(scanner));
                                              } else {
                                                s = malloc(yyget_leng(scanner) + 2);
                                                if (!s) {
                                                  LOGMEM(trg->ctx);
                                                  YYABORT;
                                                }
                                                s[0]='/';
                                                memcpy(s + 1, yyget_text(scanner), yyget_leng(scanner) + 1);
                                              }
                                            }

    break;

  case 569:

    { if (s) {
                                              s = ly_realloc(s,strlen(s) + yyget_leng(scanner) + 1);
                                              if (!s) {
                                                LOGMEM(trg->ctx);
                                                YYABORT;
                                              }
                                              strcat(s, yyget_text(scanner));
                                            } else {
                                              s = strdup(yyget_text(scanner));
                                              if (!s) {
                                                LOGMEM(trg->ctx);
                                                YYABORT;
                                              }
                                            }
                                          }

    break;

  case 571:

    { tmp_s = yyget_text(scanner); }

    break;

  case 572:

    { s = strdup(tmp_s);
                                                                if (!s) {
                                                                  LOGMEM(trg->ctx);
                                                                  YYABORT;
                                                                }
                                                                s[strlen(s) - 1] = '\0';
                                                             }

    break;

  case 573:

    { tmp_s = yyget_text(scanner); }

    break;

  case 574:

    { s = strdup(tmp_s);
                                                      if (!s) {
                                                        LOGMEM(trg->ctx);
                                                        YYABORT;
                                                      }
                                                      s[strlen(s) - 1] = '\0';
                                                    }

    break;

  case 598:

    { /* convert it to uint32_t */
                                                unsigned long val;

                                                val = strtoul(yyget_text(scanner), NULL, 10);
                                                if (val > UINT32_MAX) {
                                                    LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Converted number is very long.");
                                                    YYABORT;
                                                }
                                                (yyval.uint) = (uint32_t) val;
                                             }

    break;

  case 599:

    { (yyval.uint) = 0; }

    break;

  case 600:

    { (yyval.uint) = (yyvsp[0].uint); }

    break;

  case 601:

    { (yyval.i) = 0; }

    break;

  case 602:

    { /* convert it to int32_t */
                             int64_t val;

                             val = strtoll(yyget_text(scanner), NULL, 10);
                             if (val < INT32_MIN || val > INT32_MAX) {
                                 LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL,
                                        "The number is not in the correct range (INT32_MIN..INT32_MAX): \"%d\"",val);
                                 YYABORT;
                             }
                             (yyval.i) = (int32_t) val;
                           }

    break;

  case 608:

    { if (lyp_check_identifier(trg->ctx, s, LY_IDENT_SIMPLE, trg, NULL)) {
                    free(s);
                    YYABORT;
                }
              }

    break;

  case 613:

    { char *tmp;

               if ((tmp = strchr(s, ':'))) {
                 *tmp = '\0';
                 /* check prefix */
                 if (lyp_check_identifier(trg->ctx, s, LY_IDENT_SIMPLE, trg, NULL)) {
                   free(s);
                   YYABORT;
                 }
                 /* check identifier */
                 if (lyp_check_identifier(trg->ctx, tmp + 1, LY_IDENT_SIMPLE, trg, NULL)) {
                   free(s);
                   YYABORT;
                 }
                 *tmp = ':';
               } else {
                 /* check identifier */
                 if (lyp_check_identifier(trg->ctx, s, LY_IDENT_SIMPLE, trg, NULL)) {
                   free(s);
                   YYABORT;
                 }
               }
             }

    break;

  case 614:

    { s = (yyvsp[-1].str); }

    break;

  case 615:

    { s = (yyvsp[-3].str); }

    break;

  case 616:

    { actual_type = backup_type;
                 backup_type = NODE;
                 (yyval.str) = s;
                 s = NULL;
               }

    break;

  case 617:

    { actual_type = backup_type;
                           backup_type = NODE;
                         }

    break;

  case 618:

    { (yyval.str) = s;
                          s = NULL;
                        }

    break;

  case 622:

    { actual_type = (yyvsp[-1].backup_token).token;
                     actual = (yyvsp[-1].backup_token).actual;
                   }

    break;

  case 623:

    { (yyval.backup_token).token = actual_type;
                                                (yyval.backup_token).actual = actual;
                                                if (!(actual = yang_read_ext(trg, (actual) ? actual : trg, (yyvsp[-1].str), s,
                                                                             actual_type, backup_type, is_ext_instance))) {
                                                  YYABORT;
                                                }
                                                s = NULL;
                                                actual_type = EXTENSION_INSTANCE;
                                              }

    break;

  case 624:

    { (yyval.str) = s; s = NULL; }

    break;

  case 639:

    {  struct yang_ext_substmt *substmt = ((struct lys_ext_instance *)actual)->parent;
        int32_t length = 0, old_length = 0;
        char *tmp_value;

        if (!substmt) {
          substmt = calloc(1, sizeof *substmt);
          if (!substmt) {
            LOGMEM(trg->ctx);
            YYABORT;
          }
          ((struct lys_ext_instance *)actual)->parent = substmt;
        }
        length = strlen((yyvsp[-2].str));
        old_length = (substmt->ext_substmt) ? strlen(substmt->ext_substmt) + 2 : 2;
        tmp_value = realloc(substmt->ext_substmt, old_length + length + 1);
        if (!tmp_value) {
          LOGMEM(trg->ctx);
          YYABORT;
        }
        substmt->ext_substmt = tmp_value;
        tmp_value += old_length - 2;
        memcpy(tmp_value, (yyvsp[-2].str), length);
        tmp_value[length] = ' ';
        tmp_value[length + 1] = '\0';
        tmp_value[length + 2] = '\0';
      }

    break;

  case 640:

    {  struct yang_ext_substmt *substmt = ((struct lys_ext_instance *)actual)->parent;
        int32_t length;
        char *tmp_value, **array;
        int i = 0;

        if (!substmt) {
          substmt = calloc(1, sizeof *substmt);
          if (!substmt) {
            LOGMEM(trg->ctx);
            YYABORT;
          }
          ((struct lys_ext_instance *)actual)->parent = substmt;
        }
        length = strlen((yyvsp[-2].str));
        if (!substmt->ext_modules) {
          array = malloc(2 * sizeof *substmt->ext_modules);
        } else {
          for (i = 0; substmt->ext_modules[i]; ++i);
          array = realloc(substmt->ext_modules, (i + 2) * sizeof *substmt->ext_modules);
        }
        if (!array) {
          LOGMEM(trg->ctx);
          YYABORT;
        }
        substmt->ext_modules = array;
        array[i + 1] = NULL;
        tmp_value = malloc(length + 2);
        if (!tmp_value) {
          LOGMEM(trg->ctx);
          YYABORT;
        }
        array[i] = tmp_value;
        memcpy(tmp_value, (yyvsp[-2].str), length);
        tmp_value[length] = '\0';
        tmp_value[length + 1] = '\0';
      }

    break;

  case 643:

    { (yyval.str) = yyget_text(scanner); }

    break;

  case 644:

    { (yyval.str) = yyget_text(scanner); }

    break;

  case 656:

    { s = strdup(yyget_text(scanner));
                  if (!s) {
                    LOGMEM(trg->ctx);
                    YYABORT;
                  }
                }

    break;

  case 749:

    { s = strdup(yyget_text(scanner));
                          if (!s) {
                            LOGMEM(trg->ctx);
                            YYABORT;
                          }
                        }

    break;

  case 750:

    { s = strdup(yyget_text(scanner));
                                    if (!s) {
                                      LOGMEM(trg->ctx);
                                      YYABORT;
                                    }
                                  }

    break;

  case 751:

    { struct lys_type **type;

                             type = (struct lys_type **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                            "type", LY_STMT_TYPE);
                             if (!type) {
                               YYABORT;
                             }
                             /* allocate type structure */
                             (*type) = calloc(1, sizeof **type);
                             if (!*type) {
                               LOGMEM(trg->ctx);
                               YYABORT;
                             }

                             /* HACK for unres */
                             (*type)->parent = (struct lys_tpdf *)ext_instance;
                             (yyval.v) = actual = *type;
                             is_ext_instance = 0;
                            }

    break;

  case 752:

    { struct lys_tpdf **tpdf;

                                tpdf = (struct lys_tpdf **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                               "typedef", LY_STMT_TYPEDEF);
                                if (!tpdf) {
                                  YYABORT;
                                }
                                /* allocate typedef structure */
                                (*tpdf) = calloc(1, sizeof **tpdf);
                                if (!*tpdf) {
                                  LOGMEM(trg->ctx);
                                  YYABORT;
                                }

                                (yyval.v) = actual = *tpdf;
                                is_ext_instance = 0;
                              }

    break;

  case 753:

    { struct lys_iffeature **iffeature;

                                 iffeature = (struct lys_iffeature **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                          "if-feature", LY_STMT_IFFEATURE);
                                 if (!iffeature) {
                                   YYABORT;
                                 }
                                 /* allocate typedef structure */
                                 (*iffeature) = calloc(1, sizeof **iffeature);
                                 if (!*iffeature) {
                                   LOGMEM(trg->ctx);
                                   YYABORT;
                                 }
                                 (yyval.v) = actual = *iffeature;
                               }

    break;

  case 754:

    { struct lys_restr **restr;
                                    LY_STMT stmt;

                                    s = yyget_text(scanner);
                                    if (!strcmp(s, "must")) {
                                      stmt = LY_STMT_MUST;
                                    } else if (!strcmp(s, "pattern")) {
                                      stmt = LY_STMT_PATTERN;
                                    } else if (!strcmp(s, "range")) {
                                      stmt = LY_STMT_RANGE;
                                    } else {
                                      stmt = LY_STMT_LENGTH;
                                    }
                                    restr = (struct lys_restr **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name, s, stmt);
                                    if (!restr) {
                                      YYABORT;
                                    }
                                    /* allocate structure for must */
                                    (*restr) = calloc(1, sizeof(struct lys_restr));
                                    if (!*restr) {
                                      LOGMEM(trg->ctx);
                                      YYABORT;
                                    }
                                    (yyval.v) = actual = *restr;
                                    s = NULL;
                                  }

    break;

  case 755:

    { actual = yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name, "when", LY_STMT_WHEN);
                             if (!actual) {
                               YYABORT;
                             }
                             (yyval.v) = actual;
                           }

    break;

  case 756:

    { struct lys_revision **rev;
                                 int i;

                                 rev = (struct lys_revision **)yang_getplace_for_extcomplex_struct(ext_instance, &i, ext_name,
                                                                                                   "revision", LY_STMT_REVISION);
                                 if (!rev) {
                                   YYABORT;
                                 }
                                 rev[i] = calloc(1, sizeof **rev);
                                 if (!rev[i]) {
                                   LOGMEM(trg->ctx);
                                   YYABORT;
                                 }
                                 actual = rev[i];
                                 (yyval.revisions).revision = rev;
                                 (yyval.revisions).index = i;
                               }

    break;

  case 757:

    { LY_STMT stmt;

                                s = yyget_text(scanner);
                                if (!strcmp(s, "action")) {
                                  stmt = LY_STMT_ACTION;
                                } else if (!strcmp(s, "anydata")) {
                                  stmt = LY_STMT_ANYDATA;
                                } else if (!strcmp(s, "anyxml")) {
                                  stmt = LY_STMT_ANYXML;
                                } else if (!strcmp(s, "case")) {
                                  stmt = LY_STMT_CASE;
                                } else if (!strcmp(s, "choice")) {
                                  stmt = LY_STMT_CHOICE;
                                } else if (!strcmp(s, "container")) {
                                  stmt = LY_STMT_CONTAINER;
                                } else if (!strcmp(s, "grouping")) {
                                  stmt = LY_STMT_GROUPING;
                                } else if (!strcmp(s, "input")) {
                                  stmt = LY_STMT_INPUT;
                                } else if (!strcmp(s, "leaf")) {
                                  stmt = LY_STMT_LEAF;
                                } else if (!strcmp(s, "leaf-list")) {
                                  stmt = LY_STMT_LEAFLIST;
                                } else if (!strcmp(s, "list")) {
                                  stmt = LY_STMT_LIST;
                                } else if (!strcmp(s, "notification")) {
                                  stmt = LY_STMT_NOTIFICATION;
                                } else if (!strcmp(s, "output")) {
                                  stmt = LY_STMT_OUTPUT;
                                } else {
                                  stmt = LY_STMT_USES;
                                }
                                if (yang_extcomplex_node(ext_instance, ext_name, s, *param->node, stmt)) {
                                  YYABORT;
                                }
                                actual = NULL;
                                s = NULL;
                                is_ext_instance = 0;
                              }

    break;

  case 758:

    { LOGERR(trg->ctx, ly_errno, "Extension's substatement \"%s\" not supported.", yyget_text(scanner)); }

    break;

  case 790:

    { actual_type = EXTENSION_INSTANCE;
                                actual = ext_instance;
                                if (!is_ext_instance) {
                                  LOGVAL(trg->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
                                  YYABORT;
                                }
                                (yyval.i) = 0;
                              }

    break;

  case 792:

    { if (yang_read_extcomplex_str(trg, ext_instance, "prefix", ext_name, s,
                                                                  0, LY_STMT_PREFIX)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 793:

    { if (yang_read_extcomplex_str(trg, ext_instance, "description", ext_name, s,
                                                                       0, LY_STMT_DESCRIPTION)) {
                                            YYABORT;
                                          }
                                        }

    break;

  case 794:

    { if (yang_read_extcomplex_str(trg, ext_instance, "reference", ext_name, s,
                                                                     0, LY_STMT_REFERENCE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 795:

    { if (yang_read_extcomplex_str(trg, ext_instance, "units", ext_name, s,
                                                                     0, LY_STMT_UNITS)) {
                                      YYABORT;
                                    }
                                  }

    break;

  case 796:

    { if (yang_read_extcomplex_str(trg, ext_instance, "base", ext_name, s,
                                                                0, LY_STMT_BASE)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 797:

    { if (yang_read_extcomplex_str(trg, ext_instance, "contact", ext_name, s,
                                                                     0, LY_STMT_CONTACT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 798:

    { if (yang_read_extcomplex_str(trg, ext_instance, "default", ext_name, s,
                                                                     0, LY_STMT_DEFAULT)) {
                                        YYABORT;
                                      }
                                    }

    break;

  case 799:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-message", ext_name, s,
                                                                         0, LY_STMT_ERRMSG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 800:

    { if (yang_read_extcomplex_str(trg, ext_instance, "error-app-tag", ext_name, s,
                                                                         0, LY_STMT_ERRTAG)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 801:

    { if (yang_read_extcomplex_str(trg, ext_instance, "key", ext_name, s,
                                                               0, LY_STMT_KEY)) {
                                    YYABORT;
                                  }
                                }

    break;

  case 802:

    { if (yang_read_extcomplex_str(trg, ext_instance, "namespace", ext_name, s,
                                                                     0, LY_STMT_NAMESPACE)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 803:

    { if (yang_read_extcomplex_str(trg, ext_instance, "organization", ext_name, s,
                                                                        0, LY_STMT_ORGANIZATION)) {
                                             YYABORT;
                                           }
                                         }

    break;

  case 804:

    { if (yang_read_extcomplex_str(trg, ext_instance, "path", ext_name, s,
                                                                0, LY_STMT_PATH)) {
                                     YYABORT;
                                   }
                                 }

    break;

  case 805:

    { if (yang_read_extcomplex_str(trg, ext_instance, "presence", ext_name, s,
                                                                    0, LY_STMT_PRESENCE)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 806:

    { if (yang_read_extcomplex_str(trg, ext_instance, "revision-date", ext_name, s,
                                                                         0, LY_STMT_REVISIONDATE)) {
                                              YYABORT;
                                            }
                                          }

    break;

  case 807:

    { struct lys_type *type = (yyvsp[-2].v);

       if (yang_fill_type(trg, type, (struct yang_type *)type->der, ext_instance, param->unres)) {
         yang_type_free(trg->ctx, type);
         YYABORT;
       }
       if (unres_schema_add_node(trg, param->unres, type, UNRES_TYPE_DER_EXT, NULL) == -1) {
         yang_type_free(trg->ctx, type);
         YYABORT;
       }
       actual = ext_instance;
       is_ext_instance = 1;
     }

    break;

  case 808:

    { struct lys_tpdf *tpdf = (yyvsp[-2].v);

       if (yang_fill_type(trg, &tpdf->type, (struct yang_type *)tpdf->type.der, tpdf, param->unres)) {
         yang_type_free(trg->ctx, &tpdf->type);
       }
       if (yang_check_ext_instance(trg, &tpdf->ext, tpdf->ext_size, tpdf, param->unres)) {
         YYABORT;
       }
       if (unres_schema_add_node(trg, param->unres, &tpdf->type, UNRES_TYPE_DER_TPDF, (struct lys_node *)ext_instance) == -1) {
         yang_type_free(trg->ctx, &tpdf->type);
         YYABORT;
       }
       /* check default value*/
       if (unres_schema_add_node(trg, param->unres, &tpdf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&tpdf->dflt)) == -1)  {
         YYABORT;
       }
       actual = ext_instance;
       is_ext_instance = 1;
     }

    break;

  case 809:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "status", LY_STMT_STATUS,
                                                                    (yyvsp[0].i), LYS_STATUS_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 810:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "config", LY_STMT_CONFIG,
                                                                    (yyvsp[0].i), LYS_CONFIG_MASK)) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 811:

    { if (yang_fill_extcomplex_flags(ext_instance, ext_name, "mandatory", LY_STMT_MANDATORY,
                                                                       (yyvsp[0].i), LYS_MAND_MASK)) {
                                          YYABORT;
                                        }
                                      }

    break;

  case 812:

    { if ((yyvsp[-1].i) & LYS_ORDERED_MASK) {
                                            LOGVAL(trg->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "ordered by", ext_name);
                                            YYABORT;
                                         }
                                         if ((yyvsp[0].i) & LYS_USERORDERED) {
                                           if (yang_fill_extcomplex_flags(ext_instance, ext_name, "ordered-by", LY_STMT_ORDEREDBY,
                                                                          (yyvsp[0].i), LYS_USERORDERED)) {
                                             YYABORT;
                                           }
                                         }
                                         (yyvsp[-1].i) |= (yyvsp[0].i);
                                         (yyval.i) = (yyvsp[-1].i);
                                       }

    break;

  case 813:

    { if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "require-instance",
                                                                              LY_STMT_REQINSTANCE, (yyvsp[0].i))) {
                                                 YYABORT;
                                               }
                                             }

    break;

  case 814:

    { if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "modifier", LY_STMT_MODIFIER, 0)) {
                                         YYABORT;
                                       }
                                     }

    break;

  case 815:

    { /* range check */
       if ((yyvsp[0].uint) < 1 || (yyvsp[0].uint) > 18) {
         LOGVAL(trg->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", (yyvsp[0].uint), "fraction-digits");
         YYABORT;
       }
       if (yang_fill_extcomplex_uint8(ext_instance, ext_name, "fraction-digits", LY_STMT_DIGITS, (yyvsp[0].uint))) {
         YYABORT;
       }
     }

    break;

  case 816:

    { uint32_t **val;

                                           val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                  "min-elements", LY_STMT_MIN);
                                           if (!val) {
                                             YYABORT;
                                           }
                                           /* store the value */
                                           *val = malloc(sizeof(uint32_t));
                                           if (!*val) {
                                             LOGMEM(trg->ctx);
                                             YYABORT;
                                           }
                                           **val = (yyvsp[0].uint);
                                         }

    break;

  case 817:

    { uint32_t **val;

                                           val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                  "max-elements", LY_STMT_MAX);
                                           if (!val) {
                                             YYABORT;
                                           }
                                           /* store the value */
                                           *val = malloc(sizeof(uint32_t));
                                           if (!*val) {
                                             LOGMEM(trg->ctx);
                                             YYABORT;
                                           }
                                           **val = (yyvsp[0].uint);
                                         }

    break;

  case 818:

    { uint32_t **val;

                                       val = (uint32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                              "position", LY_STMT_POSITION);
                                       if (!val) {
                                         YYABORT;
                                       }
                                       /* store the value */
                                       *val = malloc(sizeof(uint32_t));
                                       if (!*val) {
                                         LOGMEM(trg->ctx);
                                         YYABORT;
                                       }
                                       **val = (yyvsp[0].uint);
                                     }

    break;

  case 819:

    { int32_t **val;

                                    val = (int32_t **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                          "value", LY_STMT_VALUE);
                                    if (!val) {
                                      YYABORT;
                                    }
                                    /* store the value */
                                    *val = malloc(sizeof(int32_t));
                                    if (!*val) {
                                      LOGMEM(trg->ctx);
                                      YYABORT;
                                    }
                                    **val = (yyvsp[0].i);
                                  }

    break;

  case 820:

    { struct lys_unique **unique;
                                     int rc;

                                     unique = (struct lys_unique **)yang_getplace_for_extcomplex_struct(ext_instance, NULL, ext_name,
                                                                                                        "unique", LY_STMT_UNIQUE);
                                     if (!unique) {
                                       YYABORT;
                                     }
                                     *unique = calloc(1, sizeof(struct lys_unique));
                                     if (!*unique) {
                                       LOGMEM(trg->ctx);
                                       YYABORT;
                                     }
                                     rc = yang_fill_unique(trg, (struct lys_node_list *)ext_instance, *unique, s, param->unres);
                                     free(s);
                                     s = NULL;
                                     if (rc) {
                                       YYABORT;
                                     }
                                   }

    break;

  case 821:

    { struct lys_iffeature *iffeature;

       iffeature = (yyvsp[-2].v);
       s = (char *)iffeature->features;
       iffeature->features = NULL;
       if (yang_fill_iffeature(trg, iffeature, ext_instance, s, param->unres, 0)) {
         YYABORT;
       }
       if (yang_check_ext_instance(trg, &iffeature->ext, iffeature->ext_size, iffeature, param->unres)) {
         YYABORT;
       }
       s = NULL;
       actual = ext_instance;
     }

    break;

  case 823:

    { if (yang_check_ext_instance(trg, &((struct lys_restr *)(yyvsp[-2].v))->ext, ((struct lys_restr *)(yyvsp[-2].v))->ext_size, (yyvsp[-2].v), param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 824:

    { if (yang_check_ext_instance(trg, &(*(struct lys_when **)(yyvsp[-2].v))->ext, (*(struct lys_when **)(yyvsp[-2].v))->ext_size,
                                   *(struct lys_when **)(yyvsp[-2].v), param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 825:

    { int i;

       for (i = 0; i < (yyvsp[-2].revisions).index; ++i) {
         if (!strcmp((yyvsp[-2].revisions).revision[i]->date, (yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->date)) {
           LOGWRN(trg->ctx, "Module's revisions are not unique (%s).", (yyvsp[-2].revisions).revision[i]->date);
           break;
         }
       }
       if (yang_check_ext_instance(trg, &(yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->ext, (yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index]->ext_size,
                                   &(yyvsp[-2].revisions).revision[(yyvsp[-2].revisions).index], param->unres)) {
         YYABORT;
       }
       actual = ext_instance;
     }

    break;

  case 826:

    { actual = ext_instance;
                                                                    is_ext_instance = 1;
                                                                  }

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
  *++yylsp = yyloc;

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
      yyerror (&yylloc, scanner, param, YY_("syntax error"));
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
        yyerror (&yylloc, scanner, param, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, scanner, param);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, scanner, param);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, scanner, param, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, scanner, param);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, param);
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



void
yyerror(YYLTYPE *yylloc, void *scanner, struct yang_parameter *param, ...)
{
  free(*param->value);
  if (yylloc->first_line != -1) {
    if (*param->data_node && (*param->data_node) == (*param->actual_node)) {
      LOGVAL(param->module->ctx, LYE_INSTMT, LY_VLOG_LYS, *param->data_node, yyget_text(scanner));
    } else {
      LOGVAL(param->module->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, yyget_text(scanner));
    }
  }
}
