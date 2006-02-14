/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_OPEN = 258,
     TOK_CLOSE = 259,
     TOK_SEMI = 260,
     TOK_STRING = 261,
     TOK_INTEGER = 262,
     TOK_FLOAT = 263,
     TOK_BOOLEAN = 264,
     TOK_IP6TYPE = 265,
     TOK_DEBUGLEVEL = 266,
     TOK_IPVERSION = 267,
     TOK_HNA4 = 268,
     TOK_HNA6 = 269,
     TOK_PLUGIN = 270,
     TOK_INTERFACE = 271,
     TOK_NOINT = 272,
     TOK_TOS = 273,
     TOK_WILLINGNESS = 274,
     TOK_IPCCON = 275,
     TOK_USEHYST = 276,
     TOK_HYSTSCALE = 277,
     TOK_HYSTUPPER = 278,
     TOK_HYSTLOWER = 279,
     TOK_POLLRATE = 280,
     TOK_TCREDUNDANCY = 281,
     TOK_MPRCOVERAGE = 282,
     TOK_LQ_LEVEL = 283,
     TOK_LQ_FISH = 284,
     TOK_LQ_DLIMIT = 285,
     TOK_LQ_WSIZE = 286,
     TOK_LQ_MULT = 287,
     TOK_CLEAR_SCREEN = 288,
     TOK_PLNAME = 289,
     TOK_PLPARAM = 290,
     TOK_HOSTLABEL = 291,
     TOK_NETLABEL = 292,
     TOK_MAXIPC = 293,
     TOK_IP4BROADCAST = 294,
     TOK_IP6ADDRTYPE = 295,
     TOK_IP6MULTISITE = 296,
     TOK_IP6MULTIGLOBAL = 297,
     TOK_IFWEIGHT = 298,
     TOK_HELLOINT = 299,
     TOK_HELLOVAL = 300,
     TOK_TCINT = 301,
     TOK_TCVAL = 302,
     TOK_MIDINT = 303,
     TOK_MIDVAL = 304,
     TOK_HNAINT = 305,
     TOK_HNAVAL = 306,
     TOK_IP4_ADDR = 307,
     TOK_IP6_ADDR = 308,
     TOK_DEFAULT = 309,
     TOK_COMMENT = 310
   };
#endif
/* Tokens.  */
#define TOK_OPEN 258
#define TOK_CLOSE 259
#define TOK_SEMI 260
#define TOK_STRING 261
#define TOK_INTEGER 262
#define TOK_FLOAT 263
#define TOK_BOOLEAN 264
#define TOK_IP6TYPE 265
#define TOK_DEBUGLEVEL 266
#define TOK_IPVERSION 267
#define TOK_HNA4 268
#define TOK_HNA6 269
#define TOK_PLUGIN 270
#define TOK_INTERFACE 271
#define TOK_NOINT 272
#define TOK_TOS 273
#define TOK_WILLINGNESS 274
#define TOK_IPCCON 275
#define TOK_USEHYST 276
#define TOK_HYSTSCALE 277
#define TOK_HYSTUPPER 278
#define TOK_HYSTLOWER 279
#define TOK_POLLRATE 280
#define TOK_TCREDUNDANCY 281
#define TOK_MPRCOVERAGE 282
#define TOK_LQ_LEVEL 283
#define TOK_LQ_FISH 284
#define TOK_LQ_DLIMIT 285
#define TOK_LQ_WSIZE 286
#define TOK_LQ_MULT 287
#define TOK_CLEAR_SCREEN 288
#define TOK_PLNAME 289
#define TOK_PLPARAM 290
#define TOK_HOSTLABEL 291
#define TOK_NETLABEL 292
#define TOK_MAXIPC 293
#define TOK_IP4BROADCAST 294
#define TOK_IP6ADDRTYPE 295
#define TOK_IP6MULTISITE 296
#define TOK_IP6MULTIGLOBAL 297
#define TOK_IFWEIGHT 298
#define TOK_HELLOINT 299
#define TOK_HELLOVAL 300
#define TOK_TCINT 301
#define TOK_TCVAL 302
#define TOK_MIDINT 303
#define TOK_MIDVAL 304
#define TOK_HNAINT 305
#define TOK_HNAVAL 306
#define TOK_IP4_ADDR 307
#define TOK_IP6_ADDR 308
#define TOK_DEFAULT 309
#define TOK_COMMENT 310




/* Copy the first part of user declarations.  */
#line 1 "oparse.y"


/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: oparse.y,v 1.28 2005/11/17 04:25:44 tlopatic Exp $
 */


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "olsrd_conf.h"

#define PARSER_DEBUG 0

#define YYSTYPE struct conf_token *

void yyerror(char *);
int yylex(void);

static int ifs_in_curr_cfg = 0;

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg);

static int lq_mult_helper(YYSTYPE ip_addr_arg, YYSTYPE mult_arg)
{
  union olsr_ip_addr addr;
  int i;
  struct olsr_if *walker;
  struct olsr_lq_mult *mult;

#if PARSER_DEBUG > 0
  printf("\tLinkQualityMult %s %0.2f\n",
         (ip_addr_arg != NULL) ? ip_addr_arg->string : "any",
         mult_arg->floating);
#endif

  memset(&addr, 0, sizeof (union olsr_ip_addr));

  if(ip_addr_arg != NULL &&
     inet_pton(cnf->ip_version, ip_addr_arg->string, &addr) < 0)
  {
    fprintf(stderr, "Cannot parse IP address %s.\n", ip_addr_arg->string);
    return -1;
  }

  walker = cnf->interfaces;

  for (i = 0; i < ifs_in_curr_cfg; i++)
  {
    mult = malloc(sizeof (struct olsr_lq_mult));

    if (mult == NULL)
    {
      fprintf(stderr, "Out of memory (LQ multiplier).\n");
      return -1;
    }

    memcpy(&mult->addr, &addr, sizeof (union olsr_ip_addr));
    mult->val = mult_arg->floating;

    mult->next = walker->cnf->lq_mult;
    walker->cnf->lq_mult = mult;

    walker = walker->next;
  }

  if (ip_addr_arg != NULL)
  {
    free(ip_addr_arg->string);
    free(ip_addr_arg);
  }

  free(mult_arg);

  return 0;
}


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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 327 "oparse.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   111

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  62
/* YYNRULES -- Number of rules. */
#define YYNRULES  113
/* YYNRULES -- Number of states. */
#define YYNSTATES  168

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   310

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
      55
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    49,    52,    55,    58,    61,    65,
      66,    69,    71,    73,    77,    78,    81,    83,    85,    89,
      90,    93,    95,    97,    99,   101,   104,   105,   108,   112,
     113,   116,   118,   120,   122,   124,   126,   128,   130,   132,
     134,   136,   138,   140,   142,   144,   146,   150,   151,   154,
     156,   158,   161,   164,   168,   171,   174,   177,   180,   183,
     186,   189,   192,   195,   198,   201,   204,   207,   211,   215,
     219,   222,   225,   228,   231,   233,   235,   238,   241,   244,
     247,   250,   253,   256,   259,   262,   265,   268,   271,   275,
     278,   281,   284,   288
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      57,     0,    -1,    -1,    57,    59,    -1,    57,    58,    -1,
      94,    -1,    95,    -1,   100,    -1,   101,    -1,   102,    -1,
     103,    -1,   104,    -1,   105,    -1,   106,    -1,   107,    -1,
     108,    -1,   109,    -1,   110,    -1,   111,    -1,   112,    -1,
     113,    -1,   114,    -1,   117,    -1,    13,    60,    -1,    14,
      63,    -1,    20,    66,    -1,    69,    71,    -1,   115,    74,
      -1,     3,    61,     4,    -1,    -1,    61,    62,    -1,   117,
      -1,    96,    -1,     3,    64,     4,    -1,    -1,    64,    65,
      -1,   117,    -1,    97,    -1,     3,    67,     4,    -1,    -1,
      67,    68,    -1,   117,    -1,    77,    -1,    78,    -1,    79,
      -1,    98,    70,    -1,    -1,    70,    99,    -1,     3,    72,
       4,    -1,    -1,    72,    73,    -1,   117,    -1,    80,    -1,
      81,    -1,    82,    -1,    83,    -1,    84,    -1,    85,    -1,
      86,    -1,    87,    -1,    88,    -1,    89,    -1,    90,    -1,
      91,    -1,    92,    -1,    93,    -1,     3,    75,     4,    -1,
      -1,    75,    76,    -1,   116,    -1,   117,    -1,    38,     7,
      -1,    36,    52,    -1,    37,    52,    52,    -1,    43,     7,
      -1,    39,    52,    -1,    40,    10,    -1,    41,    53,    -1,
      42,    53,    -1,    44,     8,    -1,    45,     8,    -1,    46,
       8,    -1,    47,     8,    -1,    48,     8,    -1,    49,     8,
      -1,    50,     8,    -1,    51,     8,    -1,    32,    54,     8,
      -1,    32,    52,     8,    -1,    32,    53,     8,    -1,    11,
       7,    -1,    12,     7,    -1,    52,    52,    -1,    53,     7,
      -1,    16,    -1,     6,    -1,    17,     9,    -1,    18,     7,
      -1,    19,     7,    -1,    21,     9,    -1,    22,     8,    -1,
      23,     8,    -1,    24,     8,    -1,    25,     8,    -1,    26,
       7,    -1,    27,     7,    -1,    28,     7,    -1,    29,     7,
      -1,    30,     7,     8,    -1,    31,     7,    -1,    33,     9,
      -1,    15,     6,    -1,    35,     6,     6,    -1,    55,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   185,   185,   186,   187,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   210,   211,   212,   213,   214,   217,   220,
     220,   223,   224,   227,   230,   230,   233,   234,   237,   240,
     240,   243,   244,   245,   246,   249,   252,   252,   255,   258,
     258,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   278,   281,   281,   284,
     285,   289,   299,   324,   358,   378,   405,   435,   464,   491,
     509,   527,   544,   561,   579,   596,   613,   631,   637,   643,
     651,   661,   679,   716,   754,   761,   791,   801,   811,   825,
     842,   851,   860,   868,   878,   886,   894,   902,   910,   919,
     927,   938,   962,   986
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_OPEN", "TOK_CLOSE", "TOK_SEMI",
  "TOK_STRING", "TOK_INTEGER", "TOK_FLOAT", "TOK_BOOLEAN", "TOK_IP6TYPE",
  "TOK_DEBUGLEVEL", "TOK_IPVERSION", "TOK_HNA4", "TOK_HNA6", "TOK_PLUGIN",
  "TOK_INTERFACE", "TOK_NOINT", "TOK_TOS", "TOK_WILLINGNESS", "TOK_IPCCON",
  "TOK_USEHYST", "TOK_HYSTSCALE", "TOK_HYSTUPPER", "TOK_HYSTLOWER",
  "TOK_POLLRATE", "TOK_TCREDUNDANCY", "TOK_MPRCOVERAGE", "TOK_LQ_LEVEL",
  "TOK_LQ_FISH", "TOK_LQ_DLIMIT", "TOK_LQ_WSIZE", "TOK_LQ_MULT",
  "TOK_CLEAR_SCREEN", "TOK_PLNAME", "TOK_PLPARAM", "TOK_HOSTLABEL",
  "TOK_NETLABEL", "TOK_MAXIPC", "TOK_IP4BROADCAST", "TOK_IP6ADDRTYPE",
  "TOK_IP6MULTISITE", "TOK_IP6MULTIGLOBAL", "TOK_IFWEIGHT", "TOK_HELLOINT",
  "TOK_HELLOVAL", "TOK_TCINT", "TOK_TCVAL", "TOK_MIDINT", "TOK_MIDVAL",
  "TOK_HNAINT", "TOK_HNAVAL", "TOK_IP4_ADDR", "TOK_IP6_ADDR",
  "TOK_DEFAULT", "TOK_COMMENT", "$accept", "conf", "stmt", "block",
  "hna4body", "hna4stmts", "hna4stmt", "hna6body", "hna6stmts", "hna6stmt",
  "ipcbody", "ipcstmts", "ipcstmt", "ifblock", "ifnicks", "ifbody",
  "ifstmts", "ifstmt", "plbody", "plstmts", "plstmt", "imaxipc", "ipchost",
  "ipcnet", "iifweight", "isetip4br", "isetip6addrt", "isetip6mults",
  "isetip6multg", "isethelloint", "isethelloval", "isettcint", "isettcval",
  "isetmidint", "isetmidval", "isethnaint", "isethnaval", "isetlqmult",
  "idebug", "iipversion", "ihna4entry", "ihna6entry", "ifstart", "ifnick",
  "bnoint", "atos", "awillingness", "busehyst", "fhystscale", "fhystupper",
  "fhystlower", "fpollrate", "atcredundancy", "amprcoverage", "alq_level",
  "alq_fish", "alq_dlimit", "alq_wsize", "bclear_screen", "plblock",
  "plparam", "vcomment", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    56,    57,    57,    57,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    59,    59,    59,    59,    59,    60,    61,
      61,    62,    62,    63,    64,    64,    65,    65,    66,    67,
      67,    68,    68,    68,    68,    69,    70,    70,    71,    72,
      72,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    74,    75,    75,    76,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    93,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     3,     0,
       2,     1,     1,     3,     0,     2,     1,     1,     3,     0,
       2,     1,     1,     1,     1,     2,     0,     2,     3,     0,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     0,     2,     1,
       1,     2,     2,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     3,     3,
       2,     2,     2,     2,     1,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     2,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   113,     4,     3,     0,     5,
       6,    46,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,     0,    22,    90,
      91,    29,    23,    34,    24,   111,    96,    97,    98,    39,
      25,    99,   100,   101,   102,   103,   104,   105,   106,   107,
       0,   109,   110,    49,    26,    45,    67,    27,     0,     0,
       0,   108,     0,    95,    47,     0,    28,     0,    30,    32,
      31,    33,     0,    35,    37,    36,    38,     0,     0,     0,
      40,    42,    43,    44,    41,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    51,    66,     0,    68,    69,
      70,    92,    93,    72,     0,    71,     0,     0,     0,    75,
      76,    77,    78,    74,    79,    80,    81,    82,    83,    84,
      85,    86,     0,    73,    88,    89,    87,   112
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,    26,    27,    52,    78,    88,    54,    79,    93,
      60,    80,   100,    28,    75,    74,    82,   120,    77,    85,
     138,   101,   102,   103,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,    29,    30,
      89,    94,    31,    84,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     139,    48
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -15
static const yysigned_char yypact[] =
{
     -15,     0,   -15,    -6,    -5,     1,     7,    32,   -15,    -4,
      33,    54,    36,    -2,    59,    61,    62,    67,    55,    69,
      70,    71,    72,    73,    75,   -15,   -15,   -15,    78,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,    79,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
      77,   -15,   -15,   -15,   -15,    80,   -15,   -15,     4,     5,
      -1,   -15,     2,   -15,   -15,    28,   -15,    35,   -15,   -15,
     -15,   -15,    81,   -15,   -15,   -15,   -15,    37,    38,    84,
     -15,   -15,   -15,   -15,   -15,   -15,    20,    40,    83,    41,
      42,    89,    90,    91,    92,    93,    94,    95,    96,    97,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   100,   -15,   -15,
     -15,   -15,   -15,   -15,    45,   -15,    99,   101,   102,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   105,   -15,   -15,   -15,   -15,   -15
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,   -15,
     -15,   -14
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       2,    49,    50,    96,    51,    56,   105,    61,    86,    91,
      53,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,   136,    24,   106,    97,    98,    99,    55,    59,
      57,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    25,    25,    87,    25,    92,    25,
      25,    58,    66,   137,    90,    95,   104,    62,   135,    63,
      64,   140,   146,   147,   148,    65,    67,    68,    69,    70,
      71,    73,    76,    25,    72,    81,    83,   141,   142,   143,
     144,   145,   149,   150,   151,   152,   153,   163,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   164,     0,   165,
     166,   167
};

static const yysigned_char yycheck[] =
{
       0,     7,     7,     4,     3,     9,     4,     9,     4,     4,
       3,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,     4,    33,    32,    36,    37,    38,     6,     3,
       7,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    55,    55,    52,    55,    53,    55,
      55,     7,     7,    35,    78,    79,    80,     8,    82,     8,
       8,    85,    52,    53,    54,     8,     7,     7,     7,     7,
       7,     3,     3,    55,     9,     8,     6,    52,     7,    52,
      52,     7,    52,    10,    53,    53,     7,    52,     8,     8,
       8,     8,     8,     8,     8,     8,     6,     8,    -1,     8,
       8,     6
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    57,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    33,    55,    58,    59,    69,    94,
      95,    98,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   117,     7,
       7,     3,    60,     3,    63,     6,     9,     7,     7,     3,
      66,     9,     8,     8,     8,     8,     7,     7,     7,     7,
       7,     7,     9,     3,    71,    70,     3,    74,    61,    64,
      67,     8,    72,     6,    99,    75,     4,    52,    62,    96,
     117,     4,    53,    65,    97,   117,     4,    36,    37,    38,
      68,    77,    78,    79,   117,     4,    32,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      73,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,   117,     4,    35,    76,   116,
     117,    52,     7,    52,    52,     7,    52,    53,    54,    52,
      10,    53,    53,     7,     8,     8,     8,     8,     8,     8,
       8,     8,     6,    52,     8,     8,     8,     6
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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
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
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
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
      size_t yyn = 0;
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

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
        case 71:
#line 290 "oparse.y"
    {
  cnf->ipc_connections = (yyvsp[0])->integer;

  cnf->open_ipc = cnf->ipc_connections ? OLSR_TRUE : OLSR_FALSE;

  free((yyvsp[0]));
;}
    break;

  case 72:
#line 300 "oparse.y"
    {
  struct in_addr in;
  struct ipc_host *ipch;

  if(PARSER_DEBUG) printf("\tIPC host: %s\n", (yyvsp[0])->string);
  
  if(inet_aton((yyvsp[0])->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address IPC %s\n", (yyvsp[0])->string);
      return -1;
    }

  ipch = malloc(sizeof(struct ipc_host));
  ipch->host.v4 = in.s_addr;

  ipch->next = cnf->ipc_hosts;
  cnf->ipc_hosts = ipch;

  free((yyvsp[0])->string);
  free((yyvsp[0]));

;}
    break;

  case 73:
#line 325 "oparse.y"
    {
  struct in_addr in1, in2;
  struct ipc_net *ipcn;

  if(PARSER_DEBUG) printf("\tIPC net: %s/%s\n", (yyvsp[-1])->string, (yyvsp[0])->string);
  
  if(inet_aton((yyvsp[-1])->string, &in1) == 0)
    {
      fprintf(stderr, "Failed converting IP net IPC %s\n", (yyvsp[-1])->string);
      return -1;
    }

  if(inet_aton((yyvsp[0])->string, &in2) == 0)
    {
      fprintf(stderr, "Failed converting IP mask IPC %s\n", (yyvsp[0])->string);
      return -1;
    }

  ipcn = malloc(sizeof(struct ipc_net));
  ipcn->net.v4 = in1.s_addr;
  ipcn->mask.v4 = in2.s_addr;

  ipcn->next = cnf->ipc_nets;
  cnf->ipc_nets = ipcn;

  free((yyvsp[-1])->string);
  free((yyvsp[-1]));
  free((yyvsp[0])->string);
  free((yyvsp[0]));

;}
    break;

  case 74:
#line 359 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("Fixed willingness: %d\n", (yyvsp[0])->integer);

  while(ifcnt)
    {
      ifs->cnf->weight.value = (yyvsp[0])->integer;
      ifs->cnf->weight.fixed = OLSR_TRUE;

      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 75:
#line 379 "oparse.y"
    {
  struct in_addr in;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv4 broadcast: %s\n", (yyvsp[0])->string);

  if(inet_aton((yyvsp[0])->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[0])->string);
      return -1;
    }

  while(ifcnt)
    {
      ifs->cnf->ipv4_broadcast.v4 = in.s_addr;

      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0])->string);
  free((yyvsp[0]));
;}
    break;

  case 76:
#line 406 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if((yyvsp[0])->boolean)
    {
      while(ifcnt)
	{
	  ifs->cnf->ipv6_addrtype = IPV6_ADDR_SITELOCAL;
	  
	  ifs = ifs->next;
	  ifcnt--;
	}
    }
  else
    {
      while(ifcnt)
	{
	  ifs->cnf->ipv6_addrtype = 0;
	  
	  ifs = ifs->next;
	  ifcnt--;
	}
    }

  free((yyvsp[0]));
;}
    break;

  case 77:
#line 436 "oparse.y"
    {
  struct in6_addr in6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv6 site-local multicast: %s\n", (yyvsp[0])->string);

  if(inet_pton(AF_INET6, (yyvsp[0])->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[0])->string);
      return -1;
    }

  while(ifcnt)
    {
      memcpy(&ifs->cnf->ipv6_multi_site.v6, &in6, sizeof(struct in6_addr));
      
      ifs = ifs->next;
      ifcnt--;
    }


  free((yyvsp[0])->string);
  free((yyvsp[0]));
;}
    break;

  case 78:
#line 465 "oparse.y"
    {
  struct in6_addr in6;
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tIPv6 global multicast: %s\n", (yyvsp[0])->string);

  if(inet_pton(AF_INET6, (yyvsp[0])->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[0])->string);
      return -1;
    }

  while(ifcnt)
    {
      memcpy(&ifs->cnf->ipv6_multi_glbl.v6, &in6, sizeof(struct in6_addr));
      
      ifs = ifs->next;
      ifcnt--;
    }


  free((yyvsp[0])->string);
  free((yyvsp[0]));
;}
    break;

  case 79:
#line 492 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHELLO interval: %0.2f\n", (yyvsp[0])->floating);

  while(ifcnt)
    {
      ifs->cnf->hello_params.emission_interval = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 80:
#line 510 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHELLO validity: %0.2f\n", (yyvsp[0])->floating);

  while(ifcnt)
    {
      ifs->cnf->hello_params.validity_time = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 81:
#line 528 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tTC interval: %0.2f\n", (yyvsp[0])->floating);

  while(ifcnt)
    {
      ifs->cnf->tc_params.emission_interval = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }
  free((yyvsp[0]));
;}
    break;

  case 82:
#line 545 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;
  
  if(PARSER_DEBUG) printf("\tTC validity: %0.2f\n", (yyvsp[0])->floating);
  while(ifcnt)
    {
      ifs->cnf->tc_params.validity_time = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 83:
#line 562 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;


  if(PARSER_DEBUG) printf("\tMID interval: %0.2f\n", (yyvsp[0])->floating);
  while(ifcnt)
    {
      ifs->cnf->mid_params.emission_interval = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 84:
#line 580 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tMID validity: %0.2f\n", (yyvsp[0])->floating);
  while(ifcnt)
    {
      ifs->cnf->mid_params.validity_time = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 85:
#line 597 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;
  
  if(PARSER_DEBUG) printf("\tHNA interval: %0.2f\n", (yyvsp[0])->floating);
  while(ifcnt)
    {
      ifs->cnf->hna_params.emission_interval = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 86:
#line 614 "oparse.y"
    {
  int ifcnt = ifs_in_curr_cfg;
  struct olsr_if *ifs = cnf->interfaces;

  if(PARSER_DEBUG) printf("\tHNA validity: %0.2f\n", (yyvsp[0])->floating);
  while(ifcnt)
    {
      ifs->cnf->hna_params.validity_time = (yyvsp[0])->floating;
      
      ifs = ifs->next;
      ifcnt--;
    }

  free((yyvsp[0]));
;}
    break;

  case 87:
#line 632 "oparse.y"
    {
  if (lq_mult_helper((yyvsp[-1]), (yyvsp[0])) < 0)
    YYABORT;
;}
    break;

  case 88:
#line 638 "oparse.y"
    {
  if (lq_mult_helper((yyvsp[-1]), (yyvsp[0])) < 0)
    YYABORT;
;}
    break;

  case 89:
#line 644 "oparse.y"
    {
  if (lq_mult_helper((yyvsp[-1]), (yyvsp[0])) < 0)
    YYABORT;
;}
    break;

  case 90:
#line 652 "oparse.y"
    {

  cnf->debug_level = (yyvsp[0])->integer;
  if(PARSER_DEBUG) printf("Debug level: %d\n", cnf->debug_level);
  free((yyvsp[0]));
;}
    break;

  case 91:
#line 662 "oparse.y"
    {
  if((yyvsp[0])->integer == 4)
    cnf->ip_version = AF_INET;
  else if((yyvsp[0])->integer == 6)
    cnf->ip_version = AF_INET6;
  else
    {
      fprintf(stderr, "IPversion must be 4 or 6!\n");
      YYABORT;
    }

  if(PARSER_DEBUG) printf("IpVersion: %d\n", (yyvsp[0])->integer);
  free((yyvsp[0]));
;}
    break;

  case 92:
#line 680 "oparse.y"
    {
  struct hna4_entry *h = malloc(sizeof(struct hna4_entry));
  struct in_addr in;

  if(PARSER_DEBUG) printf("HNA IPv4 entry: %s/%s\n", (yyvsp[-1])->string, (yyvsp[0])->string);

  if(h == NULL)
    {
      fprintf(stderr, "Out of memory(HNA4)\n");
      YYABORT;
    }

  if(inet_aton((yyvsp[-1])->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[-1])->string);
      return -1;
    }
  h->net.v4 = in.s_addr;
  if(inet_aton((yyvsp[0])->string, &in) == 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[-1])->string);
      return -1;
    }
  h->netmask.v4 = in.s_addr;
  /* Queue */
  h->next = cnf->hna4_entries;
  cnf->hna4_entries = h;

  free((yyvsp[-1])->string);
  free((yyvsp[-1]));
  free((yyvsp[0])->string);
  free((yyvsp[0]));

;}
    break;

  case 93:
#line 717 "oparse.y"
    {
  struct hna6_entry *h = malloc(sizeof(struct hna6_entry));
  struct in6_addr in6;

  if(PARSER_DEBUG) printf("HNA IPv6 entry: %s/%d\n", (yyvsp[-1])->string, (yyvsp[0])->integer);

  if(h == NULL)
    {
      fprintf(stderr, "Out of memory(HNA6)\n");
      YYABORT;
    }

  if(inet_pton(AF_INET6, (yyvsp[-1])->string, &in6) < 0)
    {
      fprintf(stderr, "Failed converting IP address %s\n", (yyvsp[-1])->string);
      return -1;
    }
  memcpy(&h->net, &in6, sizeof(struct in6_addr));

  if(((yyvsp[0])->integer < 0) || ((yyvsp[0])->integer > 128))
    {
      fprintf(stderr, "Illegal IPv6 prefix length %d\n", (yyvsp[0])->integer);
      return -1;
    }

  h->prefix_len = (yyvsp[0])->integer;
  /* Queue */
  h->next = cnf->hna6_entries;
  cnf->hna6_entries = h;

  free((yyvsp[-1])->string);
  free((yyvsp[-1]));
  free((yyvsp[0]));

;}
    break;

  case 94:
#line 755 "oparse.y"
    {
  if(PARSER_DEBUG) printf("setting ifs_in_curr_cfg = 0\n");
  ifs_in_curr_cfg = 0;
;}
    break;

  case 95:
#line 762 "oparse.y"
    {
  struct olsr_if *in = malloc(sizeof(struct olsr_if));
  
  if(in == NULL)
    {
      fprintf(stderr, "Out of memory(ADD IF)\n");
      YYABORT;
    }

  in->cnf = get_default_if_config();

  if(in->cnf == NULL)
    {
      fprintf(stderr, "Out of memory(ADD IFRULE)\n");
      YYABORT;
    }

  in->name = (yyvsp[0])->string;

  /* Queue */
  in->next = cnf->interfaces;
  cnf->interfaces = in;

  ifs_in_curr_cfg++;

  free((yyvsp[0]));
;}
    break;

  case 96:
#line 792 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Noint set to %d\n", (yyvsp[0])->boolean);

  cnf->allow_no_interfaces = (yyvsp[0])->boolean;

  free((yyvsp[0]));
;}
    break;

  case 97:
#line 802 "oparse.y"
    {
  if(PARSER_DEBUG) printf("TOS: %d\n", (yyvsp[0])->integer);
  cnf->tos = (yyvsp[0])->integer;

  free((yyvsp[0]));

;}
    break;

  case 98:
#line 812 "oparse.y"
    {
  cnf->willingness_auto = OLSR_FALSE;

  if(PARSER_DEBUG) printf("Willingness: %d\n", (yyvsp[0])->integer);
  cnf->willingness = (yyvsp[0])->integer;

  free((yyvsp[0]));

;}
    break;

  case 99:
#line 826 "oparse.y"
    {
  cnf->use_hysteresis = (yyvsp[0])->boolean;
  if(cnf->use_hysteresis)
    {
      if(PARSER_DEBUG) printf("Hysteresis enabled\n");
    }
  else
    {
      if(PARSER_DEBUG) printf("Hysteresis disabled\n");
    }
  free((yyvsp[0]));

;}
    break;

  case 100:
#line 843 "oparse.y"
    {
  cnf->hysteresis_param.scaling = (yyvsp[0])->floating;
  if(PARSER_DEBUG) printf("Hysteresis Scaling: %0.2f\n", (yyvsp[0])->floating);
  free((yyvsp[0]));
;}
    break;

  case 101:
#line 852 "oparse.y"
    {
  cnf->hysteresis_param.thr_high = (yyvsp[0])->floating;
  if(PARSER_DEBUG) printf("Hysteresis UpperThr: %0.2f\n", (yyvsp[0])->floating);
  free((yyvsp[0]));
;}
    break;

  case 102:
#line 861 "oparse.y"
    {
  cnf->hysteresis_param.thr_low = (yyvsp[0])->floating;
  if(PARSER_DEBUG) printf("Hysteresis LowerThr: %0.2f\n", (yyvsp[0])->floating);
  free((yyvsp[0]));
;}
    break;

  case 103:
#line 869 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Pollrate %0.2f\n", (yyvsp[0])->floating);
  cnf->pollrate = (yyvsp[0])->floating;

  free((yyvsp[0]));
;}
    break;

  case 104:
#line 879 "oparse.y"
    {
  if(PARSER_DEBUG) printf("TC redundancy %d\n", (yyvsp[0])->integer);
  cnf->tc_redundancy = (yyvsp[0])->integer;
  free((yyvsp[0]));
;}
    break;

  case 105:
#line 887 "oparse.y"
    {
  if(PARSER_DEBUG) printf("MPR coverage %d\n", (yyvsp[0])->integer);
  cnf->mpr_coverage = (yyvsp[0])->integer;
  free((yyvsp[0]));
;}
    break;

  case 106:
#line 895 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Link quality level %d\n", (yyvsp[0])->integer);
  cnf->lq_level = (yyvsp[0])->integer;
  free((yyvsp[0]));
;}
    break;

  case 107:
#line 903 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Link quality fish eye %d\n", (yyvsp[0])->integer);
  cnf->lq_fish = (yyvsp[0])->integer;
  free((yyvsp[0]));
;}
    break;

  case 108:
#line 911 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Link quality dijkstra limit %d, %0.2f\n", (yyvsp[-1])->integer, (yyvsp[0])->floating);
  cnf->lq_dlimit = (yyvsp[-1])->integer;
  cnf->lq_dinter = (yyvsp[0])->floating;
  free((yyvsp[-1]));
;}
    break;

  case 109:
#line 920 "oparse.y"
    {
  if(PARSER_DEBUG) printf("Link quality window size %d\n", (yyvsp[0])->integer);
  cnf->lq_wsize = (yyvsp[0])->integer;
  free((yyvsp[0]));
;}
    break;

  case 110:
#line 928 "oparse.y"
    {
  cnf->clear_screen = (yyvsp[0])->boolean;

  if (PARSER_DEBUG)
    printf("Clear screen %s\n", cnf->clear_screen ? "enabled" : "disabled");

  free((yyvsp[0]));
;}
    break;

  case 111:
#line 939 "oparse.y"
    {
  struct plugin_entry *pe = malloc(sizeof(struct plugin_entry));
  
  if(pe == NULL)
    {
      fprintf(stderr, "Out of memory(ADD PL)\n");
      YYABORT;
    }

  pe->name = (yyvsp[0])->string;

  pe->params = NULL;
  
  if(PARSER_DEBUG) printf("Plugin: %s\n", (yyvsp[0])->string);

  /* Queue */
  pe->next = cnf->plugins;
  cnf->plugins = pe;

  free((yyvsp[0]));
;}
    break;

  case 112:
#line 963 "oparse.y"
    {
  struct plugin_param *pp = malloc(sizeof(struct plugin_param));
  
  if(pp == NULL)
    {
      fprintf(stderr, "Out of memory(ADD PP)\n");
      YYABORT;
    }
  
  if(PARSER_DEBUG) printf("Plugin param key:\"%s\" val: \"%s\"\n", (yyvsp[-1])->string, (yyvsp[0])->string);
  
  pp->key = (yyvsp[-1])->string;
  pp->value = (yyvsp[0])->string;

  /* Queue */
  pp->next = cnf->plugins->params;
  cnf->plugins->params = pp;

  free((yyvsp[-1]));
  free((yyvsp[0]));
;}
    break;

  case 113:
#line 987 "oparse.y"
    {
    //if(PARSER_DEBUG) printf("Comment\n");
;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2233 "oparse.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
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
	  int yychecklim = YYLAST - yyn;
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
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
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
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 994 "oparse.y"


void yyerror (char *string)
{
  fprintf(stderr, "Config line %d: %s\n", current_line, string);
}

