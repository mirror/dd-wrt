
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



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 22 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"

/*

 */

#define DONTDECLARE_MALLOC

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "ld.h"
#include "ldexp.h"
#include "ldver.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"
#include "ldmisc.h"
#include "ldmain.h"
#include "mri.h"
#include "ldctor.h"
#include "ldlex.h"

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

static enum section_type sectype;

lang_memory_region_type *region;

bfd_boolean ldgram_want_filename = TRUE;
FILE *saved_script_handle = NULL;
bfd_boolean force_make_executable = FALSE;

bfd_boolean ldgram_in_script = FALSE;
bfd_boolean ldgram_had_equals = FALSE;
bfd_boolean ldgram_had_keep = FALSE;
char *ldgram_vers_current_lang = NULL;

#define ERROR_NAME_MAX 20
static char *error_names[ERROR_NAME_MAX];
static int error_index;
#define PUSH_ERROR(x) if (error_index < ERROR_NAME_MAX) error_names[error_index] = x; error_index++;
#define POP_ERROR()   error_index--;


/* Line 189 of yacc.c  */
#line 120 "y.tab.c"

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
     INT = 258,
     NAME = 259,
     LNAME = 260,
     OREQ = 261,
     ANDEQ = 262,
     RSHIFTEQ = 263,
     LSHIFTEQ = 264,
     DIVEQ = 265,
     MULTEQ = 266,
     MINUSEQ = 267,
     PLUSEQ = 268,
     OROR = 269,
     ANDAND = 270,
     NE = 271,
     EQ = 272,
     GE = 273,
     LE = 274,
     RSHIFT = 275,
     LSHIFT = 276,
     UNARY = 277,
     END = 278,
     ALIGN_K = 279,
     BLOCK = 280,
     BIND = 281,
     QUAD = 282,
     SQUAD = 283,
     LONG = 284,
     SHORT = 285,
     BYTE = 286,
     SECTIONS = 287,
     PHDRS = 288,
     SORT = 289,
     DATA_SEGMENT_ALIGN = 290,
     DATA_SEGMENT_END = 291,
     SIZEOF_HEADERS = 292,
     OUTPUT_FORMAT = 293,
     FORCE_COMMON_ALLOCATION = 294,
     OUTPUT_ARCH = 295,
     INHIBIT_COMMON_ALLOCATION = 296,
     INCLUDE = 297,
     MEMORY = 298,
     DEFSYMEND = 299,
     NOLOAD = 300,
     DSECT = 301,
     COPY = 302,
     INFO = 303,
     OVERLAY = 304,
     DEFINED = 305,
     TARGET_K = 306,
     SEARCH_DIR = 307,
     MAP = 308,
     ENTRY = 309,
     NEXT = 310,
     SIZEOF = 311,
     ADDR = 312,
     LOADADDR = 313,
     MAX_K = 314,
     MIN_K = 315,
     STARTUP = 316,
     HLL = 317,
     SYSLIB = 318,
     FLOAT = 319,
     NOFLOAT = 320,
     NOCROSSREFS = 321,
     ORIGIN = 322,
     FILL = 323,
     LENGTH = 324,
     CREATE_OBJECT_SYMBOLS = 325,
     INPUT = 326,
     GROUP = 327,
     OUTPUT = 328,
     CONSTRUCTORS = 329,
     ALIGNMOD = 330,
     AT = 331,
     SUBALIGN = 332,
     PROVIDE = 333,
     CHIP = 334,
     LIST = 335,
     SECT = 336,
     ABSOLUTE = 337,
     LOAD = 338,
     NEWLINE = 339,
     ENDWORD = 340,
     ORDER = 341,
     NAMEWORD = 342,
     ASSERT_K = 343,
     FORMAT = 344,
     PUBLIC = 345,
     BASE = 346,
     ALIAS = 347,
     TRUNCATE = 348,
     REL = 349,
     INPUT_SCRIPT = 350,
     INPUT_MRI_SCRIPT = 351,
     INPUT_DEFSYM = 352,
     CASE = 353,
     EXTERN = 354,
     START = 355,
     VERS_TAG = 356,
     VERS_IDENTIFIER = 357,
     GLOBAL = 358,
     LOCAL = 359,
     VERSIONK = 360,
     INPUT_VERSION_SCRIPT = 361,
     KEEP = 362,
     EXCLUDE_FILE = 363
   };
#endif
/* Tokens.  */
#define INT 258
#define NAME 259
#define LNAME 260
#define OREQ 261
#define ANDEQ 262
#define RSHIFTEQ 263
#define LSHIFTEQ 264
#define DIVEQ 265
#define MULTEQ 266
#define MINUSEQ 267
#define PLUSEQ 268
#define OROR 269
#define ANDAND 270
#define NE 271
#define EQ 272
#define GE 273
#define LE 274
#define RSHIFT 275
#define LSHIFT 276
#define UNARY 277
#define END 278
#define ALIGN_K 279
#define BLOCK 280
#define BIND 281
#define QUAD 282
#define SQUAD 283
#define LONG 284
#define SHORT 285
#define BYTE 286
#define SECTIONS 287
#define PHDRS 288
#define SORT 289
#define DATA_SEGMENT_ALIGN 290
#define DATA_SEGMENT_END 291
#define SIZEOF_HEADERS 292
#define OUTPUT_FORMAT 293
#define FORCE_COMMON_ALLOCATION 294
#define OUTPUT_ARCH 295
#define INHIBIT_COMMON_ALLOCATION 296
#define INCLUDE 297
#define MEMORY 298
#define DEFSYMEND 299
#define NOLOAD 300
#define DSECT 301
#define COPY 302
#define INFO 303
#define OVERLAY 304
#define DEFINED 305
#define TARGET_K 306
#define SEARCH_DIR 307
#define MAP 308
#define ENTRY 309
#define NEXT 310
#define SIZEOF 311
#define ADDR 312
#define LOADADDR 313
#define MAX_K 314
#define MIN_K 315
#define STARTUP 316
#define HLL 317
#define SYSLIB 318
#define FLOAT 319
#define NOFLOAT 320
#define NOCROSSREFS 321
#define ORIGIN 322
#define FILL 323
#define LENGTH 324
#define CREATE_OBJECT_SYMBOLS 325
#define INPUT 326
#define GROUP 327
#define OUTPUT 328
#define CONSTRUCTORS 329
#define ALIGNMOD 330
#define AT 331
#define SUBALIGN 332
#define PROVIDE 333
#define CHIP 334
#define LIST 335
#define SECT 336
#define ABSOLUTE 337
#define LOAD 338
#define NEWLINE 339
#define ENDWORD 340
#define ORDER 341
#define NAMEWORD 342
#define ASSERT_K 343
#define FORMAT 344
#define PUBLIC 345
#define BASE 346
#define ALIAS 347
#define TRUNCATE 348
#define REL 349
#define INPUT_SCRIPT 350
#define INPUT_MRI_SCRIPT 351
#define INPUT_DEFSYM 352
#define CASE 353
#define EXTERN 354
#define START 355
#define VERS_TAG 356
#define VERS_IDENTIFIER 357
#define GLOBAL 358
#define LOCAL 359
#define VERSIONK 360
#define INPUT_VERSION_SCRIPT 361
#define KEEP 362
#define EXCLUDE_FILE 363




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 67 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"

  bfd_vma integer;
  struct big_int
    {
      bfd_vma integer;
      char *str;
    } bigint;
  fill_type *fill;
  char *name;
  const char *cname;
  struct wildcard_spec wildcard;
  struct wildcard_list *wildcard_list;
  struct name_list *name_list;
  int token;
  union etree_union *etree;
  struct phdr_info
    {
      bfd_boolean filehdr;
      bfd_boolean phdrs;
      union etree_union *at;
      union etree_union *flags;
    } phdr;
  struct lang_nocrossref *nocrossref;
  struct lang_output_section_phdr_list *section_phdr;
  struct bfd_elf_version_deps *deflist;
  struct bfd_elf_version_expr *versyms;
  struct bfd_elf_version_tree *versnode;



/* Line 214 of yacc.c  */
#line 403 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 415 "y.tab.c"

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
#define YYFINAL  14
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1436

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  132
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  108
/* YYNRULES -- Number of rules.  */
#define YYNRULES  300
/* YYNRULES -- Number of states.  */
#define YYNSTATES  617

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   363

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   130,     2,     2,     2,    34,    21,     2,
      37,   127,    32,    30,   125,    31,     2,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    16,   126,
      24,     6,    25,    15,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   128,     2,   129,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,    19,    52,   131,     2,     2,     2,
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
       5,     7,     8,     9,    10,    11,    12,    13,    14,    17,
      18,    22,    23,    26,    27,    28,    29,    35,    36,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    12,    15,    17,    18,    23,
      24,    27,    31,    32,    35,    40,    42,    44,    47,    49,
      54,    59,    63,    66,    71,    75,    80,    85,    90,    95,
     100,   103,   106,   109,   114,   119,   122,   125,   128,   131,
     132,   138,   141,   142,   146,   149,   150,   152,   156,   158,
     162,   163,   165,   169,   171,   174,   178,   179,   182,   185,
     186,   188,   190,   192,   194,   196,   198,   200,   202,   204,
     206,   211,   216,   221,   226,   235,   240,   242,   244,   249,
     250,   256,   261,   262,   268,   273,   278,   280,   284,   287,
     289,   293,   296,   301,   304,   307,   308,   313,   316,   318,
     320,   322,   324,   330,   335,   344,   347,   349,   353,   355,
     357,   361,   366,   368,   369,   375,   378,   380,   382,   384,
     389,   391,   396,   401,   404,   406,   407,   409,   411,   413,
     415,   417,   419,   421,   424,   425,   427,   429,   431,   433,
     435,   437,   439,   441,   443,   445,   449,   453,   460,   462,
     463,   469,   472,   476,   477,   478,   486,   490,   494,   495,
     499,   501,   504,   506,   509,   514,   519,   523,   527,   529,
     534,   538,   539,   541,   543,   544,   547,   551,   552,   555,
     558,   562,   567,   570,   573,   576,   580,   584,   588,   592,
     596,   600,   604,   608,   612,   616,   620,   624,   628,   632,
     636,   640,   646,   650,   654,   659,   661,   663,   668,   673,
     678,   683,   688,   695,   700,   705,   707,   714,   721,   728,
     732,   733,   738,   739,   744,   745,   746,   747,   748,   749,
     750,   768,   769,   770,   771,   772,   773,   792,   793,   794,
     802,   804,   806,   808,   810,   812,   816,   817,   820,   824,
     827,   834,   845,   848,   850,   851,   853,   856,   857,   858,
     862,   863,   864,   865,   866,   878,   883,   884,   887,   888,
     889,   896,   898,   899,   903,   909,   910,   914,   915,   918,
     919,   925,   927,   930,   935,   941,   948,   950,   953,   954,
     957,   962,   967,   976,   978,   982,   983,   993,   994,  1002,
    1003
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     133,     0,    -1,   111,   147,    -1,   112,   137,    -1,   122,
     228,    -1,   113,   135,    -1,     4,    -1,    -1,   136,     4,
       6,   192,    -1,    -1,   138,   139,    -1,   139,   140,   100,
      -1,    -1,    95,   192,    -1,    95,   192,   125,   192,    -1,
       4,    -1,    96,    -1,   102,   142,    -1,   101,    -1,   106,
       4,     6,   192,    -1,   106,     4,   125,   192,    -1,   106,
       4,   192,    -1,   105,     4,    -1,    97,     4,   125,   192,
      -1,    97,     4,   192,    -1,    97,     4,     6,   192,    -1,
      38,     4,     6,   192,    -1,    38,     4,   125,   192,    -1,
      91,     4,     6,   192,    -1,    91,     4,   125,   192,    -1,
      98,   144,    -1,    99,   143,    -1,   103,     4,    -1,   108,
       4,   125,     4,    -1,   108,     4,   125,     3,    -1,   107,
     192,    -1,   109,     3,    -1,   114,   145,    -1,   115,   146,
      -1,    -1,    58,   134,   141,   139,    36,    -1,   116,     4,
      -1,    -1,   142,   125,     4,    -1,   142,     4,    -1,    -1,
       4,    -1,   143,   125,     4,    -1,     4,    -1,   144,   125,
       4,    -1,    -1,     4,    -1,   145,   125,     4,    -1,     4,
      -1,   146,     4,    -1,   146,   125,     4,    -1,    -1,   148,
     149,    -1,   149,   150,    -1,    -1,   174,    -1,   154,    -1,
     220,    -1,   183,    -1,   184,    -1,   186,    -1,   188,    -1,
     156,    -1,   230,    -1,   126,    -1,    67,    37,     4,   127,
      -1,    68,    37,   134,   127,    -1,    89,    37,   134,   127,
      -1,    54,    37,     4,   127,    -1,    54,    37,     4,   125,
       4,   125,     4,   127,    -1,    56,    37,     4,   127,    -1,
      55,    -1,    57,    -1,    87,    37,   153,   127,    -1,    -1,
      88,   151,    37,   153,   127,    -1,    69,    37,   134,   127,
      -1,    -1,    58,   134,   152,   149,    36,    -1,    82,    37,
     189,   127,    -1,   115,    37,   146,   127,    -1,     4,    -1,
     153,   125,     4,    -1,   153,     4,    -1,     5,    -1,   153,
     125,     5,    -1,   153,     5,    -1,    46,    51,   155,    52,
      -1,   155,   196,    -1,   155,   156,    -1,    -1,    70,    37,
       4,   127,    -1,   172,   171,    -1,     4,    -1,    32,    -1,
      15,    -1,   157,    -1,   124,    37,   159,   127,   157,    -1,
      48,    37,   157,   127,    -1,    48,    37,   124,    37,   159,
     127,   157,   127,    -1,   159,   157,    -1,   157,    -1,   160,
     173,   158,    -1,   158,    -1,     4,    -1,   128,   160,   129,
      -1,   158,    37,   160,   127,    -1,   161,    -1,    -1,   123,
      37,   163,   161,   127,    -1,   172,   171,    -1,    86,    -1,
     126,    -1,    90,    -1,    48,    37,    90,   127,    -1,   162,
      -1,   167,    37,   190,   127,    -1,    84,    37,   168,   127,
      -1,   165,   164,    -1,   164,    -1,    -1,   165,    -1,    41,
      -1,    42,    -1,    43,    -1,    44,    -1,    45,    -1,   190,
      -1,     6,   168,    -1,    -1,    14,    -1,    13,    -1,    12,
      -1,    11,    -1,    10,    -1,     9,    -1,     8,    -1,     7,
      -1,   126,    -1,   125,    -1,     4,     6,   190,    -1,     4,
     170,   190,    -1,    94,    37,     4,     6,   190,   127,    -1,
     125,    -1,    -1,    59,    51,   176,   175,    52,    -1,   175,
     176,    -1,   175,   125,   176,    -1,    -1,    -1,     4,   177,
     180,    16,   178,   173,   179,    -1,    83,     6,   190,    -1,
      85,     6,   190,    -1,    -1,    37,   181,   127,    -1,   182,
      -1,   181,   182,    -1,     4,    -1,   130,     4,    -1,    77,
      37,   134,   127,    -1,    78,    37,   185,   127,    -1,    78,
      37,   127,    -1,   185,   173,   134,    -1,   134,    -1,    79,
      37,   187,   127,    -1,   187,   173,   134,    -1,    -1,    80,
      -1,    81,    -1,    -1,     4,   189,    -1,     4,   125,   189,
      -1,    -1,   191,   192,    -1,    31,   192,    -1,    37,   192,
     127,    -1,    71,    37,   192,   127,    -1,   130,   192,    -1,
      30,   192,    -1,   131,   192,    -1,   192,    32,   192,    -1,
     192,    33,   192,    -1,   192,    34,   192,    -1,   192,    30,
     192,    -1,   192,    31,   192,    -1,   192,    29,   192,    -1,
     192,    28,   192,    -1,   192,    23,   192,    -1,   192,    22,
     192,    -1,   192,    27,   192,    -1,   192,    26,   192,    -1,
     192,    24,   192,    -1,   192,    25,   192,    -1,   192,    21,
     192,    -1,   192,    20,   192,    -1,   192,    19,   192,    -1,
     192,    15,   192,    16,   192,    -1,   192,    18,   192,    -1,
     192,    17,   192,    -1,    66,    37,     4,   127,    -1,     3,
      -1,    53,    -1,    72,    37,     4,   127,    -1,    73,    37,
       4,   127,    -1,    74,    37,     4,   127,    -1,    98,    37,
     192,   127,    -1,    38,    37,   192,   127,    -1,    49,    37,
     192,   125,   192,   127,    -1,    50,    37,   192,   127,    -1,
      39,    37,   192,   127,    -1,     4,    -1,    75,    37,   192,
     125,   192,   127,    -1,    76,    37,   192,   125,   192,   127,
      -1,   104,    37,   192,   125,     4,   127,    -1,    92,    25,
       4,    -1,    -1,    92,    37,   192,   127,    -1,    -1,    93,
      37,   192,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,   197,   211,   194,   195,   198,    51,   199,   166,    52,
     200,   214,   193,   215,   169,   201,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   202,   212,   213,   194,   195,   203,
      51,   204,   216,    52,   205,   214,   193,   215,   169,   206,
     173,    -1,    -1,    -1,    88,   207,   211,   208,    51,   155,
      52,    -1,    61,    -1,    62,    -1,    63,    -1,    64,    -1,
      65,    -1,    37,   209,   127,    -1,    -1,    37,   127,    -1,
     192,   210,    16,    -1,   210,    16,    -1,    40,    37,   192,
     127,   210,    16,    -1,    40,    37,   192,   127,    39,    37,
     192,   127,   210,    16,    -1,   192,    16,    -1,    16,    -1,
      -1,    82,    -1,    25,     4,    -1,    -1,    -1,   215,    16,
       4,    -1,    -1,    -1,    -1,    -1,   216,     4,   217,    51,
     166,    52,   218,   215,   169,   219,   173,    -1,    47,    51,
     221,    52,    -1,    -1,   221,   222,    -1,    -1,    -1,     4,
     223,   225,   226,   224,   126,    -1,   192,    -1,    -1,     4,
     227,   226,    -1,    92,    37,   192,   127,   226,    -1,    -1,
      37,   192,   127,    -1,    -1,   229,   232,    -1,    -1,   231,
     121,    51,   232,    52,    -1,   233,    -1,   232,   233,    -1,
      51,   235,    52,   126,    -1,   117,    51,   235,    52,   126,
      -1,   117,    51,   235,    52,   234,   126,    -1,   117,    -1,
     234,   117,    -1,    -1,   236,   126,    -1,   119,    16,   236,
     126,    -1,   120,    16,   236,   126,    -1,   119,    16,   236,
     126,   120,    16,   236,   126,    -1,   118,    -1,   236,   126,
     118,    -1,    -1,   236,   126,   115,     4,    51,   237,   236,
     239,    52,    -1,    -1,   115,     4,    51,   238,   236,   239,
      52,    -1,    -1,   126,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   162,   162,   163,   164,   165,   169,   173,   173,   183,
     183,   196,   197,   201,   202,   203,   206,   209,   210,   211,
     213,   215,   217,   219,   221,   223,   225,   227,   229,   231,
     233,   234,   235,   237,   239,   241,   243,   245,   246,   248,
     247,   251,   253,   257,   258,   259,   263,   265,   269,   271,
     276,   277,   278,   282,   284,   286,   291,   291,   302,   303,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   321,   323,   325,   328,   330,   332,   334,   336,   338,
     337,   341,   344,   343,   347,   351,   355,   358,   361,   364,
     367,   370,   376,   380,   381,   382,   386,   388,   394,   398,
     402,   409,   415,   421,   427,   436,   445,   456,   465,   476,
     484,   488,   495,   497,   496,   503,   504,   508,   509,   514,
     519,   520,   525,   532,   533,   536,   538,   542,   544,   546,
     548,   550,   555,   565,   567,   571,   573,   575,   577,   579,
     581,   583,   585,   590,   590,   595,   599,   607,   615,   615,
     619,   623,   624,   625,   630,   629,   637,   645,   655,   656,
     660,   661,   665,   667,   672,   677,   678,   683,   685,   691,
     693,   695,   699,   701,   707,   710,   719,   730,   730,   736,
     738,   740,   742,   744,   746,   749,   751,   753,   755,   757,
     759,   761,   763,   765,   767,   769,   771,   773,   775,   777,
     779,   781,   783,   785,   787,   789,   791,   794,   796,   798,
     800,   802,   804,   806,   808,   810,   812,   814,   816,   822,
     823,   827,   828,   832,   833,   836,   839,   841,   847,   849,
     836,   856,   858,   860,   865,   867,   855,   877,   879,   877,
     887,   888,   889,   890,   891,   895,   896,   897,   901,   902,
     907,   908,   913,   914,   919,   920,   925,   927,   932,   935,
     948,   952,   957,   959,   950,   967,   970,   972,   976,   977,
     976,   986,  1031,  1034,  1046,  1055,  1058,  1067,  1067,  1081,
    1081,  1091,  1092,  1096,  1100,  1104,  1111,  1115,  1123,  1126,
    1130,  1134,  1138,  1145,  1149,  1154,  1153,  1164,  1163,  1175,
    1177
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "NAME", "LNAME", "'='", "OREQ",
  "ANDEQ", "RSHIFTEQ", "LSHIFTEQ", "DIVEQ", "MULTEQ", "MINUSEQ", "PLUSEQ",
  "'?'", "':'", "OROR", "ANDAND", "'|'", "'^'", "'&'", "NE", "EQ", "'<'",
  "'>'", "GE", "LE", "RSHIFT", "LSHIFT", "'+'", "'-'", "'*'", "'/'", "'%'",
  "UNARY", "END", "'('", "ALIGN_K", "BLOCK", "BIND", "QUAD", "SQUAD",
  "LONG", "SHORT", "BYTE", "SECTIONS", "PHDRS", "SORT",
  "DATA_SEGMENT_ALIGN", "DATA_SEGMENT_END", "'{'", "'}'", "SIZEOF_HEADERS",
  "OUTPUT_FORMAT", "FORCE_COMMON_ALLOCATION", "OUTPUT_ARCH",
  "INHIBIT_COMMON_ALLOCATION", "INCLUDE", "MEMORY", "DEFSYMEND", "NOLOAD",
  "DSECT", "COPY", "INFO", "OVERLAY", "DEFINED", "TARGET_K", "SEARCH_DIR",
  "MAP", "ENTRY", "NEXT", "SIZEOF", "ADDR", "LOADADDR", "MAX_K", "MIN_K",
  "STARTUP", "HLL", "SYSLIB", "FLOAT", "NOFLOAT", "NOCROSSREFS", "ORIGIN",
  "FILL", "LENGTH", "CREATE_OBJECT_SYMBOLS", "INPUT", "GROUP", "OUTPUT",
  "CONSTRUCTORS", "ALIGNMOD", "AT", "SUBALIGN", "PROVIDE", "CHIP", "LIST",
  "SECT", "ABSOLUTE", "LOAD", "NEWLINE", "ENDWORD", "ORDER", "NAMEWORD",
  "ASSERT_K", "FORMAT", "PUBLIC", "BASE", "ALIAS", "TRUNCATE", "REL",
  "INPUT_SCRIPT", "INPUT_MRI_SCRIPT", "INPUT_DEFSYM", "CASE", "EXTERN",
  "START", "VERS_TAG", "VERS_IDENTIFIER", "GLOBAL", "LOCAL", "VERSIONK",
  "INPUT_VERSION_SCRIPT", "KEEP", "EXCLUDE_FILE", "','", "';'", "')'",
  "'['", "']'", "'!'", "'~'", "$accept", "file", "filename", "defsym_expr",
  "$@1", "mri_script_file", "$@2", "mri_script_lines",
  "mri_script_command", "$@3", "ordernamelist", "mri_load_name_list",
  "mri_abs_name_list", "casesymlist", "extern_name_list", "script_file",
  "$@4", "ifile_list", "ifile_p1", "$@5", "$@6", "input_list", "sections",
  "sec_or_group_p1", "statement_anywhere", "wildcard_name",
  "wildcard_spec", "exclude_name_list", "file_NAME_list",
  "input_section_spec_no_keep", "input_section_spec", "$@7", "statement",
  "statement_list", "statement_list_opt", "length", "fill_exp", "fill_opt",
  "assign_op", "end", "assignment", "opt_comma", "memory",
  "memory_spec_list", "memory_spec", "$@8", "origin_spec", "length_spec",
  "attributes_opt", "attributes_list", "attributes_string", "startup",
  "high_level_library", "high_level_library_NAME_list",
  "low_level_library", "low_level_library_NAME_list",
  "floating_point_support", "nocrossref_list", "mustbe_exp", "$@9", "exp",
  "memspec_at_opt", "opt_at", "opt_subalign", "section", "$@10", "$@11",
  "$@12", "$@13", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20",
  "$@21", "type", "atype", "opt_exp_with_type", "opt_exp_without_type",
  "opt_nocrossrefs", "memspec_opt", "phdr_opt", "overlay_section", "$@22",
  "$@23", "$@24", "phdrs", "phdr_list", "phdr", "$@25", "$@26",
  "phdr_type", "phdr_qualifiers", "phdr_val", "version_script_file",
  "$@27", "version", "$@28", "vers_nodes", "vers_node", "verdep",
  "vers_tag", "vers_defns", "@29", "@30", "opt_semicolon", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,    61,   261,   262,   263,
     264,   265,   266,   267,   268,    63,    58,   269,   270,   124,
      94,    38,   271,   272,    60,    62,   273,   274,   275,   276,
      43,    45,    42,    47,    37,   277,   278,    40,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   123,   125,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,    44,    59,    41,    91,    93,
      33,   126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   132,   133,   133,   133,   133,   134,   136,   135,   138,
     137,   139,   139,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   141,
     140,   140,   140,   142,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   146,   146,   146,   148,   147,   149,   149,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   151,
     150,   150,   152,   150,   150,   150,   153,   153,   153,   153,
     153,   153,   154,   155,   155,   155,   156,   156,   157,   157,
     157,   158,   158,   158,   158,   159,   159,   160,   160,   161,
     161,   161,   162,   163,   162,   164,   164,   164,   164,   164,
     164,   164,   164,   165,   165,   166,   166,   167,   167,   167,
     167,   167,   168,   169,   169,   170,   170,   170,   170,   170,
     170,   170,   170,   171,   171,   172,   172,   172,   173,   173,
     174,   175,   175,   175,   177,   176,   178,   179,   180,   180,
     181,   181,   182,   182,   183,   184,   184,   185,   185,   186,
     187,   187,   188,   188,   189,   189,   189,   191,   190,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   193,
     193,   194,   194,   195,   195,   197,   198,   199,   200,   201,
     196,   202,   203,   204,   205,   206,   196,   207,   208,   196,
     209,   209,   209,   209,   209,   210,   210,   210,   211,   211,
     211,   211,   212,   212,   213,   213,   214,   214,   215,   215,
     216,   217,   218,   219,   216,   220,   221,   221,   223,   224,
     222,   225,   226,   226,   226,   227,   227,   229,   228,   231,
     230,   232,   232,   233,   233,   233,   234,   234,   235,   235,
     235,   235,   235,   236,   236,   237,   236,   238,   236,   239,
     239
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     2,     1,     0,     4,     0,
       2,     3,     0,     2,     4,     1,     1,     2,     1,     4,
       4,     3,     2,     4,     3,     4,     4,     4,     4,     4,
       2,     2,     2,     4,     4,     2,     2,     2,     2,     0,
       5,     2,     0,     3,     2,     0,     1,     3,     1,     3,
       0,     1,     3,     1,     2,     3,     0,     2,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     4,     4,     4,     8,     4,     1,     1,     4,     0,
       5,     4,     0,     5,     4,     4,     1,     3,     2,     1,
       3,     2,     4,     2,     2,     0,     4,     2,     1,     1,
       1,     1,     5,     4,     8,     2,     1,     3,     1,     1,
       3,     4,     1,     0,     5,     2,     1,     1,     1,     4,
       1,     4,     4,     2,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     6,     1,     0,
       5,     2,     3,     0,     0,     7,     3,     3,     0,     3,
       1,     2,     1,     2,     4,     4,     3,     3,     1,     4,
       3,     0,     1,     1,     0,     2,     3,     0,     2,     2,
       3,     4,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     3,     3,     4,     1,     1,     4,     4,     4,
       4,     4,     6,     4,     4,     1,     6,     6,     6,     3,
       0,     4,     0,     4,     0,     0,     0,     0,     0,     0,
      17,     0,     0,     0,     0,     0,    18,     0,     0,     7,
       1,     1,     1,     1,     1,     3,     0,     2,     3,     2,
       6,    10,     2,     1,     0,     1,     2,     0,     0,     3,
       0,     0,     0,     0,    11,     4,     0,     2,     0,     0,
       6,     1,     0,     3,     5,     0,     3,     0,     2,     0,
       5,     1,     2,     4,     5,     6,     1,     2,     0,     2,
       4,     4,     8,     1,     3,     0,     9,     0,     7,     0,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,    56,     9,     7,   277,     0,     2,    59,     3,    12,
       5,     0,     4,     0,     1,    57,    10,     0,   288,     0,
     278,   281,     0,     0,     0,     0,    76,     0,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   172,   173,
       0,     0,    79,     0,     0,     0,    69,    58,    61,    67,
       0,    60,    63,    64,    65,    66,    62,    68,     0,    15,
       0,     0,     0,     0,    16,     0,     0,     0,    18,    45,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   293,     0,     0,     0,     0,   288,   282,   177,
     142,   141,   140,   139,   138,   137,   136,   135,   177,    95,
     266,     0,     0,     6,    82,     0,     0,     0,     0,     0,
       0,     0,   171,   174,     0,     0,     0,     0,     0,   144,
     143,    97,     0,     0,    39,     0,   205,   215,     0,     0,
       0,     0,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,     0,    48,
      30,    46,    31,    17,    32,    22,     0,    35,     0,    36,
      51,    37,    53,    38,    41,    11,     8,     0,     0,     0,
       0,   289,     0,   145,     0,   146,     0,     0,     0,     0,
      59,   154,   153,     0,     0,     0,     0,     0,   166,   168,
     149,   149,   174,     0,    86,    89,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,   183,   179,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,   184,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
      44,     0,     0,     0,    21,     0,     0,    54,     0,   297,
       0,     0,   283,     0,   294,     0,   178,   225,    92,   231,
     237,    94,    93,   268,   265,   267,     0,    73,    75,   279,
     158,     0,    70,    71,    81,    96,   164,   148,   165,     0,
     169,     0,   174,   175,    84,    88,    91,     0,    78,     0,
      72,   177,    85,     0,    26,    27,    42,    28,    29,   180,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,   202,   200,   199,   198,   193,
     192,   196,   197,   195,   194,   191,   190,   188,   189,   185,
     186,   187,    14,    25,    23,    49,    47,    43,    19,    20,
      34,    33,    52,    55,     0,   290,   291,     0,   286,   284,
       0,   246,     0,   246,     0,     0,    83,     0,     0,   150,
       0,   151,   167,   170,   176,    87,    90,    80,     0,   280,
      40,   211,   214,     0,   213,   204,   181,   207,   208,   209,
       0,     0,   210,     0,     0,   299,     0,   295,   287,   285,
       0,     0,   246,     0,   222,   253,     0,   254,   238,   271,
     272,     0,   162,     0,     0,   160,     0,   152,   147,     0,
       0,     0,     0,   201,   300,     0,     0,     0,   240,   241,
     242,   243,   244,   247,     0,     0,     0,     0,   249,     0,
     224,   252,   255,   222,     0,   275,     0,   269,     0,   163,
     159,   161,     0,   149,   212,   216,   217,   218,   298,     0,
     299,   245,     0,   248,     0,     0,   226,   224,    95,     0,
     272,     0,     0,    74,   177,     0,   292,     0,   246,     0,
       0,     0,   232,     0,     0,   273,     0,   270,   156,     0,
     155,   296,     0,     0,   221,     0,   227,     0,   239,   276,
     272,   177,     0,   250,   223,   125,   233,   274,   157,     0,
     109,   100,    99,   127,   128,   129,   130,   131,     0,     0,
     116,   118,     0,     0,   117,     0,   101,     0,   112,   120,
     124,   126,     0,     0,     0,   260,   246,     0,   177,   113,
       0,    98,     0,   108,   149,     0,   123,   228,   177,   115,
       0,     0,     0,     0,     0,     0,   132,     0,   106,     0,
       0,   110,     0,   149,   257,     0,   261,   234,   251,   119,
       0,   103,   122,    98,     0,     0,   105,   107,   111,     0,
     220,   121,     0,   257,     0,   114,   102,   256,     0,   258,
     125,   220,     0,     0,   134,     0,   258,     0,   219,   177,
       0,   229,   262,   134,   104,   133,   259,   149,   258,   235,
     230,   134,   149,   263,   236,   149,   264
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,   104,    10,    11,     8,     9,    16,    79,   204,
     153,   152,   150,   161,   163,     6,     7,    15,    47,   115,
     180,   196,    48,   176,    49,   526,   527,   559,   544,   528,
     529,   557,   530,   531,   532,   533,   555,   601,    98,   121,
      50,   562,    51,   281,   182,   280,   453,   490,   368,   414,
     415,    52,    53,   190,    54,   191,    55,   193,   556,   174,
     209,   589,   440,   466,   272,   361,   481,   505,   564,   607,
     362,   497,   535,   583,   612,   363,   444,   434,   403,   404,
     407,   443,   580,   594,   550,   582,   608,   615,    56,   177,
     275,   364,   472,   410,   447,   470,    12,    13,    57,    58,
      20,    21,   360,    85,    86,   427,   354,   425
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -559
static const yytype_int16 yypact[] =
{
      94,  -559,  -559,  -559,  -559,    57,  -559,  -559,  -559,  -559,
    -559,    58,  -559,   -12,  -559,   640,   726,    73,   102,    49,
     -12,  -559,   357,    63,    81,    87,  -559,   162,  -559,   204,
     176,   202,   208,   210,   214,   226,   230,   231,  -559,  -559,
     232,   241,  -559,   242,   243,   244,  -559,  -559,  -559,  -559,
      93,  -559,  -559,  -559,  -559,  -559,  -559,  -559,    92,  -559,
     258,   204,   287,   575,  -559,   289,   290,   291,  -559,  -559,
     292,   293,   294,   575,   299,   301,   302,   303,   304,   205,
     575,   306,  -559,   296,   297,   262,   193,   102,  -559,  -559,
    -559,  -559,  -559,  -559,  -559,  -559,  -559,  -559,  -559,  -559,
    -559,   314,   316,  -559,  -559,   318,   322,   204,   204,   323,
     204,    10,  -559,   326,   124,   300,   204,   327,   303,  -559,
    -559,  -559,   282,    36,  -559,    68,  -559,  -559,   575,   575,
     575,   317,   319,   335,   338,  -559,   339,   340,   341,   342,
     343,   344,   345,   346,   349,   575,   575,  1162,   286,  -559,
     209,  -559,   215,    13,  -559,  -559,   392,  1402,   228,  -559,
    -559,   263,  -559,    27,  -559,  -559,  1402,   336,   -27,   -27,
     229,     8,   347,  -559,   575,  -559,    25,    37,    61,   264,
    -559,  -559,  -559,   265,   266,   267,   270,   273,  -559,  -559,
      70,    84,    31,   274,  -559,  -559,    20,   124,   276,   383,
      67,   -12,   575,   575,  -559,   575,   575,  -559,  -559,   718,
     575,   575,   575,   575,   400,   575,   401,   402,   404,   575,
     575,   575,   575,  -559,  -559,   575,   575,   575,   575,   575,
     575,   575,   575,   575,   575,   575,   575,   575,   575,   575,
     575,   575,   575,   575,   575,   575,   575,  1402,   406,   409,
    -559,   410,   575,   575,  1402,   103,   411,  -559,   416,  -559,
     295,   307,  -559,   421,  -559,   -77,  1402,   357,  -559,  -559,
    -559,  -559,  -559,  -559,  -559,  -559,   424,  -559,  -559,   721,
     389,     6,  -559,  -559,  -559,  -559,  -559,  -559,  -559,   204,
    -559,   204,   326,  -559,  -559,  -559,  -559,   239,  -559,    60,
    -559,  -559,  -559,   -19,  1402,  1402,   758,  1402,  1402,  -559,
     860,   880,  1193,   900,   305,   920,   308,   309,   310,  1213,
    1233,   940,  1271,  1344,   637,  1251,  1320,   455,  1357,   422,
     422,   225,   225,   225,   225,   199,   199,   192,   192,  -559,
    -559,  -559,  1402,  1402,  1402,  -559,  -559,  -559,  1402,  1402,
    -559,  -559,  -559,  -559,   -27,     0,     8,   387,  -559,  -559,
     -45,   471,   523,   471,   575,   332,  -559,     4,   418,  -559,
     318,  -559,  -559,  -559,  -559,  -559,  -559,  -559,   312,  -559,
    -559,  -559,  -559,   575,  -559,  -559,  -559,  -559,  -559,  -559,
     575,   575,  -559,   439,   575,   334,   428,  -559,  -559,  -559,
     211,   425,  1302,   445,   377,  -559,  1382,   388,  -559,  1402,
      24,   467,  -559,   468,     3,  -559,   390,  -559,  -559,   971,
     991,  1011,   364,  1402,     8,   440,   -27,   -27,  -559,  -559,
    -559,  -559,  -559,  -559,   366,   575,   139,   478,  -559,   458,
     405,  -559,  -559,   377,   446,   462,   463,  -559,   376,  -559,
    -559,  -559,   498,   380,  -559,  -559,  -559,  -559,  -559,   381,
     334,  -559,  1031,  -559,   575,   469,  -559,   405,  -559,   575,
      24,   575,   386,  -559,  -559,   429,     8,   461,   198,  1051,
     575,   464,  -559,   158,  1082,  -559,  1102,  -559,  -559,   510,
    -559,  -559,   481,   503,  -559,  1122,  -559,   474,  -559,  -559,
      24,  -559,   575,  -559,  -559,   588,  -559,  -559,  -559,  1142,
     337,  -559,  -559,  -559,  -559,  -559,  -559,  -559,   491,   492,
    -559,  -559,   493,   494,  -559,    12,  -559,   495,  -559,  -559,
    -559,   588,   482,   496,    93,  -559,   499,    65,  -559,  -559,
      95,  -559,   501,  -559,   -47,    12,  -559,  -559,  -559,  -559,
      50,   519,   413,   504,   423,   430,  -559,    11,  -559,    15,
      72,  -559,    12,   113,   524,   431,  -559,  -559,  -559,  -559,
      95,  -559,  -559,   432,   436,    95,  -559,  -559,  -559,   544,
     459,  -559,   505,   524,    19,  -559,  -559,  -559,   527,  -559,
     588,   459,    95,   551,    30,   512,  -559,   438,  -559,  -559,
     562,  -559,  -559,    30,  -559,  -559,  -559,   380,  -559,  -559,
    -559,    30,   380,  -559,  -559,   380,  -559
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -559,  -559,   -55,  -559,  -559,  -559,  -559,   363,  -559,  -559,
    -559,  -559,  -559,  -559,   450,  -559,  -559,   391,  -559,  -559,
    -559,   373,  -559,   106,  -172,  -467,  -477,     7,    35,    26,
    -559,  -559,    51,  -559,    -9,  -559,   -14,  -502,  -559,    52,
    -468,  -188,  -559,  -559,  -259,  -559,  -559,  -559,  -559,  -559,
     170,  -559,  -559,  -559,  -559,  -559,  -559,  -171,   -89,  -559,
     -62,    -4,   145,   123,  -559,  -559,  -559,  -559,  -559,  -559,
    -559,  -559,  -559,  -559,  -559,  -559,  -559,  -559,  -382,   237,
    -559,  -559,    21,  -558,  -559,  -559,  -559,  -559,  -559,  -559,
    -559,  -559,  -559,  -559,  -425,  -559,  -559,  -559,  -559,  -559,
     407,   -15,  -559,   506,  -156,  -559,  -559,   131
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -280
static const yytype_int16 yytable[] =
{
     173,   147,   289,   291,   271,    88,   124,   412,   412,   175,
     181,   157,   260,   261,   103,   573,   541,   250,   166,   541,
     437,   293,   371,   541,   295,   296,   511,   511,   445,   267,
     511,   257,    18,   379,   511,   192,   599,   534,   603,    18,
     358,   273,   202,   512,   512,   485,   600,   512,   543,   359,
     611,   512,   184,   185,   566,   187,   189,    14,   369,   542,
     542,   198,    17,   534,   295,   296,   207,   208,   543,   541,
     554,   257,   398,   558,   205,   507,   541,   268,   287,    80,
     511,   399,   561,   223,   224,   577,   247,   511,    81,   274,
     269,    82,   576,   554,   254,    34,   493,   512,    19,   541,
      87,   609,   567,   558,   512,    19,   350,   351,   586,   613,
     511,   417,   266,   270,    99,   263,   446,   576,   264,    44,
     396,   374,   534,   263,   101,   597,   264,   512,   194,   195,
     450,   370,   100,   413,   413,   523,   523,   188,   251,   525,
     304,   305,   575,   307,   308,   297,   592,   298,   310,   311,
     312,   313,   258,   315,   551,   552,   292,   319,   320,   321,
     322,   203,   267,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   297,   276,   377,   277,   553,
     348,   349,   258,   206,   302,   287,   553,   288,   395,   102,
     428,   429,   430,   431,   432,     1,     2,     3,   103,   287,
     498,   290,   378,   122,   126,   127,     4,    81,   119,   120,
      82,    83,    84,   269,   241,   242,   243,   105,    34,   239,
     240,   241,   242,   243,   372,   436,   373,   492,   287,   106,
     578,   128,   129,   375,   376,   107,   270,   108,   130,   131,
     132,   109,    44,   237,   238,   239,   240,   241,   242,   243,
     133,   134,   123,   110,   135,   475,   433,   111,   112,   113,
     459,   460,   428,   429,   430,   431,   432,   136,   114,   116,
     117,   118,   137,   138,   139,   140,   141,   142,    88,   126,
     127,   125,   245,   148,   149,   151,   154,   155,   156,   402,
     406,   402,   409,   158,   159,   165,   160,   162,   164,   143,
     167,   271,   168,   169,   170,   144,   128,   129,   178,   171,
     179,   419,   181,   130,   131,   132,   183,   186,   420,   421,
     192,   199,   423,   201,   248,   133,   134,   197,   433,   135,
     249,   145,   146,    89,    90,    91,    92,    93,    94,    95,
      96,    97,   136,   255,   210,   262,   211,   137,   138,   139,
     140,   141,   142,    89,    90,    91,    92,    93,    94,    95,
      96,    97,   212,   462,   -98,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   143,   488,   222,   259,   256,   301,
     144,   278,   282,   283,   284,   126,   127,   285,   252,   265,
     286,   294,   479,   300,   314,   316,   317,   484,   318,   486,
     345,   246,   508,   346,   347,   352,   145,   146,   495,   610,
     353,   355,   128,   129,   614,   357,   367,   616,   365,   130,
     131,   132,   385,   356,   416,   387,   388,   389,   397,   418,
     509,   133,   134,   422,   426,   135,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   411,   136,   565,
     424,   438,   435,   137,   138,   139,   140,   141,   142,   439,
     442,   448,   449,   452,   126,   127,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     143,   457,   458,   461,   463,   464,   144,   468,   465,   469,
     471,   128,   129,   473,   474,   287,   480,   476,   400,   131,
     132,   401,   487,   491,   489,   496,   501,   253,   502,   503,
     133,   134,   145,   146,   135,   506,   126,   127,   537,   538,
     539,   540,   545,   548,   547,   568,   436,   136,   560,   405,
     569,   570,   137,   138,   139,   140,   141,   142,   587,   579,
     571,   588,   593,   128,   129,   598,   590,   572,   581,  -109,
     130,   131,   132,   585,   602,   604,   606,   306,   200,   143,
     299,   279,   133,   134,   483,   144,   135,   584,   126,   127,
     563,   595,   546,   574,   451,   605,   549,   596,   467,   136,
     482,   477,   510,   172,   137,   138,   139,   140,   141,   142,
     408,   145,   146,   511,   591,   128,   129,     0,   303,     0,
       0,     0,   130,   131,   132,     0,     0,     0,     0,     0,
     512,   143,     0,     0,   133,   134,     0,   144,   135,   513,
     514,   515,   516,   517,     0,     0,   518,     0,     0,     0,
       0,   136,     0,     0,    22,     0,   137,   138,   139,   140,
     141,   142,     0,   145,   146,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   519,   143,   520,     0,     0,     0,   521,   144,
       0,     0,    44,     0,     0,     0,    23,    24,     0,     0,
       0,     0,     0,     0,    25,    26,    27,    28,    29,    30,
       0,     0,     0,     0,     0,   145,   146,    31,    32,    33,
      34,   522,   523,     0,   524,     0,   525,    35,    36,    37,
      38,    39,    40,     0,     0,    22,     0,    41,    42,    43,
      59,     0,     0,   225,    44,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,     0,     0,    45,     0,   366,     0,     0,
       0,  -279,    59,     0,    60,     0,    46,    23,    24,     0,
       0,     0,     0,     0,     0,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    61,     0,     0,     0,    31,    32,
      33,    34,     0,     0,   380,     0,    60,     0,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,    41,    42,
      43,     0,     0,     0,     0,    44,    61,    62,     0,     0,
       0,    63,    64,    65,    66,    67,   -42,    68,    69,    70,
       0,    71,    72,    73,    74,    75,    45,     0,     0,     0,
      76,    77,    78,     0,     0,   309,     0,    46,     0,    62,
       0,     0,     0,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    71,    72,    73,    74,    75,     0,     0,
       0,     0,    76,    77,    78,   225,     0,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   225,     0,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   225,     0,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   225,     0,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   225,     0,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   225,   381,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   225,   382,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   225,   384,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   225,   386,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   225,   392,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   225,   454,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   225,   455,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   225,   456,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   225,   478,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   225,   494,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   225,   499,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   225,   500,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   225,   504,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,     0,   536,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   225,   244,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   225,   383,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,     0,   390,   436,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,     0,     0,     0,   391,   225,
     394,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,     0,     0,     0,     0,   393,   225,   441,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   225,     0,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243
};

static const yytype_int16 yycheck[] =
{
      89,    63,   190,   191,   176,    20,    61,     4,     4,    98,
       4,    73,   168,   169,     4,     4,     4,     4,    80,     4,
     402,   192,   281,     4,     4,     5,    15,    15,     4,     4,
      15,     4,    51,    52,    15,     4,     6,   505,   596,    51,
     117,     4,     6,    32,    32,   470,    16,    32,   525,   126,
     608,    32,   107,   108,     4,   110,   111,     0,    52,    48,
      48,   116,     4,   531,     4,     5,   128,   129,   545,     4,
     537,     4,   117,   540,     6,   500,     4,    52,   125,     6,
      15,   126,   129,   145,   146,   562,   148,    15,   115,    52,
      65,   118,   559,   560,   156,    70,   478,    32,   117,     4,
      51,   603,    52,   570,    32,   117,     3,     4,   575,   611,
      15,   370,   174,    88,    51,   115,    92,   584,   118,    94,
     120,   292,   590,   115,    37,   592,   118,    32,     4,     5,
     127,   125,    51,   130,   130,   124,   124,   127,   125,   128,
     202,   203,   127,   205,   206,   125,   127,   127,   210,   211,
     212,   213,   125,   215,   536,    90,   125,   219,   220,   221,
     222,   125,     4,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   125,   125,   127,   127,   124,
     252,   253,   125,   125,   127,   125,   124,   127,   354,    37,
      61,    62,    63,    64,    65,   111,   112,   113,     4,   125,
      52,   127,   301,   121,     3,     4,   122,   115,   125,   126,
     118,   119,   120,    65,    32,    33,    34,    51,    70,    30,
      31,    32,    33,    34,   289,    37,   291,    39,   125,    37,
     127,    30,    31,     4,     5,    37,    88,    37,    37,    38,
      39,    37,    94,    28,    29,    30,    31,    32,    33,    34,
      49,    50,     4,    37,    53,   453,   127,    37,    37,    37,
     426,   427,    61,    62,    63,    64,    65,    66,    37,    37,
      37,    37,    71,    72,    73,    74,    75,    76,   303,     3,
       4,     4,     6,     4,     4,     4,     4,     4,     4,   361,
     362,   363,   364,     4,     3,   100,     4,     4,     4,    98,
       4,   483,    16,    16,    52,   104,    30,    31,     4,   126,
       4,   383,     4,    37,    38,    39,     4,     4,   390,   391,
       4,     4,   394,    51,   125,    49,    50,    37,   127,    53,
     125,   130,   131,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    66,   125,    37,   126,    37,    71,    72,    73,
      74,    75,    76,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    37,   435,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    98,   474,    37,    51,   125,     6,
     104,   127,   127,   127,   127,     3,     4,   127,     6,    52,
     127,   127,   464,   127,     4,     4,     4,   469,     4,   471,
       4,   125,   501,     4,     4,     4,   130,   131,   480,   607,
       4,   126,    30,    31,   612,     4,    37,   615,     4,    37,
      38,    39,   127,   126,    16,   127,   127,   127,    51,   127,
     502,    49,    50,     4,    16,    53,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,   125,    66,   548,
     126,    16,    37,    71,    72,    73,    74,    75,    76,    92,
      82,     4,     4,    83,     3,     4,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      98,   127,    52,   127,    16,    37,   104,    51,    93,    37,
      37,    30,    31,   127,     6,   125,    37,   126,    37,    38,
      39,    40,   126,    52,    85,    51,     6,   125,    37,    16,
      49,    50,   130,   131,    53,    51,     3,     4,    37,    37,
      37,    37,    37,    37,    52,    16,    37,    66,    37,    16,
     127,    37,    71,    72,    73,    74,    75,    76,     4,    25,
     127,    92,    25,    30,    31,     4,    51,   127,   127,   127,
      37,    38,    39,   127,    52,   127,     4,   204,   118,    98,
     197,   180,    49,    50,   468,   104,    53,   570,     3,     4,
     545,   590,   531,   557,   414,   599,   534,   591,   443,    66,
     467,   460,     4,    87,    71,    72,    73,    74,    75,    76,
     363,   130,   131,    15,   583,    30,    31,    -1,   201,    -1,
      -1,    -1,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,
      32,    98,    -1,    -1,    49,    50,    -1,   104,    53,    41,
      42,    43,    44,    45,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    66,    -1,    -1,     4,    -1,    71,    72,    73,    74,
      75,    76,    -1,   130,   131,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    84,    98,    86,    -1,    -1,    -1,    90,   104,
      -1,    -1,    94,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,    59,
      -1,    -1,    -1,    -1,    -1,   130,   131,    67,    68,    69,
      70,   123,   124,    -1,   126,    -1,   128,    77,    78,    79,
      80,    81,    82,    -1,    -1,     4,    -1,    87,    88,    89,
       4,    -1,    -1,    15,    94,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    -1,   115,    -1,    36,    -1,    -1,
      -1,   121,     4,    -1,    38,    -1,   126,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    55,    56,    57,    58,
      59,    -1,    -1,    -1,    58,    -1,    -1,    -1,    67,    68,
      69,    70,    -1,    -1,    36,    -1,    38,    -1,    77,    78,
      79,    80,    81,    82,    -1,    -1,    -1,    -1,    87,    88,
      89,    -1,    -1,    -1,    -1,    94,    58,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      -1,   105,   106,   107,   108,   109,   115,    -1,    -1,    -1,
     114,   115,   116,    -1,    -1,   127,    -1,   126,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    99,    -1,   101,
     102,   103,    -1,   105,   106,   107,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,   116,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    15,   127,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    15,   127,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    15,   127,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    15,   127,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    15,   127,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,   127,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,   127,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,   127,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,   127,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,   127,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,   127,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    15,   127,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    15,   127,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,   127,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    15,   125,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,   125,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,   125,    37,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    -1,    -1,   125,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    -1,    -1,    -1,    -1,   125,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    15,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   111,   112,   113,   122,   133,   147,   148,   137,   138,
     135,   136,   228,   229,     0,   149,   139,     4,    51,   117,
     232,   233,     4,    46,    47,    54,    55,    56,    57,    58,
      59,    67,    68,    69,    70,    77,    78,    79,    80,    81,
      82,    87,    88,    89,    94,   115,   126,   150,   154,   156,
     172,   174,   183,   184,   186,   188,   220,   230,   231,     4,
      38,    58,    91,    95,    96,    97,    98,    99,   101,   102,
     103,   105,   106,   107,   108,   109,   114,   115,   116,   140,
       6,   115,   118,   119,   120,   235,   236,    51,   233,     6,
       7,     8,     9,    10,    11,    12,    13,    14,   170,    51,
      51,    37,    37,     4,   134,    51,    37,    37,    37,    37,
      37,    37,    37,    37,    37,   151,    37,    37,    37,   125,
     126,   171,   121,     4,   134,     4,     3,     4,    30,    31,
      37,    38,    39,    49,    50,    53,    66,    71,    72,    73,
      74,    75,    76,    98,   104,   130,   131,   192,     4,     4,
     144,     4,   143,   142,     4,     4,     4,   192,     4,     3,
       4,   145,     4,   146,     4,   100,   192,     4,    16,    16,
      52,   126,   235,   190,   191,   190,   155,   221,     4,     4,
     152,     4,   176,     4,   134,   134,     4,   134,   127,   134,
     185,   187,     4,   189,     4,     5,   153,    37,   134,     4,
     146,    51,     6,   125,   141,     6,   125,   192,   192,   192,
      37,    37,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,   192,   192,    15,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,   125,     6,   125,   192,   125,   125,
       4,   125,     6,   125,   192,   125,   125,     4,   125,    51,
     236,   236,   126,   115,   118,    52,   192,     4,    52,    65,
      88,   156,   196,     4,    52,   222,   125,   127,   127,   149,
     177,   175,   127,   127,   127,   127,   127,   125,   127,   173,
     127,   173,   125,   189,   127,     4,     5,   125,   127,   153,
     127,     6,   127,   232,   192,   192,   139,   192,   192,   127,
     192,   192,   192,   192,     4,   192,     4,     4,     4,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,   192,   192,   192,   192,   192,
     192,   192,   192,   192,   192,     4,     4,     4,   192,   192,
       3,     4,     4,     4,   238,   126,   126,     4,   117,   126,
     234,   197,   202,   207,   223,     4,    36,    37,   180,    52,
     125,   176,   134,   134,   189,     4,     5,   127,   190,    52,
      36,   127,   127,   125,   127,   127,   127,   127,   127,   127,
     125,   125,   127,   125,    16,   236,   120,    51,   117,   126,
      37,    40,   192,   210,   211,    16,   192,   212,   211,   192,
     225,   125,     4,   130,   181,   182,    16,   176,   127,   192,
     192,   192,     4,   192,   126,   239,    16,   237,    61,    62,
      63,    64,    65,   127,   209,    37,    37,   210,    16,    92,
     194,    16,    82,   213,   208,     4,    92,   226,     4,     4,
     127,   182,    83,   178,   127,   127,   127,   127,    52,   236,
     236,   127,   192,    16,    37,    93,   195,   194,    51,    37,
     227,    37,   224,   127,     6,   173,   126,   239,   127,   192,
      37,   198,   195,   155,   192,   226,   192,   126,   190,    85,
     179,    52,    39,   210,   127,   192,    51,   203,    52,   127,
     127,     6,    37,    16,   127,   199,    51,   226,   190,   192,
       4,    15,    32,    41,    42,    43,    44,    45,    48,    84,
      86,    90,   123,   124,   126,   128,   157,   158,   161,   162,
     164,   165,   166,   167,   172,   204,   127,    37,    37,    37,
      37,     4,    48,   158,   160,    37,   164,    52,    37,   171,
     216,   210,    90,   124,   157,   168,   190,   163,   157,   159,
      37,   129,   173,   160,   200,   190,     4,    52,    16,   127,
      37,   127,   127,     4,   161,   127,   157,   158,   127,    25,
     214,   127,   217,   205,   159,   127,   157,     4,    92,   193,
      51,   214,   127,    25,   215,   166,   193,   157,     4,     6,
      16,   169,    52,   215,   127,   168,     4,   201,   218,   169,
     173,   215,   206,   169,   173,   219,   173
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
        case 7:

/* Line 1455 of yacc.c  */
#line 173 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_defsym(); }
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 175 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate();
		  lang_add_assignment(exp_assop((yyvsp[(3) - (4)].token),(yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)));
		}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 183 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_mri_script ();
		  PUSH_ERROR (_("MRI style script"));
		}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 188 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate ();
		  mri_draw_tree ();
		  POP_ERROR ();
		}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 203 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			einfo(_("%P%F: unrecognised keyword in MRI style script '%s'\n"),(yyvsp[(1) - (1)].name));
			}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 206 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			config.map_filename = "-";
			}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 212 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_public((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree)); }
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 214 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_public((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree)); }
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 216 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_public((yyvsp[(2) - (3)].name), (yyvsp[(3) - (3)].etree)); }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 218 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_format((yyvsp[(2) - (2)].name)); }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 220 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_output_section((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree));}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 222 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_output_section((yyvsp[(2) - (3)].name), (yyvsp[(3) - (3)].etree));}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 224 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_output_section((yyvsp[(2) - (4)].name), (yyvsp[(4) - (4)].etree));}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 226 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_align((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 228 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_align((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 230 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_alignmod((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 232 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_alignmod((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].etree)); }
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 236 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_name((yyvsp[(2) - (2)].name)); }
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 238 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_alias((yyvsp[(2) - (4)].name),(yyvsp[(4) - (4)].name),0);}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 240 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_alias ((yyvsp[(2) - (4)].name), 0, (int) (yyvsp[(4) - (4)].bigint).integer); }
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 242 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_base((yyvsp[(2) - (2)].etree)); }
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 244 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_truncate ((unsigned int) (yyvsp[(2) - (2)].bigint).integer); }
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 248 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 250 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 252 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_entry ((yyvsp[(2) - (2)].name), FALSE); }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 257 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_order((yyvsp[(3) - (3)].name)); }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 258 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_order((yyvsp[(2) - (2)].name)); }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 264 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_load((yyvsp[(1) - (1)].name)); }
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 265 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_load((yyvsp[(3) - (3)].name)); }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 270 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_only_load((yyvsp[(1) - (1)].name)); }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 272 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { mri_only_load((yyvsp[(3) - (3)].name)); }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 276 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.name) = NULL; }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 283 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlang_add_undef ((yyvsp[(1) - (1)].name)); }
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 285 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlang_add_undef ((yyvsp[(2) - (2)].name)); }
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 287 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlang_add_undef ((yyvsp[(3) - (3)].name)); }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 291 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
	 ldlex_both();
	}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 295 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
	ldlex_popstate();
	}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 320 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_target((yyvsp[(3) - (4)].name)); }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 322 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldfile_add_library_path ((yyvsp[(3) - (4)].name), FALSE); }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 324 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_output((yyvsp[(3) - (4)].name), 1); }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 326 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_output_format ((yyvsp[(3) - (4)].name), (char *) NULL,
					    (char *) NULL, 1); }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 329 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_output_format ((yyvsp[(3) - (8)].name), (yyvsp[(5) - (8)].name), (yyvsp[(7) - (8)].name), 1); }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 331 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldfile_set_output_arch((yyvsp[(3) - (4)].name)); }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 333 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { command_line.force_common_definition = TRUE ; }
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 335 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { command_line.inhibit_common_definition = TRUE ; }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 338 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_enter_group (); }
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 340 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_leave_group (); }
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 342 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_map((yyvsp[(3) - (4)].name)); }
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 344 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_script (); ldfile_open_command_file((yyvsp[(2) - (2)].name)); }
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 346 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); }
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 348 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_add_nocrossref ((yyvsp[(3) - (4)].nocrossref));
		}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 356 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(1) - (1)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 359 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(3) - (3)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 362 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(2) - (2)].name),lang_input_file_is_search_file_enum,
				 (char *)NULL); }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 365 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(1) - (1)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 368 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(3) - (3)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 371 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_input_file((yyvsp[(2) - (2)].name),lang_input_file_is_l_enum,
				 (char *)NULL); }
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 387 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_add_entry ((yyvsp[(3) - (4)].name), FALSE); }
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 395 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.cname) = (yyvsp[(1) - (1)].name);
			}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 399 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.cname) = "*";
			}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 403 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.cname) = "?";
			}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 410 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(1) - (1)].cname);
			  (yyval.wildcard).sorted = FALSE;
			  (yyval.wildcard).exclude_name_list = NULL;
			}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 416 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(5) - (5)].cname);
			  (yyval.wildcard).sorted = FALSE;
			  (yyval.wildcard).exclude_name_list = (yyvsp[(3) - (5)].name_list);
			}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 422 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(3) - (4)].cname);
			  (yyval.wildcard).sorted = TRUE;
			  (yyval.wildcard).exclude_name_list = NULL;
			}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 428 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.wildcard).name = (yyvsp[(7) - (8)].cname);
			  (yyval.wildcard).sorted = TRUE;
			  (yyval.wildcard).exclude_name_list = (yyvsp[(5) - (8)].name_list);
			}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 437 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = (yyvsp[(2) - (2)].cname);
			  tmp->next = (yyvsp[(1) - (2)].name_list);
			  (yyval.name_list) = tmp;
			}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 446 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  struct name_list *tmp;
			  tmp = (struct name_list *) xmalloc (sizeof *tmp);
			  tmp->name = (yyvsp[(1) - (1)].cname);
			  tmp->next = NULL;
			  (yyval.name_list) = tmp;
			}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 457 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = (yyvsp[(1) - (3)].wildcard_list);
			  tmp->spec = (yyvsp[(3) - (3)].wildcard);
			  (yyval.wildcard_list) = tmp;
			}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 466 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  struct wildcard_list *tmp;
			  tmp = (struct wildcard_list *) xmalloc (sizeof *tmp);
			  tmp->next = NULL;
			  tmp->spec = (yyvsp[(1) - (1)].wildcard);
			  (yyval.wildcard_list) = tmp;
			}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 477 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  struct wildcard_spec tmp;
			  tmp.name = (yyvsp[(1) - (1)].name);
			  tmp.exclude_name_list = NULL;
			  tmp.sorted = FALSE;
			  lang_add_wild (&tmp, NULL, ldgram_had_keep);
			}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 485 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_add_wild (NULL, (yyvsp[(2) - (3)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 489 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_add_wild (&(yyvsp[(1) - (4)].wildcard), (yyvsp[(3) - (4)].wildcard_list), ldgram_had_keep);
			}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 497 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldgram_had_keep = TRUE; }
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 499 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldgram_had_keep = FALSE; }
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 505 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
 		lang_add_attribute(lang_object_symbols_statement_enum);
	      	}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 510 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {

		  lang_add_attribute(lang_constructors_statement_enum);
		}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 515 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  constructors_sorted = TRUE;
		  lang_add_attribute (lang_constructors_statement_enum);
		}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 521 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_add_data ((int) (yyvsp[(1) - (4)].integer), (yyvsp[(3) - (4)].etree));
			}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 526 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_add_fill ((yyvsp[(3) - (4)].fill));
			}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 543 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 545 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 547 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 549 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 551 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].token); }
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 556 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.fill) = exp_get_fill ((yyvsp[(1) - (1)].etree),
				     0,
				     "fill value",
				     lang_first_phase_enum);
		}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 566 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.fill) = (yyvsp[(2) - (2)].fill); }
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 567 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.fill) = (fill_type *) 0; }
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 572 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '+'; }
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 574 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '-'; }
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 576 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '*'; }
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 578 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '/'; }
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 580 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = LSHIFT; }
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 582 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = RSHIFT; }
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 584 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '&'; }
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 586 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.token) = '|'; }
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 596 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_add_assignment (exp_assop ((yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].name), (yyvsp[(3) - (3)].etree)));
		}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 600 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_add_assignment (exp_assop ('=', (yyvsp[(1) - (3)].name),
						  exp_binop ((yyvsp[(2) - (3)].token),
							     exp_nameop (NAME,
									 (yyvsp[(1) - (3)].name)),
							     (yyvsp[(3) - (3)].etree))));
		}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 608 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_add_assignment (exp_provide ((yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].etree)));
		}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 630 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { region = lang_memory_region_lookup((yyvsp[(1) - (1)].name)); }
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 633 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 638 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { region->current =
		 region->origin =
		 exp_get_vma((yyvsp[(3) - (3)].etree), 0L,"origin", lang_first_phase_enum);
}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 646 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { region->length = exp_get_vma((yyvsp[(3) - (3)].etree),
					       ~((bfd_vma)0),
					       "length",
					       lang_first_phase_enum);
		}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 655 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { /* dummy action to avoid bison 1.25 error message */ }
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 666 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_set_flags (region, (yyvsp[(1) - (1)].name), 0); }
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 668 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_set_flags (region, (yyvsp[(2) - (2)].name), 1); }
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 673 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_startup((yyvsp[(3) - (4)].name)); }
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 679 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldemul_hll((char *)NULL); }
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 684 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldemul_hll((yyvsp[(3) - (3)].name)); }
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 686 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldemul_hll((yyvsp[(1) - (1)].name)); }
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 694 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldemul_syslib((yyvsp[(3) - (3)].name)); }
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 700 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_float(TRUE); }
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 702 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { lang_float(FALSE); }
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 707 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.nocrossref) = NULL;
		}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 711 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = (yyvsp[(1) - (2)].name);
		  n->next = (yyvsp[(2) - (2)].nocrossref);
		  (yyval.nocrossref) = n;
		}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 720 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  struct lang_nocrossref *n;

		  n = (struct lang_nocrossref *) xmalloc (sizeof *n);
		  n->name = (yyvsp[(1) - (3)].name);
		  n->next = (yyvsp[(3) - (3)].nocrossref);
		  (yyval.nocrossref) = n;
		}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 730 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_expression(); }
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 732 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate(); (yyval.etree)=(yyvsp[(2) - (2)].etree);}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 737 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop('-', (yyvsp[(2) - (2)].etree)); }
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 739 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(2) - (3)].etree); }
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 741 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop((int) (yyvsp[(1) - (4)].integer),(yyvsp[(3) - (4)].etree)); }
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 743 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop('!', (yyvsp[(2) - (2)].etree)); }
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 745 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(2) - (2)].etree); }
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 747 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop('~', (yyvsp[(2) - (2)].etree));}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 750 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('*', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 752 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('/', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 754 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('%', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 756 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('+', (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 758 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('-' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 760 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(LSHIFT , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 762 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(RSHIFT , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 764 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(EQ , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 766 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(NE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 768 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(LE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 770 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(GE , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 772 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('<' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 774 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('>' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 776 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('&' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 778 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('^' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 780 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop('|' , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 782 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_trinop('?' , (yyvsp[(1) - (5)].etree), (yyvsp[(3) - (5)].etree), (yyvsp[(5) - (5)].etree)); }
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 784 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(ANDAND , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 786 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop(OROR , (yyvsp[(1) - (3)].etree), (yyvsp[(3) - (3)].etree)); }
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 788 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(DEFINED, (yyvsp[(3) - (4)].name)); }
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 790 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_bigintop ((yyvsp[(1) - (1)].bigint).integer, (yyvsp[(1) - (1)].bigint).str); }
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 792 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(SIZEOF_HEADERS,0); }
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 795 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(SIZEOF,(yyvsp[(3) - (4)].name)); }
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 797 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(ADDR,(yyvsp[(3) - (4)].name)); }
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 799 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(LOADADDR,(yyvsp[(3) - (4)].name)); }
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 801 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop(ABSOLUTE, (yyvsp[(3) - (4)].etree)); }
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 803 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop(ALIGN_K,(yyvsp[(3) - (4)].etree)); }
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 805 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop (DATA_SEGMENT_ALIGN, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree)); }
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 807 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop(DATA_SEGMENT_END, (yyvsp[(3) - (4)].etree)); }
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 809 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_unop(ALIGN_K,(yyvsp[(3) - (4)].etree)); }
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 811 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_nameop(NAME,(yyvsp[(1) - (1)].name)); }
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 813 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop (MAX_K, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree) ); }
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 815 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_binop (MIN_K, (yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].etree) ); }
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 817 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = exp_assert ((yyvsp[(3) - (6)].etree), (yyvsp[(5) - (6)].name)); }
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 822 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.name) = (yyvsp[(3) - (3)].name); }
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 823 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.name) = 0; }
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 827 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (4)].etree); }
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 828 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = 0; }
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 832 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (4)].etree); }
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 833 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = 0; }
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 836 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_expression(); }
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 839 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); ldlex_script (); }
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 841 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_enter_output_section_statement((yyvsp[(1) - (7)].name), (yyvsp[(3) - (7)].etree),
							      sectype,
							      0, 0, (yyvsp[(5) - (7)].etree), (yyvsp[(4) - (7)].etree));
			}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 847 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 849 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate ();
		  lang_leave_output_section_statement ((yyvsp[(15) - (15)].fill), (yyvsp[(12) - (15)].name), (yyvsp[(14) - (15)].section_phdr), (yyvsp[(13) - (15)].name));
		}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 854 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 856 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_expression (); }
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 858 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); ldlex_script (); }
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 860 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  lang_enter_overlay ((yyvsp[(3) - (8)].etree), (yyvsp[(6) - (8)].etree));
			}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 865 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 867 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  ldlex_popstate ();
			  lang_leave_overlay ((yyvsp[(5) - (16)].etree), (int) (yyvsp[(4) - (16)].integer),
					      (yyvsp[(16) - (16)].fill), (yyvsp[(13) - (16)].name), (yyvsp[(15) - (16)].section_phdr), (yyvsp[(14) - (16)].name));
			}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 877 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_expression (); }
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 879 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate ();
		  lang_add_assignment (exp_assop ('=', ".", (yyvsp[(3) - (3)].etree)));
		}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 887 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = noload_section; }
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 888 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = dsect_section; }
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 889 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = copy_section; }
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 890 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = info_section; }
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 891 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = overlay_section; }
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 896 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = normal_section; }
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 897 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { sectype = normal_section; }
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 901 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(1) - (3)].etree); }
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 902 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (etree_type *)NULL;  }
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 907 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (6)].etree); }
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 909 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(3) - (10)].etree); }
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 913 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (yyvsp[(1) - (2)].etree); }
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 914 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.etree) = (etree_type *) NULL;  }
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 919 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = 0; }
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 921 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.integer) = 1; }
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 926 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.name) = (yyvsp[(2) - (2)].name); }
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 927 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { (yyval.name) = "*default*"; }
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 932 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.section_phdr) = NULL;
		}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 936 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  struct lang_output_section_phdr_list *n;

		  n = ((struct lang_output_section_phdr_list *)
		       xmalloc (sizeof *n));
		  n->name = (yyvsp[(3) - (3)].name);
		  n->used = FALSE;
		  n->next = (yyvsp[(1) - (3)].section_phdr);
		  (yyval.section_phdr) = n;
		}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 952 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  ldlex_script ();
			  lang_enter_overlay_section ((yyvsp[(2) - (2)].name));
			}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 957 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); ldlex_expression (); }
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 959 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  ldlex_popstate ();
			  lang_leave_overlay_section ((yyvsp[(9) - (9)].fill), (yyvsp[(8) - (9)].section_phdr));
			}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 976 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_expression (); }
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 977 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    { ldlex_popstate (); }
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 979 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_new_phdr ((yyvsp[(1) - (6)].name), (yyvsp[(3) - (6)].etree), (yyvsp[(4) - (6)].phdr).filehdr, (yyvsp[(4) - (6)].phdr).phdrs, (yyvsp[(4) - (6)].phdr).at,
				 (yyvsp[(4) - (6)].phdr).flags);
		}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 987 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.etree) = (yyvsp[(1) - (1)].etree);

		  if ((yyvsp[(1) - (1)].etree)->type.node_class == etree_name
		      && (yyvsp[(1) - (1)].etree)->type.node_code == NAME)
		    {
		      const char *s;
		      unsigned int i;
		      static const char * const phdr_types[] =
			{
			  "PT_NULL", "PT_LOAD", "PT_DYNAMIC",
			  "PT_INTERP", "PT_NOTE", "PT_SHLIB",
			  "PT_PHDR", "PT_TLS"
			};

		      s = (yyvsp[(1) - (1)].etree)->name.name;
		      for (i = 0;
			   i < sizeof phdr_types / sizeof phdr_types[0];
			   i++)
			if (strcmp (s, phdr_types[i]) == 0)
			  {
			    (yyval.etree) = exp_intop (i);
			    break;
			  }
		      if (i == sizeof phdr_types / sizeof phdr_types[0])
			{
			  if (strcmp (s, "PT_GNU_EH_FRAME") == 0)
			    (yyval.etree) = exp_intop (0x6474e550);
			  else if (strcmp (s, "PT_GNU_STACK") == 0)
			    (yyval.etree) = exp_intop (0x6474e551);
			  else
			    {
			      einfo (_("\
%X%P:%S: unknown phdr type `%s' (try integer literal)\n"),
				     s);
			      (yyval.etree) = exp_intop (0);
			    }
			}
		    }
		}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1031 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  memset (&(yyval.phdr), 0, sizeof (struct phdr_info));
		}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1035 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.phdr) = (yyvsp[(3) - (3)].phdr);
		  if (strcmp ((yyvsp[(1) - (3)].name), "FILEHDR") == 0 && (yyvsp[(2) - (3)].etree) == NULL)
		    (yyval.phdr).filehdr = TRUE;
		  else if (strcmp ((yyvsp[(1) - (3)].name), "PHDRS") == 0 && (yyvsp[(2) - (3)].etree) == NULL)
		    (yyval.phdr).phdrs = TRUE;
		  else if (strcmp ((yyvsp[(1) - (3)].name), "FLAGS") == 0 && (yyvsp[(2) - (3)].etree) != NULL)
		    (yyval.phdr).flags = (yyvsp[(2) - (3)].etree);
		  else
		    einfo (_("%X%P:%S: PHDRS syntax error at `%s'\n"), (yyvsp[(1) - (3)].name));
		}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1047 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.phdr) = (yyvsp[(5) - (5)].phdr);
		  (yyval.phdr).at = (yyvsp[(3) - (5)].etree);
		}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1055 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.etree) = NULL;
		}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1059 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.etree) = (yyvsp[(2) - (3)].etree);
		}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1067 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_version_file ();
		  PUSH_ERROR (_("VERSION script"));
		}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1072 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate ();
		  POP_ERROR ();
		}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1081 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_version_script ();
		}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1085 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  ldlex_popstate ();
		}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1097 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_register_vers_node (NULL, (yyvsp[(2) - (4)].versnode), NULL);
		}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1101 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_register_vers_node ((yyvsp[(1) - (5)].name), (yyvsp[(3) - (5)].versnode), NULL);
		}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1105 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  lang_register_vers_node ((yyvsp[(1) - (6)].name), (yyvsp[(3) - (6)].versnode), (yyvsp[(5) - (6)].deflist));
		}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1112 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.deflist) = lang_add_vers_depend (NULL, (yyvsp[(1) - (1)].name));
		}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1116 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.deflist) = lang_add_vers_depend ((yyvsp[(1) - (2)].deflist), (yyvsp[(2) - (2)].name));
		}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1123 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node (NULL, NULL);
		}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1127 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(1) - (2)].versyms), NULL);
		}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1131 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(3) - (4)].versyms), NULL);
		}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1135 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node (NULL, (yyvsp[(3) - (4)].versyms));
		}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1139 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versnode) = lang_new_vers_node ((yyvsp[(3) - (8)].versyms), (yyvsp[(7) - (8)].versyms));
		}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1146 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern (NULL, (yyvsp[(1) - (1)].name), ldgram_vers_current_lang);
		}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1150 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
		  (yyval.versyms) = lang_new_vers_pattern ((yyvsp[(1) - (3)].versyms), (yyvsp[(3) - (3)].name), ldgram_vers_current_lang);
		}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1154 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.name) = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = (yyvsp[(4) - (5)].name);
			}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1159 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.versyms) = (yyvsp[(7) - (9)].versyms);
			  ldgram_vers_current_lang = (yyvsp[(6) - (9)].name);
			}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1164 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.name) = ldgram_vers_current_lang;
			  ldgram_vers_current_lang = (yyvsp[(2) - (3)].name);
			}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1169 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"
    {
			  (yyval.versyms) = (yyvsp[(5) - (7)].versyms);
			  ldgram_vers_current_lang = (yyvsp[(4) - (7)].name);
			}
    break;



/* Line 1455 of yacc.c  */
#line 4227 "y.tab.c"
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
#line 1180 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"

void
yyerror(arg)
     const char *arg;
{
  if (ldfile_assumed_script)
    einfo (_("%P:%s: file format not recognized; treating as linker script\n"),
	   ldfile_input_filename);
  if (error_index > 0 && error_index < ERROR_NAME_MAX)
     einfo ("%P%F:%S: %s in %s\n", arg, error_names[error_index-1]);
  else
     einfo ("%P%F:%S: %s\n", arg);
}

