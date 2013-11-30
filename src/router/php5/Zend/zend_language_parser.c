/* A Bison parser, made by GNU Bison 2.6.5.  */

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
#define YYBISON_VERSION "2.6.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yylval          zendlval
#define yychar          zendchar
#define yydebug         zenddebug
#define yynerrs         zendnerrs

/* Copy the first part of user declarations.  */
/* Line 360 of yacc.c  */
#line 1 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"

/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/*
 * LALR shift/reduce conflicts and how they are resolved:
 *
 * - 2 shift/reduce conflicts due to the dangling elseif/else ambiguity. Solved by shift.
 *
 */


#include "zend_compile.h"
#include "zend.h"
#include "zend_list.h"
#include "zend_globals.h"
#include "zend_API.h"
#include "zend_constants.h"

#define YYSIZE_T size_t
#define yytnamerr zend_yytnamerr
static YYSIZE_T zend_yytnamerr(char*, const char*);

#define YYERROR_VERBOSE
#define YYSTYPE znode


/* Line 360 of yacc.c  */
#line 121 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.c"

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
   by #include "zend_language_parser.h".  */
#ifndef YY_ZEND_HOME_SEG_DEV_NORTHSTAR_SRC_ROUTER_PHP5_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
# define YY_ZEND_HOME_SEG_DEV_NORTHSTAR_SRC_ROUTER_PHP5_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int zenddebug;
#endif
/* "%code requires" blocks.  */
/* Line 376 of yacc.c  */
#line 50 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"

#ifdef ZTS
# define YYPARSE_PARAM tsrm_ls
# define YYLEX_PARAM tsrm_ls
#endif


/* Line 376 of yacc.c  */
#line 161 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.c"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     T_REQUIRE_ONCE = 258,
     T_REQUIRE = 259,
     T_EVAL = 260,
     T_INCLUDE_ONCE = 261,
     T_INCLUDE = 262,
     T_LOGICAL_OR = 263,
     T_LOGICAL_XOR = 264,
     T_LOGICAL_AND = 265,
     T_PRINT = 266,
     T_YIELD = 267,
     T_SR_EQUAL = 268,
     T_SL_EQUAL = 269,
     T_XOR_EQUAL = 270,
     T_OR_EQUAL = 271,
     T_AND_EQUAL = 272,
     T_MOD_EQUAL = 273,
     T_CONCAT_EQUAL = 274,
     T_DIV_EQUAL = 275,
     T_MUL_EQUAL = 276,
     T_MINUS_EQUAL = 277,
     T_PLUS_EQUAL = 278,
     T_BOOLEAN_OR = 279,
     T_BOOLEAN_AND = 280,
     T_IS_NOT_IDENTICAL = 281,
     T_IS_IDENTICAL = 282,
     T_IS_NOT_EQUAL = 283,
     T_IS_EQUAL = 284,
     T_IS_GREATER_OR_EQUAL = 285,
     T_IS_SMALLER_OR_EQUAL = 286,
     T_SR = 287,
     T_SL = 288,
     T_INSTANCEOF = 289,
     T_UNSET_CAST = 290,
     T_BOOL_CAST = 291,
     T_OBJECT_CAST = 292,
     T_ARRAY_CAST = 293,
     T_STRING_CAST = 294,
     T_DOUBLE_CAST = 295,
     T_INT_CAST = 296,
     T_DEC = 297,
     T_INC = 298,
     T_CLONE = 299,
     T_NEW = 300,
     T_EXIT = 301,
     T_IF = 302,
     T_ELSEIF = 303,
     T_ELSE = 304,
     T_ENDIF = 305,
     T_LNUMBER = 306,
     T_DNUMBER = 307,
     T_STRING = 308,
     T_STRING_VARNAME = 309,
     T_VARIABLE = 310,
     T_NUM_STRING = 311,
     T_INLINE_HTML = 312,
     T_CHARACTER = 313,
     T_BAD_CHARACTER = 314,
     T_ENCAPSED_AND_WHITESPACE = 315,
     T_CONSTANT_ENCAPSED_STRING = 316,
     T_ECHO = 317,
     T_DO = 318,
     T_WHILE = 319,
     T_ENDWHILE = 320,
     T_FOR = 321,
     T_ENDFOR = 322,
     T_FOREACH = 323,
     T_ENDFOREACH = 324,
     T_DECLARE = 325,
     T_ENDDECLARE = 326,
     T_AS = 327,
     T_SWITCH = 328,
     T_ENDSWITCH = 329,
     T_CASE = 330,
     T_DEFAULT = 331,
     T_BREAK = 332,
     T_CONTINUE = 333,
     T_GOTO = 334,
     T_FUNCTION = 335,
     T_CONST = 336,
     T_RETURN = 337,
     T_TRY = 338,
     T_CATCH = 339,
     T_FINALLY = 340,
     T_THROW = 341,
     T_USE = 342,
     T_INSTEADOF = 343,
     T_GLOBAL = 344,
     T_PUBLIC = 345,
     T_PROTECTED = 346,
     T_PRIVATE = 347,
     T_FINAL = 348,
     T_ABSTRACT = 349,
     T_STATIC = 350,
     T_VAR = 351,
     T_UNSET = 352,
     T_ISSET = 353,
     T_EMPTY = 354,
     T_HALT_COMPILER = 355,
     T_CLASS = 356,
     T_TRAIT = 357,
     T_INTERFACE = 358,
     T_EXTENDS = 359,
     T_IMPLEMENTS = 360,
     T_OBJECT_OPERATOR = 361,
     T_DOUBLE_ARROW = 362,
     T_LIST = 363,
     T_ARRAY = 364,
     T_CALLABLE = 365,
     T_CLASS_C = 366,
     T_TRAIT_C = 367,
     T_METHOD_C = 368,
     T_FUNC_C = 369,
     T_LINE = 370,
     T_FILE = 371,
     T_COMMENT = 372,
     T_DOC_COMMENT = 373,
     T_OPEN_TAG = 374,
     T_OPEN_TAG_WITH_ECHO = 375,
     T_CLOSE_TAG = 376,
     T_WHITESPACE = 377,
     T_START_HEREDOC = 378,
     T_END_HEREDOC = 379,
     T_DOLLAR_OPEN_CURLY_BRACES = 380,
     T_CURLY_OPEN = 381,
     T_PAAMAYIM_NEKUDOTAYIM = 382,
     T_NAMESPACE = 383,
     T_NS_C = 384,
     T_DIR = 385,
     T_NS_SEPARATOR = 386
   };
#endif
/* Tokens.  */
#define END 0
#define T_REQUIRE_ONCE 258
#define T_REQUIRE 259
#define T_EVAL 260
#define T_INCLUDE_ONCE 261
#define T_INCLUDE 262
#define T_LOGICAL_OR 263
#define T_LOGICAL_XOR 264
#define T_LOGICAL_AND 265
#define T_PRINT 266
#define T_YIELD 267
#define T_SR_EQUAL 268
#define T_SL_EQUAL 269
#define T_XOR_EQUAL 270
#define T_OR_EQUAL 271
#define T_AND_EQUAL 272
#define T_MOD_EQUAL 273
#define T_CONCAT_EQUAL 274
#define T_DIV_EQUAL 275
#define T_MUL_EQUAL 276
#define T_MINUS_EQUAL 277
#define T_PLUS_EQUAL 278
#define T_BOOLEAN_OR 279
#define T_BOOLEAN_AND 280
#define T_IS_NOT_IDENTICAL 281
#define T_IS_IDENTICAL 282
#define T_IS_NOT_EQUAL 283
#define T_IS_EQUAL 284
#define T_IS_GREATER_OR_EQUAL 285
#define T_IS_SMALLER_OR_EQUAL 286
#define T_SR 287
#define T_SL 288
#define T_INSTANCEOF 289
#define T_UNSET_CAST 290
#define T_BOOL_CAST 291
#define T_OBJECT_CAST 292
#define T_ARRAY_CAST 293
#define T_STRING_CAST 294
#define T_DOUBLE_CAST 295
#define T_INT_CAST 296
#define T_DEC 297
#define T_INC 298
#define T_CLONE 299
#define T_NEW 300
#define T_EXIT 301
#define T_IF 302
#define T_ELSEIF 303
#define T_ELSE 304
#define T_ENDIF 305
#define T_LNUMBER 306
#define T_DNUMBER 307
#define T_STRING 308
#define T_STRING_VARNAME 309
#define T_VARIABLE 310
#define T_NUM_STRING 311
#define T_INLINE_HTML 312
#define T_CHARACTER 313
#define T_BAD_CHARACTER 314
#define T_ENCAPSED_AND_WHITESPACE 315
#define T_CONSTANT_ENCAPSED_STRING 316
#define T_ECHO 317
#define T_DO 318
#define T_WHILE 319
#define T_ENDWHILE 320
#define T_FOR 321
#define T_ENDFOR 322
#define T_FOREACH 323
#define T_ENDFOREACH 324
#define T_DECLARE 325
#define T_ENDDECLARE 326
#define T_AS 327
#define T_SWITCH 328
#define T_ENDSWITCH 329
#define T_CASE 330
#define T_DEFAULT 331
#define T_BREAK 332
#define T_CONTINUE 333
#define T_GOTO 334
#define T_FUNCTION 335
#define T_CONST 336
#define T_RETURN 337
#define T_TRY 338
#define T_CATCH 339
#define T_FINALLY 340
#define T_THROW 341
#define T_USE 342
#define T_INSTEADOF 343
#define T_GLOBAL 344
#define T_PUBLIC 345
#define T_PROTECTED 346
#define T_PRIVATE 347
#define T_FINAL 348
#define T_ABSTRACT 349
#define T_STATIC 350
#define T_VAR 351
#define T_UNSET 352
#define T_ISSET 353
#define T_EMPTY 354
#define T_HALT_COMPILER 355
#define T_CLASS 356
#define T_TRAIT 357
#define T_INTERFACE 358
#define T_EXTENDS 359
#define T_IMPLEMENTS 360
#define T_OBJECT_OPERATOR 361
#define T_DOUBLE_ARROW 362
#define T_LIST 363
#define T_ARRAY 364
#define T_CALLABLE 365
#define T_CLASS_C 366
#define T_TRAIT_C 367
#define T_METHOD_C 368
#define T_FUNC_C 369
#define T_LINE 370
#define T_FILE 371
#define T_COMMENT 372
#define T_DOC_COMMENT 373
#define T_OPEN_TAG 374
#define T_OPEN_TAG_WITH_ECHO 375
#define T_CLOSE_TAG 376
#define T_WHITESPACE 377
#define T_START_HEREDOC 378
#define T_END_HEREDOC 379
#define T_DOLLAR_OPEN_CURLY_BRACES 380
#define T_CURLY_OPEN 381
#define T_PAAMAYIM_NEKUDOTAYIM 382
#define T_NAMESPACE 383
#define T_NS_C 384
#define T_DIR 385
#define T_NS_SEPARATOR 386



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int zendparse (void *YYPARSE_PARAM);
#else
int zendparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int zendparse (void);
#else
int zendparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_ZEND_HOME_SEG_DEV_NORTHSTAR_SRC_ROUTER_PHP5_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 379 of yacc.c  */
#line 462 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.c"

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5126

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  160
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  218
/* YYNRULES -- Number of rules.  */
#define YYNRULES  545
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1002

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   386

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,   159,     2,   156,    48,    32,     2,
     151,   152,    46,    43,     8,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   153,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   157,    31,     2,   158,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   154,    30,   155,    51,     2,     2,     2,
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
       5,     6,     7,     9,    10,    11,    12,    13,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    28,
      29,    33,    34,    35,    36,    39,    40,    41,    42,    50,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,    10,    11,    13,    17,    19,
      21,    23,    28,    32,    33,    40,    41,    47,    51,    54,
      58,    60,    62,    66,    69,    74,    80,    85,    86,    90,
      91,    93,    95,    97,   102,   104,   107,   111,   112,   113,
     121,   122,   123,   134,   135,   136,   142,   143,   144,   152,
     153,   154,   155,   168,   169,   174,   177,   181,   184,   188,
     191,   195,   199,   202,   206,   210,   214,   216,   219,   225,
     226,   227,   238,   239,   240,   251,   252,   259,   261,   262,
     263,   272,   276,   280,   281,   282,   283,   284,   285,   299,
     300,   301,   307,   309,   310,   312,   315,   316,   317,   328,
     330,   334,   336,   338,   340,   341,   343,   344,   355,   356,
     365,   366,   374,   376,   379,   381,   384,   385,   388,   390,
     391,   394,   395,   398,   400,   404,   405,   408,   410,   413,
     414,   420,   422,   427,   429,   434,   436,   441,   445,   451,
     455,   460,   465,   471,   472,   473,   480,   481,   487,   489,
     491,   493,   498,   499,   500,   506,   507,   508,   515,   516,
     519,   520,   524,   526,   527,   530,   534,   540,   545,   550,
     556,   564,   571,   572,   574,   576,   578,   581,   585,   589,
     591,   593,   596,   600,   604,   609,   613,   615,   617,   620,
     625,   629,   635,   637,   641,   644,   645,   646,   651,   654,
     656,   657,   667,   671,   673,   677,   679,   683,   684,   686,
     688,   691,   694,   697,   701,   703,   707,   709,   711,   715,
     720,   724,   725,   727,   729,   733,   735,   737,   738,   740,
     742,   745,   747,   749,   751,   753,   755,   757,   761,   767,
     769,   773,   779,   784,   788,   790,   791,   793,   794,   799,
     801,   804,   806,   811,   815,   816,   820,   822,   824,   825,
     826,   829,   830,   835,   836,   844,   848,   853,   854,   862,
     865,   869,   873,   877,   881,   885,   889,   893,   897,   901,
     905,   909,   912,   915,   918,   921,   922,   927,   928,   933,
     934,   939,   940,   945,   949,   953,   957,   961,   965,   969,
     973,   977,   981,   985,   989,   993,   996,   999,  1002,  1005,
    1009,  1013,  1017,  1021,  1025,  1029,  1033,  1037,  1041,  1043,
    1045,  1046,  1052,  1053,  1054,  1062,  1063,  1069,  1071,  1074,
    1077,  1080,  1083,  1086,  1089,  1092,  1095,  1096,  1100,  1102,
    1104,  1106,  1110,  1113,  1115,  1116,  1127,  1128,  1140,  1143,
    1146,  1151,  1156,  1161,  1166,  1171,  1176,  1180,  1182,  1183,
    1188,  1192,  1197,  1199,  1202,  1203,  1207,  1208,  1214,  1215,
    1220,  1221,  1227,  1228,  1234,  1235,  1241,  1242,  1248,  1249,
    1253,  1255,  1257,  1261,  1264,  1266,  1270,  1273,  1275,  1277,
    1278,  1279,  1286,  1288,  1291,  1292,  1295,  1296,  1299,  1301,
    1302,  1304,  1306,  1307,  1309,  1311,  1313,  1315,  1317,  1319,
    1321,  1323,  1325,  1327,  1329,  1333,  1336,  1338,  1340,  1342,
    1346,  1349,  1352,  1355,  1360,  1364,  1366,  1368,  1372,  1374,
    1376,  1378,  1380,  1384,  1387,  1389,  1393,  1397,  1399,  1400,
    1403,  1404,  1406,  1412,  1416,  1420,  1422,  1424,  1426,  1430,
    1434,  1436,  1438,  1440,  1441,  1442,  1450,  1452,  1455,  1456,
    1457,  1462,  1467,  1472,  1473,  1476,  1478,  1480,  1481,  1483,
    1486,  1490,  1494,  1496,  1501,  1502,  1508,  1510,  1512,  1514,
    1516,  1519,  1521,  1526,  1531,  1533,  1535,  1540,  1541,  1543,
    1545,  1546,  1549,  1554,  1559,  1561,  1563,  1567,  1569,  1572,
    1576,  1578,  1580,  1581,  1587,  1588,  1589,  1592,  1598,  1602,
    1606,  1608,  1615,  1620,  1625,  1628,  1631,  1634,  1636,  1639,
    1641,  1642,  1648,  1652,  1656,  1663,  1667,  1669,  1671,  1673,
    1678,  1683,  1688,  1691,  1694,  1699,  1702,  1705,  1707,  1708,
    1713,  1715,  1717,  1721,  1725,  1729
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     161,     0,    -1,   162,    -1,    -1,   162,   163,   165,    -1,
      -1,    72,    -1,   164,   150,    72,    -1,   174,    -1,   209,
      -1,   210,    -1,   119,   151,   152,   153,    -1,   147,   164,
     153,    -1,    -1,   147,   164,   154,   166,   162,   155,    -1,
      -1,   147,   154,   167,   162,   155,    -1,   106,   168,   153,
      -1,   170,   153,    -1,   168,     8,   169,    -1,   169,    -1,
     164,    -1,   164,    91,    72,    -1,   150,   164,    -1,   150,
     164,    91,    72,    -1,   170,     8,    72,    14,   326,    -1,
     100,    72,    14,   326,    -1,    -1,   171,   172,   173,    -1,
      -1,   174,    -1,   209,    -1,   210,    -1,   119,   151,   152,
     153,    -1,   175,    -1,    72,    27,    -1,   154,   171,   155,
      -1,    -1,    -1,    66,   333,   176,   174,   177,   236,   240,
      -1,    -1,    -1,    66,   333,    27,   178,   171,   179,   238,
     241,    69,   153,    -1,    -1,    -1,    83,   180,   333,   181,
     235,    -1,    -1,    -1,    82,   182,   174,    83,   183,   333,
     153,    -1,    -1,    -1,    -1,    85,   151,   274,   153,   184,
     274,   153,   185,   274,   152,   186,   226,    -1,    -1,    92,
     333,   187,   230,    -1,    96,   153,    -1,    96,   332,   153,
      -1,    97,   153,    -1,    97,   332,   153,    -1,   101,   153,
      -1,   101,   285,   153,    -1,   101,   337,   153,    -1,   299,
     153,    -1,   108,   247,   153,    -1,   114,   249,   153,    -1,
      81,   273,   153,    -1,    76,    -1,   332,   153,    -1,   116,
     151,   207,   152,   153,    -1,    -1,    -1,    87,   151,   337,
      91,   188,   224,   223,   152,   189,   227,    -1,    -1,    -1,
      87,   151,   285,    91,   190,   224,   223,   152,   191,   227,
      -1,    -1,    89,   192,   151,   229,   152,   228,    -1,   153,
      -1,    -1,    -1,   102,   193,   154,   171,   155,   195,   194,
     200,    -1,   105,   332,   153,    -1,    98,    72,   153,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   151,   196,   315,   197,
      74,   152,   198,   154,   171,   155,   199,   202,    -1,    -1,
      -1,   104,   201,   154,   171,   155,    -1,   203,    -1,    -1,
     204,    -1,   203,   204,    -1,    -1,    -1,   103,   151,   315,
     205,    74,   152,   206,   154,   171,   155,    -1,   208,    -1,
     207,     8,   208,    -1,   337,    -1,   212,    -1,   214,    -1,
      -1,    32,    -1,    -1,   302,   211,    72,   213,   151,   242,
     152,   154,   171,   155,    -1,    -1,   217,    72,   218,   215,
     221,   154,   250,   155,    -1,    -1,   219,    72,   216,   220,
     154,   250,   155,    -1,   120,    -1,   113,   120,    -1,   121,
      -1,   112,   120,    -1,    -1,   123,   315,    -1,   122,    -1,
      -1,   123,   222,    -1,    -1,   124,   222,    -1,   315,    -1,
     222,     8,   315,    -1,    -1,   126,   224,    -1,   337,    -1,
      32,   337,    -1,    -1,   127,   151,   225,   362,   152,    -1,
     174,    -1,    27,   171,    86,   153,    -1,   174,    -1,    27,
     171,    88,   153,    -1,   174,    -1,    27,   171,    90,   153,
      -1,    72,    14,   326,    -1,   229,     8,    72,    14,   326,
      -1,   154,   231,   155,    -1,   154,   153,   231,   155,    -1,
      27,   231,    93,   153,    -1,    27,   153,   231,    93,   153,
      -1,    -1,    -1,   231,    94,   332,   234,   232,   171,    -1,
      -1,   231,    95,   234,   233,   171,    -1,    27,    -1,   153,
      -1,   174,    -1,    27,   171,    84,   153,    -1,    -1,    -1,
     236,    67,   333,   237,   174,    -1,    -1,    -1,   238,    67,
     333,    27,   239,   171,    -1,    -1,    68,   174,    -1,    -1,
      68,    27,   171,    -1,   243,    -1,    -1,   244,    74,    -1,
     244,    32,    74,    -1,   244,    32,    74,    14,   326,    -1,
     244,    74,    14,   326,    -1,   243,     8,   244,    74,    -1,
     243,     8,   244,    32,    74,    -1,   243,     8,   244,    32,
      74,    14,   326,    -1,   243,     8,   244,    74,    14,   326,
      -1,    -1,   128,    -1,   129,    -1,   315,    -1,   151,   152,
      -1,   151,   246,   152,    -1,   151,   299,   152,    -1,   285,
      -1,   337,    -1,    32,   335,    -1,   246,     8,   285,    -1,
     246,     8,   337,    -1,   246,     8,    32,   335,    -1,   247,
       8,   248,    -1,   248,    -1,    74,    -1,   156,   334,    -1,
     156,   154,   332,   155,    -1,   249,     8,    74,    -1,   249,
       8,    74,    14,   326,    -1,    74,    -1,    74,    14,   326,
      -1,   250,   251,    -1,    -1,    -1,   267,   252,   271,   153,
      -1,   272,   153,    -1,   254,    -1,    -1,   268,   302,   211,
      72,   253,   151,   242,   152,   266,    -1,   106,   255,   256,
      -1,   315,    -1,   255,     8,   315,    -1,   153,    -1,   154,
     257,   155,    -1,    -1,   258,    -1,   259,    -1,   258,   259,
      -1,   260,   153,    -1,   264,   153,    -1,   263,   107,   261,
      -1,   315,    -1,   261,     8,   315,    -1,    72,    -1,   263,
      -1,   315,   146,    72,    -1,   262,    91,   265,    72,    -1,
     262,    91,   270,    -1,    -1,   270,    -1,   153,    -1,   154,
     171,   155,    -1,   269,    -1,   115,    -1,    -1,   269,    -1,
     270,    -1,   269,   270,    -1,   109,    -1,   110,    -1,   111,
      -1,   114,    -1,   113,    -1,   112,    -1,   271,     8,    74,
      -1,   271,     8,    74,    14,   326,    -1,    74,    -1,    74,
      14,   326,    -1,   272,     8,    72,    14,   326,    -1,   100,
      72,    14,   326,    -1,   273,     8,   332,    -1,   332,    -1,
      -1,   275,    -1,    -1,   275,     8,   276,   332,    -1,   332,
      -1,   277,   341,    -1,   341,    -1,   278,    62,   356,   157,
      -1,    62,   356,   157,    -1,    -1,   278,   280,   277,    -1,
     278,    -1,   277,    -1,    -1,    -1,   282,   279,    -1,    -1,
      64,   316,   284,   324,    -1,    -1,   127,   151,   286,   362,
     152,    14,   332,    -1,   337,    14,   332,    -1,   337,    14,
      32,   337,    -1,    -1,   337,    14,    32,    64,   316,   287,
     324,    -1,    63,   332,    -1,   337,    25,   332,    -1,   337,
      24,   332,    -1,   337,    23,   332,    -1,   337,    22,   332,
      -1,   337,    21,   332,    -1,   337,    20,   332,    -1,   337,
      19,   332,    -1,   337,    18,   332,    -1,   337,    17,   332,
      -1,   337,    16,   332,    -1,   337,    15,   332,    -1,   336,
      61,    -1,    61,   336,    -1,   336,    60,    -1,    60,   336,
      -1,    -1,   332,    28,   288,   332,    -1,    -1,   332,    29,
     289,   332,    -1,    -1,   332,     9,   290,   332,    -1,    -1,
     332,    11,   291,   332,    -1,   332,    10,   332,    -1,   332,
      30,   332,    -1,   332,    32,   332,    -1,   332,    31,   332,
      -1,   332,    45,   332,    -1,   332,    43,   332,    -1,   332,
      44,   332,    -1,   332,    46,   332,    -1,   332,    47,   332,
      -1,   332,    48,   332,    -1,   332,    42,   332,    -1,   332,
      41,   332,    -1,    43,   332,    -1,    44,   332,    -1,    49,
     332,    -1,    51,   332,    -1,   332,    34,   332,    -1,   332,
      33,   332,    -1,   332,    36,   332,    -1,   332,    35,   332,
      -1,   332,    37,   332,    -1,   332,    40,   332,    -1,   332,
      38,   332,    -1,   332,    39,   332,    -1,   332,    50,   316,
      -1,   333,    -1,   283,    -1,    -1,   151,   283,   152,   292,
     281,    -1,    -1,    -1,   332,    26,   293,   332,    27,   294,
     332,    -1,    -1,   332,    26,    27,   295,   332,    -1,   371,
      -1,    59,   332,    -1,    58,   332,    -1,    57,   332,    -1,
      56,   332,    -1,    55,   332,    -1,    54,   332,    -1,    53,
     332,    -1,    65,   322,    -1,    -1,    52,   296,   332,    -1,
     328,    -1,   300,    -1,   301,    -1,   158,   323,   158,    -1,
      12,   332,    -1,    13,    -1,    -1,   302,   211,   297,   151,
     242,   152,   303,   154,   171,   155,    -1,    -1,   114,   302,
     211,   298,   151,   242,   152,   303,   154,   171,   155,    -1,
      13,   285,    -1,    13,   337,    -1,    13,   332,   126,   285,
      -1,    13,   332,   126,   337,    -1,   301,    62,   356,   157,
      -1,   300,    62,   356,   157,    -1,    80,    62,   356,   157,
      -1,   128,   151,   365,   152,    -1,    62,   365,   157,    -1,
      99,    -1,    -1,   106,   151,   304,   152,    -1,   304,     8,
      74,    -1,   304,     8,    32,    74,    -1,    74,    -1,    32,
      74,    -1,    -1,   164,   306,   245,    -1,    -1,   147,   150,
     164,   307,   245,    -1,    -1,   150,   164,   308,   245,    -1,
      -1,   314,   146,   360,   309,   245,    -1,    -1,   314,   146,
     347,   310,   245,    -1,    -1,   349,   146,   360,   311,   245,
      -1,    -1,   349,   146,   347,   312,   245,    -1,    -1,   347,
     313,   245,    -1,   114,    -1,   164,    -1,   147,   150,   164,
      -1,   150,   164,    -1,   164,    -1,   147,   150,   164,    -1,
     150,   164,    -1,   314,    -1,   317,    -1,    -1,    -1,   353,
     125,   318,   357,   319,   320,    -1,   353,    -1,   320,   321,
      -1,    -1,   125,   357,    -1,    -1,   151,   152,    -1,   333,
      -1,    -1,    79,    -1,   367,    -1,    -1,   245,    -1,    70,
      -1,    71,    -1,    80,    -1,   134,    -1,   135,    -1,   149,
      -1,   131,    -1,   132,    -1,   133,    -1,   148,    -1,   142,
      79,   143,    -1,   142,   143,    -1,   325,    -1,   376,    -1,
     164,    -1,   147,   150,   164,    -1,   150,   164,    -1,    43,
     326,    -1,    44,   326,    -1,   128,   151,   329,   152,    -1,
      62,   329,   157,    -1,   327,    -1,   130,    -1,   314,   146,
      72,    -1,    73,    -1,   377,    -1,   375,    -1,   164,    -1,
     147,   150,   164,    -1,   150,   164,    -1,   325,    -1,   159,
     367,   159,    -1,   142,   367,   143,    -1,   130,    -1,    -1,
     331,   330,    -1,    -1,     8,    -1,   331,     8,   326,   126,
     326,    -1,   331,     8,   326,    -1,   326,   126,   326,    -1,
     326,    -1,   334,    -1,   285,    -1,   151,   332,   152,    -1,
     151,   299,   152,    -1,   337,    -1,   337,    -1,   337,    -1,
      -1,    -1,   352,   125,   338,   357,   339,   346,   340,    -1,
     352,    -1,   340,   341,    -1,    -1,    -1,   125,   357,   342,
     346,    -1,   343,    62,   356,   157,    -1,   344,    62,   356,
     157,    -1,    -1,   345,   245,    -1,   344,    -1,   343,    -1,
      -1,   354,    -1,   361,   354,    -1,   314,   146,   347,    -1,
     349,   146,   347,    -1,   354,    -1,   350,    62,   356,   157,
      -1,    -1,   305,   351,    62,   356,   157,    -1,   353,    -1,
     350,    -1,   305,    -1,   354,    -1,   361,   354,    -1,   348,
      -1,   354,    62,   356,   157,    -1,   354,   154,   332,   155,
      -1,   355,    -1,    74,    -1,   156,   154,   332,   155,    -1,
      -1,   332,    -1,   359,    -1,    -1,   347,   358,    -1,   359,
      62,   356,   157,    -1,   359,   154,   332,   155,    -1,   360,
      -1,    72,    -1,   154,   332,   155,    -1,   156,    -1,   361,
     156,    -1,   362,     8,   363,    -1,   363,    -1,   337,    -1,
      -1,   127,   151,   364,   362,   152,    -1,    -1,    -1,   366,
     330,    -1,   366,     8,   332,   126,   332,    -1,   366,     8,
     332,    -1,   332,   126,   332,    -1,   332,    -1,   366,     8,
     332,   126,    32,   335,    -1,   366,     8,    32,   335,    -1,
     332,   126,    32,   335,    -1,    32,   335,    -1,   367,   368,
      -1,   367,    79,    -1,   368,    -1,    79,   368,    -1,    74,
      -1,    -1,    74,    62,   369,   370,   157,    -1,    74,   125,
      72,    -1,   144,   332,   155,    -1,   144,    73,    62,   332,
     157,   155,    -1,   145,   337,   155,    -1,    72,    -1,    75,
      -1,    74,    -1,   117,   151,   372,   152,    -1,   118,   151,
     337,   152,    -1,   118,   151,   285,   152,    -1,     7,   332,
      -1,     6,   332,    -1,     5,   151,   332,   152,    -1,     4,
     332,    -1,     3,   332,    -1,   374,    -1,    -1,   372,     8,
     373,   374,    -1,   337,    -1,   285,    -1,   314,   146,    72,
      -1,   349,   146,    72,    -1,   314,   146,   120,    -1,   314,
     146,   120,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   219,   219,   223,   223,   224,   228,   229,   233,   234,
     235,   236,   237,   238,   238,   240,   240,   242,   243,   247,
     248,   252,   253,   254,   255,   259,   260,   264,   264,   265,
     270,   271,   272,   273,   278,   279,   283,   284,   284,   284,
     285,   285,   285,   286,   286,   286,   287,   287,   287,   291,
     293,   295,   288,   297,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   313,
     314,   312,   317,   318,   316,   320,   320,   321,   322,   323,
     322,   325,   326,   330,   331,   332,   333,   334,   331,   338,
     339,   339,   343,   344,   348,   349,   353,   353,   353,   357,
     358,   362,   366,   370,   374,   375,   380,   380,   387,   386,
     393,   392,   402,   403,   404,   405,   409,   410,   414,   417,
     419,   422,   424,   428,   429,   433,   434,   438,   439,   440,
     440,   444,   445,   450,   451,   456,   457,   462,   463,   468,
     469,   470,   471,   476,   477,   477,   478,   478,   483,   484,
     489,   490,   495,   497,   497,   501,   503,   503,   507,   509,
     513,   515,   520,   521,   526,   527,   528,   529,   530,   531,
     532,   533,   538,   539,   540,   541,   546,   547,   548,   553,
     554,   555,   556,   557,   558,   562,   563,   568,   569,   570,
     575,   576,   577,   578,   584,   585,   590,   590,   591,   592,
     593,   593,   599,   603,   604,   608,   609,   612,   614,   618,
     619,   623,   624,   628,   632,   633,   637,   638,   642,   646,
     647,   651,   652,   656,   657,   661,   662,   666,   667,   671,
     672,   676,   677,   678,   679,   680,   681,   685,   686,   687,
     688,   692,   693,   697,   698,   703,   704,   708,   708,   709,
     713,   714,   718,   719,   723,   723,   724,   725,   729,   730,
     730,   735,   735,   739,   739,   740,   741,   742,   742,   743,
     744,   745,   746,   747,   748,   749,   750,   751,   752,   753,
     754,   755,   756,   757,   758,   759,   759,   760,   760,   761,
     761,   762,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   774,   775,   776,   777,   778,   779,
     780,   781,   782,   783,   784,   785,   786,   787,   788,   789,
     790,   790,   791,   792,   791,   794,   794,   796,   797,   798,
     799,   800,   801,   802,   803,   804,   805,   805,   806,   807,
     808,   809,   810,   811,   812,   812,   815,   815,   821,   822,
     823,   824,   828,   829,   830,   833,   834,   837,   840,   842,
     846,   847,   848,   849,   853,   853,   855,   855,   857,   857,
     859,   859,   861,   861,   863,   863,   865,   865,   867,   867,
     872,   873,   874,   875,   879,   880,   881,   887,   888,   893,
     894,   893,   896,   901,   902,   907,   911,   912,   913,   917,
     918,   919,   924,   925,   930,   931,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   960,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   978,   979,
     982,   984,   988,   989,   990,   991,   995,   996,  1000,  1001,
    1006,  1011,  1016,  1021,  1022,  1021,  1024,  1028,  1029,  1034,
    1034,  1038,  1039,  1043,  1043,  1048,  1049,  1050,  1054,  1055,
    1059,  1060,  1065,  1069,  1070,  1070,  1075,  1076,  1077,  1082,
    1083,  1084,  1088,  1089,  1090,  1095,  1096,  1100,  1101,  1106,
    1107,  1107,  1111,  1112,  1113,  1117,  1118,  1122,  1123,  1127,
    1128,  1133,  1134,  1134,  1135,  1140,  1141,  1145,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1156,  1157,  1158,  1159,  1165,
    1166,  1166,  1167,  1168,  1169,  1170,  1175,  1176,  1177,  1182,
    1183,  1184,  1185,  1186,  1187,  1188,  1189,  1193,  1194,  1194,
    1198,  1199,  1203,  1204,  1208,  1212
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined",
  "\"require_once (T_REQUIRE_ONCE)\"", "\"require (T_REQUIRE)\"",
  "\"eval (T_EVAL)\"", "\"include_once (T_INCLUDE_ONCE)\"",
  "\"include (T_INCLUDE)\"", "','", "\"or (T_LOGICAL_OR)\"",
  "\"xor (T_LOGICAL_XOR)\"", "\"and (T_LOGICAL_AND)\"",
  "\"print (T_PRINT)\"", "\"yield (T_YIELD)\"", "'='",
  "\">>= (T_SR_EQUAL)\"", "\"<<= (T_SL_EQUAL)\"", "\"^= (T_XOR_EQUAL)\"",
  "\"|= (T_OR_EQUAL)\"", "\"&= (T_AND_EQUAL)\"", "\"%= (T_MOD_EQUAL)\"",
  "\".= (T_CONCAT_EQUAL)\"", "\"/= (T_DIV_EQUAL)\"",
  "\"*= (T_MUL_EQUAL)\"", "\"-= (T_MINUS_EQUAL)\"",
  "\"+= (T_PLUS_EQUAL)\"", "'?'", "':'", "\"|| (T_BOOLEAN_OR)\"",
  "\"&& (T_BOOLEAN_AND)\"", "'|'", "'^'", "'&'",
  "\"!== (T_IS_NOT_IDENTICAL)\"", "\"=== (T_IS_IDENTICAL)\"",
  "\"!= (T_IS_NOT_EQUAL)\"", "\"== (T_IS_EQUAL)\"", "'<'", "'>'",
  "\">= (T_IS_GREATER_OR_EQUAL)\"", "\"<= (T_IS_SMALLER_OR_EQUAL)\"",
  "\">> (T_SR)\"", "\"<< (T_SL)\"", "'+'", "'-'", "'.'", "'*'", "'/'",
  "'%'", "'!'", "\"instanceof (T_INSTANCEOF)\"", "'~'", "'@'",
  "\"(unset) (T_UNSET_CAST)\"", "\"(bool) (T_BOOL_CAST)\"",
  "\"(object) (T_OBJECT_CAST)\"", "\"(array) (T_ARRAY_CAST)\"",
  "\"(string) (T_STRING_CAST)\"", "\"(double) (T_DOUBLE_CAST)\"",
  "\"(int) (T_INT_CAST)\"", "\"-- (T_DEC)\"", "\"++ (T_INC)\"", "'['",
  "\"clone (T_CLONE)\"", "\"new (T_NEW)\"", "\"exit (T_EXIT)\"",
  "\"if (T_IF)\"", "\"elseif (T_ELSEIF)\"", "\"else (T_ELSE)\"",
  "\"endif (T_ENDIF)\"", "\"integer number (T_LNUMBER)\"",
  "\"floating-point number (T_DNUMBER)\"", "\"identifier (T_STRING)\"",
  "\"variable name (T_STRING_VARNAME)\"", "\"variable (T_VARIABLE)\"",
  "\"number (T_NUM_STRING)\"", "T_INLINE_HTML", "T_CHARACTER",
  "T_BAD_CHARACTER",
  "\"quoted-string and whitespace (T_ENCAPSED_AND_WHITESPACE)\"",
  "\"quoted-string (T_CONSTANT_ENCAPSED_STRING)\"", "\"echo (T_ECHO)\"",
  "\"do (T_DO)\"", "\"while (T_WHILE)\"", "\"endwhile (T_ENDWHILE)\"",
  "\"for (T_FOR)\"", "\"endfor (T_ENDFOR)\"", "\"foreach (T_FOREACH)\"",
  "\"endforeach (T_ENDFOREACH)\"", "\"declare (T_DECLARE)\"",
  "\"enddeclare (T_ENDDECLARE)\"", "\"as (T_AS)\"",
  "\"switch (T_SWITCH)\"", "\"endswitch (T_ENDSWITCH)\"",
  "\"case (T_CASE)\"", "\"default (T_DEFAULT)\"", "\"break (T_BREAK)\"",
  "\"continue (T_CONTINUE)\"", "\"goto (T_GOTO)\"",
  "\"function (T_FUNCTION)\"", "\"const (T_CONST)\"",
  "\"return (T_RETURN)\"", "\"try (T_TRY)\"", "\"catch (T_CATCH)\"",
  "\"finally (T_FINALLY)\"", "\"throw (T_THROW)\"", "\"use (T_USE)\"",
  "\"insteadof (T_INSTEADOF)\"", "\"global (T_GLOBAL)\"",
  "\"public (T_PUBLIC)\"", "\"protected (T_PROTECTED)\"",
  "\"private (T_PRIVATE)\"", "\"final (T_FINAL)\"",
  "\"abstract (T_ABSTRACT)\"", "\"static (T_STATIC)\"", "\"var (T_VAR)\"",
  "\"unset (T_UNSET)\"", "\"isset (T_ISSET)\"", "\"empty (T_EMPTY)\"",
  "\"__halt_compiler (T_HALT_COMPILER)\"", "\"class (T_CLASS)\"",
  "\"trait (T_TRAIT)\"", "\"interface (T_INTERFACE)\"",
  "\"extends (T_EXTENDS)\"", "\"implements (T_IMPLEMENTS)\"",
  "\"-> (T_OBJECT_OPERATOR)\"", "\"=> (T_DOUBLE_ARROW)\"",
  "\"list (T_LIST)\"", "\"array (T_ARRAY)\"", "\"callable (T_CALLABLE)\"",
  "\"__CLASS__ (T_CLASS_C)\"", "\"__TRAIT__ (T_TRAIT_C)\"",
  "\"__METHOD__ (T_METHOD_C)\"", "\"__FUNCTION__ (T_FUNC_C)\"",
  "\"__LINE__ (T_LINE)\"", "\"__FILE__ (T_FILE)\"",
  "\"comment (T_COMMENT)\"", "\"doc comment (T_DOC_COMMENT)\"",
  "\"open tag (T_OPEN_TAG)\"",
  "\"open tag with echo (T_OPEN_TAG_WITH_ECHO)\"",
  "\"close tag (T_CLOSE_TAG)\"", "\"whitespace (T_WHITESPACE)\"",
  "\"heredoc start (T_START_HEREDOC)\"", "\"heredoc end (T_END_HEREDOC)\"",
  "\"${ (T_DOLLAR_OPEN_CURLY_BRACES)\"", "\"{$ (T_CURLY_OPEN)\"",
  "\":: (T_PAAMAYIM_NEKUDOTAYIM)\"", "\"namespace (T_NAMESPACE)\"",
  "\"__NAMESPACE__ (T_NS_C)\"", "\"__DIR__ (T_DIR)\"",
  "\"\\\\ (T_NS_SEPARATOR)\"", "'('", "')'", "';'", "'{'", "'}'", "'$'",
  "']'", "'`'", "'\"'", "$accept", "start", "top_statement_list", "$@1",
  "namespace_name", "top_statement", "$@2", "$@3", "use_declarations",
  "use_declaration", "constant_declaration", "inner_statement_list", "$@4",
  "inner_statement", "statement", "unticked_statement", "$@5", "$@6",
  "$@7", "$@8", "$@9", "@10", "$@11", "$@12", "$@13", "$@14", "$@15",
  "$@16", "$@17", "$@18", "$@19", "$@20", "$@21", "$@22", "$@23",
  "catch_statement", "$@24", "$@25", "$@26", "$@27", "finally_statement",
  "$@28", "additional_catches", "non_empty_additional_catches",
  "additional_catch", "@29", "$@30", "unset_variables", "unset_variable",
  "function_declaration_statement", "class_declaration_statement",
  "is_reference", "unticked_function_declaration_statement", "$@31",
  "unticked_class_declaration_statement", "$@32", "$@33",
  "class_entry_type", "extends_from", "interface_entry",
  "interface_extends_list", "implements_list", "interface_list",
  "foreach_optional_arg", "foreach_variable", "$@34", "for_statement",
  "foreach_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "$@35", "$@36", "case_separator",
  "while_statement", "elseif_list", "$@37", "new_elseif_list", "$@38",
  "else_single", "new_else_single", "parameter_list",
  "non_empty_parameter_list", "optional_class_type",
  "function_call_parameter_list", "non_empty_function_call_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "class_statement_list", "class_statement", "$@39", "$@40",
  "trait_use_statement", "trait_list", "trait_adaptations",
  "trait_adaptation_list", "non_empty_trait_adaptation_list",
  "trait_adaptation_statement", "trait_precedence", "trait_reference_list",
  "trait_method_reference", "trait_method_reference_fully_qualified",
  "trait_alias", "trait_modifiers", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "echo_expr_list", "for_expr", "non_empty_for_expr", "$@41",
  "chaining_method_or_property", "chaining_dereference",
  "chaining_instance_call", "$@42", "instance_call", "$@43", "new_expr",
  "$@44", "expr_without_variable", "$@45", "$@46", "$@47", "$@48", "$@49",
  "$@50", "@51", "$@52", "$@53", "$@54", "$@55", "@56", "@57",
  "yield_expr", "combined_scalar_offset", "combined_scalar", "function",
  "lexical_vars", "lexical_var_list", "function_call", "@58", "@59", "@60",
  "@61", "$@62", "$@63", "$@64", "$@65", "class_name",
  "fully_qualified_class_name", "class_name_reference",
  "dynamic_class_name_reference", "$@66", "$@67",
  "dynamic_class_name_variable_properties",
  "dynamic_class_name_variable_property", "exit_expr", "backticks_expr",
  "ctor_arguments", "common_scalar", "static_scalar",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "non_empty_static_array_pair_list", "expr",
  "parenthesis_expr", "r_variable", "w_variable", "rw_variable",
  "variable", "$@68", "$@69", "variable_properties", "variable_property",
  "$@70", "array_method_dereference", "method", "@71", "method_or_not",
  "variable_without_objects", "static_member", "variable_class_name",
  "array_function_dereference", "$@72",
  "base_variable_with_function_calls", "base_variable",
  "reference_variable", "compound_variable", "dim_offset",
  "object_property", "$@73", "object_dim_list", "variable_name",
  "simple_indirect_reference", "assignment_list",
  "assignment_list_element", "$@74", "array_pair_list",
  "non_empty_array_pair_list", "encaps_list", "encaps_var", "$@75",
  "encaps_var_offset", "internal_functions_in_yacc", "isset_variables",
  "$@76", "isset_variable", "class_constant", "static_class_name_scalar",
  "class_name_scalar", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,    44,   263,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    63,    58,   279,   280,
     124,    94,    38,   281,   282,   283,   284,    60,    62,   285,
     286,   287,   288,    43,    45,    46,    42,    47,    37,    33,
     289,   126,    64,   290,   291,   292,   293,   294,   295,   296,
     297,   298,    91,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,    40,    41,    59,   123,   125,    36,    93,    96,    34
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   160,   161,   163,   162,   162,   164,   164,   165,   165,
     165,   165,   165,   166,   165,   167,   165,   165,   165,   168,
     168,   169,   169,   169,   169,   170,   170,   172,   171,   171,
     173,   173,   173,   173,   174,   174,   175,   176,   177,   175,
     178,   179,   175,   180,   181,   175,   182,   183,   175,   184,
     185,   186,   175,   187,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   188,
     189,   175,   190,   191,   175,   192,   175,   175,   193,   194,
     175,   175,   175,   195,   196,   197,   198,   199,   195,   200,
     201,   200,   202,   202,   203,   203,   205,   206,   204,   207,
     207,   208,   209,   210,   211,   211,   213,   212,   215,   214,
     216,   214,   217,   217,   217,   217,   218,   218,   219,   220,
     220,   221,   221,   222,   222,   223,   223,   224,   224,   225,
     224,   226,   226,   227,   227,   228,   228,   229,   229,   230,
     230,   230,   230,   231,   232,   231,   233,   231,   234,   234,
     235,   235,   236,   237,   236,   238,   239,   238,   240,   240,
     241,   241,   242,   242,   243,   243,   243,   243,   243,   243,
     243,   243,   244,   244,   244,   244,   245,   245,   245,   246,
     246,   246,   246,   246,   246,   247,   247,   248,   248,   248,
     249,   249,   249,   249,   250,   250,   252,   251,   251,   251,
     253,   251,   254,   255,   255,   256,   256,   257,   257,   258,
     258,   259,   259,   260,   261,   261,   262,   262,   263,   264,
     264,   265,   265,   266,   266,   267,   267,   268,   268,   269,
     269,   270,   270,   270,   270,   270,   270,   271,   271,   271,
     271,   272,   272,   273,   273,   274,   274,   276,   275,   275,
     277,   277,   278,   278,   280,   279,   279,   279,   281,   282,
     281,   284,   283,   286,   285,   285,   285,   287,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   288,   285,   289,   285,   290,
     285,   291,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     292,   285,   293,   294,   285,   295,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   296,   285,   285,   285,
     285,   285,   285,   285,   297,   285,   298,   285,   299,   299,
     299,   299,   300,   300,   300,   301,   301,   302,   303,   303,
     304,   304,   304,   304,   306,   305,   307,   305,   308,   305,
     309,   305,   310,   305,   311,   305,   312,   305,   313,   305,
     314,   314,   314,   314,   315,   315,   315,   316,   316,   318,
     319,   317,   317,   320,   320,   321,   322,   322,   322,   323,
     323,   323,   324,   324,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   327,   328,   328,
     328,   328,   328,   328,   328,   328,   328,   328,   329,   329,
     330,   330,   331,   331,   331,   331,   332,   332,   333,   333,
     334,   335,   336,   338,   339,   337,   337,   340,   340,   342,
     341,   343,   343,   345,   344,   346,   346,   346,   347,   347,
     348,   348,   349,   350,   351,   350,   352,   352,   352,   353,
     353,   353,   354,   354,   354,   355,   355,   356,   356,   357,
     358,   357,   359,   359,   359,   360,   360,   361,   361,   362,
     362,   363,   364,   363,   363,   365,   365,   366,   366,   366,
     366,   366,   366,   366,   366,   367,   367,   367,   367,   368,
     369,   368,   368,   368,   368,   368,   370,   370,   370,   371,
     371,   371,   371,   371,   371,   371,   371,   372,   373,   372,
     374,   374,   375,   375,   376,   377
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     3,     0,     1,     3,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     3,
       1,     1,     3,     2,     4,     5,     4,     0,     3,     0,
       1,     1,     1,     4,     1,     2,     3,     0,     0,     7,
       0,     0,    10,     0,     0,     5,     0,     0,     7,     0,
       0,     0,    12,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     2,     3,     3,     3,     1,     2,     5,     0,
       0,    10,     0,     0,    10,     0,     6,     1,     0,     0,
       8,     3,     3,     0,     0,     0,     0,     0,    13,     0,
       0,     5,     1,     0,     1,     2,     0,     0,    10,     1,
       3,     1,     1,     1,     0,     1,     0,    10,     0,     8,
       0,     7,     1,     2,     1,     2,     0,     2,     1,     0,
       2,     0,     2,     1,     3,     0,     2,     1,     2,     0,
       5,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     0,     0,     6,     0,     5,     1,     1,
       1,     4,     0,     0,     5,     0,     0,     6,     0,     2,
       0,     3,     1,     0,     2,     3,     5,     4,     4,     5,
       7,     6,     0,     1,     1,     1,     2,     3,     3,     1,
       1,     2,     3,     3,     4,     3,     1,     1,     2,     4,
       3,     5,     1,     3,     2,     0,     0,     4,     2,     1,
       0,     9,     3,     1,     3,     1,     3,     0,     1,     1,
       2,     2,     2,     3,     1,     3,     1,     1,     3,     4,
       3,     0,     1,     1,     3,     1,     1,     0,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     3,     5,     1,
       3,     5,     4,     3,     1,     0,     1,     0,     4,     1,
       2,     1,     4,     3,     0,     3,     1,     1,     0,     0,
       2,     0,     4,     0,     7,     3,     4,     0,     7,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     0,     4,     0,     4,     0,
       4,     0,     4,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       0,     5,     0,     0,     7,     0,     5,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     0,     3,     1,     1,
       1,     3,     2,     1,     0,    10,     0,    11,     2,     2,
       4,     4,     4,     4,     4,     4,     3,     1,     0,     4,
       3,     4,     1,     2,     0,     3,     0,     5,     0,     4,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     3,
       1,     1,     3,     2,     1,     3,     2,     1,     1,     0,
       0,     6,     1,     2,     0,     2,     0,     2,     1,     0,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     3,
       2,     2,     2,     4,     3,     1,     1,     3,     1,     1,
       1,     1,     3,     2,     1,     3,     3,     1,     0,     2,
       0,     1,     5,     3,     3,     1,     1,     1,     3,     3,
       1,     1,     1,     0,     0,     7,     1,     2,     0,     0,
       4,     4,     4,     0,     2,     1,     1,     0,     1,     2,
       3,     3,     1,     4,     0,     5,     1,     1,     1,     1,
       2,     1,     4,     4,     1,     1,     4,     0,     1,     1,
       0,     2,     4,     4,     1,     1,     3,     1,     2,     3,
       1,     1,     0,     5,     0,     0,     2,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     2,     1,     2,     1,
       0,     5,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     2,     2,     4,     2,     2,     1,     0,     4,
       1,     1,     3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       5,     0,     3,     1,     0,     0,     0,     0,     0,     0,
       0,   343,     0,     0,     0,     0,   336,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   505,     0,     0,   396,
       0,   404,   405,     6,   428,   485,    66,   406,     0,    46,
      43,     0,     0,    75,     0,     0,     0,     0,   357,     0,
       0,    78,     0,     0,     0,     0,     0,   380,     0,     0,
       0,     0,   112,   114,   118,     0,     0,   437,   410,   411,
     412,   407,   408,     0,     0,   413,   409,     0,     0,    77,
      29,   497,   399,     0,   431,     4,     0,     8,    34,     9,
      10,   102,   103,     0,     0,   319,   447,     0,   339,   340,
     104,   478,     0,   434,   338,     0,   318,   446,     0,   450,
     378,   481,     0,   477,   456,   476,   479,   484,     0,   327,
     430,   429,   343,     6,   380,     0,   104,   536,   535,     0,
     533,   532,   342,   447,     0,   450,   305,   306,   307,   308,
       0,   334,   333,   332,   331,   330,   329,   328,   380,     0,
       0,   364,     0,   284,   452,     0,   282,     0,   510,     0,
     440,   269,     0,     0,   381,   387,   261,   388,     0,   392,
     479,     0,     0,   335,   398,     0,    37,    35,   487,     0,
     244,     0,     0,   245,     0,     0,    53,    55,     0,    57,
       0,     0,     0,    59,   447,     0,   450,     0,     0,     0,
      21,     0,    20,   187,     0,     0,   186,   115,   113,   192,
       0,   104,     0,     0,     0,     0,   263,   505,   519,     0,
     415,     0,     0,     0,   517,     0,    15,     0,   433,   319,
       0,     0,    27,     0,   400,     0,   401,     0,     0,     0,
       0,     0,    18,   116,   110,    62,   487,   487,   105,   344,
       0,     0,   289,     0,   291,   322,   285,   287,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    67,   283,
     281,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   487,   453,   487,     0,   498,
     480,   344,     0,     0,   337,     0,   368,     0,     0,   514,
     451,     0,   356,   441,   506,     0,   383,     0,   402,     0,
     389,   480,   397,    40,     0,   488,     0,     0,    65,     0,
      44,     0,   246,   249,   447,   450,     0,     0,    56,    58,
      82,     0,    60,    61,    29,    81,    23,     0,     0,    17,
       0,   188,   450,     0,    63,     0,     0,    64,   346,     0,
      99,   101,   447,   450,     0,   537,   447,   450,     0,   504,
       0,   520,     0,   414,   518,   428,     0,     0,   516,   436,
     515,   432,     5,    12,    13,     0,   320,   449,   448,    36,
       0,     0,   341,   435,     7,     0,   365,     0,     0,   108,
     119,     0,     0,   106,     0,   487,   542,   545,     0,   470,
     468,   370,     0,     0,   293,     0,   325,     0,     0,     0,
     294,   296,   295,   310,   309,   312,   311,   313,   315,   316,
     314,   304,   303,   298,   299,   297,   300,   301,   302,   317,
       0,   265,   280,   279,   278,   277,   276,   275,   274,   273,
     272,   271,   270,   379,   543,   471,   374,     0,     0,     0,
       0,   534,   447,   450,   366,   495,     0,   509,     0,   508,
     382,   470,   403,   262,   471,     0,    29,    38,   354,   243,
      47,     0,    49,   247,    72,    69,     0,     0,   143,   143,
      54,     0,     0,   438,   406,     0,   426,     0,     0,     0,
     418,     0,   416,    26,   425,   417,    27,     0,    22,    19,
       0,   185,   193,   190,     0,     0,     0,   538,   529,   531,
     530,    11,     0,   501,     0,   500,   355,     0,   522,     0,
     523,   525,     0,     3,     5,   369,   258,     0,    28,    30,
      31,    32,   486,     0,   176,     0,   447,     0,   450,     0,
       0,     0,   384,   117,   121,     0,     0,   353,   352,     0,
     172,     0,     0,     0,     0,   469,   290,   292,     0,     0,
     286,   288,     0,   266,     0,     0,   473,   490,   454,   489,
     494,   482,   483,   513,   512,     0,   390,    27,   152,     0,
      29,   150,    45,   245,     0,     0,     0,     0,     0,     0,
     143,     0,   143,     0,   421,   422,   445,     0,   440,   438,
       0,     0,   420,     0,    83,    24,   189,     0,   172,   100,
      68,     0,   502,   504,     0,   526,   528,   527,     0,     0,
     367,    16,     3,   321,     0,     0,   181,     0,   177,   178,
      25,     0,   386,     0,     0,   120,   123,   195,   172,   173,
     174,     0,   162,     0,   175,   475,   496,   373,   371,   326,
     323,   267,   377,   375,   491,   467,   487,     0,     0,   507,
     394,   155,   158,     0,    27,     0,   248,     0,     0,   125,
     127,   125,   137,     0,    29,   135,    76,     0,     0,     0,
       0,     0,   139,     0,   424,   441,   439,     0,   419,   427,
     544,     0,    79,   191,     0,   539,   504,   499,     0,   521,
       0,    14,   487,     0,   257,   256,   260,   251,     0,     0,
     447,   450,   385,   122,   195,     0,   227,     0,   358,   172,
       0,   164,     0,   402,   466,   465,     0,   458,     0,     0,
     511,   391,   160,     0,     0,    39,    48,     0,    50,   128,
     129,     0,     0,     0,     0,    27,     0,   141,     0,   148,
     149,   146,   140,   444,   443,   423,    84,    89,   358,     0,
     264,   524,     0,   459,   250,   487,     0,    33,   184,   227,
     124,     0,     0,   231,   232,   233,   236,   235,   234,   226,
     111,   194,   199,   196,     0,   225,   229,     0,     0,     0,
       0,     0,   165,     0,   324,   268,   487,   487,   464,   455,
     492,   493,     0,   393,     0,     0,     0,   153,   159,   151,
     245,   504,   126,    73,    70,   138,     0,   142,   144,    29,
       0,     0,    90,    80,     0,   503,   253,   467,     0,   255,
     109,     0,     0,   203,     0,   104,   230,     0,   198,    29,
       0,    29,     0,   168,     0,   167,     0,     0,   457,   395,
       0,    29,     0,     0,     0,     0,     0,     0,   136,    29,
      27,   442,    85,     0,    29,   460,   252,     0,     0,   205,
     207,   202,   239,     0,     0,     0,    27,     0,   362,     0,
      27,   169,     0,   166,   461,   462,   156,    27,    42,   154,
      51,   130,    29,   133,    74,    71,    27,     0,    29,    27,
     242,   204,     6,     0,   208,   209,     0,     0,   217,     0,
       0,     0,     0,   197,   200,     0,   107,   363,     0,   359,
     345,     0,   171,    29,     0,    27,     0,    27,   347,   206,
     210,   211,   221,     0,   212,     0,   240,   237,     0,   241,
       0,   360,   170,    27,    29,   131,    52,     0,    86,    91,
       0,   220,   213,   214,   218,     0,   172,   361,    27,   134,
       0,   219,     0,   238,     0,     0,    29,   215,     0,   132,
      27,   223,    29,   201,    87,    27,    93,   224,     0,    88,
      92,    94,     0,    95,    96,     0,     0,    97,     0,    29,
      27,    98
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    84,    85,   534,   382,   201,   202,
      86,   232,   390,   538,   903,    88,   324,   588,   476,   671,
     182,   481,   181,   589,   593,   820,   934,   337,   596,   867,
     595,   866,   185,   197,   767,   702,   831,   907,   970,   986,
     833,   873,   989,   990,   991,   995,   998,   359,   360,    89,
      90,   249,    91,   559,    92,   554,   400,    93,   399,    94,
     556,   644,   645,   752,   679,   821,   956,   904,   686,   487,
     490,   601,   869,   829,   761,   592,   672,   863,   742,   933,
     745,   816,   651,   652,   653,   472,   545,   205,   206,   210,
     726,   791,   844,   948,   792,   842,   881,   913,   914,   915,
     916,   962,   917,   918,   919,   960,   983,   793,   794,   795,
     796,   883,   797,   179,   331,   332,   594,   714,   715,   716,
     776,   633,   634,    95,   318,    96,   369,   733,   418,   419,
     413,   415,   536,   417,   732,   568,   140,   404,   514,    97,
      98,    99,   126,   800,   889,   101,   240,   532,   385,   564,
     563,   575,   574,   293,   102,   654,   166,   167,   475,   670,
     741,   813,   173,   235,   473,   103,   606,   504,   104,   607,
     314,   608,   105,   106,   107,   309,   108,   109,   458,   665,
     809,   717,   837,   734,   735,   736,   737,   110,   111,   112,
     113,   250,   114,   115,   116,   117,   326,   578,   664,   579,
     580,   118,   524,   525,   706,   159,   160,   223,   224,   527,
     628,   119,   364,   621,   365,   120,   505,   121
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -770
static const yytype_int16 yypact[] =
{
    -770,    72,    80,  -770,  1334,  3870,  3870,   -49,  3870,  3870,
    3870,  3870,  3870,  3870,  3870,  3870,  -770,  3870,  3870,  3870,
    3870,  3870,  3870,  3870,   -40,   -40,  2540,  3870,   146,   -17,
      87,  -770,  -770,    84,  -770,  -770,  -770,   184,  3870,  -770,
    -770,   110,   112,  -770,    87,  2673,  2806,   193,  -770,   196,
    2939,  -770,  3870,   -15,    39,   152,   155,     4,   126,   136,
     148,   182,  -770,  -770,  -770,   220,   227,  -770,  -770,  -770,
    -770,  -770,  -770,   210,   157,  -770,  -770,   223,  4003,  -770,
    -770,   154,   218,   273,   -69,  -770,    29,  -770,  -770,  -770,
    -770,  -770,  -770,   288,   313,  -770,  -770,   230,   325,   328,
     363,   335,   257,  -770,  -770,  1325,  -770,  -770,    67,  1759,
    -770,  -770,   261,   347,   287,  -770,   128,  -770,    96,  -770,
    -770,  -770,  -770,  -770,   321,   279,   363,  4931,  4931,  3870,
    4931,  4931,  1481,   -35,  4694,  1010,  -770,  -770,   373,  -770,
    3870,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,   291,
     223,   -67,   297,  -770,  -770,   300,  -770,   -40,  4737,   268,
     440,  -770,   326,   223,   333,   304,  -770,  -770,   324,   362,
      68,    96,  3072,  -770,  -770,  4003,   466,  -770,  3870,    30,
    4931,  2274,    87,  3870,  3870,   344,  -770,  -770,  4482,  -770,
    4526,   351,   486,  -770,   352,  4931,  1523,   383,  4568,   223,
     -31,    33,  -770,  -770,   272,    38,  -770,  -770,  -770,   504,
      43,   363,   -40,  3870,  3870,   381,  -770,  2540,   169,   -46,
    -770,  4136,   -40,   320,  -770,   223,  -770,   303,   -42,   395,
     400,  4610,   384,  3870,    -8,   397,   358,    -8,   161,   498,
     428,   511,  -770,   461,  -770,  -770,  3870,  3870,  -770,   516,
     527,   165,  -770,  3870,  -770,   564,  -770,  -770,  3870,  3870,
    3870,  3870,  3870,  3870,  3870,  3870,  3870,  3870,  3870,  3870,
    3870,  3870,  3870,  3870,  3870,  3870,  3870,   146,  -770,  -770,
    -770,  3205,  3870,  3870,  3870,  3870,  3870,  3870,  3870,  3870,
    3870,  3870,  3870,   428,   156,  3870,  -770,  3870,  3870,   154,
     -22,  -770,  4652,  3870,  -770,   223,   -54,   176,   176,  -770,
    -770,  3338,  -770,  3471,  -770,   223,   333,   162,   428,   162,
    -770,   -13,  -770,  -770,  2274,  4931,   435,  3870,  -770,   514,
    -770,   447,   593,  4931,   512,  2688,   533,    62,  -770,  -770,
    -770,  4804,  -770,  -770,  -770,  -770,   130,   535,   -15,  -770,
    3870,  -770,  -770,    39,  -770,  4804,   534,  -770,  -770,    14,
    -770,  -770,    19,   804,    40,  -770,   458,  1572,   459,   284,
     462,  -770,   539,  -770,  -770,   551,  1256,   465,  -770,  -770,
    -770,   229,  -770,  -770,  -770,   428,  -770,  -770,  -770,  -770,
    1609,  4270,  -770,  -770,  -770,  2407,  -770,   607,   117,  -770,
     499,   474,   476,  -770,   472,  3870,   488,  -770,  3870,   489,
     -13,  -770,    96,  3870,  5010,  3870,  -770,  3870,  3870,  3870,
    2457,  2590,  2721,  2854,  2854,  2854,  2854,  1213,  1213,  1213,
    1213,   582,   582,   519,   519,   519,   373,   373,   373,  -770,
     277,  1481,  1481,  1481,  1481,  1481,  1481,  1481,  1481,  1481,
    1481,  1481,  1481,  -770,   488,   491,  -770,   490,   176,   493,
    4312,  -770,   102,  1096,   -45,  -770,   -40,  4931,   -40,  4796,
     333,  -770,  -770,  -770,  -770,   176,  -770,  -770,  -770,  4931,
    -770,  1742,  -770,  -770,  -770,  -770,   631,    48,   495,   501,
    -770,  4804,  4804,  4804,  -770,   500,  -770,   -12,   496,   223,
     -67,   506,  -770,  -770,  -770,  -770,   505,   583,  -770,  -770,
    4354,  -770,  -770,   642,   508,   -40,   517,  -770,  -770,  -770,
    -770,  -770,   521,  -770,    50,  -770,  -770,   364,  -770,  3870,
    -770,  -770,   428,   518,  -770,  -770,   189,   524,  -770,  -770,
    -770,  -770,  -770,   -40,  -770,    53,    55,   525,   994,  4804,
     528,   223,   333,  -770,   555,   117,   526,  -770,  -770,   530,
     330,   529,  4398,   428,   428,   -13,  4972,  1481,  3870,  4859,
    5055,  5076,   146,  -770,   428,   428,  -770,  -770,  -770,     9,
    -770,  -770,  -770,  -770,  -770,  3604,  -770,   442,  -770,    87,
    -770,  -770,  -770,  3870,  3870,   254,   254,  4804,   611,  1875,
    -770,   422,  -770,   275,  -770,  -770,   558,   537,   677,  4804,
     547,   223,   -54,     1,   589,  -770,  -770,  4804,   330,  -770,
    -770,  3870,  -770,   284,   682,  -770,  -770,  -770,   541,   114,
    -770,  -770,   544,  -770,   204,   550,  -770,  3737,  -770,  -770,
    -770,   223,   333,   117,   549,   698,  -770,  -770,   330,  -770,
    -770,   556,   699,   195,  -770,  -770,  -770,  -770,  -770,  5033,
    -770,  -770,  -770,  -770,  -770,   559,  3870,  3870,   -40,  4931,
    -770,  -770,   394,   567,   628,   568,  4931,   -40,   573,   599,
    -770,   599,  -770,   719,  -770,  -770,  -770,   449,   584,  3870,
      12,   311,  -770,  4804,  -770,  4804,  -770,   586,   -45,  -770,
    -770,   594,  -770,  -770,   588,  -770,   284,  -770,  3870,  -770,
     591,  -770,  3870,   176,   619,   239,  -770,  -770,   595,   -40,
      56,  1083,   333,   698,  -770,   117,   736,   597,   644,   379,
     678,   737,  3870,   428,   691,   692,   428,  -770,   598,  4440,
    -770,   632,   411,    87,  2274,  -770,  -770,   603,  -770,  -770,
    -770,   254,   608,   610,  4804,   669,   612,  -770,  1525,  -770,
    -770,  -770,  -770,  -770,   637,  -770,  -770,   662,   644,    57,
    1481,  -770,   616,  -770,  -770,  3870,   619,  -770,  -770,   867,
    -770,   695,   117,  -770,  -770,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,   321,   605,  -770,    44,   614,   618,
     620,   209,   761,  4804,  5033,  -770,  3870,  3870,  -770,   619,
    -770,  -770,   176,  -770,    87,   749,   708,  -770,  -770,  -770,
    3870,   284,  -770,  -770,  -770,  -770,   627,  -770,  -770,  -770,
    4804,   117,  -770,  -770,   629,  -770,  -770,   559,   624,   619,
    -770,   768,    25,  -770,   712,   363,  -770,   715,  -770,  -770,
     224,  -770,   714,   775,  4804,  -770,   633,   634,  -770,  -770,
     766,  -770,   641,  2274,   646,    60,  2008,  2008,  -770,  -770,
     419,  -770,  -770,   645,  -770,  -770,  -770,  4804,   117,  -770,
     190,  -770,   782,    46,   729,   789,   649,   731,  -770,    61,
     654,   796,  4804,  -770,  -770,  -770,  -770,   744,  -770,  -770,
    -770,  -770,  -770,  -770,  -770,  -770,   427,   740,  -770,   660,
    -770,  -770,   741,   676,   190,  -770,   680,   743,   733,   690,
     706,  4804,   781,  -770,  -770,  4804,  -770,  -770,   293,  -770,
    -770,  4804,  -770,  -770,  2141,   769,   707,   705,  -770,  -770,
    -770,  -770,   554,   117,  -770,   790,  -770,   849,   716,  -770,
     795,  -770,  -770,   482,  -770,  -770,  -770,   717,  -770,  -770,
     799,   800,   866,  -770,  -770,  4804,   330,  -770,   797,  -770,
     721,  -770,   117,  -770,   724,   728,  -770,  -770,   336,  -770,
     727,  -770,  -770,  -770,  -770,   730,   783,  -770,   738,  -770,
     783,  -770,   117,  -770,  -770,   810,   742,  -770,   734,  -770,
     745,  -770
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -770,  -770,  -362,  -770,   -24,  -770,  -770,  -770,  -770,   545,
    -770,  -138,  -770,  -770,     3,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,   -98,  -770,  -770,  -770,   380,   509,
     513,  -123,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,   253,   228,  -575,  -770,  -770,    37,  -770,  -770,
    -770,  -383,  -770,  -770,   147,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -605,  -770,   179,  -198,  -770,  -770,   557,  -770,
     187,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,    -1,
    -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,
    -769,  -770,  -770,  -770,  -576,  -770,  -770,   138,  -770,  -770,
    -770,  -770,  -770,   837,  -770,    -3,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,  -770,   -60,
    -770,  -770,    -2,   160,  -770,  -770,  -770,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,   -19,  -386,  -263,  -770,  -770,  -770,
    -770,  -770,  -770,  -770,   183,   387,   144,  -770,  -770,   331,
     337,  -770,   912,   -14,   739,  -443,   473,    20,  -770,  -770,
    -770,  -594,  -770,  -770,  -770,  -770,   107,  -232,  -770,    66,
    -770,  -770,  -770,   -18,    -4,  -770,  -211,  -464,  -770,  -770,
     106,    65,  -647,   329,  -770,   732,  -770,   479,   322,  -770,
    -770,  -770,  -770,  -770,   332,  -770,  -770,  -770
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -542
static const yytype_int16 yytable[] =
{
     151,   151,   100,   301,   164,   152,   152,    87,   133,   165,
     169,   586,   553,   704,   439,   174,   176,   675,   230,   409,
     533,   681,   515,   583,   170,   584,   846,  -541,   218,   200,
     186,   135,   123,   878,    35,   401,   402,   241,   327,   759,
     297,   348,   396,   727,   154,   154,   353,   194,   517,   297,
     227,   356,   847,   228,   922,   211,   598,   123,   623,   769,
     347,   637,   455,  -179,  -182,   623,   218,   610,   623,   928,
     196,   666,     3,   699,   148,   409,   455,  -381,   209,  -381,
      -2,   239,  -364,   239,   457,   471,   459,   474,   358,   488,
     155,   155,  -383,   171,   168,   453,   239,   373,   221,   222,
     636,  -382,   129,    48,  -383,   239,   603,   149,   239,  -368,
     150,   177,   230,   203,   300,   230,    81,  -348,  -348,   239,
     774,   700,   211,   252,   253,   254,   306,   279,   280,  -469,
     297,   220,   298,   151,   172,   199,   221,   222,   152,   316,
     255,   298,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   667,   277,   760,   516,   321,   330,   646,
      35,  -541,   632,   961,   865,   346,   822,   310,   879,   880,
     151,   334,   242,   328,   329,   152,   349,   535,   151,   123,
     297,   354,   518,   152,   561,   204,   357,   848,   151,   923,
     599,   381,   624,   152,   335,   638,   506,  -179,  -182,   835,
     362,   366,   901,   929,  -472,   858,   489,   687,   123,   691,
      35,   507,   298,   155,   352,   740,   577,   730,   454,   123,
      35,   371,   361,   363,   367,   218,    35,   406,   175,    35,
     378,   852,   377,   577,   864,   774,   178,   410,   465,   773,
      35,  -259,   299,   164,  -350,  -350,   887,   646,   165,   169,
     148,   183,   912,   184,   550,   191,   712,   551,   192,   731,
     155,   710,   207,   170,  -472,   208,   778,   212,   155,  -468,
     239,   464,   298,   853,   218,   407,   677,   213,   155,   219,
     410,   470,   218,   162,   372,   123,   163,   234,   888,   214,
     462,   775,    81,   410,   410,   221,   222,   225,   233,   661,
     408,   226,    81,   410,  -259,   410,   412,   500,    81,   408,
     393,    81,   501,   463,   200,   950,   123,   477,    35,   713,
     408,   500,    81,   215,   630,   547,   501,   550,   587,   780,
     551,   572,   171,   168,   123,   151,    35,   218,   859,   123,
     152,    35,   237,   220,   221,   222,   123,   411,    35,   412,
     243,   974,   221,   222,  -254,   657,   658,   951,   148,   689,
     690,   216,   412,   412,   552,  -382,   662,   663,   217,   239,
    -366,   678,   412,   245,   412,   244,   148,   246,   100,   523,
     247,   148,   546,   539,   218,   248,   843,  -474,   148,   378,
     456,   149,   123,   251,   150,   689,   690,   294,   565,   295,
      81,   522,   296,   411,   456,   548,   151,   221,   222,   149,
      48,   152,   150,   277,   149,   312,   350,   150,    81,   225,
     692,   149,   218,    81,   150,   155,   625,   378,   626,   627,
      81,   305,   151,   307,   151,   872,   308,   152,   313,   152,
     317,   123,   674,   239,   410,   738,   383,   384,   649,   650,
     573,   743,   744,   379,   221,   222,   762,   500,   500,   500,
     319,   410,   501,   501,   501,   612,   315,   550,   814,   815,
     551,   577,  -163,   239,   591,   503,   310,   320,   310,   981,
     982,   151,   911,   323,   920,   336,   152,   153,   156,   512,
     341,   772,   221,   222,   340,   342,   155,   649,   650,   -41,
     -41,   -41,  -147,  -147,  -147,   688,   689,   690,   355,   151,
    -145,  -145,  -145,   412,   152,   500,   550,   642,   920,   551,
     501,   552,   155,   368,   155,   361,   552,   344,   808,   389,
     412,   374,   756,   689,   690,   380,   755,   386,   164,  -157,
    -157,  -157,   387,   165,   169,   392,   374,   963,   380,   374,
     380,   236,   238,   310,   838,   274,   275,   276,   170,   277,
     394,   151,   151,   500,  -147,   673,   152,   152,   501,   395,
     577,   155,  -145,   397,   398,   500,   977,   698,   403,   405,
     501,   416,   478,   500,   552,   856,   857,   480,   501,   151,
     482,   483,   685,   484,   152,   486,   994,   508,   513,   155,
     519,   528,   521,   529,   526,   680,   680,   722,   362,   552,
     531,   549,   555,   560,   552,   271,   272,   273,   274,   275,
     276,   557,   277,   558,   720,   604,   605,   171,   168,  -495,
    -372,   363,  -376,   523,   151,   597,   611,   576,   600,   152,
     581,   609,   613,   151,   602,   615,   617,   721,   152,   618,
     614,   155,   155,   783,   784,   785,   786,   787,   788,   500,
     620,   500,   622,   631,   501,   635,   501,   639,   641,   643,
     647,   648,   151,   683,   693,   695,   655,   152,   310,   155,
     373,   870,   701,   640,   694,   151,   708,   749,   709,   711,
     152,   552,   718,   724,  -228,   552,   725,   729,   728,   410,
    -463,   886,   747,   890,   783,   784,   785,   786,   787,   788,
     746,   748,   884,   897,   750,   751,   523,   151,   502,   817,
     500,   906,   152,   754,   155,   501,   909,   757,   765,   310,
     768,   682,   502,   155,   713,   766,   771,   818,   777,   798,
     799,   803,   802,   806,   807,   810,   819,   812,   552,   826,
     823,   703,   824,   830,   935,   827,   832,   841,   849,   850,
     937,   680,   155,   836,   851,   854,   861,   862,   412,   500,
     868,   876,   877,   874,   501,   155,   882,   885,   891,   892,
     894,   895,   845,   896,   898,   953,   921,   151,   900,   908,
     860,   924,   152,   925,   926,   927,   500,   552,   410,   930,
     931,   501,  -540,  -161,   936,   938,   968,   155,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     500,   939,  -216,   941,   942,   501,   781,   763,   980,   764,
     943,   523,   782,   944,   985,   783,   784,   785,   786,   787,
     788,   789,   945,   500,   552,   947,   552,   957,   501,   958,
     959,  1000,   964,   965,  -452,  -452,   899,   966,   500,   967,
     969,   971,  -222,   501,   972,   976,   978,   412,   502,   502,
     502,   979,   984,   975,   996,   987,   988,   155,   999,   992,
     552,   790,   993,   509,   997,   619,   723,   500,   825,   540,
    1001,   500,   501,   541,   905,   828,   501,   500,   801,   753,
     511,   779,   501,   940,   839,   229,   805,   127,   128,   552,
     130,   131,   132,   134,   136,   137,   138,   139,   834,   141,
     142,   143,   144,   145,   146,   147,   502,   955,   158,   161,
     697,   500,   552,   351,   875,   696,   501,   855,   552,   370,
     180,     0,   707,   705,     0,     0,  -540,   188,   190,     0,
       0,     0,   195,     0,   198,     0,     0,   781,   552,     0,
       0,     0,     0,   782,   871,     0,   783,   784,   785,   786,
     787,   788,   789,     0,   502,     0,     0,     0,     0,     0,
     231,     0,     0,     0,     0,     0,   502,     0,   893,     0,
       0,     0,  -180,     0,   502,     0,     0,     0,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   910,   840,     0,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   932,     0,     0,     0,
       0,   302,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   304,     0,  -452,  -452,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   946,     0,     0,     0,   949,
    -452,  -452,     0,     0,     0,   952,     0,     0,     0,     0,
     502,     0,   502,     0,   231,     0,     0,   231,     0,     0,
     325,  -183,     0,     0,     0,   333,   195,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   973,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,     0,     0,   195,   195,     0,     0,   158,
       0,     0,     0,   376,     0,     0,     0,     0,     0,     0,
       0,   502,     0,  -452,  -452,   391,  -180,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -452,  -452,   325,   325,
       0,     0,  -349,  -349,     0,   414,     0,     0,     0,     0,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,     0,
     502,     0,     0,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,     0,   325,     0,   325,
     460,     0,     0,     0,     0,   195,     0,   502,     0,     0,
       0,     0,     0,   467,     0,   469,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -183,     0,     0,     0,   479,
       0,   502,     0,     0,     0,     0,     0,     0,  -351,  -351,
    -542,  -542,  -542,  -542,   269,   270,   271,   272,   273,   274,
     275,   276,   510,   277,   502,   252,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   502,
       0,     0,   255,     0,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,     0,   277,   195,   502,     0,
       0,     0,   502,     0,     0,     0,     0,   325,   502,     0,
     562,     0,     0,     0,     0,   566,     0,   567,     0,   569,
     570,   571,     0,     0,   252,   253,   254,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,    11,     0,     0,
       0,   255,   502,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,     0,   277,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,   530,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,    49,    50,    51,     0,     0,    52,
      53,   629,    54,     0,     0,     0,    55,    56,    57,     0,
      58,    59,    60,    61,    62,    63,    64,     0,     0,     0,
       0,    65,    66,     0,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,    73,     0,   278,     0,
     659,    74,    75,    76,    77,    78,     0,    79,    80,     0,
      81,     0,    82,    83,     0,     0,     0,   669,     0,     0,
       0,     0,     0,     0,     0,   333,   676,   255,     0,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
       0,   277,     0,   195,   252,   253,   254,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   195,
       0,   255,   759,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,     0,   277,     0,     0,   325,   739,
       0,     0,     0,  -452,  -452,     0,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,     0,
       0,   758,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
     770,    10,    11,     0,   325,     0,     0,     0,     0,     0,
       0,     0,  -452,  -452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   804,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,   343,     0,   760,    31,
      32,    33,    34,    35,     0,    36,     0,   325,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,     0,    52,     0,     0,    54,   325,   325,
       0,    55,    56,    57,   520,    58,    59,    60,   537,    62,
      63,    64,   333,     0,     0,     0,    65,    66,     0,    67,
      68,    69,    70,    71,    72,     5,     6,     7,     8,     9,
       0,    73,     0,     0,    10,    11,   125,    75,    76,    77,
      78,     0,    79,    80,     0,    81,     0,    82,    83,   590,
       0,     0,     0,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,  -452,
    -452,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,     0,    52,     0,     0,
      54,     0,     0,     0,     0,     0,    57,     0,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,    69,    70,    71,    72,     5,     6,
       7,     8,     9,     0,    73,     0,     0,    10,    11,   125,
      75,    76,    77,    78,     0,    79,    80,     0,    81,     0,
      82,    83,   684,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,     0,
      52,     0,     0,    54,     0,     0,     0,     0,     0,    57,
       0,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,     0,    65,    66,     0,    67,    68,    69,    70,    71,
      72,     5,     6,     7,     8,     9,     0,    73,     0,     0,
      10,    11,   125,    75,    76,    77,    78,     0,    79,    80,
       0,    81,     0,    82,    83,   902,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,     0,    52,     0,     0,    54,     0,     0,     0,
       0,     0,    57,     0,    58,    59,    60,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,     0,    67,    68,
      69,    70,    71,    72,     5,     6,     7,     8,     9,     0,
      73,     0,     0,    10,    11,   125,    75,    76,    77,    78,
       0,    79,    80,     0,    81,     0,    82,    83,   954,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,     0,    52,     0,     0,    54,
       0,     0,     0,     0,     0,    57,     0,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,    69,    70,    71,    72,     5,     6,     7,
       8,     9,     0,    73,     0,     0,    10,    11,   125,    75,
      76,    77,    78,     0,    79,    80,     0,    81,     0,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,     0,    52,
       0,     0,    54,     0,     0,     0,     0,     0,    57,     0,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,    69,    70,    71,    72,
       5,     6,     7,     8,     9,     0,    73,     0,     0,    10,
      11,   125,    75,    76,    77,    78,     0,    79,    80,     0,
      81,     0,    82,    83,     0,     0,     0,     0,     0,   543,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,   123,
      34,    35,     0,     0,     0,     0,     0,    37,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,    48,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   124,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,     0,    65,    66,     0,    67,    68,    69,
      70,    71,    72,     5,     6,     7,     8,     9,     0,    73,
       0,     0,    10,   122,   125,    75,    76,    77,    78,   544,
       0,     0,     0,    81,     0,    82,    83,     0,     0,     0,
       0,     0,   157,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,   123,    34,    35,     0,     0,     0,     0,     0,
      37,     0,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,    48,
     277,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   124,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,    69,    70,    71,    72,     5,     6,     7,     8,
       9,     0,    73,     0,     0,    10,   122,   125,    75,    76,
      77,    78,     0,     0,     0,     0,    81,     0,    82,    83,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,   123,    34,    35,  -452,  -452,
       0,     0,     0,    37,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
       0,   277,    48,     0,     0,     0,     0,     0,     0,   485,
       0,     0,     0,     0,     0,     0,     0,   124,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,    69,    70,    71,    72,     5,
       6,     7,     8,     9,     0,    73,     0,     0,    10,   122,
     125,    75,    76,    77,    78,     0,   187,     0,     0,    81,
       0,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,   123,    34,
      35,     0,     0,     0,     0,     0,    37,  -542,  -542,  -542,
    -542,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,     0,   277,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     124,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,    69,    70,
      71,    72,     5,     6,     7,     8,     9,     0,    73,     0,
       0,    10,   122,   125,    75,    76,    77,    78,     0,   189,
       0,     0,    81,     0,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,   123,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   124,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,     0,    65,    66,     0,    67,
      68,    69,    70,    71,    72,     5,     6,     7,     8,     9,
       0,    73,     0,     0,    10,    11,   125,    75,    76,    77,
      78,     0,   193,     0,     0,    81,     0,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,   123,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   124,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      66,     0,    67,    68,    69,    70,    71,    72,     5,     6,
       7,     8,     9,     0,    73,     0,     0,    10,   122,   125,
      75,    76,    77,    78,   322,     0,     0,     0,    81,     0,
      82,    83,     0,     0,     0,     0,     0,   440,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,   123,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   124,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,     0,    65,    66,     0,    67,    68,    69,    70,    71,
      72,     5,     6,     7,     8,     9,     0,    73,     0,     0,
      10,   122,   125,    75,    76,    77,    78,     0,     0,     0,
       0,    81,     0,    82,    83,     0,     0,     0,     0,     0,
     466,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
     123,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   124,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,     0,    65,    66,     0,    67,    68,
      69,    70,    71,    72,     5,     6,     7,     8,     9,     0,
      73,     0,     0,    10,   122,   125,    75,    76,    77,    78,
       0,     0,     0,     0,    81,     0,    82,    83,     0,     0,
       0,     0,     0,   468,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,   123,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   124,     0,     0,    59,    60,
       0,     0,     0,     0,     0,     0,     0,     0,    65,    66,
       0,    67,    68,    69,    70,    71,    72,     5,     6,     7,
       8,     9,     0,    73,     0,     0,    10,   122,   125,    75,
      76,    77,    78,     0,     0,     0,     0,    81,     0,    82,
      83,     0,     0,     0,     0,     0,   668,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,   123,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   124,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,    69,    70,    71,    72,
       5,     6,     7,     8,     9,     0,    73,     0,     0,    10,
     122,   125,    75,    76,    77,    78,     0,     0,     0,     0,
      81,     0,    82,    83,     0,     0,     0,     0,     0,   719,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,   123,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   124,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,     0,    65,    66,     0,    67,    68,    69,
      70,    71,    72,     5,     6,     7,     8,     9,     0,    73,
       0,     0,    10,   122,   125,    75,    76,    77,    78,     0,
       0,     0,     0,    81,     0,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,   123,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   124,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    66,     0,
      67,    68,    69,    70,    71,    72,     5,     6,     7,     8,
       9,     0,    73,     0,     0,    10,    11,   125,    75,    76,
      77,    78,     0,     0,     0,     0,    81,     0,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,   123,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   124,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,     0,
      65,    66,     0,    67,    68,    69,    70,    71,    72,     5,
       6,     7,     8,     9,     0,    73,     0,     0,    10,   122,
     125,    75,    76,    77,    78,     0,     0,     0,     0,    81,
       0,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,   123,   375,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     124,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,     0,    65,    66,     0,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,    73,   252,
     253,   254,     0,   125,    75,    76,    77,    78,     0,     0,
       0,     0,    81,     0,    82,    83,   255,     0,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,     0,
     277,   252,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,     0,   277,   252,   253,   254,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     255,     0,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,     0,   277,     0,     0,   252,   253,   254,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   542,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,     0,   277,   252,
     253,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   582,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,     0,
     277,   252,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,   616,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,     0,   277,     0,     0,   252,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   255,   656,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,     0,   277,   252,   253,   254,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   811,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,     0,   277,   252,
     253,   254,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   338,   255,     0,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,     0,
     277,   252,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,   339,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,     0,   277,   252,   253,   254,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     255,   345,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,     0,   277,     0,   252,   253,   254,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   388,   255,     0,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,     0,   277,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   461,   252,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     303,     0,   255,     0,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,     0,   277,   491,   492,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   311,     0,     0,   493,     0,   252,   253,
     254,     0,     0,     0,    31,    32,   123,     0,     0,     0,
       0,     0,     0,     0,   494,   255,   660,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,     0,   277,
       0,     0,     0,     0,     0,     0,     0,     0,   148,     0,
       0,     0,   585,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   495,     0,   496,    68,    69,    70,    71,    72,
     252,   253,   254,     0,     0,     0,   497,     0,     0,     0,
       0,   498,    75,    76,   499,     0,     0,   255,     0,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
       0,   277,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,     0,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   254,   277,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,     0,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,     0,
     277,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,     0,   277,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,     0,   277,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,     0,   277
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-770)))

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-542)))

static const yytype_int16 yycheck[] =
{
      24,    25,     4,   126,    28,    24,    25,     4,    11,    28,
      28,   475,   398,   618,   277,    29,    30,   593,    78,   251,
     382,   596,     8,   466,    28,   468,   795,     8,    74,    53,
      44,    11,    72,     8,    74,   246,   247,     8,     8,    27,
      62,     8,   240,   648,    24,    25,     8,    50,     8,    62,
      74,     8,     8,    77,     8,    57,     8,    72,     8,   706,
      91,     8,   294,     8,     8,     8,    74,    79,     8,     8,
      50,    62,     0,    72,   114,   307,   308,   146,    74,   146,
       0,   150,   151,   150,   295,   317,   297,   319,   211,    27,
      24,    25,   146,    28,    28,   293,   150,   143,   144,   145,
     543,   146,   151,    99,   146,   150,   489,   147,   150,   151,
     150,    27,   172,    74,   118,   175,   156,   152,   153,   150,
     714,   120,   124,     9,    10,    11,   150,    60,    61,   151,
      62,   143,   154,   157,   151,   150,   144,   145,   157,   163,
      26,   154,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   154,    50,   153,   152,   171,   182,   555,
      74,   152,   534,   942,   821,   199,   751,   157,   153,   154,
     204,   184,   153,   153,   181,   204,   153,   385,   212,    72,
      62,   153,   152,   212,   405,   156,   153,   153,   222,   153,
     152,   225,   152,   222,   184,   152,   344,   152,   152,   152,
     213,   214,   152,   152,   146,   809,   154,   600,    72,   602,
      74,    91,   154,   157,   204,   668,   458,    32,    72,    72,
      74,    62,   212,   213,   214,    74,    74,    72,   151,    74,
      79,    32,   222,   475,   820,   839,    62,   251,    72,   713,
      74,    62,   156,   277,   152,   153,    32,   643,   277,   277,
     114,   151,    72,   151,   147,    72,    62,   150,    72,    74,
     204,   157,   120,   277,   146,   120,   719,   151,   212,   151,
     150,   305,   154,    74,    74,   120,    32,   151,   222,    79,
     294,   315,    74,   147,   125,    72,   150,    79,    74,   151,
     303,    62,   156,   307,   308,   144,   145,   150,   154,   572,
     154,   154,   156,   317,   125,   319,   251,   341,   156,   154,
     159,   156,   341,   303,   348,    32,    72,   324,    74,   125,
     154,   355,   156,   151,   532,   395,   355,   147,   476,   725,
     150,    64,   277,   277,    72,   369,    74,    74,   812,    72,
     369,    74,    79,   143,   144,   145,    72,   251,    74,   294,
      72,   966,   144,   145,   125,   563,   564,    74,   114,    94,
      95,   151,   307,   308,   398,   146,   574,   575,   151,   150,
     151,   127,   317,   153,   319,    72,   114,    62,   390,   369,
      62,   114,   395,   390,    74,    32,   782,    62,   114,    79,
     294,   147,    72,   146,   150,    94,    95,   146,   412,    62,
     156,   127,   125,   307,   308,   395,   440,   144,   145,   147,
      99,   440,   150,    50,   147,   157,   154,   150,   156,   150,
     155,   147,    74,   156,   150,   369,    72,    79,    74,    75,
     156,   150,   466,   146,   468,   831,   146,   466,     8,   468,
     146,    72,   590,   150,   458,   666,   153,   154,   128,   129,
     440,    67,    68,   143,   144,   145,   155,   491,   492,   493,
     146,   475,   491,   492,   493,   499,   150,   147,    67,    68,
     150,   713,   152,   150,   481,   341,   466,   125,   468,   153,
     154,   515,   878,    27,   880,   151,   515,    24,    25,   355,
      14,   712,   144,   145,   153,   153,   440,   128,   129,    67,
      68,    69,    93,    94,    95,    93,    94,    95,    14,   543,
      93,    94,    95,   458,   543,   549,   147,   551,   914,   150,
     549,   555,   466,   152,   468,   515,   560,   154,   736,   155,
     475,   219,    93,    94,    95,   223,   684,   152,   572,    67,
      68,    69,   152,   572,   572,   158,   234,   943,   236,   237,
     238,    82,    83,   543,   775,    46,    47,    48,   572,    50,
      72,   595,   596,   597,   155,   589,   595,   596,   597,   151,
     812,   515,   155,    72,   123,   609,   972,   611,    72,    62,
     609,    27,   157,   617,   618,   806,   807,    83,   617,   623,
     153,     8,   599,    91,   623,    72,   992,    72,    74,   543,
     152,    72,   153,    62,   152,   595,   596,   641,   621,   643,
     155,    14,   123,   151,   648,    43,    44,    45,    46,    47,
      48,   157,    50,   157,   637,   491,   492,   572,   572,   151,
     151,   621,   151,   623,   668,    14,   150,   157,   153,   668,
     157,   151,   146,   677,   153,    72,    14,   637,   677,   151,
     155,   595,   596,   109,   110,   111,   112,   113,   114,   693,
     153,   695,   151,   155,   693,   151,   695,   152,   150,   124,
     154,   151,   706,    72,   126,     8,   157,   706,   668,   623,
     143,   829,   103,   549,   157,   719,    14,   677,   157,   155,
     719,   725,   152,   154,    99,   729,     8,     8,   152,   713,
     151,   849,    84,   851,   109,   110,   111,   112,   113,   114,
     153,   153,   845,   861,   151,   126,   706,   751,   341,   743,
     754,   869,   751,    14,   668,   754,   874,   153,   152,   719,
     152,   597,   355,   677,   125,   151,   155,   744,   153,   152,
     106,    14,    74,    62,    62,   157,   153,   125,   782,    90,
     152,   617,   152,   126,   902,   153,   104,    72,   154,   151,
     908,   751,   706,   157,   154,    14,    27,    69,   713,   803,
     153,   157,    14,   154,   803,   719,    74,    72,    74,    14,
     157,   157,   794,    27,   153,   933,    14,   821,   152,   154,
     814,    72,   821,    14,   155,    74,   830,   831,   812,   155,
      14,   830,     8,    69,    74,   155,   954,   751,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     854,   155,    91,   153,    91,   854,   100,   693,   976,   695,
     107,   821,   106,   153,   982,   109,   110,   111,   112,   113,
     114,   115,   146,   877,   878,    74,   880,    88,   877,   152,
     155,   999,    72,    14,    60,    61,   863,   151,   892,    74,
     153,    72,    72,   892,     8,   154,   152,   812,   491,   492,
     493,   153,   155,    86,    74,   155,   103,   821,   154,   151,
     914,   155,   990,   348,   152,   515,   643,   921,   754,   390,
     155,   925,   921,   390,   867,   758,   925,   931,   729,   681,
     353,   724,   931,   914,   776,    78,   733,     5,     6,   943,
       8,     9,    10,    11,    12,    13,    14,    15,   768,    17,
      18,    19,    20,    21,    22,    23,   549,   934,    26,    27,
     609,   965,   966,   204,   837,   608,   965,   803,   972,   217,
      38,    -1,   623,   621,    -1,    -1,   152,    45,    46,    -1,
      -1,    -1,    50,    -1,    52,    -1,    -1,   100,   992,    -1,
      -1,    -1,    -1,   106,   830,    -1,   109,   110,   111,   112,
     113,   114,   115,    -1,   597,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,   609,    -1,   854,    -1,
      -1,    -1,     8,    -1,   617,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,   877,   155,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,   892,    -1,    -1,    -1,
      -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,    -1,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   921,    -1,    -1,    -1,   925,
      60,    61,    -1,    -1,    -1,   931,    -1,    -1,    -1,    -1,
     693,    -1,   695,    -1,   172,    -1,    -1,   175,    -1,    -1,
     178,     8,    -1,    -1,    -1,   183,   184,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   965,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,   213,   214,    -1,    -1,   217,
      -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   754,    -1,    60,    61,   233,   152,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,   246,   247,
      -1,    -1,   152,   153,    -1,   253,    -1,    -1,    -1,    -1,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,    -1,
     803,    -1,    -1,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,    -1,    -1,   295,    -1,   297,
     298,    -1,    -1,    -1,    -1,   303,    -1,   830,    -1,    -1,
      -1,    -1,    -1,   311,    -1,   313,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,   327,
      -1,   854,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   350,    50,   877,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   892,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,   395,   921,    -1,
      -1,    -1,   925,    -1,    -1,    -1,    -1,   405,   931,    -1,
     408,    -1,    -1,    -1,    -1,   413,    -1,   415,    -1,   417,
     418,   419,    -1,    -1,     9,    10,    11,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,    -1,
      -1,    26,   965,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      76,   155,    -1,    -1,    80,    81,    82,    83,    -1,    85,
      -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,
      96,    97,    98,    99,   100,   101,   102,    -1,    -1,   105,
     106,   529,   108,    -1,    -1,    -1,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,   153,    -1,
     568,   147,   148,   149,   150,   151,    -1,   153,   154,    -1,
     156,    -1,   158,   159,    -1,    -1,    -1,   585,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   593,   594,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,   621,     9,    10,    11,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   637,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,   666,   667,
      -1,    -1,    -1,    60,    61,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,   689,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
     708,    12,    13,    -1,   712,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   732,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,   153,    -1,   153,    70,
      71,    72,    73,    74,    -1,    76,    -1,   775,    -1,    80,
      81,    82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,
      -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,
     101,   102,    -1,    -1,   105,    -1,    -1,   108,   806,   807,
      -1,   112,   113,   114,   152,   116,   117,   118,   119,   120,
     121,   122,   820,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,     3,     4,     5,     6,     7,
      -1,   142,    -1,    -1,    12,    13,   147,   148,   149,   150,
     151,    -1,   153,   154,    -1,   156,    -1,   158,   159,    27,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    76,    60,
      61,    -1,    80,    81,    82,    83,    -1,    85,    -1,    87,
      -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,   101,   102,    -1,    -1,   105,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,   114,    -1,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,   135,     3,     4,
       5,     6,     7,    -1,   142,    -1,    -1,    12,    13,   147,
     148,   149,   150,   151,    -1,   153,   154,    -1,   156,    -1,
     158,   159,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,    -1,
      85,    -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,
      -1,    96,    97,    98,    99,    -1,   101,   102,    -1,    -1,
     105,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,   114,
      -1,   116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
     135,     3,     4,     5,     6,     7,    -1,   142,    -1,    -1,
      12,    13,   147,   148,   149,   150,   151,    -1,   153,   154,
      -1,   156,    -1,   158,   159,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,
      82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,
      92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,
     102,    -1,    -1,   105,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,   114,    -1,   116,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,   135,     3,     4,     5,     6,     7,    -1,
     142,    -1,    -1,    12,    13,   147,   148,   149,   150,   151,
      -1,   153,   154,    -1,   156,    -1,   158,   159,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    82,    83,    -1,    85,    -1,    87,    -1,
      89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,
      99,    -1,   101,   102,    -1,    -1,   105,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,   114,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,   135,     3,     4,     5,
       6,     7,    -1,   142,    -1,    -1,    12,    13,   147,   148,
     149,   150,   151,    -1,   153,   154,    -1,   156,    -1,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    -1,    80,    81,    82,    83,    -1,    85,
      -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,
      96,    97,    98,    99,    -1,   101,   102,    -1,    -1,   105,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,   114,    -1,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
       3,     4,     5,     6,     7,    -1,   142,    -1,    -1,    12,
      13,   147,   148,   149,   150,   151,    -1,   153,   154,    -1,
     156,    -1,   158,   159,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    80,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    99,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,   135,     3,     4,     5,     6,     7,    -1,   142,
      -1,    -1,    12,    13,   147,   148,   149,   150,   151,   152,
      -1,    -1,    -1,   156,    -1,   158,   159,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    99,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,   135,     3,     4,     5,     6,
       7,    -1,   142,    -1,    -1,    12,    13,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,   156,    -1,   158,   159,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    60,    61,
      -1,    -1,    -1,    80,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    99,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,   135,     3,
       4,     5,     6,     7,    -1,   142,    -1,    -1,    12,    13,
     147,   148,   149,   150,   151,    -1,   153,    -1,    -1,   156,
      -1,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,   135,     3,     4,     5,     6,     7,    -1,   142,    -1,
      -1,    12,    13,   147,   148,   149,   150,   151,    -1,   153,
      -1,    -1,   156,    -1,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,     3,     4,     5,     6,     7,
      -1,   142,    -1,    -1,    12,    13,   147,   148,   149,   150,
     151,    -1,   153,    -1,    -1,   156,    -1,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,   135,     3,     4,
       5,     6,     7,    -1,   142,    -1,    -1,    12,    13,   147,
     148,   149,   150,   151,   152,    -1,    -1,    -1,   156,    -1,
     158,   159,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
     135,     3,     4,     5,     6,     7,    -1,   142,    -1,    -1,
      12,    13,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,   156,    -1,   158,   159,    -1,    -1,    -1,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,   135,     3,     4,     5,     6,     7,    -1,
     142,    -1,    -1,    12,    13,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,   156,    -1,   158,   159,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,   135,     3,     4,     5,
       6,     7,    -1,   142,    -1,    -1,    12,    13,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,   156,    -1,   158,
     159,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
       3,     4,     5,     6,     7,    -1,   142,    -1,    -1,    12,
      13,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
     156,    -1,   158,   159,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,   135,     3,     4,     5,     6,     7,    -1,   142,
      -1,    -1,    12,    13,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,   156,    -1,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,   135,     3,     4,     5,     6,
       7,    -1,   142,    -1,    -1,    12,    13,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,   156,    -1,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,   135,     3,
       4,     5,     6,     7,    -1,   142,    -1,    -1,    12,    13,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,   156,
      -1,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,   142,     9,
      10,    11,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,   156,    -1,   158,   159,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   155,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   155,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   155,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   155,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   155,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   153,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   153,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     126,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   126,    -1,    -1,    62,    -1,     9,    10,
      11,    -1,    -1,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   128,    -1,   130,   131,   132,   133,   134,   135,
       9,    10,    11,    -1,    -1,    -1,   142,    -1,    -1,    -1,
      -1,   147,   148,   149,   150,    -1,    -1,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    11,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   161,   162,     0,   163,     3,     4,     5,     6,     7,
      12,    13,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    76,    80,    81,    82,
      83,    85,    87,    89,    92,    96,    97,    98,    99,   100,
     101,   102,   105,   106,   108,   112,   113,   114,   116,   117,
     118,   119,   120,   121,   122,   127,   128,   130,   131,   132,
     133,   134,   135,   142,   147,   148,   149,   150,   151,   153,
     154,   156,   158,   159,   164,   165,   170,   174,   175,   209,
     210,   212,   214,   217,   219,   283,   285,   299,   300,   301,
     302,   305,   314,   325,   328,   332,   333,   334,   336,   337,
     347,   348,   349,   350,   352,   353,   354,   355,   361,   371,
     375,   377,    13,    72,   114,   147,   302,   332,   332,   151,
     332,   332,   332,   285,   332,   337,   332,   332,   332,   332,
     296,   332,   332,   332,   332,   332,   332,   332,   114,   147,
     150,   164,   314,   336,   337,   349,   336,    32,   332,   365,
     366,   332,   147,   150,   164,   314,   316,   317,   349,   353,
     354,   361,   151,   322,   333,   151,   333,    27,    62,   273,
     332,   182,   180,   151,   151,   192,   333,   153,   332,   153,
     332,    72,    72,   153,   285,   332,   337,   193,   332,   150,
     164,   168,   169,    74,   156,   247,   248,   120,   120,    74,
     249,   302,   151,   151,   151,   151,   151,   151,    74,    79,
     143,   144,   145,   367,   368,   150,   154,   164,   164,   283,
     299,   332,   171,   154,    79,   323,   367,    79,   367,   150,
     306,     8,   153,    72,    72,   153,    62,    62,    32,   211,
     351,   146,     9,    10,    11,    26,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    50,   153,    60,
      61,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   313,   146,    62,   125,    62,   154,   156,
     354,   211,   332,   126,   332,   150,   164,   146,   146,   335,
     337,   126,   157,     8,   330,   150,   164,   146,   284,   146,
     125,   354,   152,    27,   176,   332,   356,     8,   153,   174,
     333,   274,   275,   332,   285,   337,   151,   187,   153,   153,
     153,    14,   153,   153,   154,   153,   164,    91,     8,   153,
     154,   334,   337,     8,   153,    14,     8,   153,   211,   207,
     208,   337,   285,   337,   372,   374,   285,   337,   152,   286,
     365,    62,   125,   143,   368,    73,   332,   337,    79,   143,
     368,   164,   167,   153,   154,   308,   152,   152,   152,   155,
     172,   332,   158,   159,    72,   151,   245,    72,   123,   218,
     216,   356,   356,    72,   297,    62,    72,   120,   154,   347,
     354,   360,   361,   290,   332,   291,    27,   293,   288,   289,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   316,
      32,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   245,    72,   347,   360,   356,   338,   356,
     332,   152,   285,   337,   164,    72,    32,   332,    32,   332,
     164,   347,   245,   324,   347,   318,   178,   174,   157,   332,
      83,   181,   153,     8,    91,    91,    72,   229,    27,   154,
     230,    43,    44,    62,    80,   128,   130,   142,   147,   150,
     164,   314,   325,   326,   327,   376,   171,    91,    72,   169,
     332,   248,   326,    74,   298,     8,   152,     8,   152,   152,
     152,   153,   127,   337,   362,   363,   152,   369,    72,    62,
     155,   155,   307,   162,   166,   245,   292,   119,   173,   174,
     209,   210,   155,    32,   152,   246,   285,   299,   337,    14,
     147,   150,   164,   315,   215,   123,   220,   157,   157,   213,
     151,   356,   332,   310,   309,   354,   332,   332,   295,   332,
     332,   332,    64,   337,   312,   311,   157,   347,   357,   359,
     360,   157,   155,   335,   335,   126,   357,   171,   177,   183,
      27,   174,   235,   184,   276,   190,   188,    14,     8,   152,
     153,   231,   153,   231,   326,   326,   326,   329,   331,   151,
      79,   150,   164,   146,   155,    72,   155,    14,   151,   208,
     153,   373,   151,     8,   152,    72,    74,    75,   370,   332,
     245,   155,   162,   281,   282,   151,   335,     8,   152,   152,
     326,   150,   164,   124,   221,   222,   315,   154,   151,   128,
     129,   242,   243,   244,   315,   157,   155,   245,   245,   332,
      27,   316,   245,   245,   358,   339,    62,   154,    32,   332,
     319,   179,   236,   333,   171,   274,   332,    32,   127,   224,
     337,   224,   326,    72,    27,   174,   228,   231,    93,    94,
      95,   231,   155,   126,   157,     8,   330,   329,   164,    72,
     120,   103,   195,   326,   242,   374,   364,   363,    14,   157,
     157,   155,    62,   125,   277,   278,   279,   341,   152,    32,
     285,   337,   164,   222,   154,     8,   250,   242,   152,     8,
      32,    74,   294,   287,   343,   344,   345,   346,   356,   332,
     335,   320,   238,    67,    68,   240,   153,    84,   153,   337,
     151,   126,   223,   223,    14,   171,    93,   153,   332,    27,
     153,   234,   155,   326,   326,   152,   151,   194,   152,   362,
     332,   155,   356,   357,   341,    62,   280,   153,   335,   250,
     315,   100,   106,   109,   110,   111,   112,   113,   114,   115,
     155,   251,   254,   267,   268,   269,   270,   272,   152,   106,
     303,   244,    74,    14,   332,   324,    62,    62,   245,   340,
     157,   155,   125,   321,    67,    68,   241,   333,   174,   153,
     185,   225,   224,   152,   152,   326,    90,   153,   234,   233,
     126,   196,   104,   200,   303,   152,   157,   342,   356,   277,
     155,    72,   255,   315,   252,   302,   270,     8,   153,   154,
     151,   154,    32,    74,    14,   326,   356,   356,   341,   357,
     333,    27,    69,   237,   274,   362,   191,   189,   153,   232,
     171,   326,   315,   201,   154,   346,   157,    14,     8,   153,
     154,   256,    74,   271,   211,    72,   171,    32,    74,   304,
     171,    74,    14,   326,   157,   157,    27,   171,   153,   174,
     152,   152,    27,   174,   227,   227,   171,   197,   154,   171,
     326,   315,    72,   257,   258,   259,   260,   262,   263,   264,
     315,    14,     8,   153,    72,    14,   155,    74,     8,   152,
     155,    14,   326,   239,   186,   171,    74,   171,   155,   155,
     259,   153,    91,   107,   153,   146,   326,    74,   253,   326,
      32,    74,   326,   171,    27,   174,   226,    88,   152,   155,
     265,   270,   261,   315,    72,    14,   151,    74,   171,   153,
     198,    72,     8,   326,   242,    86,   154,   315,   152,   153,
     171,   153,   154,   266,   155,   171,   199,   155,   103,   202,
     203,   204,   151,   204,   315,   205,    74,   152,   206,   154,
     171,   155
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
  YYSIZE_T yysize1;
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
                yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
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

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
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
/* Line 1778 of yacc.c  */
#line 219 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_compilation(TSRMLS_C); }
    break;

  case 3:
/* Line 1778 of yacc.c  */
#line 223 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_info(TSRMLS_C); }
    break;

  case 4:
/* Line 1778 of yacc.c  */
#line 223 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { HANDLE_INTERACTIVE(); }
    break;

  case 6:
/* Line 1778 of yacc.c  */
#line 228 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 7:
/* Line 1778 of yacc.c  */
#line 229 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_build_namespace_name(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 8:
/* Line 1778 of yacc.c  */
#line 233 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_verify_namespace(TSRMLS_C); }
    break;

  case 9:
/* Line 1778 of yacc.c  */
#line 234 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_verify_namespace(TSRMLS_C); zend_do_early_binding(TSRMLS_C); }
    break;

  case 10:
/* Line 1778 of yacc.c  */
#line 235 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_verify_namespace(TSRMLS_C); zend_do_early_binding(TSRMLS_C); }
    break;

  case 11:
/* Line 1778 of yacc.c  */
#line 236 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_halt_compiler_register(TSRMLS_C); YYACCEPT; }
    break;

  case 12:
/* Line 1778 of yacc.c  */
#line 237 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_namespace(&(yyvsp[(2) - (3)]), 0 TSRMLS_CC); }
    break;

  case 13:
/* Line 1778 of yacc.c  */
#line 238 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_namespace(&(yyvsp[(2) - (3)]), 1 TSRMLS_CC); }
    break;

  case 14:
/* Line 1778 of yacc.c  */
#line 239 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_namespace(TSRMLS_C); }
    break;

  case 15:
/* Line 1778 of yacc.c  */
#line 240 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_namespace(NULL, 1 TSRMLS_CC); }
    break;

  case 16:
/* Line 1778 of yacc.c  */
#line 241 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_namespace(TSRMLS_C); }
    break;

  case 17:
/* Line 1778 of yacc.c  */
#line 242 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_verify_namespace(TSRMLS_C); }
    break;

  case 18:
/* Line 1778 of yacc.c  */
#line 243 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_verify_namespace(TSRMLS_C); }
    break;

  case 21:
/* Line 1778 of yacc.c  */
#line 252 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use(&(yyvsp[(1) - (1)]), NULL, 0 TSRMLS_CC); }
    break;

  case 22:
/* Line 1778 of yacc.c  */
#line 253 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), 0 TSRMLS_CC); }
    break;

  case 23:
/* Line 1778 of yacc.c  */
#line 254 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use(&(yyvsp[(2) - (2)]), NULL, 1 TSRMLS_CC); }
    break;

  case 24:
/* Line 1778 of yacc.c  */
#line 255 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use(&(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]), 1 TSRMLS_CC); }
    break;

  case 25:
/* Line 1778 of yacc.c  */
#line 259 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_constant(&(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)]) TSRMLS_CC); }
    break;

  case 26:
/* Line 1778 of yacc.c  */
#line 260 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_constant(&(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 27:
/* Line 1778 of yacc.c  */
#line 264 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_info(TSRMLS_C); }
    break;

  case 28:
/* Line 1778 of yacc.c  */
#line 264 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { HANDLE_INTERACTIVE(); }
    break;

  case 33:
/* Line 1778 of yacc.c  */
#line 273 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_error(E_COMPILE_ERROR, "__HALT_COMPILER() can only be used from the outermost scope"); }
    break;

  case 34:
/* Line 1778 of yacc.c  */
#line 278 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { DO_TICKS(); }
    break;

  case 35:
/* Line 1778 of yacc.c  */
#line 279 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_label(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 37:
/* Line 1778 of yacc.c  */
#line 284 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_cond(&(yyvsp[(2) - (2)]), &(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 38:
/* Line 1778 of yacc.c  */
#line 284 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_after_statement(&(yyvsp[(1) - (4)]), 1 TSRMLS_CC); }
    break;

  case 39:
/* Line 1778 of yacc.c  */
#line 284 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_end(TSRMLS_C); }
    break;

  case 40:
/* Line 1778 of yacc.c  */
#line 285 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_cond(&(yyvsp[(2) - (3)]), &(yyvsp[(1) - (3)]) TSRMLS_CC); }
    break;

  case 41:
/* Line 1778 of yacc.c  */
#line 285 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_after_statement(&(yyvsp[(1) - (5)]), 1 TSRMLS_CC); }
    break;

  case 42:
/* Line 1778 of yacc.c  */
#line 285 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_end(TSRMLS_C); }
    break;

  case 43:
/* Line 1778 of yacc.c  */
#line 286 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (1)]).u.op.opline_num = get_next_op_number(CG(active_op_array)); }
    break;

  case 44:
/* Line 1778 of yacc.c  */
#line 286 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_while_cond(&(yyvsp[(3) - (3)]), &(yyval) TSRMLS_CC); }
    break;

  case 45:
/* Line 1778 of yacc.c  */
#line 286 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_while_end(&(yyvsp[(1) - (5)]), &(yyvsp[(4) - (5)]) TSRMLS_CC); }
    break;

  case 46:
/* Line 1778 of yacc.c  */
#line 287 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (1)]).u.op.opline_num = get_next_op_number(CG(active_op_array));  zend_do_do_while_begin(TSRMLS_C); }
    break;

  case 47:
/* Line 1778 of yacc.c  */
#line 287 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(4) - (4)]).u.op.opline_num = get_next_op_number(CG(active_op_array)); }
    break;

  case 48:
/* Line 1778 of yacc.c  */
#line 287 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_do_while_end(&(yyvsp[(1) - (7)]), &(yyvsp[(4) - (7)]), &(yyvsp[(6) - (7)]) TSRMLS_CC); }
    break;

  case 49:
/* Line 1778 of yacc.c  */
#line 291 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_free(&(yyvsp[(3) - (4)]) TSRMLS_CC); (yyvsp[(4) - (4)]).u.op.opline_num = get_next_op_number(CG(active_op_array)); }
    break;

  case 50:
/* Line 1778 of yacc.c  */
#line 293 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_info(TSRMLS_C); zend_do_for_cond(&(yyvsp[(6) - (7)]), &(yyvsp[(7) - (7)]) TSRMLS_CC); }
    break;

  case 51:
/* Line 1778 of yacc.c  */
#line 295 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_free(&(yyvsp[(9) - (10)]) TSRMLS_CC); zend_do_for_before_statement(&(yyvsp[(4) - (10)]), &(yyvsp[(7) - (10)]) TSRMLS_CC); }
    break;

  case 52:
/* Line 1778 of yacc.c  */
#line 296 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_for_end(&(yyvsp[(7) - (12)]) TSRMLS_CC); }
    break;

  case 53:
/* Line 1778 of yacc.c  */
#line 297 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_switch_cond(&(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 54:
/* Line 1778 of yacc.c  */
#line 297 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_switch_end(&(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 55:
/* Line 1778 of yacc.c  */
#line 298 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_brk_cont(ZEND_BRK, NULL TSRMLS_CC); }
    break;

  case 56:
/* Line 1778 of yacc.c  */
#line 299 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_brk_cont(ZEND_BRK, &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 57:
/* Line 1778 of yacc.c  */
#line 300 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_brk_cont(ZEND_CONT, NULL TSRMLS_CC); }
    break;

  case 58:
/* Line 1778 of yacc.c  */
#line 301 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_brk_cont(ZEND_CONT, &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 59:
/* Line 1778 of yacc.c  */
#line 302 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_return(NULL, 0 TSRMLS_CC); }
    break;

  case 60:
/* Line 1778 of yacc.c  */
#line 303 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_return(&(yyvsp[(2) - (3)]), 0 TSRMLS_CC); }
    break;

  case 61:
/* Line 1778 of yacc.c  */
#line 304 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_return(&(yyvsp[(2) - (3)]), 1 TSRMLS_CC); }
    break;

  case 62:
/* Line 1778 of yacc.c  */
#line 305 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_free(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 66:
/* Line 1778 of yacc.c  */
#line 309 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_echo(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 67:
/* Line 1778 of yacc.c  */
#line 310 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_free(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 69:
/* Line 1778 of yacc.c  */
#line 313 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_begin(&(yyvsp[(1) - (4)]), &(yyvsp[(2) - (4)]), &(yyvsp[(3) - (4)]), &(yyvsp[(4) - (4)]), 1 TSRMLS_CC); }
    break;

  case 70:
/* Line 1778 of yacc.c  */
#line 314 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_cont(&(yyvsp[(1) - (8)]), &(yyvsp[(2) - (8)]), &(yyvsp[(4) - (8)]), &(yyvsp[(6) - (8)]), &(yyvsp[(7) - (8)]) TSRMLS_CC); }
    break;

  case 71:
/* Line 1778 of yacc.c  */
#line 315 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_end(&(yyvsp[(1) - (10)]), &(yyvsp[(4) - (10)]) TSRMLS_CC); }
    break;

  case 72:
/* Line 1778 of yacc.c  */
#line 317 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_begin(&(yyvsp[(1) - (4)]), &(yyvsp[(2) - (4)]), &(yyvsp[(3) - (4)]), &(yyvsp[(4) - (4)]), 0 TSRMLS_CC); }
    break;

  case 73:
/* Line 1778 of yacc.c  */
#line 318 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_cont(&(yyvsp[(1) - (8)]), &(yyvsp[(2) - (8)]), &(yyvsp[(4) - (8)]), &(yyvsp[(6) - (8)]), &(yyvsp[(7) - (8)]) TSRMLS_CC); }
    break;

  case 74:
/* Line 1778 of yacc.c  */
#line 319 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_foreach_end(&(yyvsp[(1) - (10)]), &(yyvsp[(4) - (10)]) TSRMLS_CC); }
    break;

  case 75:
/* Line 1778 of yacc.c  */
#line 320 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (1)]).u.op.opline_num = get_next_op_number(CG(active_op_array)); zend_do_declare_begin(TSRMLS_C); }
    break;

  case 76:
/* Line 1778 of yacc.c  */
#line 320 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_end(&(yyvsp[(1) - (6)]) TSRMLS_CC); }
    break;

  case 78:
/* Line 1778 of yacc.c  */
#line 322 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_try(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 79:
/* Line 1778 of yacc.c  */
#line 323 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_bind_catch(&(yyvsp[(1) - (6)]), &(yyvsp[(6) - (6)]) TSRMLS_CC); }
    break;

  case 80:
/* Line 1778 of yacc.c  */
#line 324 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_finally(&(yyvsp[(1) - (8)]), &(yyvsp[(6) - (8)]), &(yyvsp[(8) - (8)]) TSRMLS_CC); }
    break;

  case 81:
/* Line 1778 of yacc.c  */
#line 325 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_throw(&(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 82:
/* Line 1778 of yacc.c  */
#line 326 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_goto(&(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 83:
/* Line 1778 of yacc.c  */
#line 330 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 84:
/* Line 1778 of yacc.c  */
#line 331 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_initialize_try_catch_element(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 85:
/* Line 1778 of yacc.c  */
#line 332 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_first_catch(&(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 86:
/* Line 1778 of yacc.c  */
#line 333 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_catch(&(yyvsp[(1) - (7)]), &(yyvsp[(4) - (7)]), &(yyvsp[(6) - (7)]), &(yyvsp[(2) - (7)]) TSRMLS_CC); }
    break;

  case 87:
/* Line 1778 of yacc.c  */
#line 334 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_catch(&(yyvsp[(1) - (11)]) TSRMLS_CC); }
    break;

  case 88:
/* Line 1778 of yacc.c  */
#line 335 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_mark_last_catch(&(yyvsp[(2) - (13)]), &(yyvsp[(13) - (13)]) TSRMLS_CC); (yyval) = (yyvsp[(1) - (13)]);}
    break;

  case 89:
/* Line 1778 of yacc.c  */
#line 338 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 90:
/* Line 1778 of yacc.c  */
#line 339 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_finally(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 91:
/* Line 1778 of yacc.c  */
#line 339 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (5)]); }
    break;

  case 92:
/* Line 1778 of yacc.c  */
#line 343 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 93:
/* Line 1778 of yacc.c  */
#line 344 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = -1; }
    break;

  case 94:
/* Line 1778 of yacc.c  */
#line 348 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 95:
/* Line 1778 of yacc.c  */
#line 349 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 96:
/* Line 1778 of yacc.c  */
#line 353 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = get_next_op_number(CG(active_op_array)); }
    break;

  case 97:
/* Line 1778 of yacc.c  */
#line 353 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_catch(&(yyvsp[(1) - (6)]), &(yyvsp[(3) - (6)]), &(yyvsp[(5) - (6)]), NULL TSRMLS_CC); }
    break;

  case 98:
/* Line 1778 of yacc.c  */
#line 353 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_catch(&(yyvsp[(1) - (10)]) TSRMLS_CC); }
    break;

  case 101:
/* Line 1778 of yacc.c  */
#line 362 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_UNSET, 0 TSRMLS_CC); zend_do_unset(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 102:
/* Line 1778 of yacc.c  */
#line 366 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { DO_TICKS(); }
    break;

  case 103:
/* Line 1778 of yacc.c  */
#line 370 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { DO_TICKS(); }
    break;

  case 104:
/* Line 1778 of yacc.c  */
#line 374 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = ZEND_RETURN_VAL; }
    break;

  case 105:
/* Line 1778 of yacc.c  */
#line 375 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = ZEND_RETURN_REF; }
    break;

  case 106:
/* Line 1778 of yacc.c  */
#line 380 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_function_declaration(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), 0, (yyvsp[(2) - (3)]).op_type, NULL TSRMLS_CC); }
    break;

  case 107:
/* Line 1778 of yacc.c  */
#line 382 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_declaration(&(yyvsp[(1) - (10)]) TSRMLS_CC); }
    break;

  case 108:
/* Line 1778 of yacc.c  */
#line 387 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_class_declaration(&(yyvsp[(1) - (3)]), &(yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 109:
/* Line 1778 of yacc.c  */
#line 391 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_class_declaration(&(yyvsp[(1) - (8)]), &(yyvsp[(3) - (8)]) TSRMLS_CC); }
    break;

  case 110:
/* Line 1778 of yacc.c  */
#line 393 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_class_declaration(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]), NULL TSRMLS_CC); }
    break;

  case 111:
/* Line 1778 of yacc.c  */
#line 397 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_class_declaration(&(yyvsp[(1) - (7)]), NULL TSRMLS_CC); }
    break;

  case 112:
/* Line 1778 of yacc.c  */
#line 402 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); (yyval).EA = 0; }
    break;

  case 113:
/* Line 1778 of yacc.c  */
#line 403 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); (yyval).EA = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 114:
/* Line 1778 of yacc.c  */
#line 404 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); (yyval).EA = ZEND_ACC_TRAIT; }
    break;

  case 115:
/* Line 1778 of yacc.c  */
#line 405 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); (yyval).EA = ZEND_ACC_FINAL_CLASS; }
    break;

  case 116:
/* Line 1778 of yacc.c  */
#line 409 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 117:
/* Line 1778 of yacc.c  */
#line 410 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_class(&(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 118:
/* Line 1778 of yacc.c  */
#line 414 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); (yyval).EA = ZEND_ACC_INTERFACE; }
    break;

  case 123:
/* Line 1778 of yacc.c  */
#line 428 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_implements_interface(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 124:
/* Line 1778 of yacc.c  */
#line 429 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_implements_interface(&(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 125:
/* Line 1778 of yacc.c  */
#line 433 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 126:
/* Line 1778 of yacc.c  */
#line 434 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 127:
/* Line 1778 of yacc.c  */
#line 438 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (1)])); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 128:
/* Line 1778 of yacc.c  */
#line 439 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(2) - (2)])); (yyval) = (yyvsp[(2) - (2)]);  (yyval).EA |= ZEND_PARSED_REFERENCE_VARIABLE; }
    break;

  case 129:
/* Line 1778 of yacc.c  */
#line 440 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_list_init(TSRMLS_C); }
    break;

  case 130:
/* Line 1778 of yacc.c  */
#line 440 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (5)]); (yyval).EA = ZEND_PARSED_LIST_EXPR; }
    break;

  case 137:
/* Line 1778 of yacc.c  */
#line 462 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_stmt(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 138:
/* Line 1778 of yacc.c  */
#line 463 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_stmt(&(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)]) TSRMLS_CC); }
    break;

  case 139:
/* Line 1778 of yacc.c  */
#line 468 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 140:
/* Line 1778 of yacc.c  */
#line 469 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); }
    break;

  case 141:
/* Line 1778 of yacc.c  */
#line 470 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (4)]); }
    break;

  case 142:
/* Line 1778 of yacc.c  */
#line 471 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (5)]); }
    break;

  case 143:
/* Line 1778 of yacc.c  */
#line 476 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 144:
/* Line 1778 of yacc.c  */
#line 477 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_info(TSRMLS_C);  zend_do_case_before_statement(&(yyvsp[(1) - (4)]), &(yyvsp[(2) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 145:
/* Line 1778 of yacc.c  */
#line 477 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_case_after_statement(&(yyval), &(yyvsp[(2) - (6)]) TSRMLS_CC); (yyval).op_type = IS_CONST; }
    break;

  case 146:
/* Line 1778 of yacc.c  */
#line 478 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_info(TSRMLS_C);  zend_do_default_before_statement(&(yyvsp[(1) - (3)]), &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 147:
/* Line 1778 of yacc.c  */
#line 478 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_case_after_statement(&(yyval), &(yyvsp[(2) - (5)]) TSRMLS_CC); (yyval).op_type = IS_CONST; }
    break;

  case 153:
/* Line 1778 of yacc.c  */
#line 497 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_cond(&(yyvsp[(3) - (3)]), &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 154:
/* Line 1778 of yacc.c  */
#line 497 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_after_statement(&(yyvsp[(2) - (5)]), 0 TSRMLS_CC); }
    break;

  case 156:
/* Line 1778 of yacc.c  */
#line 503 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_cond(&(yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 157:
/* Line 1778 of yacc.c  */
#line 503 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_if_after_statement(&(yyvsp[(2) - (6)]), 0 TSRMLS_CC); }
    break;

  case 164:
/* Line 1778 of yacc.c  */
#line 526 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; (yyval).u.op.num=1; zend_do_receive_arg(ZEND_RECV, &(yyvsp[(2) - (2)]), &(yyval), NULL, &(yyvsp[(1) - (2)]), 0 TSRMLS_CC); }
    break;

  case 165:
/* Line 1778 of yacc.c  */
#line 527 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; (yyval).u.op.num=1; zend_do_receive_arg(ZEND_RECV, &(yyvsp[(3) - (3)]), &(yyval), NULL, &(yyvsp[(1) - (3)]), 1 TSRMLS_CC); }
    break;

  case 166:
/* Line 1778 of yacc.c  */
#line 528 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; (yyval).u.op.num=1; zend_do_receive_arg(ZEND_RECV_INIT, &(yyvsp[(3) - (5)]), &(yyval), &(yyvsp[(5) - (5)]), &(yyvsp[(1) - (5)]), 1 TSRMLS_CC); }
    break;

  case 167:
/* Line 1778 of yacc.c  */
#line 529 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; (yyval).u.op.num=1; zend_do_receive_arg(ZEND_RECV_INIT, &(yyvsp[(2) - (4)]), &(yyval), &(yyvsp[(4) - (4)]), &(yyvsp[(1) - (4)]), 0 TSRMLS_CC); }
    break;

  case 168:
/* Line 1778 of yacc.c  */
#line 530 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval)=(yyvsp[(1) - (4)]); (yyval).u.op.num++; zend_do_receive_arg(ZEND_RECV, &(yyvsp[(4) - (4)]), &(yyval), NULL, &(yyvsp[(3) - (4)]), 0 TSRMLS_CC); }
    break;

  case 169:
/* Line 1778 of yacc.c  */
#line 531 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval)=(yyvsp[(1) - (5)]); (yyval).u.op.num++; zend_do_receive_arg(ZEND_RECV, &(yyvsp[(5) - (5)]), &(yyval), NULL, &(yyvsp[(3) - (5)]), 1 TSRMLS_CC); }
    break;

  case 170:
/* Line 1778 of yacc.c  */
#line 532 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval)=(yyvsp[(1) - (7)]); (yyval).u.op.num++; zend_do_receive_arg(ZEND_RECV_INIT, &(yyvsp[(5) - (7)]), &(yyval), &(yyvsp[(7) - (7)]), &(yyvsp[(3) - (7)]), 1 TSRMLS_CC); }
    break;

  case 171:
/* Line 1778 of yacc.c  */
#line 533 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval)=(yyvsp[(1) - (6)]); (yyval).u.op.num++; zend_do_receive_arg(ZEND_RECV_INIT, &(yyvsp[(4) - (6)]), &(yyval), &(yyvsp[(6) - (6)]), &(yyvsp[(3) - (6)]), 0 TSRMLS_CC); }
    break;

  case 172:
/* Line 1778 of yacc.c  */
#line 538 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 173:
/* Line 1778 of yacc.c  */
#line 539 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; Z_TYPE((yyval).u.constant)=IS_ARRAY; }
    break;

  case 174:
/* Line 1778 of yacc.c  */
#line 540 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; Z_TYPE((yyval).u.constant)=IS_CALLABLE; }
    break;

  case 175:
/* Line 1778 of yacc.c  */
#line 541 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 176:
/* Line 1778 of yacc.c  */
#line 546 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 0; }
    break;

  case 177:
/* Line 1778 of yacc.c  */
#line 547 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 178:
/* Line 1778 of yacc.c  */
#line 548 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 1; zend_do_pass_param(&(yyvsp[(2) - (3)]), ZEND_SEND_VAL, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 179:
/* Line 1778 of yacc.c  */
#line 553 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 1;  zend_do_pass_param(&(yyvsp[(1) - (1)]), ZEND_SEND_VAL, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 180:
/* Line 1778 of yacc.c  */
#line 554 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 1;  zend_do_pass_param(&(yyvsp[(1) - (1)]), ZEND_SEND_VAR, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 181:
/* Line 1778 of yacc.c  */
#line 555 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 1;  zend_do_pass_param(&(yyvsp[(2) - (2)]), ZEND_SEND_REF, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 182:
/* Line 1778 of yacc.c  */
#line 556 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant)=Z_LVAL((yyvsp[(1) - (3)]).u.constant)+1;  zend_do_pass_param(&(yyvsp[(3) - (3)]), ZEND_SEND_VAL, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 183:
/* Line 1778 of yacc.c  */
#line 557 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant)=Z_LVAL((yyvsp[(1) - (3)]).u.constant)+1;  zend_do_pass_param(&(yyvsp[(3) - (3)]), ZEND_SEND_VAR, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 184:
/* Line 1778 of yacc.c  */
#line 558 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant)=Z_LVAL((yyvsp[(1) - (4)]).u.constant)+1;  zend_do_pass_param(&(yyvsp[(4) - (4)]), ZEND_SEND_REF, Z_LVAL((yyval).u.constant) TSRMLS_CC); }
    break;

  case 185:
/* Line 1778 of yacc.c  */
#line 562 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_global_variable(&(yyvsp[(3) - (3)]), NULL, ZEND_FETCH_GLOBAL_LOCK TSRMLS_CC); }
    break;

  case 186:
/* Line 1778 of yacc.c  */
#line 563 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_global_variable(&(yyvsp[(1) - (1)]), NULL, ZEND_FETCH_GLOBAL_LOCK TSRMLS_CC); }
    break;

  case 187:
/* Line 1778 of yacc.c  */
#line 568 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 188:
/* Line 1778 of yacc.c  */
#line 569 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 189:
/* Line 1778 of yacc.c  */
#line 570 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); }
    break;

  case 190:
/* Line 1778 of yacc.c  */
#line 575 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_static_variable(&(yyvsp[(3) - (3)]), NULL, ZEND_FETCH_STATIC TSRMLS_CC); }
    break;

  case 191:
/* Line 1778 of yacc.c  */
#line 576 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_static_variable(&(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)]), ZEND_FETCH_STATIC TSRMLS_CC); }
    break;

  case 192:
/* Line 1778 of yacc.c  */
#line 577 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_static_variable(&(yyvsp[(1) - (1)]), NULL, ZEND_FETCH_STATIC TSRMLS_CC); }
    break;

  case 193:
/* Line 1778 of yacc.c  */
#line 578 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_static_variable(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), ZEND_FETCH_STATIC TSRMLS_CC); }
    break;

  case 196:
/* Line 1778 of yacc.c  */
#line 590 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { CG(access_type) = Z_LVAL((yyvsp[(1) - (1)]).u.constant); }
    break;

  case 200:
/* Line 1778 of yacc.c  */
#line 593 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_function_declaration(&(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]), 1, (yyvsp[(3) - (4)]).op_type, &(yyvsp[(1) - (4)]) TSRMLS_CC); }
    break;

  case 201:
/* Line 1778 of yacc.c  */
#line 595 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_abstract_method(&(yyvsp[(4) - (9)]), &(yyvsp[(1) - (9)]), &(yyvsp[(9) - (9)]) TSRMLS_CC); zend_do_end_function_declaration(&(yyvsp[(2) - (9)]) TSRMLS_CC); }
    break;

  case 203:
/* Line 1778 of yacc.c  */
#line 603 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use_trait(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 204:
/* Line 1778 of yacc.c  */
#line 604 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_use_trait(&(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 213:
/* Line 1778 of yacc.c  */
#line 628 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_add_trait_precedence(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 214:
/* Line 1778 of yacc.c  */
#line 632 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_resolve_class_name(&(yyvsp[(1) - (1)]), ZEND_FETCH_CLASS_GLOBAL, 1 TSRMLS_CC); zend_init_list(&(yyval).u.op.ptr, Z_STRVAL((yyvsp[(1) - (1)]).u.constant) TSRMLS_CC); }
    break;

  case 215:
/* Line 1778 of yacc.c  */
#line 633 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_resolve_class_name(&(yyvsp[(3) - (3)]), ZEND_FETCH_CLASS_GLOBAL, 1 TSRMLS_CC); zend_add_to_list(&(yyvsp[(1) - (3)]).u.op.ptr, Z_STRVAL((yyvsp[(3) - (3)]).u.constant) TSRMLS_CC); (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 216:
/* Line 1778 of yacc.c  */
#line 637 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_prepare_reference(&(yyval), NULL, &(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 217:
/* Line 1778 of yacc.c  */
#line 638 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 218:
/* Line 1778 of yacc.c  */
#line 642 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_prepare_reference(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 219:
/* Line 1778 of yacc.c  */
#line 646 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_add_trait_alias(&(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]), &(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 220:
/* Line 1778 of yacc.c  */
#line 647 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_add_trait_alias(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), NULL TSRMLS_CC); }
    break;

  case 221:
/* Line 1778 of yacc.c  */
#line 651 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 0x0; }
    break;

  case 222:
/* Line 1778 of yacc.c  */
#line 652 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 223:
/* Line 1778 of yacc.c  */
#line 656 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_ABSTRACT; }
    break;

  case 224:
/* Line 1778 of yacc.c  */
#line 657 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 0;	}
    break;

  case 225:
/* Line 1778 of yacc.c  */
#line 661 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 226:
/* Line 1778 of yacc.c  */
#line 662 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_PUBLIC; }
    break;

  case 227:
/* Line 1778 of yacc.c  */
#line 666 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_PUBLIC; }
    break;

  case 228:
/* Line 1778 of yacc.c  */
#line 667 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]);  if (!(Z_LVAL((yyval).u.constant) & ZEND_ACC_PPP_MASK)) { Z_LVAL((yyval).u.constant) |= ZEND_ACC_PUBLIC; } }
    break;

  case 229:
/* Line 1778 of yacc.c  */
#line 671 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 230:
/* Line 1778 of yacc.c  */
#line 672 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = zend_do_verify_access_types(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 231:
/* Line 1778 of yacc.c  */
#line 676 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_PUBLIC; }
    break;

  case 232:
/* Line 1778 of yacc.c  */
#line 677 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_PROTECTED; }
    break;

  case 233:
/* Line 1778 of yacc.c  */
#line 678 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_PRIVATE; }
    break;

  case 234:
/* Line 1778 of yacc.c  */
#line 679 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_STATIC; }
    break;

  case 235:
/* Line 1778 of yacc.c  */
#line 680 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_ABSTRACT; }
    break;

  case 236:
/* Line 1778 of yacc.c  */
#line 681 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = ZEND_ACC_FINAL; }
    break;

  case 237:
/* Line 1778 of yacc.c  */
#line 685 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_property(&(yyvsp[(3) - (3)]), NULL, CG(access_type) TSRMLS_CC); }
    break;

  case 238:
/* Line 1778 of yacc.c  */
#line 686 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_property(&(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)]), CG(access_type) TSRMLS_CC); }
    break;

  case 239:
/* Line 1778 of yacc.c  */
#line 687 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_property(&(yyvsp[(1) - (1)]), NULL, CG(access_type) TSRMLS_CC); }
    break;

  case 240:
/* Line 1778 of yacc.c  */
#line 688 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_property(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), CG(access_type) TSRMLS_CC); }
    break;

  case 241:
/* Line 1778 of yacc.c  */
#line 692 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_class_constant(&(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)]) TSRMLS_CC); }
    break;

  case 242:
/* Line 1778 of yacc.c  */
#line 693 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_declare_class_constant(&(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 243:
/* Line 1778 of yacc.c  */
#line 697 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_echo(&(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 244:
/* Line 1778 of yacc.c  */
#line 698 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_echo(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 245:
/* Line 1778 of yacc.c  */
#line 703 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST;  Z_TYPE((yyval).u.constant) = IS_BOOL;  Z_LVAL((yyval).u.constant) = 1; }
    break;

  case 246:
/* Line 1778 of yacc.c  */
#line 704 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 247:
/* Line 1778 of yacc.c  */
#line 708 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_free(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 248:
/* Line 1778 of yacc.c  */
#line 708 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(4) - (4)]); }
    break;

  case 249:
/* Line 1778 of yacc.c  */
#line 709 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 250:
/* Line 1778 of yacc.c  */
#line 713 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = (yyvsp[(2) - (2)]).EA; }
    break;

  case 251:
/* Line 1778 of yacc.c  */
#line 714 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = (yyvsp[(1) - (1)]).EA; }
    break;

  case 252:
/* Line 1778 of yacc.c  */
#line 718 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 253:
/* Line 1778 of yacc.c  */
#line 719 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pop_object(&(yyvsp[(1) - (3)]) TSRMLS_CC); fetch_array_dim(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 254:
/* Line 1778 of yacc.c  */
#line 723 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 255:
/* Line 1778 of yacc.c  */
#line 723 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (3)]); }
    break;

  case 256:
/* Line 1778 of yacc.c  */
#line 724 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(1) - (1)]) TSRMLS_CC); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 257:
/* Line 1778 of yacc.c  */
#line 725 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 258:
/* Line 1778 of yacc.c  */
#line 729 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(0) - (0)]); }
    break;

  case 259:
/* Line 1778 of yacc.c  */
#line 730 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(0) - (0)]) TSRMLS_CC); zend_do_begin_variable_parse(TSRMLS_C); }
    break;

  case 260:
/* Line 1778 of yacc.c  */
#line 731 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pop_object(&(yyval) TSRMLS_CC); zend_do_end_variable_parse(&(yyvsp[(2) - (2)]), BP_VAR_R, 0 TSRMLS_CC); }
    break;

  case 261:
/* Line 1778 of yacc.c  */
#line 735 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_extended_fcall_begin(TSRMLS_C); zend_do_begin_new_object(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 262:
/* Line 1778 of yacc.c  */
#line 735 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_new_object(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]) TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 263:
/* Line 1778 of yacc.c  */
#line 739 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_list_init(TSRMLS_C); }
    break;

  case 264:
/* Line 1778 of yacc.c  */
#line 739 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_list_end(&(yyval), &(yyvsp[(7) - (7)]) TSRMLS_CC); }
    break;

  case 265:
/* Line 1778 of yacc.c  */
#line 740 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_assign(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 266:
/* Line 1778 of yacc.c  */
#line 741 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (4)])); zend_do_end_variable_parse(&(yyvsp[(4) - (4)]), BP_VAR_W, 1 TSRMLS_CC); zend_do_end_variable_parse(&(yyvsp[(1) - (4)]), BP_VAR_W, 0 TSRMLS_CC); zend_do_assign_ref(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 267:
/* Line 1778 of yacc.c  */
#line 742 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_error(E_DEPRECATED, "Assigning the return value of new by reference is deprecated");  zend_check_writable_variable(&(yyvsp[(1) - (5)])); zend_do_extended_fcall_begin(TSRMLS_C); zend_do_begin_new_object(&(yyvsp[(4) - (5)]), &(yyvsp[(5) - (5)]) TSRMLS_CC); }
    break;

  case 268:
/* Line 1778 of yacc.c  */
#line 742 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_new_object(&(yyvsp[(3) - (7)]), &(yyvsp[(4) - (7)]), &(yyvsp[(7) - (7)]) TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C); zend_do_end_variable_parse(&(yyvsp[(1) - (7)]), BP_VAR_W, 0 TSRMLS_CC); (yyvsp[(3) - (7)]).EA = ZEND_PARSED_NEW; zend_do_assign_ref(&(yyval), &(yyvsp[(1) - (7)]), &(yyvsp[(3) - (7)]) TSRMLS_CC); }
    break;

  case 269:
/* Line 1778 of yacc.c  */
#line 743 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_clone(&(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 270:
/* Line 1778 of yacc.c  */
#line 744 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_ADD, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 271:
/* Line 1778 of yacc.c  */
#line 745 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_SUB, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 272:
/* Line 1778 of yacc.c  */
#line 746 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_MUL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 273:
/* Line 1778 of yacc.c  */
#line 747 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_DIV, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 274:
/* Line 1778 of yacc.c  */
#line 748 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_CONCAT, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 275:
/* Line 1778 of yacc.c  */
#line 749 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_MOD, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 276:
/* Line 1778 of yacc.c  */
#line 750 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_BW_AND, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 277:
/* Line 1778 of yacc.c  */
#line 751 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_BW_OR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 278:
/* Line 1778 of yacc.c  */
#line 752 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_BW_XOR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 279:
/* Line 1778 of yacc.c  */
#line 753 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_SL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 280:
/* Line 1778 of yacc.c  */
#line 754 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_check_writable_variable(&(yyvsp[(1) - (3)])); zend_do_end_variable_parse(&(yyvsp[(1) - (3)]), BP_VAR_RW, 0 TSRMLS_CC); zend_do_binary_assign_op(ZEND_ASSIGN_SR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 281:
/* Line 1778 of yacc.c  */
#line 755 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_post_incdec(&(yyval), &(yyvsp[(1) - (2)]), ZEND_POST_INC TSRMLS_CC); }
    break;

  case 282:
/* Line 1778 of yacc.c  */
#line 756 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pre_incdec(&(yyval), &(yyvsp[(2) - (2)]), ZEND_PRE_INC TSRMLS_CC); }
    break;

  case 283:
/* Line 1778 of yacc.c  */
#line 757 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_post_incdec(&(yyval), &(yyvsp[(1) - (2)]), ZEND_POST_DEC TSRMLS_CC); }
    break;

  case 284:
/* Line 1778 of yacc.c  */
#line 758 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pre_incdec(&(yyval), &(yyvsp[(2) - (2)]), ZEND_PRE_DEC TSRMLS_CC); }
    break;

  case 285:
/* Line 1778 of yacc.c  */
#line 759 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_or_begin(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 286:
/* Line 1778 of yacc.c  */
#line 759 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_or_end(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 287:
/* Line 1778 of yacc.c  */
#line 760 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_begin(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 288:
/* Line 1778 of yacc.c  */
#line 760 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_end(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 289:
/* Line 1778 of yacc.c  */
#line 761 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_or_begin(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 290:
/* Line 1778 of yacc.c  */
#line 761 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_or_end(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 291:
/* Line 1778 of yacc.c  */
#line 762 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_begin(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 292:
/* Line 1778 of yacc.c  */
#line 762 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_end(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 293:
/* Line 1778 of yacc.c  */
#line 763 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_BOOL_XOR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 294:
/* Line 1778 of yacc.c  */
#line 764 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_BW_OR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 295:
/* Line 1778 of yacc.c  */
#line 765 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_BW_AND, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 296:
/* Line 1778 of yacc.c  */
#line 766 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_BW_XOR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 297:
/* Line 1778 of yacc.c  */
#line 767 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_CONCAT, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 298:
/* Line 1778 of yacc.c  */
#line 768 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_ADD, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 299:
/* Line 1778 of yacc.c  */
#line 769 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_SUB, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 300:
/* Line 1778 of yacc.c  */
#line 770 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_MUL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 301:
/* Line 1778 of yacc.c  */
#line 771 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_DIV, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 302:
/* Line 1778 of yacc.c  */
#line 772 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_MOD, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 303:
/* Line 1778 of yacc.c  */
#line 773 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_SL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 304:
/* Line 1778 of yacc.c  */
#line 774 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_SR, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 305:
/* Line 1778 of yacc.c  */
#line 775 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_LONG(&(yyvsp[(1) - (2)]).u.constant, 0); if ((yyvsp[(2) - (2)]).op_type == IS_CONST) { add_function(&(yyvsp[(2) - (2)]).u.constant, &(yyvsp[(1) - (2)]).u.constant, &(yyvsp[(2) - (2)]).u.constant TSRMLS_CC); (yyval) = (yyvsp[(2) - (2)]); } else { (yyvsp[(1) - (2)]).op_type = IS_CONST; INIT_PZVAL(&(yyvsp[(1) - (2)]).u.constant); zend_do_binary_op(ZEND_ADD, &(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); } }
    break;

  case 306:
/* Line 1778 of yacc.c  */
#line 776 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_LONG(&(yyvsp[(1) - (2)]).u.constant, 0); if ((yyvsp[(2) - (2)]).op_type == IS_CONST) { sub_function(&(yyvsp[(2) - (2)]).u.constant, &(yyvsp[(1) - (2)]).u.constant, &(yyvsp[(2) - (2)]).u.constant TSRMLS_CC); (yyval) = (yyvsp[(2) - (2)]); } else { (yyvsp[(1) - (2)]).op_type = IS_CONST; INIT_PZVAL(&(yyvsp[(1) - (2)]).u.constant); zend_do_binary_op(ZEND_SUB, &(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); } }
    break;

  case 307:
/* Line 1778 of yacc.c  */
#line 777 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_unary_op(ZEND_BOOL_NOT, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 308:
/* Line 1778 of yacc.c  */
#line 778 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_unary_op(ZEND_BW_NOT, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 309:
/* Line 1778 of yacc.c  */
#line 779 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_IDENTICAL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 310:
/* Line 1778 of yacc.c  */
#line 780 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_NOT_IDENTICAL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 311:
/* Line 1778 of yacc.c  */
#line 781 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_EQUAL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 312:
/* Line 1778 of yacc.c  */
#line 782 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_NOT_EQUAL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 313:
/* Line 1778 of yacc.c  */
#line 783 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_SMALLER, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 314:
/* Line 1778 of yacc.c  */
#line 784 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_SMALLER_OR_EQUAL, &(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 315:
/* Line 1778 of yacc.c  */
#line 785 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_SMALLER, &(yyval), &(yyvsp[(3) - (3)]), &(yyvsp[(1) - (3)]) TSRMLS_CC); }
    break;

  case 316:
/* Line 1778 of yacc.c  */
#line 786 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_binary_op(ZEND_IS_SMALLER_OR_EQUAL, &(yyval), &(yyvsp[(3) - (3)]), &(yyvsp[(1) - (3)]) TSRMLS_CC); }
    break;

  case 317:
/* Line 1778 of yacc.c  */
#line 787 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_instanceof(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), 0 TSRMLS_CC); }
    break;

  case 318:
/* Line 1778 of yacc.c  */
#line 788 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 319:
/* Line 1778 of yacc.c  */
#line 789 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 320:
/* Line 1778 of yacc.c  */
#line 790 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 321:
/* Line 1778 of yacc.c  */
#line 790 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(5) - (5)]); }
    break;

  case 322:
/* Line 1778 of yacc.c  */
#line 791 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_qm_op(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 323:
/* Line 1778 of yacc.c  */
#line 792 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_qm_true(&(yyvsp[(4) - (5)]), &(yyvsp[(2) - (5)]), &(yyvsp[(5) - (5)]) TSRMLS_CC); }
    break;

  case 324:
/* Line 1778 of yacc.c  */
#line 793 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_qm_false(&(yyval), &(yyvsp[(7) - (7)]), &(yyvsp[(2) - (7)]), &(yyvsp[(5) - (7)]) TSRMLS_CC); }
    break;

  case 325:
/* Line 1778 of yacc.c  */
#line 794 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_jmp_set(&(yyvsp[(1) - (3)]), &(yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 326:
/* Line 1778 of yacc.c  */
#line 795 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_jmp_set_else(&(yyval), &(yyvsp[(5) - (5)]), &(yyvsp[(2) - (5)]), &(yyvsp[(3) - (5)]) TSRMLS_CC); }
    break;

  case 327:
/* Line 1778 of yacc.c  */
#line 796 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 328:
/* Line 1778 of yacc.c  */
#line 797 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_LONG TSRMLS_CC); }
    break;

  case 329:
/* Line 1778 of yacc.c  */
#line 798 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_DOUBLE TSRMLS_CC); }
    break;

  case 330:
/* Line 1778 of yacc.c  */
#line 799 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_STRING TSRMLS_CC); }
    break;

  case 331:
/* Line 1778 of yacc.c  */
#line 800 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_ARRAY TSRMLS_CC); }
    break;

  case 332:
/* Line 1778 of yacc.c  */
#line 801 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_OBJECT TSRMLS_CC); }
    break;

  case 333:
/* Line 1778 of yacc.c  */
#line 802 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_BOOL TSRMLS_CC); }
    break;

  case 334:
/* Line 1778 of yacc.c  */
#line 803 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_cast(&(yyval), &(yyvsp[(2) - (2)]), IS_NULL TSRMLS_CC); }
    break;

  case 335:
/* Line 1778 of yacc.c  */
#line 804 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_exit(&(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 336:
/* Line 1778 of yacc.c  */
#line 805 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_silence(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 337:
/* Line 1778 of yacc.c  */
#line 805 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_silence(&(yyvsp[(1) - (3)]) TSRMLS_CC); (yyval) = (yyvsp[(3) - (3)]); }
    break;

  case 338:
/* Line 1778 of yacc.c  */
#line 806 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 339:
/* Line 1778 of yacc.c  */
#line 807 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); }
    break;

  case 340:
/* Line 1778 of yacc.c  */
#line 808 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 341:
/* Line 1778 of yacc.c  */
#line 809 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_shell_exec(&(yyval), &(yyvsp[(2) - (3)]) TSRMLS_CC); }
    break;

  case 342:
/* Line 1778 of yacc.c  */
#line 810 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_print(&(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 343:
/* Line 1778 of yacc.c  */
#line 811 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_yield(&(yyval), NULL, NULL, 0 TSRMLS_CC); }
    break;

  case 344:
/* Line 1778 of yacc.c  */
#line 812 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_lambda_function_declaration(&(yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]).op_type, 0 TSRMLS_CC); }
    break;

  case 345:
/* Line 1778 of yacc.c  */
#line 814 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_declaration(&(yyvsp[(1) - (10)]) TSRMLS_CC); (yyval) = (yyvsp[(3) - (10)]); }
    break;

  case 346:
/* Line 1778 of yacc.c  */
#line 815 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_lambda_function_declaration(&(yyval), &(yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]).op_type, 1 TSRMLS_CC); }
    break;

  case 347:
/* Line 1778 of yacc.c  */
#line 817 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_declaration(&(yyvsp[(2) - (11)]) TSRMLS_CC); (yyval) = (yyvsp[(4) - (11)]); }
    break;

  case 348:
/* Line 1778 of yacc.c  */
#line 821 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_yield(&(yyval), &(yyvsp[(2) - (2)]), NULL, 0 TSRMLS_CC); }
    break;

  case 349:
/* Line 1778 of yacc.c  */
#line 822 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_yield(&(yyval), &(yyvsp[(2) - (2)]), NULL, 1 TSRMLS_CC); }
    break;

  case 350:
/* Line 1778 of yacc.c  */
#line 823 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_yield(&(yyval), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]), 0 TSRMLS_CC); }
    break;

  case 351:
/* Line 1778 of yacc.c  */
#line 824 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_yield(&(yyval), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]), 1 TSRMLS_CC); }
    break;

  case 352:
/* Line 1778 of yacc.c  */
#line 828 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 353:
/* Line 1778 of yacc.c  */
#line 829 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 354:
/* Line 1778 of yacc.c  */
#line 830 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (4)]).EA = 0; zend_do_begin_variable_parse(TSRMLS_C); fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 355:
/* Line 1778 of yacc.c  */
#line 833 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); }
    break;

  case 356:
/* Line 1778 of yacc.c  */
#line 834 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 357:
/* Line 1778 of yacc.c  */
#line 837 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = CG(zend_lineno); }
    break;

  case 360:
/* Line 1778 of yacc.c  */
#line 846 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_lexical_variable(&(yyvsp[(3) - (3)]), 0 TSRMLS_CC); }
    break;

  case 361:
/* Line 1778 of yacc.c  */
#line 847 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_lexical_variable(&(yyvsp[(4) - (4)]), 1 TSRMLS_CC); }
    break;

  case 362:
/* Line 1778 of yacc.c  */
#line 848 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_lexical_variable(&(yyvsp[(1) - (1)]), 0 TSRMLS_CC); }
    break;

  case 363:
/* Line 1778 of yacc.c  */
#line 849 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_lexical_variable(&(yyvsp[(2) - (2)]), 1 TSRMLS_CC); }
    break;

  case 364:
/* Line 1778 of yacc.c  */
#line 853 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = zend_do_begin_function_call(&(yyvsp[(1) - (1)]), 1 TSRMLS_CC); }
    break;

  case 365:
/* Line 1778 of yacc.c  */
#line 854 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(&(yyvsp[(1) - (3)]), &(yyval), &(yyvsp[(3) - (3)]), 0, (yyvsp[(2) - (3)]).u.op.opline_num TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C); }
    break;

  case 366:
/* Line 1778 of yacc.c  */
#line 855 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (3)]).op_type = IS_CONST; ZVAL_EMPTY_STRING(&(yyvsp[(1) - (3)]).u.constant);  zend_do_build_namespace_name(&(yyvsp[(1) - (3)]), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); (yyval).u.op.opline_num = zend_do_begin_function_call(&(yyvsp[(1) - (3)]), 0 TSRMLS_CC); }
    break;

  case 367:
/* Line 1778 of yacc.c  */
#line 856 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(&(yyvsp[(1) - (5)]), &(yyval), &(yyvsp[(5) - (5)]), 0, (yyvsp[(4) - (5)]).u.op.opline_num TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C); }
    break;

  case 368:
/* Line 1778 of yacc.c  */
#line 857 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = zend_do_begin_function_call(&(yyvsp[(2) - (2)]), 0 TSRMLS_CC); }
    break;

  case 369:
/* Line 1778 of yacc.c  */
#line 858 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(&(yyvsp[(2) - (4)]), &(yyval), &(yyvsp[(4) - (4)]), 0, (yyvsp[(3) - (4)]).u.op.opline_num TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C); }
    break;

  case 370:
/* Line 1778 of yacc.c  */
#line 859 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).u.op.opline_num = zend_do_begin_class_member_function_call(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 371:
/* Line 1778 of yacc.c  */
#line 860 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call((yyvsp[(4) - (5)]).u.op.opline_num?NULL:&(yyvsp[(3) - (5)]), &(yyval), &(yyvsp[(5) - (5)]), (yyvsp[(4) - (5)]).u.op.opline_num, (yyvsp[(4) - (5)]).u.op.opline_num TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 372:
/* Line 1778 of yacc.c  */
#line 861 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(3) - (3)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_begin_class_member_function_call(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 373:
/* Line 1778 of yacc.c  */
#line 862 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(NULL, &(yyval), &(yyvsp[(5) - (5)]), 1, 1 TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 374:
/* Line 1778 of yacc.c  */
#line 863 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_class_member_function_call(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 375:
/* Line 1778 of yacc.c  */
#line 864 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(NULL, &(yyval), &(yyvsp[(5) - (5)]), 1, 1 TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 376:
/* Line 1778 of yacc.c  */
#line 865 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(3) - (3)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_begin_class_member_function_call(&(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 377:
/* Line 1778 of yacc.c  */
#line 866 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(NULL, &(yyval), &(yyvsp[(5) - (5)]), 1, 1 TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 378:
/* Line 1778 of yacc.c  */
#line 867 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_begin_dynamic_function_call(&(yyvsp[(1) - (1)]), 0 TSRMLS_CC); }
    break;

  case 379:
/* Line 1778 of yacc.c  */
#line 868 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(&(yyvsp[(1) - (3)]), &(yyval), &(yyvsp[(3) - (3)]), 0, 1 TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C);}
    break;

  case 380:
/* Line 1778 of yacc.c  */
#line 872 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; ZVAL_STRINGL(&(yyval).u.constant, "static", sizeof("static")-1, 1);}
    break;

  case 381:
/* Line 1778 of yacc.c  */
#line 873 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 382:
/* Line 1778 of yacc.c  */
#line 874 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; ZVAL_EMPTY_STRING(&(yyval).u.constant);  zend_do_build_namespace_name(&(yyval), &(yyval), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 383:
/* Line 1778 of yacc.c  */
#line 875 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { char *tmp = estrndup(Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); memcpy(&(tmp[1]), Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); tmp[0] = '\\'; efree(Z_STRVAL((yyvsp[(2) - (2)]).u.constant)); Z_STRVAL((yyvsp[(2) - (2)]).u.constant) = tmp; ++Z_STRLEN((yyvsp[(2) - (2)]).u.constant); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 384:
/* Line 1778 of yacc.c  */
#line 879 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 385:
/* Line 1778 of yacc.c  */
#line 880 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; ZVAL_EMPTY_STRING(&(yyval).u.constant);  zend_do_build_namespace_name(&(yyval), &(yyval), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 386:
/* Line 1778 of yacc.c  */
#line 881 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { char *tmp = estrndup(Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); memcpy(&(tmp[1]), Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); tmp[0] = '\\'; efree(Z_STRVAL((yyvsp[(2) - (2)]).u.constant)); Z_STRVAL((yyvsp[(2) - (2)]).u.constant) = tmp; ++Z_STRLEN((yyvsp[(2) - (2)]).u.constant); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 387:
/* Line 1778 of yacc.c  */
#line 887 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_class(&(yyval), &(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 388:
/* Line 1778 of yacc.c  */
#line 888 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_fetch_class(&(yyval), &(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 389:
/* Line 1778 of yacc.c  */
#line 893 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 390:
/* Line 1778 of yacc.c  */
#line 894 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 391:
/* Line 1778 of yacc.c  */
#line 895 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pop_object(&(yyval) TSRMLS_CC); (yyval).EA = ZEND_PARSED_MEMBER; }
    break;

  case 392:
/* Line 1778 of yacc.c  */
#line 896 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 395:
/* Line 1778 of yacc.c  */
#line 907 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 396:
/* Line 1778 of yacc.c  */
#line 911 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { memset(&(yyval), 0, sizeof(znode)); (yyval).op_type = IS_UNUSED; }
    break;

  case 397:
/* Line 1778 of yacc.c  */
#line 912 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { memset(&(yyval), 0, sizeof(znode)); (yyval).op_type = IS_UNUSED; }
    break;

  case 398:
/* Line 1778 of yacc.c  */
#line 913 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 399:
/* Line 1778 of yacc.c  */
#line 917 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_EMPTY_STRING(&(yyval).u.constant); INIT_PZVAL(&(yyval).u.constant); (yyval).op_type = IS_CONST; }
    break;

  case 400:
/* Line 1778 of yacc.c  */
#line 918 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 401:
/* Line 1778 of yacc.c  */
#line 919 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 402:
/* Line 1778 of yacc.c  */
#line 924 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 0; }
    break;

  case 403:
/* Line 1778 of yacc.c  */
#line 925 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 404:
/* Line 1778 of yacc.c  */
#line 930 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 405:
/* Line 1778 of yacc.c  */
#line 931 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 406:
/* Line 1778 of yacc.c  */
#line 932 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 407:
/* Line 1778 of yacc.c  */
#line 933 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 408:
/* Line 1778 of yacc.c  */
#line 934 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 409:
/* Line 1778 of yacc.c  */
#line 935 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 410:
/* Line 1778 of yacc.c  */
#line 936 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 411:
/* Line 1778 of yacc.c  */
#line 937 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 412:
/* Line 1778 of yacc.c  */
#line 938 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 413:
/* Line 1778 of yacc.c  */
#line 939 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 414:
/* Line 1778 of yacc.c  */
#line 940 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 415:
/* Line 1778 of yacc.c  */
#line 941 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_EMPTY_STRING(&(yyval).u.constant); INIT_PZVAL(&(yyval).u.constant); (yyval).op_type = IS_CONST; }
    break;

  case 416:
/* Line 1778 of yacc.c  */
#line 946 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 417:
/* Line 1778 of yacc.c  */
#line 947 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 418:
/* Line 1778 of yacc.c  */
#line 948 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(1) - (1)]), ZEND_CT, 1 TSRMLS_CC); }
    break;

  case 419:
/* Line 1778 of yacc.c  */
#line 949 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; ZVAL_EMPTY_STRING(&(yyval).u.constant);  zend_do_build_namespace_name(&(yyval), &(yyval), &(yyvsp[(3) - (3)]) TSRMLS_CC); (yyvsp[(3) - (3)]) = (yyval); zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(3) - (3)]), ZEND_CT, 0 TSRMLS_CC); }
    break;

  case 420:
/* Line 1778 of yacc.c  */
#line 950 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { char *tmp = estrndup(Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); memcpy(&(tmp[1]), Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); tmp[0] = '\\'; efree(Z_STRVAL((yyvsp[(2) - (2)]).u.constant)); Z_STRVAL((yyvsp[(2) - (2)]).u.constant) = tmp; ++Z_STRLEN((yyvsp[(2) - (2)]).u.constant); zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(2) - (2)]), ZEND_CT, 0 TSRMLS_CC); }
    break;

  case 421:
/* Line 1778 of yacc.c  */
#line 951 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_LONG(&(yyvsp[(1) - (2)]).u.constant, 0); add_function(&(yyvsp[(2) - (2)]).u.constant, &(yyvsp[(1) - (2)]).u.constant, &(yyvsp[(2) - (2)]).u.constant TSRMLS_CC); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 422:
/* Line 1778 of yacc.c  */
#line 952 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { ZVAL_LONG(&(yyvsp[(1) - (2)]).u.constant, 0); sub_function(&(yyvsp[(2) - (2)]).u.constant, &(yyvsp[(1) - (2)]).u.constant, &(yyvsp[(2) - (2)]).u.constant TSRMLS_CC); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 423:
/* Line 1778 of yacc.c  */
#line 953 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); Z_TYPE((yyval).u.constant) = IS_CONSTANT_ARRAY; }
    break;

  case 424:
/* Line 1778 of yacc.c  */
#line 954 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); Z_TYPE((yyval).u.constant) = IS_CONSTANT_ARRAY; }
    break;

  case 425:
/* Line 1778 of yacc.c  */
#line 955 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 426:
/* Line 1778 of yacc.c  */
#line 956 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 427:
/* Line 1778 of yacc.c  */
#line 960 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_constant(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), ZEND_CT, 0 TSRMLS_CC); }
    break;

  case 428:
/* Line 1778 of yacc.c  */
#line 964 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 429:
/* Line 1778 of yacc.c  */
#line 965 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 430:
/* Line 1778 of yacc.c  */
#line 966 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 431:
/* Line 1778 of yacc.c  */
#line 967 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(1) - (1)]), ZEND_RT, 1 TSRMLS_CC); }
    break;

  case 432:
/* Line 1778 of yacc.c  */
#line 968 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; ZVAL_EMPTY_STRING(&(yyval).u.constant);  zend_do_build_namespace_name(&(yyval), &(yyval), &(yyvsp[(3) - (3)]) TSRMLS_CC); (yyvsp[(3) - (3)]) = (yyval); zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(3) - (3)]), ZEND_RT, 0 TSRMLS_CC); }
    break;

  case 433:
/* Line 1778 of yacc.c  */
#line 969 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { char *tmp = estrndup(Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); memcpy(&(tmp[1]), Z_STRVAL((yyvsp[(2) - (2)]).u.constant), Z_STRLEN((yyvsp[(2) - (2)]).u.constant)+1); tmp[0] = '\\'; efree(Z_STRVAL((yyvsp[(2) - (2)]).u.constant)); Z_STRVAL((yyvsp[(2) - (2)]).u.constant) = tmp; ++Z_STRLEN((yyvsp[(2) - (2)]).u.constant); zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(2) - (2)]), ZEND_RT, 0 TSRMLS_CC); }
    break;

  case 434:
/* Line 1778 of yacc.c  */
#line 970 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 435:
/* Line 1778 of yacc.c  */
#line 971 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 436:
/* Line 1778 of yacc.c  */
#line 972 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 437:
/* Line 1778 of yacc.c  */
#line 973 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { if (Z_TYPE((yyvsp[(1) - (1)]).u.constant) == IS_CONSTANT) {zend_do_fetch_constant(&(yyval), NULL, &(yyvsp[(1) - (1)]), ZEND_RT, 1 TSRMLS_CC);} else {(yyval) = (yyvsp[(1) - (1)]);} }
    break;

  case 438:
/* Line 1778 of yacc.c  */
#line 978 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; INIT_PZVAL(&(yyval).u.constant); array_init(&(yyval).u.constant); }
    break;

  case 439:
/* Line 1778 of yacc.c  */
#line 979 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 442:
/* Line 1778 of yacc.c  */
#line 988 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_static_array_element(&(yyval), &(yyvsp[(3) - (5)]), &(yyvsp[(5) - (5)])); }
    break;

  case 443:
/* Line 1778 of yacc.c  */
#line 989 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_static_array_element(&(yyval), NULL, &(yyvsp[(3) - (3)])); }
    break;

  case 444:
/* Line 1778 of yacc.c  */
#line 990 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; INIT_PZVAL(&(yyval).u.constant); array_init(&(yyval).u.constant); zend_do_add_static_array_element(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 445:
/* Line 1778 of yacc.c  */
#line 991 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_CONST; INIT_PZVAL(&(yyval).u.constant); array_init(&(yyval).u.constant); zend_do_add_static_array_element(&(yyval), NULL, &(yyvsp[(1) - (1)])); }
    break;

  case 446:
/* Line 1778 of yacc.c  */
#line 995 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 447:
/* Line 1778 of yacc.c  */
#line 996 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 448:
/* Line 1778 of yacc.c  */
#line 1000 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 449:
/* Line 1778 of yacc.c  */
#line 1001 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 450:
/* Line 1778 of yacc.c  */
#line 1006 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 451:
/* Line 1778 of yacc.c  */
#line 1011 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_W, 0 TSRMLS_CC); (yyval) = (yyvsp[(1) - (1)]);
				  zend_check_writable_variable(&(yyvsp[(1) - (1)])); }
    break;

  case 452:
/* Line 1778 of yacc.c  */
#line 1016 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_RW, 0 TSRMLS_CC); (yyval) = (yyvsp[(1) - (1)]);
				  zend_check_writable_variable(&(yyvsp[(1) - (1)])); }
    break;

  case 453:
/* Line 1778 of yacc.c  */
#line 1021 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(1) - (2)]) TSRMLS_CC); }
    break;

  case 454:
/* Line 1778 of yacc.c  */
#line 1022 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(4) - (4)]) TSRMLS_CC); }
    break;

  case 455:
/* Line 1778 of yacc.c  */
#line 1023 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pop_object(&(yyval) TSRMLS_CC); (yyval).EA = (yyvsp[(1) - (7)]).EA | ((yyvsp[(7) - (7)]).EA ? (yyvsp[(7) - (7)]).EA : (yyvsp[(6) - (7)]).EA); }
    break;

  case 456:
/* Line 1778 of yacc.c  */
#line 1024 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 457:
/* Line 1778 of yacc.c  */
#line 1028 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = (yyvsp[(2) - (2)]).EA; }
    break;

  case 458:
/* Line 1778 of yacc.c  */
#line 1029 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = 0; }
    break;

  case 459:
/* Line 1778 of yacc.c  */
#line 1034 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_push_object(&(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 460:
/* Line 1778 of yacc.c  */
#line 1034 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = (yyvsp[(4) - (4)]).EA; }
    break;

  case 461:
/* Line 1778 of yacc.c  */
#line 1038 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 462:
/* Line 1778 of yacc.c  */
#line 1039 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyvsp[(1) - (4)]).EA = ZEND_PARSED_METHOD_CALL; fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 463:
/* Line 1778 of yacc.c  */
#line 1043 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_pop_object(&(yyval) TSRMLS_CC); zend_do_begin_method_call(&(yyval) TSRMLS_CC); }
    break;

  case 464:
/* Line 1778 of yacc.c  */
#line 1044 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_function_call(&(yyvsp[(1) - (2)]), &(yyval), &(yyvsp[(2) - (2)]), 1, 1 TSRMLS_CC); zend_do_extended_fcall_end(TSRMLS_C); }
    break;

  case 465:
/* Line 1778 of yacc.c  */
#line 1048 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval).EA = ZEND_PARSED_METHOD_CALL; zend_do_push_object(&(yyval) TSRMLS_CC); }
    break;

  case 466:
/* Line 1778 of yacc.c  */
#line 1049 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); zend_do_push_object(&(yyval) TSRMLS_CC); }
    break;

  case 467:
/* Line 1778 of yacc.c  */
#line 1050 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).EA = ZEND_PARSED_MEMBER; }
    break;

  case 468:
/* Line 1778 of yacc.c  */
#line 1054 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 469:
/* Line 1778 of yacc.c  */
#line 1055 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_indirect_references(&(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 470:
/* Line 1778 of yacc.c  */
#line 1059 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (3)]); zend_do_fetch_static_member(&(yyval), &(yyvsp[(1) - (3)]) TSRMLS_CC); }
    break;

  case 471:
/* Line 1778 of yacc.c  */
#line 1060 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (3)]); zend_do_fetch_static_member(&(yyval), &(yyvsp[(1) - (3)]) TSRMLS_CC); }
    break;

  case 472:
/* Line 1778 of yacc.c  */
#line 1065 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); (yyval)=(yyvsp[(1) - (1)]);; }
    break;

  case 473:
/* Line 1778 of yacc.c  */
#line 1069 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 474:
/* Line 1778 of yacc.c  */
#line 1070 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); (yyvsp[(1) - (1)]).EA = ZEND_PARSED_FUNCTION_CALL; }
    break;

  case 475:
/* Line 1778 of yacc.c  */
#line 1071 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (5)]), &(yyvsp[(4) - (5)]) TSRMLS_CC); }
    break;

  case 476:
/* Line 1778 of yacc.c  */
#line 1075 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 477:
/* Line 1778 of yacc.c  */
#line 1076 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 478:
/* Line 1778 of yacc.c  */
#line 1077 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); (yyval) = (yyvsp[(1) - (1)]); (yyval).EA = ZEND_PARSED_FUNCTION_CALL; }
    break;

  case 479:
/* Line 1778 of yacc.c  */
#line 1082 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval).EA = ZEND_PARSED_VARIABLE; }
    break;

  case 480:
/* Line 1778 of yacc.c  */
#line 1083 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_indirect_references(&(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); (yyval).EA = ZEND_PARSED_VARIABLE; }
    break;

  case 481:
/* Line 1778 of yacc.c  */
#line 1084 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval).EA = ZEND_PARSED_STATIC_MEMBER; }
    break;

  case 482:
/* Line 1778 of yacc.c  */
#line 1088 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 483:
/* Line 1778 of yacc.c  */
#line 1089 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_string_offset(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 484:
/* Line 1778 of yacc.c  */
#line 1090 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); fetch_simple_variable(&(yyval), &(yyvsp[(1) - (1)]), 1 TSRMLS_CC); }
    break;

  case 485:
/* Line 1778 of yacc.c  */
#line 1095 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 486:
/* Line 1778 of yacc.c  */
#line 1096 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); }
    break;

  case 487:
/* Line 1778 of yacc.c  */
#line 1100 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval).op_type = IS_UNUSED; }
    break;

  case 488:
/* Line 1778 of yacc.c  */
#line 1101 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 489:
/* Line 1778 of yacc.c  */
#line 1106 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 490:
/* Line 1778 of yacc.c  */
#line 1107 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); }
    break;

  case 491:
/* Line 1778 of yacc.c  */
#line 1107 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { znode tmp_znode;  zend_do_pop_object(&tmp_znode TSRMLS_CC);  zend_do_fetch_property(&(yyval), &tmp_znode, &(yyvsp[(1) - (2)]) TSRMLS_CC);}
    break;

  case 492:
/* Line 1778 of yacc.c  */
#line 1111 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_dim(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 493:
/* Line 1778 of yacc.c  */
#line 1112 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_string_offset(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 494:
/* Line 1778 of yacc.c  */
#line 1113 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { znode tmp_znode;  zend_do_pop_object(&tmp_znode TSRMLS_CC);  zend_do_fetch_property(&(yyval), &tmp_znode, &(yyvsp[(1) - (1)]) TSRMLS_CC);}
    break;

  case 495:
/* Line 1778 of yacc.c  */
#line 1117 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 496:
/* Line 1778 of yacc.c  */
#line 1118 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 497:
/* Line 1778 of yacc.c  */
#line 1122 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant) = 1; }
    break;

  case 498:
/* Line 1778 of yacc.c  */
#line 1123 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { Z_LVAL((yyval).u.constant)++; }
    break;

  case 501:
/* Line 1778 of yacc.c  */
#line 1133 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_list_element(&(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 502:
/* Line 1778 of yacc.c  */
#line 1134 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_new_list_begin(TSRMLS_C); }
    break;

  case 503:
/* Line 1778 of yacc.c  */
#line 1134 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_new_list_end(TSRMLS_C); }
    break;

  case 504:
/* Line 1778 of yacc.c  */
#line 1135 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_list_element(NULL TSRMLS_CC); }
    break;

  case 505:
/* Line 1778 of yacc.c  */
#line 1140 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_init_array(&(yyval), NULL, NULL, 0 TSRMLS_CC); }
    break;

  case 506:
/* Line 1778 of yacc.c  */
#line 1141 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 507:
/* Line 1778 of yacc.c  */
#line 1145 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_array_element(&(yyval), &(yyvsp[(5) - (5)]), &(yyvsp[(3) - (5)]), 0 TSRMLS_CC); }
    break;

  case 508:
/* Line 1778 of yacc.c  */
#line 1146 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_array_element(&(yyval), &(yyvsp[(3) - (3)]), NULL, 0 TSRMLS_CC); }
    break;

  case 509:
/* Line 1778 of yacc.c  */
#line 1147 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_init_array(&(yyval), &(yyvsp[(3) - (3)]), &(yyvsp[(1) - (3)]), 0 TSRMLS_CC); }
    break;

  case 510:
/* Line 1778 of yacc.c  */
#line 1148 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_init_array(&(yyval), &(yyvsp[(1) - (1)]), NULL, 0 TSRMLS_CC); }
    break;

  case 511:
/* Line 1778 of yacc.c  */
#line 1149 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_array_element(&(yyval), &(yyvsp[(6) - (6)]), &(yyvsp[(3) - (6)]), 1 TSRMLS_CC); }
    break;

  case 512:
/* Line 1778 of yacc.c  */
#line 1150 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_array_element(&(yyval), &(yyvsp[(4) - (4)]), NULL, 1 TSRMLS_CC); }
    break;

  case 513:
/* Line 1778 of yacc.c  */
#line 1151 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_init_array(&(yyval), &(yyvsp[(4) - (4)]), &(yyvsp[(1) - (4)]), 1 TSRMLS_CC); }
    break;

  case 514:
/* Line 1778 of yacc.c  */
#line 1152 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_init_array(&(yyval), &(yyvsp[(2) - (2)]), NULL, 1 TSRMLS_CC); }
    break;

  case 515:
/* Line 1778 of yacc.c  */
#line 1156 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(2) - (2)]), BP_VAR_R, 0 TSRMLS_CC);  zend_do_add_variable(&(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 516:
/* Line 1778 of yacc.c  */
#line 1157 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_string(&(yyval), &(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 517:
/* Line 1778 of yacc.c  */
#line 1158 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_end_variable_parse(&(yyvsp[(1) - (1)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_add_variable(&(yyval), NULL, &(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 518:
/* Line 1778 of yacc.c  */
#line 1159 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_add_string(&(yyval), NULL, &(yyvsp[(1) - (2)]) TSRMLS_CC); zend_do_end_variable_parse(&(yyvsp[(2) - (2)]), BP_VAR_R, 0 TSRMLS_CC); zend_do_add_variable(&(yyval), &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 519:
/* Line 1778 of yacc.c  */
#line 1165 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); fetch_simple_variable(&(yyval), &(yyvsp[(1) - (1)]), 1 TSRMLS_CC); }
    break;

  case 520:
/* Line 1778 of yacc.c  */
#line 1166 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); }
    break;

  case 521:
/* Line 1778 of yacc.c  */
#line 1166 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_array_begin(&(yyval), &(yyvsp[(1) - (5)]), &(yyvsp[(4) - (5)]) TSRMLS_CC); }
    break;

  case 522:
/* Line 1778 of yacc.c  */
#line 1167 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C); fetch_simple_variable(&(yyvsp[(2) - (3)]), &(yyvsp[(1) - (3)]), 1 TSRMLS_CC); zend_do_fetch_property(&(yyval), &(yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)]) TSRMLS_CC); }
    break;

  case 523:
/* Line 1778 of yacc.c  */
#line 1168 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C);  fetch_simple_variable(&(yyval), &(yyvsp[(2) - (3)]), 1 TSRMLS_CC); }
    break;

  case 524:
/* Line 1778 of yacc.c  */
#line 1169 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_begin_variable_parse(TSRMLS_C);  fetch_array_begin(&(yyval), &(yyvsp[(2) - (6)]), &(yyvsp[(4) - (6)]) TSRMLS_CC); }
    break;

  case 525:
/* Line 1778 of yacc.c  */
#line 1170 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 526:
/* Line 1778 of yacc.c  */
#line 1175 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 527:
/* Line 1778 of yacc.c  */
#line 1176 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 528:
/* Line 1778 of yacc.c  */
#line 1177 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { fetch_simple_variable(&(yyval), &(yyvsp[(1) - (1)]), 1 TSRMLS_CC); }
    break;

  case 529:
/* Line 1778 of yacc.c  */
#line 1182 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(3) - (4)]); }
    break;

  case 530:
/* Line 1778 of yacc.c  */
#line 1183 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_isset_or_isempty(ZEND_ISEMPTY, &(yyval), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 531:
/* Line 1778 of yacc.c  */
#line 1184 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_unary_op(ZEND_BOOL_NOT, &(yyval), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 532:
/* Line 1778 of yacc.c  */
#line 1185 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_include_or_eval(ZEND_INCLUDE, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 533:
/* Line 1778 of yacc.c  */
#line 1186 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_include_or_eval(ZEND_INCLUDE_ONCE, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 534:
/* Line 1778 of yacc.c  */
#line 1187 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_include_or_eval(ZEND_EVAL, &(yyval), &(yyvsp[(3) - (4)]) TSRMLS_CC); }
    break;

  case 535:
/* Line 1778 of yacc.c  */
#line 1188 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_include_or_eval(ZEND_REQUIRE, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 536:
/* Line 1778 of yacc.c  */
#line 1189 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_include_or_eval(ZEND_REQUIRE_ONCE, &(yyval), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 537:
/* Line 1778 of yacc.c  */
#line 1193 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 538:
/* Line 1778 of yacc.c  */
#line 1194 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_begin(&(yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)]) TSRMLS_CC); }
    break;

  case 539:
/* Line 1778 of yacc.c  */
#line 1194 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_boolean_and_end(&(yyval), &(yyvsp[(1) - (4)]), &(yyvsp[(4) - (4)]), &(yyvsp[(2) - (4)]) TSRMLS_CC); }
    break;

  case 540:
/* Line 1778 of yacc.c  */
#line 1198 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_isset_or_isempty(ZEND_ISSET, &(yyval), &(yyvsp[(1) - (1)]) TSRMLS_CC); }
    break;

  case 541:
/* Line 1778 of yacc.c  */
#line 1199 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_error(E_COMPILE_ERROR, "Cannot use isset() on the result of an expression (you can use \"null !== expression\" instead)"); }
    break;

  case 542:
/* Line 1778 of yacc.c  */
#line 1203 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_constant(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), ZEND_RT, 0 TSRMLS_CC); }
    break;

  case 543:
/* Line 1778 of yacc.c  */
#line 1204 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_fetch_constant(&(yyval), &(yyvsp[(1) - (3)]), &(yyvsp[(3) - (3)]), ZEND_RT, 0 TSRMLS_CC); }
    break;

  case 544:
/* Line 1778 of yacc.c  */
#line 1208 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_resolve_class_name(&(yyval), &(yyvsp[(1) - (3)]), 1 TSRMLS_CC); }
    break;

  case 545:
/* Line 1778 of yacc.c  */
#line 1212 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"
    { zend_do_resolve_class_name(&(yyval), &(yyvsp[(1) - (3)]), 0 TSRMLS_CC); }
    break;


/* Line 1778 of yacc.c  */
#line 6461 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.c"
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


/* Line 2041 of yacc.c  */
#line 1215 "/home/seg/DEV/northstar/src/router/php5/Zend/zend_language_parser.y"


/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	if (!yyres) {
		return yystrlen(yystr);
	}
	{
		TSRMLS_FETCH();
		if (CG(parse_error) == 0) {
			char buffer[120];
			const unsigned char *end, *str, *tok1 = NULL, *tok2 = NULL;
			unsigned int len = 0, toklen = 0, yystr_len;
			
			CG(parse_error) = 1;

			if (LANG_SCNG(yy_text)[0] == 0 &&
				LANG_SCNG(yy_leng) == 1 &&
				memcmp(yystr, "\"end of file\"", sizeof("\"end of file\"") - 1) == 0) {
				yystpcpy(yyres, "end of file");
				return sizeof("end of file")-1;
			}
			
			str = LANG_SCNG(yy_text);
			end = memchr(str, '\n', LANG_SCNG(yy_leng));
			yystr_len = yystrlen(yystr);
			
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
			if (toklen) {
				snprintf(buffer, sizeof(buffer), "'%.*s' %.*s", len, str, toklen, tok1);
			} else {
				snprintf(buffer, sizeof(buffer), "'%.*s'", len, str);
			}
			yystpcpy(yyres, buffer);
			return len + (toklen ? toklen + 1 : 0) + 2;
		}		
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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
