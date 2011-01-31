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
#line 1 "ql_y.y"

/* ql.y - Q.2931 data structures description language */

/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "common.h"
#include "qgen.h"
#include "file.h"


#define MAX_TOKEN 256
#define DEFAULT_NAMELIST_FILE "default.nl"


FIELD *def = NULL;
static STRUCTURE *structures = NULL;
static const char *abort_id; /* indicates abort flag */

void yyerror(const char *s);
int yylex(void);

static NAME_LIST *get_name_list(const char *name)
{
    static NAME_LIST *name_lists = NULL;
    FILE *file;
    NAME_LIST *list;
    NAME *last,*this;
    char line[MAX_TOKEN+1];
    char path[PATH_MAX+1];
    char *start,*here,*walk;
    int searching,found;

    for (list = name_lists; list; list = list->next)
	if (list->list_name == name) return list;
    sprintf(path,"%s.nl",name);
    if (!(file = fopen(path,"r")) && !(file = fopen(strcpy(path,
      DEFAULT_NAMELIST_FILE),"r"))) yyerror("can't open list file");
    list = alloc_t(NAME_LIST);
    list->list_name = name;
    list->list = last = NULL;
    list->id = -1;
    list->next = name_lists;
    name_lists = list;
    searching = 1;
    found = 0;
    while (fgets(line,MAX_TOKEN,file)) {
	for (start = line; *start && isspace(*start); start++);
	if (!*start || *start == '#') continue;
	if ((here = strchr(start,'\n'))) *here = 0;
	for (walk = strchr(start,0)-1; walk > start && isspace(*walk); walk--)
	    *walk = 0;
	if (*start == ':') {
	    if (!(searching = strcmp(start+1,name)))
	    {
		if (found) yyerror("multiple entries");
		else found = 1;
	    }
	    continue;
	}
	if (searching) continue;
	if (!(here = strchr(start,'='))) yyerror("invalid name list");
	*here++ = 0;
	for (walk = here-2; walk > start && isspace(*walk); walk--)
	    *walk = 0;
	while (*here && isspace(*here)) here++;
	this = alloc_t(NAME);
	this->value = stralloc(start);
	this->name = stralloc(here);
	this->next = NULL;
	if (last) last->next = this;
	else list->list = this;
	last = this;
    }
    (void) fclose(file);
    if (!found) yyerror("no symbol list entry found");
    return list;
}


static FIELD *copy_block(FIELD *orig_field)
{
    FIELD *copy,**new_field;

    copy = NULL;
    new_field = &copy;
    while (orig_field) {
	*new_field = alloc_t(FIELD);
	**new_field = *orig_field;
	if (orig_field->value) {
	    (*new_field)->value = alloc_t(VALUE);
	    *(*new_field)->value = *orig_field->value;
	    switch (orig_field->value->type) {
		case vt_length:
		    (*new_field)->value->block =
		      copy_block(orig_field->value->block);
		    break;
		case vt_case:
		case vt_multi:
		    {
			TAG *orig_tag,**new_tag;

			new_tag = &(*new_field)->value->tags;
			for (orig_tag = orig_field->value->tags; orig_tag;
			  orig_tag = orig_tag->next) {
			    VALUE_LIST *orig_value,**new_value;

			    *new_tag = alloc_t(TAG);
			    **new_tag = *orig_tag;
			    new_value = &(*new_tag)->more;
			    for (orig_value = orig_tag->more; orig_value;
			      orig_value = orig_value->next) {
				*new_value = alloc_t(VALUE_LIST);
				**new_value = *orig_value;
				new_value = &(*new_value)->next;
			    }
			    (*new_tag)->block = copy_block(orig_tag->block);
			    new_tag = &(*new_tag)->next;
			}
		    }
	    }
	}
	if (orig_field->structure)
	    yyerror("sorry, can't handle nested structures");
	new_field = &(*new_field)->next;
	orig_field = orig_field->next;
    }
    return copy;
}




/* Line 189 of yacc.c  */
#line 211 "ql_y.c"

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
     TOK_BREAK = 258,
     TOK_CASE = 259,
     TOK_DEF = 260,
     TOK_DEFAULT = 261,
     TOK_LENGTH = 262,
     TOK_MULTI = 263,
     TOK_RECOVER = 264,
     TOK_ABORT = 265,
     TOK_ID = 266,
     TOK_INCLUDE = 267,
     TOK_STRING = 268
   };
#endif
/* Tokens.  */
#define TOK_BREAK 258
#define TOK_CASE 259
#define TOK_DEF 260
#define TOK_DEFAULT 261
#define TOK_LENGTH 262
#define TOK_MULTI 263
#define TOK_RECOVER 264
#define TOK_ABORT 265
#define TOK_ID 266
#define TOK_INCLUDE 267
#define TOK_STRING 268




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 139 "ql_y.y"

    const char *str;
    int num;
    FIELD *field;
    VALUE *value;
    VALUE_LIST *list;
    TAG *tag;
    NAME_LIST *nlist;



/* Line 214 of yacc.c  */
#line 285 "ql_y.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 297 "ql_y.c"

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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   65

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  23
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  46
/* YYNRULES -- Number of states.  */
#define YYNSTATES  86

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   268

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    21,    18,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    22,     2,
      17,    14,    19,     2,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,     2,    16,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     7,     8,    11,    12,    15,    20,    21,
      24,    26,    30,    33,    34,    37,    43,    44,    46,    50,
      56,    57,    60,    62,    63,    66,    67,    70,    72,    77,
      82,    86,    87,    90,    91,    93,    94,   100,   101,   108,
     109,   115,   116,   123,   124,   127,   128
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      24,     0,    -1,    25,    26,    30,    -1,    -1,    12,    25,
      -1,    -1,    26,    27,    -1,     5,    11,    14,    30,    -1,
      -1,    29,    30,    -1,    11,    -1,    15,    31,    16,    -1,
      10,    11,    -1,    -1,    32,    31,    -1,    33,    11,    41,
      17,    34,    -1,    -1,     3,    -1,    18,    36,    19,    -1,
      36,    35,    37,    19,    38,    -1,    -1,    20,    36,    -1,
      11,    -1,    -1,    21,    11,    -1,    -1,    14,    39,    -1,
      11,    -1,     4,    15,    42,    16,    -1,     8,    15,    44,
      16,    -1,    40,     7,    30,    -1,    -1,     9,    11,    -1,
      -1,    13,    -1,    -1,     6,    11,    46,    47,    30,    -1,
      -1,    11,    46,    47,    30,    43,    42,    -1,    -1,     6,
      11,    46,    47,    28,    -1,    -1,    11,    46,    47,    28,
      45,    44,    -1,    -1,    22,    11,    -1,    -1,    21,    11,
      47,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   164,   164,   175,   176,   184,   185,   189,   203,   203,
     213,   231,   236,   244,   247,   255,   279,   282,   289,   301,
     321,   324,   332,   342,   345,   353,   356,   363,   369,   376,
     382,   393,   396,   403,   406,   413,   416,   434,   433,   457,
     460,   477,   476,   500,   503,   510,   513
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_BREAK", "TOK_CASE", "TOK_DEF",
  "TOK_DEFAULT", "TOK_LENGTH", "TOK_MULTI", "TOK_RECOVER", "TOK_ABORT",
  "TOK_ID", "TOK_INCLUDE", "TOK_STRING", "'='", "'{'", "'}'", "'<'", "'-'",
  "'>'", "'@'", "','", "':'", "$accept", "all", "includes", "structures",
  "structure", "rep_block", "$@1", "block", "fields", "field", "opt_break",
  "field_cont", "opt_pos", "decimal", "opt_more", "opt_val", "value",
  "opt_recover", "opt_name_list", "tags", "@2", "rep_tags", "@3", "opt_id",
  "list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    61,   123,   125,    60,    45,    62,
      64,    44,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    23,    24,    25,    25,    26,    26,    27,    29,    28,
      30,    30,    30,    31,    31,    32,    33,    33,    34,    34,
      35,    35,    36,    37,    37,    38,    38,    39,    39,    39,
      39,    40,    40,    41,    41,    42,    42,    43,    42,    44,
      44,    45,    44,    46,    46,    47,    47
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     0,     2,     0,     2,     4,     0,     2,
       1,     3,     2,     0,     2,     5,     0,     1,     3,     5,
       0,     2,     1,     0,     2,     0,     2,     1,     4,     4,
       3,     0,     2,     0,     1,     0,     5,     0,     6,     0,
       5,     0,     6,     0,     2,     0,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     3,     0,     5,     4,     1,     0,     0,     0,    10,
      13,     6,     2,     0,    12,    17,     0,    13,     0,     0,
      11,    14,    33,     7,    34,     0,     0,    22,     0,    15,
      20,     0,     0,    23,    18,    21,     0,     0,    24,    25,
      31,    19,     0,     0,     0,    27,    26,     0,    35,    39,
      32,     0,     0,    43,     0,     0,    43,     0,    30,    43,
       0,    45,    28,    43,    45,    29,    45,    44,     0,     0,
      45,     8,     0,    45,    37,     8,    41,     0,    36,    46,
      35,    40,    39,     9,    38,    42
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     2,     3,     6,    11,    76,    77,    12,    16,    17,
      18,    29,    33,    30,    37,    41,    46,    47,    25,    54,
      80,    57,    82,    61,    69
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -62
static const yytype_int8 yypact[] =
{
      -8,    -8,    14,   -62,   -62,   -62,    -4,    16,    17,   -62,
      -1,   -62,   -62,    20,   -62,   -62,    21,    -1,    22,    11,
     -62,   -62,    23,   -62,   -62,    24,    -3,   -62,    27,   -62,
      25,    28,    27,    30,   -62,   -62,    29,    33,   -62,    32,
       9,   -62,    34,    39,    31,   -62,   -62,    36,    18,    19,
     -62,    11,    37,    35,    40,    44,    35,    43,   -62,    35,
      49,    41,   -62,    35,    41,   -62,    41,   -62,    50,    11,
      41,   -62,    11,    41,   -62,   -62,   -62,    11,   -62,   -62,
      18,   -62,    19,   -62,   -62,   -62
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -62,   -62,    38,   -62,   -62,   -31,   -62,   -19,    46,   -62,
     -62,   -62,   -62,     3,   -62,   -62,   -62,   -62,   -62,   -16,
     -62,   -17,   -62,   -40,   -61
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -17
static const yytype_int8 yytable[] =
{
      23,     7,    15,    71,     1,    72,     8,     9,    27,    75,
     -16,    10,    79,    42,     5,    28,    64,    43,    44,    66,
      45,     8,     9,    70,    52,    55,    10,    13,    14,    53,
      56,    31,    58,    22,    19,    35,    24,    20,    27,     4,
      38,    26,    50,    51,    81,    32,    40,    34,    59,    48,
      74,    36,    39,    78,    49,    63,    62,    60,    83,    65,
      67,    73,    68,    21,    84,    85
};

static const yytype_uint8 yycheck[] =
{
      19,     5,     3,    64,    12,    66,    10,    11,    11,    70,
      11,    15,    73,     4,     0,    18,    56,     8,     9,    59,
      11,    10,    11,    63,     6,     6,    15,    11,    11,    11,
      11,    28,    51,    11,    14,    32,    13,    16,    11,     1,
      11,    17,    11,     7,    75,    20,    14,    19,    11,    15,
      69,    21,    19,    72,    15,    11,    16,    22,    77,    16,
      11,    11,    21,    17,    80,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    12,    24,    25,    25,     0,    26,     5,    10,    11,
      15,    27,    30,    11,    11,     3,    31,    32,    33,    14,
      16,    31,    11,    30,    13,    41,    17,    11,    18,    34,
      36,    36,    20,    35,    19,    36,    21,    37,    11,    19,
      14,    38,     4,     8,     9,    11,    39,    40,    15,    15,
      11,     7,     6,    11,    42,     6,    11,    44,    30,    11,
      22,    46,    16,    11,    46,    16,    46,    11,    21,    47,
      46,    47,    47,    11,    30,    47,    28,    29,    30,    47,
      43,    28,    45,    30,    42,    44
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
        case 2:

/* Line 1464 of yacc.c  */
#line 165 "ql_y.y"
    {
	    STRUCTURE *walk;

	    def = (yyvsp[(3) - (3)].field);
	    for (walk = structures; walk; walk = walk->next)
		if (!walk->instances)
		    fprintf(stderr,"unused structure: %s\n",walk->id);
	}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 177 "ql_y.y"
    {
	    to_c("#%s\n",(yyvsp[(1) - (2)].str));
	    to_test("#%s\n",(yyvsp[(1) - (2)].str));
	    if (dump) to_dump("#%s\n",(yyvsp[(1) - (2)].str));
	}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 190 "ql_y.y"
    {
	    STRUCTURE *n;

	    n = alloc_t(STRUCTURE);
	    n->id = (yyvsp[(2) - (4)].str);
	    n->block = (yyvsp[(4) - (4)].field);
	    n->instances = 0;
	    n->next = structures;
	    structures = n;
	}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 203 "ql_y.y"
    {
	    abort_id = NULL;
	}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 207 "ql_y.y"
    {
	    (yyval.field) = (yyvsp[(2) - (2)].field);
	}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 214 "ql_y.y"
    {
	    STRUCTURE *walk;

	    for (walk = structures; walk; walk = walk->next)
		if (walk->id == (yyvsp[(1) - (1)].str)) break;
	    if (!walk) yyerror("no such structure");
	    walk->instances++;
	    (yyval.field) = alloc_t(FIELD);
	    (yyval.field)->id = NULL;
	    (yyval.field)->name_list = NULL;
	    (yyval.field)->value = NULL;
	    (yyval.field)->brk = 0;
	    (yyval.field)->structure = walk;
	    (yyval.field)->my_block = copy_block(walk->block);
	    (yyval.field)->next = NULL;
	    abort_id = NULL;
	}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 232 "ql_y.y"
    {
	    (yyval.field) = (yyvsp[(2) - (3)].field);
	    abort_id = NULL;
	}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 237 "ql_y.y"
    {
	    (yyval.field) = NULL;
	    abort_id = (yyvsp[(2) - (2)].str);
	}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 244 "ql_y.y"
    {
	    (yyval.field) = NULL;
	}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 248 "ql_y.y"
    {
	    (yyval.field) = (yyvsp[(1) - (2)].field);
	    (yyvsp[(1) - (2)].field)->next = (yyvsp[(2) - (2)].field);
	}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 256 "ql_y.y"
    {
	    TAG *walk;

	    (yyval.field) = (yyvsp[(5) - (5)].field);
	    (yyval.field)->name_list = (yyvsp[(3) - (5)].nlist);
	    (yyval.field)->brk = (yyvsp[(1) - (5)].num);
	    (yyval.field)->id = (yyvsp[(2) - (5)].str);
	    if ((yyval.field)->var_len == -2) {
		if (*(yyval.field)->id == '_') yyerror("var-len field must be named");
	    }
	    else if (*(yyval.field)->id == '_' && !(yyval.field)->value)
		    yyerror("unnamed fields must have value");
	    if (*(yyval.field)->id == '_' && (yyval.field)->value && (yyval.field)->value->type == vt_case)
		for (walk = (yyval.field)->value->tags; walk; walk = walk->next)
		    if (walk->more)
			yyerror("value list only allowed in named case "
			  "selections");
	    if (*(yyval.field)->id != '_' && (yyval.field)->value && (yyval.field)->value->type == vt_multi)
		yyerror("multi selectors must be unnamed");
	}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 279 "ql_y.y"
    {
	    (yyval.num) = 0;
	}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 283 "ql_y.y"
    {
	    (yyval.num) = 1;
	}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 290 "ql_y.y"
    {
	    (yyval.field) = alloc_t(FIELD);
	    (yyval.field)->size = (yyvsp[(2) - (3)].num);
	    (yyval.field)->var_len = -2; /* hack */
	    if ((yyvsp[(2) - (3)].num) & 7) yyerror("var-len field must have integral size");
	    (yyval.field)->pos = 0;
	    (yyval.field)->flush = 1;
	    (yyval.field)->value = NULL;
	    (yyval.field)->structure = NULL;
	    (yyval.field)->next = NULL;
	}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 302 "ql_y.y"
    {
	    (yyval.field) = alloc_t(FIELD);
	    (yyval.field)->size = (yyvsp[(1) - (5)].num);
	    (yyval.field)->var_len = -1;
	    (yyval.field)->pos = (yyvsp[(2) - (5)].num);
	    (yyval.field)->flush = !(yyvsp[(3) - (5)].num);
	    if ((yyval.field)->pos == -1)
	    {
		if ((yyval.field)->size & 7)
		    yyerror("position required for small fields");
		else (yyval.field)->pos = 0;
	    }
	    (yyval.field)->value = (yyvsp[(5) - (5)].value);
	    (yyval.field)->structure = NULL;
	    (yyval.field)->next = NULL;
	}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 321 "ql_y.y"
    {
	    (yyval.num) = -1;
	}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 325 "ql_y.y"
    {
	    (yyval.num) = (yyvsp[(2) - (2)].num)-1;
	    if ((yyval.num) < 0 || (yyval.num) > 7) yyerror("invalid position");
	}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 333 "ql_y.y"
    {
	    char *end;

	    (yyval.num) = strtoul((yyvsp[(1) - (1)].str),&end,10);
	    if (*end) yyerror("no a decimal number");
	}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 342 "ql_y.y"
    {
	    (yyval.num) = 0;
	}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 346 "ql_y.y"
    {
	    if (strcmp((yyvsp[(2) - (2)].str),"more")) yyerror("\"more\" expected");
	    (yyval.num) = 1;
	}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 353 "ql_y.y"
    {
	    (yyval.value) = NULL;
	}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 357 "ql_y.y"
    {
	    (yyval.value) = (yyvsp[(2) - (2)].value);
	}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 364 "ql_y.y"
    {
	    (yyval.value) = alloc_t(VALUE);
	    (yyval.value)->type = vt_id;
	    (yyval.value)->id = (yyvsp[(1) - (1)].str);
	}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 370 "ql_y.y"
    {
	    (yyval.value) = alloc_t(VALUE);
	    (yyval.value)->type = vt_case;
	    (yyval.value)->id = NULL;
	    (yyval.value)->tags = (yyvsp[(3) - (4)].tag);
	}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 377 "ql_y.y"
    {
	    (yyval.value) = alloc_t(VALUE);
	    (yyval.value)->type = vt_multi;
	    (yyval.value)->tags = (yyvsp[(3) - (4)].tag);
	}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 383 "ql_y.y"
    {
	    (yyval.value) = alloc_t(VALUE);
	    (yyval.value)->type = vt_length;
	    (yyval.value)->recovery = (yyvsp[(1) - (3)].str);
	    (yyval.value)->block = (yyvsp[(3) - (3)].field);
	    (yyval.value)->abort_id = abort_id;
	}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 393 "ql_y.y"
    {
	    (yyval.str) = NULL;
	}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 397 "ql_y.y"
    {
	    (yyval.str) = (yyvsp[(2) - (2)].str);
	}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 403 "ql_y.y"
    {
	    (yyval.nlist) = NULL;
	}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 407 "ql_y.y"
    {
	    (yyval.nlist) = get_name_list((yyvsp[(1) - (1)].str));
	}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 413 "ql_y.y"
    {
	    (yyval.tag) = NULL;
	}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 417 "ql_y.y"
    {
	    (yyval.tag) = alloc_t(TAG);
	    (yyval.tag)->deflt = 1;
	    if ((yyvsp[(3) - (5)].str)) {
		(yyval.tag)->id = (yyvsp[(2) - (5)].str);
		(yyval.tag)->value = (yyvsp[(3) - (5)].str);
	    }
	    else {
		(yyval.tag)->id = NULL;
		(yyval.tag)->value = (yyvsp[(2) - (5)].str);
	    }
	    (yyval.tag)->more = (yyvsp[(4) - (5)].list);
	    (yyval.tag)->block = (yyvsp[(5) - (5)].field);
	    (yyval.tag)->next = NULL;
	    (yyval.tag)->abort_id = abort_id;
	}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 434 "ql_y.y"
    {
	    (yyval.tag) = alloc_t(TAG);
	    (yyval.tag)->abort_id = abort_id;
	}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 439 "ql_y.y"
    {
	    (yyval.tag) = (yyvsp[(5) - (6)].tag);
	    (yyval.tag)->deflt = 0;
	    if ((yyvsp[(2) - (6)].str)) {
		(yyval.tag)->id = (yyvsp[(1) - (6)].str);
		(yyval.tag)->value = (yyvsp[(2) - (6)].str);
	    }
	    else {
		(yyval.tag)->id = NULL;
		(yyval.tag)->value = (yyvsp[(1) - (6)].str);
	    }
	    (yyval.tag)->more = (yyvsp[(3) - (6)].list);
	    (yyval.tag)->block = (yyvsp[(4) - (6)].field);
	    (yyval.tag)->next = (yyvsp[(6) - (6)].tag);
	}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 457 "ql_y.y"
    {
	    (yyval.tag) = NULL;
	}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 461 "ql_y.y"
    {
	    (yyval.tag) = alloc_t(TAG);
	    (yyval.tag)->deflt = 1;
	    if ((yyvsp[(3) - (5)].str)) {
		(yyval.tag)->id = (yyvsp[(2) - (5)].str);
		(yyval.tag)->value = (yyvsp[(3) - (5)].str);
	    }
	    else {
		(yyval.tag)->id = NULL;
		(yyval.tag)->value = (yyvsp[(2) - (5)].str);
	    }
	    (yyval.tag)->more = (yyvsp[(4) - (5)].list);
	    (yyval.tag)->block = (yyvsp[(5) - (5)].field);
	    (yyval.tag)->next = NULL;
	}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 477 "ql_y.y"
    {
	    (yyval.tag) = alloc_t(TAG);
	    (yyval.tag)->abort_id = abort_id;
	}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 482 "ql_y.y"
    {
	    (yyval.tag) = (yyvsp[(5) - (6)].tag);
	    (yyval.tag)->deflt = 0;
	    if ((yyvsp[(2) - (6)].str)) {
		(yyval.tag)->id = (yyvsp[(1) - (6)].str);
		(yyval.tag)->value = (yyvsp[(2) - (6)].str);
	    }
	    else {
		(yyval.tag)->id = NULL;
		(yyval.tag)->value = (yyvsp[(1) - (6)].str);
	    }
	    (yyval.tag)->more = (yyvsp[(3) - (6)].list);
	    (yyval.tag)->block = (yyvsp[(4) - (6)].field);
	    (yyval.tag)->next = (yyvsp[(6) - (6)].tag);
	}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 500 "ql_y.y"
    {
	    (yyval.str) = NULL;
	}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 504 "ql_y.y"
    {
	    (yyval.str) = (yyvsp[(2) - (2)].str);
	}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 510 "ql_y.y"
    {
	    (yyval.list) = NULL;
	}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 514 "ql_y.y"
    {
	    (yyval.list) = alloc_t(VALUE_LIST);
	    (yyval.list)->value = (yyvsp[(2) - (3)].str);
	    (yyval.list)->next = (yyvsp[(3) - (3)].list);
	}
    break;



/* Line 1464 of yacc.c  */
#line 2077 "ql_y.c"
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



