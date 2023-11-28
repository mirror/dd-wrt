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

/* Substitute the type names.  */
#define YYSTYPE         INI_STYPE
/* Substitute the variable and function names.  */
#define yyparse         ini_parse
#define yylex           ini_lex
#define yyerror         ini_error
#define yydebug         ini_debug
#define yynerrs         ini_nerrs


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
   | Authors: Zeev Suraski <zeev@php.net>                                 |
   |          Jani Taskinen <jani@php.net>                                |
   +----------------------------------------------------------------------+
*/

#define DEBUG_CFG_PARSER 0

#include "zend.h"
#include "zend_API.h"
#include "zend_ini.h"
#include "zend_constants.h"
#include "zend_ini_scanner.h"
#include "zend_extensions.h"

#ifdef ZEND_WIN32
#include "win32/syslog.h"
#endif

int ini_parse(void);

#define ZEND_INI_PARSER_CB	(CG(ini_parser_param))->ini_parser_cb
#define ZEND_INI_PARSER_ARG	(CG(ini_parser_param))->arg

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

#define ZEND_SYSTEM_INI CG(ini_parser_unbuffered_errors)
#define INI_ZVAL_IS_NUMBER 1

static int get_int_val(zval *op) {
	switch (Z_TYPE_P(op)) {
		case IS_LONG:
			return Z_LVAL_P(op);
		case IS_DOUBLE:
			return (int)Z_DVAL_P(op);
		case IS_STRING:
		{
			int val = atoi(Z_STRVAL_P(op));
			zend_string_free(Z_STR_P(op));
			return val;
		}
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

/* {{{ zend_ini_do_op() */
static void zend_ini_do_op(char type, zval *result, zval *op1, zval *op2)
{
	int i_result;
	int i_op1, i_op2;
	int str_len;
	char str_result[MAX_LENGTH_OF_LONG+1];

	i_op1 = get_int_val(op1);
	i_op2 = op2 ? get_int_val(op2) : 0;

	switch (type) {
		case '|':
			i_result = i_op1 | i_op2;
			break;
		case '&':
			i_result = i_op1 & i_op2;
			break;
		case '^':
			i_result = i_op1 ^ i_op2;
			break;
		case '~':
			i_result = ~i_op1;
			break;
		case '!':
			i_result = !i_op1;
			break;
		default:
			i_result = 0;
			break;
	}

	if (INI_SCNG(scanner_mode) != ZEND_INI_SCANNER_TYPED) {
		str_len = sprintf(str_result, "%d", i_result);
		ZVAL_NEW_STR(result, zend_string_init(str_result, str_len, ZEND_SYSTEM_INI));
	} else {
		ZVAL_LONG(result, i_result);
	}
}
/* }}} */

/* {{{ zend_ini_init_string() */
static void zend_ini_init_string(zval *result)
{
	if (ZEND_SYSTEM_INI) {
		ZVAL_EMPTY_PSTRING(result);
	} else {
		ZVAL_EMPTY_STRING(result);
	}
	Z_EXTRA_P(result) = 0;
}
/* }}} */

/* {{{ zend_ini_add_string() */
static void zend_ini_add_string(zval *result, zval *op1, zval *op2)
{
	int length, op1_len;

	if (Z_TYPE_P(op1) != IS_STRING) {
		/* ZEND_ASSERT(!Z_REFCOUNTED_P(op1)); */
		if (ZEND_SYSTEM_INI) {
			zend_string *tmp_str;
			zend_string *str = zval_get_tmp_string(op1, &tmp_str);
			ZVAL_PSTRINGL(op1, ZSTR_VAL(str), ZSTR_LEN(str));
			zend_tmp_string_release(tmp_str);
		} else {
			ZVAL_STR(op1, zval_get_string_func(op1));
		}
	}
	op1_len = (int)Z_STRLEN_P(op1);

	if (Z_TYPE_P(op2) != IS_STRING) {
		convert_to_string(op2);
	}
	length = op1_len + (int)Z_STRLEN_P(op2);

	ZVAL_NEW_STR(result, zend_string_extend(Z_STR_P(op1), length, ZEND_SYSTEM_INI));
	memcpy(Z_STRVAL_P(result) + op1_len, Z_STRVAL_P(op2), Z_STRLEN_P(op2) + 1);
}
/* }}} */

/* {{{ zend_ini_get_constant() */
static void zend_ini_get_constant(zval *result, zval *name)
{
	zval *c, tmp;

	/* If name contains ':' it is not a constant. Bug #26893. */
	if (!memchr(Z_STRVAL_P(name), ':', Z_STRLEN_P(name))
		   	&& (c = zend_get_constant(Z_STR_P(name))) != 0) {
		if (Z_TYPE_P(c) != IS_STRING) {
			ZVAL_COPY_OR_DUP(&tmp, c);
			if (Z_OPT_CONSTANT(tmp)) {
				zval_update_constant_ex(&tmp, NULL);
			}
			convert_to_string(&tmp);
			c = &tmp;
		}
		ZVAL_NEW_STR(result, zend_string_init(Z_STRVAL_P(c), Z_STRLEN_P(c), ZEND_SYSTEM_INI));
		if (c == &tmp) {
			zend_string_release(Z_STR(tmp));
		}
		zend_string_free(Z_STR_P(name));
	} else {
		*result = *name;
	}
}
/* }}} */

/* {{{ zend_ini_get_var() */
static void zend_ini_get_var(zval *result, zval *name, zval *fallback)
{
	zval *curval;
	char *envvar;

	/* Fetch configuration option value */
	if ((curval = zend_get_configuration_directive(Z_STR_P(name))) != NULL) {
		ZVAL_NEW_STR(result, zend_string_init(Z_STRVAL_P(curval), Z_STRLEN_P(curval), ZEND_SYSTEM_INI));
	/* ..or if not found, try ENV */
	} else if ((envvar = zend_getenv(Z_STRVAL_P(name), Z_STRLEN_P(name))) != NULL ||
			   (envvar = getenv(Z_STRVAL_P(name))) != NULL) {
		ZVAL_NEW_STR(result, zend_string_init(envvar, strlen(envvar), ZEND_SYSTEM_INI));
	/* ..or if not defined, try fallback value */
	} else if (fallback) {
		ZVAL_NEW_STR(result, zend_string_init(Z_STRVAL_P(fallback), strlen(Z_STRVAL_P(fallback)), ZEND_SYSTEM_INI));
	} else {
		zend_ini_init_string(result);
	}

}
/* }}} */

/* {{{ ini_error() */
static ZEND_COLD void ini_error(const char *msg)
{
	char *error_buf;
	int error_buf_len;

	const char *const currently_parsed_filename = zend_ini_scanner_get_filename();
	if (currently_parsed_filename) {
		error_buf_len = 128 + (int)strlen(msg) + (int)strlen(currently_parsed_filename); /* should be more than enough */
		error_buf = (char *) emalloc(error_buf_len);

		sprintf(error_buf, "%s in %s on line %d\n", msg, currently_parsed_filename, zend_ini_scanner_get_lineno());
	} else {
		error_buf = estrdup("Invalid configuration directive\n");
	}

	if (CG(ini_parser_unbuffered_errors)) {
#ifdef ZEND_WIN32
		syslog(LOG_ALERT, "PHP: %s (%s)", error_buf, GetCommandLine());
#endif
		fprintf(stderr, "PHP:  %s", error_buf);
	} else {
		zend_error(E_WARNING, "%s", error_buf);
	}
	efree(error_buf);
}
/* }}} */

/* {{{ zend_parse_ini_file() */
ZEND_API zend_result zend_parse_ini_file(zend_file_handle *fh, bool unbuffered_errors, int scanner_mode, zend_ini_parser_cb_t ini_parser_cb, void *arg)
{
	int retval;
	zend_ini_parser_param ini_parser_param;

	ini_parser_param.ini_parser_cb = ini_parser_cb;
	ini_parser_param.arg = arg;
	CG(ini_parser_param) = &ini_parser_param;

	if (zend_ini_open_file_for_scanning(fh, scanner_mode) == FAILURE) {
		return FAILURE;
	}

	CG(ini_parser_unbuffered_errors) = unbuffered_errors;
	retval = ini_parse();

	shutdown_ini_scanner();

	if (retval == 0) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ zend_parse_ini_string() */
ZEND_API zend_result zend_parse_ini_string(const char *str, bool unbuffered_errors, int scanner_mode, zend_ini_parser_cb_t ini_parser_cb, void *arg)
{
	int retval;
	zend_ini_parser_param ini_parser_param;

	ini_parser_param.ini_parser_cb = ini_parser_cb;
	ini_parser_param.arg = arg;
	CG(ini_parser_param) = &ini_parser_param;

	if (zend_ini_prepare_string_for_scanning(str, scanner_mode) == FAILURE) {
		return FAILURE;
	}

	CG(ini_parser_unbuffered_errors) = unbuffered_errors;
	retval = ini_parse();

	shutdown_ini_scanner();

	if (retval == 0) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ zval_ini_dtor() */
static void zval_ini_dtor(zval *zv)
{
	if (Z_TYPE_P(zv) == IS_STRING) {
		if (ZEND_SYSTEM_INI) {
			GC_MAKE_PERSISTENT_LOCAL(Z_STR_P(zv));
		}
		zend_string_release(Z_STR_P(zv));
	}
}
/* }}} */

static inline zend_result convert_to_number(zval *retval, const char *str, const int str_len)
{
	uint8_t type;
	int overflow;
	zend_long lval;
	double dval;

	if ((type = is_numeric_string_ex(str, str_len, &lval, &dval, 0, &overflow, NULL)) != 0) {
		if (type == IS_LONG) {
			ZVAL_LONG(retval, lval);
			return SUCCESS;
		} else if (type == IS_DOUBLE && !overflow) {
			ZVAL_DOUBLE(retval, dval);
			return SUCCESS;
		}
	}

	return FAILURE;
}

static void normalize_value(zval *zv)
{
	if (INI_SCNG(scanner_mode) != ZEND_INI_SCANNER_TYPED) {
		return;
	}

	ZEND_ASSERT(Z_EXTRA_P(zv) == 0 || Z_EXTRA_P(zv) == INI_ZVAL_IS_NUMBER);
	if (Z_EXTRA_P(zv) == INI_ZVAL_IS_NUMBER && Z_TYPE_P(zv) == IS_STRING) {
		zval number_rv;
		if (convert_to_number(&number_rv, Z_STRVAL_P(zv), Z_STRLEN_P(zv)) == SUCCESS) {
			zval_ptr_dtor(zv);
			ZVAL_COPY_VALUE(zv, &number_rv);
		}
	}
}




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
   by #include "zend_ini_parser.h".  */
#ifndef YY_INI_ZEND_ZEND_INI_PARSER_H_INCLUDED
# define YY_INI_ZEND_ZEND_INI_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef INI_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define INI_DEBUG 1
#  else
#   define INI_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define INI_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined INI_DEBUG */
#if INI_DEBUG
extern int ini_debug;
#endif

/* Token type.  */
#ifndef INI_TOKENTYPE
# define INI_TOKENTYPE
  enum ini_tokentype
  {
    END = 0,
    TC_SECTION = 258,
    TC_RAW = 259,
    TC_CONSTANT = 260,
    TC_NUMBER = 261,
    TC_STRING = 262,
    TC_WHITESPACE = 263,
    TC_LABEL = 264,
    TC_OFFSET = 265,
    TC_DOLLAR_CURLY = 266,
    TC_VARNAME = 267,
    TC_QUOTED_STRING = 268,
    TC_FALLBACK = 269,
    BOOL_TRUE = 270,
    BOOL_FALSE = 271,
    NULL_NULL = 272,
    END_OF_LINE = 273
  };
#endif

/* Value type.  */
#if ! defined INI_STYPE && ! defined INI_STYPE_IS_DECLARED
typedef zval INI_STYPE;
# define INI_STYPE_IS_TRIVIAL 1
# define INI_STYPE_IS_DECLARED 1
#endif



int ini_parse (void);

#endif /* !YY_INI_ZEND_ZEND_INI_PARSER_H_INCLUDED  */

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
         || (defined INI_STYPE_IS_TRIVIAL && INI_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   143

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  53
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  76

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,    23,     2,    31,    30,    40,    24,
      43,    44,    29,    26,    21,    27,    22,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    20,     2,
      33,    19,    34,    35,    36,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    42,    25,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,    39,    38,    32,     2,     2,     2,
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
      15,    16,    17,    18
};

#if INI_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   359,   359,   360,   364,   371,   382,   391,   392,   396,
     397,   401,   402,   403,   404,   405,   409,   410,   414,   415,
     416,   420,   421,   422,   423,   424,   425,   429,   430,   431,
     432,   433,   434,   438,   439,   440,   441,   442,   443,   444,
     448,   449,   454,   455,   459,   460,   461,   462,   463,   467,
     468,   469,   474,   475
};
#endif

#if INI_DEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "TC_SECTION", "TC_RAW",
  "TC_CONSTANT", "TC_NUMBER", "TC_STRING", "TC_WHITESPACE", "TC_LABEL",
  "TC_OFFSET", "TC_DOLLAR_CURLY", "TC_VARNAME", "TC_QUOTED_STRING",
  "TC_FALLBACK", "BOOL_TRUE", "BOOL_FALSE", "NULL_NULL", "END_OF_LINE",
  "'='", "':'", "','", "'.'", "'\"'", "'\\''", "'^'", "'+'", "'-'", "'/'",
  "'*'", "'%'", "'$'", "'~'", "'<'", "'>'", "'?'", "'@'", "'{'", "'}'",
  "'|'", "'&'", "'!'", "']'", "'('", "')'", "$accept", "statement_list",
  "statement", "section_string_or_value", "string_or_value",
  "option_offset", "encapsed_list", "var_string_list_section",
  "var_string_list", "expr", "cfg_var_ref", "fallback", "constant_literal",
  "constant_string", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    61,
      58,    44,    46,    34,    39,    94,    43,    45,    47,    42,
      37,    36,   126,    60,    62,    63,    64,   123,   125,   124,
      38,    33,    93,    40,    41
};
# endif

#define YYPACT_NINF -46

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-46)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -46,   118,   -46,    73,   -17,    81,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,     0,   -46,   -34,    94,   -46,   -46,    -1,
     -46,   -46,   -46,   -46,   -46,   -46,   -31,   102,   -46,   -46,
       6,    59,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
      28,    28,    28,   -46,   102,    25,    80,     2,   -46,   -46,
     -46,    81,   -46,   -46,   -46,   -46,   109,   -46,   -46,    72,
      28,    28,    28,   -46,    -1,   120,   102,   -20,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     1,    10,     7,    17,     8,     2,    45,    44,
      46,    47,    48,     0,    20,     0,     9,    21,    22,     0,
      50,    49,    51,    52,    53,    20,     0,    16,    27,    28,
       0,     0,     4,    20,    24,    25,    12,    13,    14,    15,
       0,     0,     0,     5,    33,    11,     0,     0,    20,    30,
      31,    43,    40,    19,    23,    18,     0,    37,    38,     0,
       0,     0,     0,    29,     0,     0,    42,     0,    26,    39,
      36,    34,    35,     6,    32,    41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -46,   -46,   -46,   -46,   -45,   -46,     4,   -46,    -4,    14,
      -3,   -46,     7,   -18
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     7,    15,    43,    26,    31,    16,    44,    45,
      28,    67,    18,    29
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      17,    27,    19,    20,    21,    22,    23,    24,    32,    50,
      13,    47,    30,    34,    36,    37,    38,    39,    75,    73,
      51,    64,    25,    35,    49,     0,    50,     0,    55,    46,
       0,    40,    20,    21,    22,    23,    24,    56,     0,    13,
      41,    49,    42,    55,    52,     0,     0,    66,    50,     0,
      60,    25,    65,    55,    57,    58,    59,     0,     0,     0,
      40,     0,    55,    49,    61,    62,     0,     0,     0,    41,
      13,    42,    53,     0,    70,    71,    72,     8,     9,    10,
      11,    12,    54,     0,    13,    20,    21,    22,    23,    24,
       0,    13,    13,    53,     0,     0,    14,    60,     8,     9,
      10,    11,    12,    63,    25,    13,    20,    21,    22,    23,
      24,    61,    62,    13,     0,     0,    69,    33,     2,     0,
      13,     3,    53,     0,     0,    48,     0,     4,     5,     0,
       0,    13,    68,    53,     0,     0,     6,     0,     0,     0,
       0,     0,     0,    74
};

static const yytype_int8 yycheck[] =
{
       3,     5,    19,     4,     5,     6,     7,     8,    42,    27,
      11,    42,    12,    16,    15,    16,    17,    18,    38,    64,
      14,    19,    23,    16,    27,    -1,    44,    -1,    31,    25,
      -1,    32,     4,     5,     6,     7,     8,    33,    -1,    11,
      41,    44,    43,    46,    38,    -1,    -1,    51,    66,    -1,
      25,    23,    48,    56,    40,    41,    42,    -1,    -1,    -1,
      32,    -1,    65,    66,    39,    40,    -1,    -1,    -1,    41,
      11,    43,    13,    -1,    60,    61,    62,     4,     5,     6,
       7,     8,    23,    -1,    11,     4,     5,     6,     7,     8,
      -1,    11,    11,    13,    -1,    -1,    23,    25,     4,     5,
       6,     7,     8,    23,    23,    11,     4,     5,     6,     7,
       8,    39,    40,    11,    -1,    -1,    44,    23,     0,    -1,
      11,     3,    13,    -1,    -1,    23,    -1,     9,    10,    -1,
      -1,    11,    23,    13,    -1,    -1,    18,    -1,    -1,    -1,
      -1,    -1,    -1,    23
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    46,     0,     3,     9,    10,    18,    47,     4,     5,
       6,     7,     8,    11,    23,    48,    52,    55,    57,    19,
       4,     5,     6,     7,     8,    23,    50,    53,    55,    58,
      12,    51,    42,    23,    55,    57,    15,    16,    17,    18,
      32,    41,    43,    49,    53,    54,    51,    42,    23,    55,
      58,    14,    38,    13,    23,    55,    51,    54,    54,    54,
      25,    39,    40,    23,    19,    51,    53,    56,    23,    44,
      54,    54,    54,    49,    23,    38
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    45,    46,    46,    47,    47,    47,    47,    47,    48,
      48,    49,    49,    49,    49,    49,    50,    50,    51,    51,
      51,    52,    52,    52,    52,    52,    52,    53,    53,    53,
      53,    53,    53,    54,    54,    54,    54,    54,    54,    54,
      55,    55,    56,    56,    57,    57,    57,    57,    57,    58,
      58,    58,    58,    58
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     3,     3,     5,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     2,     2,
       0,     1,     1,     3,     2,     2,     4,     1,     1,     3,
       2,     2,     4,     1,     3,     3,     3,     2,     2,     3,
       3,     5,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
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
#if INI_DEBUG

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
#else /* !INI_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !INI_DEBUG */


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
          case 4: /* TC_RAW  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 5: /* TC_CONSTANT  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 6: /* TC_NUMBER  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 7: /* TC_STRING  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 8: /* TC_WHITESPACE  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 9: /* TC_LABEL  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 10: /* TC_OFFSET  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 12: /* TC_VARNAME  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 15: /* BOOL_TRUE  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 16: /* BOOL_FALSE  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 17: /* NULL_NULL  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 48: /* section_string_or_value  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 49: /* string_or_value  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 50: /* option_offset  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 51: /* encapsed_list  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 52: /* var_string_list_section  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 53: /* var_string_list  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 54: /* expr  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 55: /* cfg_var_ref  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 57: /* constant_literal  */

      { zval_ini_dtor(&((*yyvaluep))); }

        break;

    case 58: /* constant_string  */

      { zval_ini_dtor(&((*yyvaluep))); }

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
        case 3:

    { (void) ini_nerrs; }

    break;

  case 4:

    {
#if DEBUG_CFG_PARSER
			printf("SECTION: [%s]\n", Z_STRVAL((yyvsp[-1])));
#endif
			ZEND_INI_PARSER_CB(&(yyvsp[-1]), NULL, NULL, ZEND_INI_PARSER_SECTION, ZEND_INI_PARSER_ARG);
			zend_string_release(Z_STR((yyvsp[-1])));
		}

    break;

  case 5:

    {
#if DEBUG_CFG_PARSER
			printf("NORMAL: '%s' = '%s'\n", Z_STRVAL((yyvsp[-2])), Z_STRVAL((yyvsp[0])));
#endif
			ZEND_INI_PARSER_CB(&(yyvsp[-2]), &(yyvsp[0]), NULL, ZEND_INI_PARSER_ENTRY, ZEND_INI_PARSER_ARG);
			if (ZEND_SYSTEM_INI) {
				GC_MAKE_PERSISTENT_LOCAL(Z_STR((yyvsp[-2])));
			}
			zend_string_release(Z_STR((yyvsp[-2])));
			zval_ini_dtor(&(yyvsp[0]));
		}

    break;

  case 6:

    {
#if DEBUG_CFG_PARSER
			printf("OFFSET: '%s'[%s] = '%s'\n", Z_STRVAL((yyvsp[-4])), Z_STRVAL((yyvsp[-3])), Z_STRVAL((yyvsp[0])));
#endif
			ZEND_INI_PARSER_CB(&(yyvsp[-4]), &(yyvsp[0]), &(yyvsp[-3]), ZEND_INI_PARSER_POP_ENTRY, ZEND_INI_PARSER_ARG);
			zend_string_release(Z_STR((yyvsp[-4])));
			zval_ini_dtor(&(yyvsp[-3]));
			zval_ini_dtor(&(yyvsp[0]));
		}

    break;

  case 7:

    { ZEND_INI_PARSER_CB(&(yyvsp[0]), NULL, NULL, ZEND_INI_PARSER_ENTRY, ZEND_INI_PARSER_ARG); zend_string_release(Z_STR((yyvsp[0]))); }

    break;

  case 9:

    { (yyval) = (yyvsp[0]); }

    break;

  case 10:

    { zend_ini_init_string(&(yyval)); }

    break;

  case 11:

    { (yyval) = (yyvsp[0]); normalize_value(&(yyval)); }

    break;

  case 12:

    { (yyval) = (yyvsp[0]); }

    break;

  case 13:

    { (yyval) = (yyvsp[0]); }

    break;

  case 14:

    { (yyval) = (yyvsp[0]); }

    break;

  case 15:

    { zend_ini_init_string(&(yyval)); }

    break;

  case 16:

    { (yyval) = (yyvsp[0]); }

    break;

  case 17:

    { zend_ini_init_string(&(yyval)); }

    break;

  case 18:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 19:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 20:

    { zend_ini_init_string(&(yyval)); }

    break;

  case 21:

    { (yyval) = (yyvsp[0]); }

    break;

  case 22:

    { (yyval) = (yyvsp[0]); }

    break;

  case 23:

    { (yyval) = (yyvsp[-1]); }

    break;

  case 24:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 25:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 26:

    { zend_ini_add_string(&(yyval), &(yyvsp[-3]), &(yyvsp[-1])); zend_string_free(Z_STR((yyvsp[-1]))); }

    break;

  case 27:

    { (yyval) = (yyvsp[0]); }

    break;

  case 28:

    { (yyval) = (yyvsp[0]); }

    break;

  case 29:

    { (yyval) = (yyvsp[-1]); }

    break;

  case 30:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 31:

    { zend_ini_add_string(&(yyval), &(yyvsp[-1]), &(yyvsp[0])); zend_string_free(Z_STR((yyvsp[0]))); }

    break;

  case 32:

    { zend_ini_add_string(&(yyval), &(yyvsp[-3]), &(yyvsp[-1])); zend_string_free(Z_STR((yyvsp[-1]))); }

    break;

  case 33:

    { (yyval) = (yyvsp[0]); }

    break;

  case 34:

    { zend_ini_do_op('|', &(yyval), &(yyvsp[-2]), &(yyvsp[0])); }

    break;

  case 35:

    { zend_ini_do_op('&', &(yyval), &(yyvsp[-2]), &(yyvsp[0])); }

    break;

  case 36:

    { zend_ini_do_op('^', &(yyval), &(yyvsp[-2]), &(yyvsp[0])); }

    break;

  case 37:

    { zend_ini_do_op('~', &(yyval), &(yyvsp[0]), NULL); }

    break;

  case 38:

    { zend_ini_do_op('!', &(yyval), &(yyvsp[0]), NULL); }

    break;

  case 39:

    { (yyval) = (yyvsp[-1]); }

    break;

  case 40:

    { zend_ini_get_var(&(yyval), &(yyvsp[-1]), NULL); zend_string_free(Z_STR((yyvsp[-1]))); }

    break;

  case 41:

    { zend_ini_get_var(&(yyval), &(yyvsp[-3]), &(yyvsp[-1])); zend_string_free(Z_STR((yyvsp[-3]))); zend_string_free(Z_STR((yyvsp[-1]))); }

    break;

  case 42:

    { (yyval) = (yyvsp[0]); }

    break;

  case 43:

    { zend_ini_init_string(&(yyval)); }

    break;

  case 44:

    { (yyval) = (yyvsp[0]); }

    break;

  case 45:

    { (yyval) = (yyvsp[0]); /*printf("TC_RAW: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 46:

    { (yyval) = (yyvsp[0]); /*printf("TC_NUMBER: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 47:

    { (yyval) = (yyvsp[0]); /*printf("TC_STRING: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 48:

    { (yyval) = (yyvsp[0]); /*printf("TC_WHITESPACE: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 49:

    { zend_ini_get_constant(&(yyval), &(yyvsp[0])); }

    break;

  case 50:

    { (yyval) = (yyvsp[0]); /*printf("TC_RAW: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 51:

    {
			(yyval) = (yyvsp[0]);
			Z_EXTRA((yyval)) = INI_ZVAL_IS_NUMBER;
			/*printf("TC_NUMBER: '%s'\n", Z_STRVAL($1));*/
		}

    break;

  case 52:

    { (yyval) = (yyvsp[0]); /*printf("TC_STRING: '%s'\n", Z_STRVAL($1));*/ }

    break;

  case 53:

    { (yyval) = (yyvsp[0]); /*printf("TC_WHITESPACE: '%s'\n", Z_STRVAL($1));*/ }

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
