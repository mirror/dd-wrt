/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse pcap_parse
#define yylex   pcap_lex
#define yyerror pcap_error
#define yylval  pcap_lval
#define yychar  pcap_char
#define yydebug pcap_debug
#define yynerrs pcap_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DST = 258,
     SRC = 259,
     HOST = 260,
     GATEWAY = 261,
     NET = 262,
     NETMASK = 263,
     PORT = 264,
     PORTRANGE = 265,
     LESS = 266,
     GREATER = 267,
     PROTO = 268,
     PROTOCHAIN = 269,
     CBYTE = 270,
     ARP = 271,
     RARP = 272,
     IP = 273,
     SCTP = 274,
     TCP = 275,
     UDP = 276,
     ICMP = 277,
     IGMP = 278,
     IGRP = 279,
     PIM = 280,
     VRRP = 281,
     ATALK = 282,
     AARP = 283,
     DECNET = 284,
     LAT = 285,
     SCA = 286,
     MOPRC = 287,
     MOPDL = 288,
     TK_BROADCAST = 289,
     TK_MULTICAST = 290,
     NUM = 291,
     INBOUND = 292,
     OUTBOUND = 293,
     PF_IFNAME = 294,
     PF_RSET = 295,
     PF_RNR = 296,
     PF_SRNR = 297,
     PF_REASON = 298,
     PF_ACTION = 299,
     LINK = 300,
     GEQ = 301,
     LEQ = 302,
     NEQ = 303,
     ID = 304,
     EID = 305,
     HID = 306,
     HID6 = 307,
     AID = 308,
     LSH = 309,
     RSH = 310,
     LEN = 311,
     IPV6 = 312,
     ICMPV6 = 313,
     AH = 314,
     ESP = 315,
     VLAN = 316,
     MPLS = 317,
     PPPOED = 318,
     PPPOES = 319,
     ISO = 320,
     ESIS = 321,
     CLNP = 322,
     ISIS = 323,
     L1 = 324,
     L2 = 325,
     IIH = 326,
     LSP = 327,
     SNP = 328,
     CSNP = 329,
     PSNP = 330,
     STP = 331,
     IPX = 332,
     NETBEUI = 333,
     LANE = 334,
     LLC = 335,
     METAC = 336,
     BCC = 337,
     SC = 338,
     ILMIC = 339,
     OAMF4EC = 340,
     OAMF4SC = 341,
     OAM = 342,
     OAMF4 = 343,
     CONNECTMSG = 344,
     METACONNECT = 345,
     VPI = 346,
     VCI = 347,
     RADIO = 348,
     SIO = 349,
     OPC = 350,
     DPC = 351,
     SLS = 352,
     AND = 353,
     OR = 354,
     UMINUS = 355
   };
#endif
#define DST 258
#define SRC 259
#define HOST 260
#define GATEWAY 261
#define NET 262
#define NETMASK 263
#define PORT 264
#define PORTRANGE 265
#define LESS 266
#define GREATER 267
#define PROTO 268
#define PROTOCHAIN 269
#define CBYTE 270
#define ARP 271
#define RARP 272
#define IP 273
#define SCTP 274
#define TCP 275
#define UDP 276
#define ICMP 277
#define IGMP 278
#define IGRP 279
#define PIM 280
#define VRRP 281
#define ATALK 282
#define AARP 283
#define DECNET 284
#define LAT 285
#define SCA 286
#define MOPRC 287
#define MOPDL 288
#define TK_BROADCAST 289
#define TK_MULTICAST 290
#define NUM 291
#define INBOUND 292
#define OUTBOUND 293
#define PF_IFNAME 294
#define PF_RSET 295
#define PF_RNR 296
#define PF_SRNR 297
#define PF_REASON 298
#define PF_ACTION 299
#define LINK 300
#define GEQ 301
#define LEQ 302
#define NEQ 303
#define ID 304
#define EID 305
#define HID 306
#define HID6 307
#define AID 308
#define LSH 309
#define RSH 310
#define LEN 311
#define IPV6 312
#define ICMPV6 313
#define AH 314
#define ESP 315
#define VLAN 316
#define MPLS 317
#define PPPOED 318
#define PPPOES 319
#define ISO 320
#define ESIS 321
#define CLNP 322
#define ISIS 323
#define L1 324
#define L2 325
#define IIH 326
#define LSP 327
#define SNP 328
#define CSNP 329
#define PSNP 330
#define STP 331
#define IPX 332
#define NETBEUI 333
#define LANE 334
#define LLC 335
#define METAC 336
#define BCC 337
#define SC 338
#define ILMIC 339
#define OAMF4EC 340
#define OAMF4SC 341
#define OAM 342
#define OAMF4 343
#define CONNECTMSG 344
#define METACONNECT 345
#define VPI 346
#define VCI 347
#define RADIO 348
#define SIO 349
#define OPC 350
#define DPC 351
#define SLS 352
#define AND 353
#define OR 354
#define UMINUS 355




/* Copy the first part of user declarations.  */
#line 1 "grammar.y"

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/grammar.y,v 1.86.2.5 2005/09/05 09:08:06 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <pcap-stdinc.h>
#else /* WIN32 */
#include <sys/types.h>
#include <sys/socket.h>
#endif /* WIN32 */

#include <stdlib.h>

#ifndef WIN32
#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <netinet/in.h>
#endif /* WIN32 */

#include <stdio.h>

#include "pcap-int.h"

#include "gencode.h"
#include "pf.h"
#include <pcap-namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(char *msg)
{
	++n_errors;
	bpf_error("%s", msg);
	/* NOTREACHED */
}

#ifndef YYBISON
int yyparse(void);

int
pcap_parse()
{
	return (yyparse());
}
#endif



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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 90 "grammar.y"
typedef union YYSTYPE {
	int i;
	bpf_u_int32 h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		int atmfieldtype;
		int mtp3fieldtype;
		struct block *b;
	} blk;
	struct block *rblk;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 389 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 401 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
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
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   591

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  40
/* YYNRULES -- Number of rules. */
#define YYNRULES  182
/* YYNRULES -- Number of states. */
#define YYNSTATES  250

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   355

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   100,     2,     2,     2,     2,   102,     2,
     109,   108,   105,   103,     2,   104,     2,   106,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   115,     2,
     112,   111,   110,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   113,     2,   114,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   101,     2,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   107
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     6,     8,     9,    11,    15,    19,    23,
      27,    29,    31,    33,    35,    39,    41,    45,    49,    51,
      55,    57,    59,    61,    64,    66,    68,    70,    74,    78,
      80,    82,    84,    87,    91,    94,    97,   100,   103,   106,
     109,   113,   115,   119,   123,   125,   127,   129,   132,   135,
     137,   138,   140,   142,   146,   150,   154,   158,   160,   162,
     164,   166,   168,   170,   172,   174,   176,   178,   180,   182,
     184,   186,   188,   190,   192,   194,   196,   198,   200,   202,
     204,   206,   208,   210,   212,   214,   216,   218,   220,   222,
     224,   226,   228,   230,   232,   234,   236,   238,   240,   242,
     244,   247,   250,   253,   256,   261,   263,   265,   268,   270,
     273,   275,   277,   279,   281,   284,   287,   290,   293,   296,
     299,   301,   303,   305,   307,   309,   311,   313,   315,   317,
     319,   321,   326,   333,   337,   341,   345,   349,   353,   357,
     361,   365,   368,   372,   374,   376,   378,   380,   382,   384,
     386,   390,   392,   394,   396,   398,   400,   402,   404,   406,
     408,   410,   412,   414,   416,   418,   420,   423,   426,   430,
     432,   434,   438,   440,   442,   444,   446,   448,   451,   454,
     458,   460,   462
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,   119,    -1,   118,    -1,    -1,   128,
      -1,   119,   120,   128,    -1,   119,   120,   122,    -1,   119,
     121,   128,    -1,   119,   121,   122,    -1,    98,    -1,    99,
      -1,   123,    -1,   145,    -1,   125,   126,   108,    -1,    49,
      -1,    51,   106,    36,    -1,    51,     8,    51,    -1,    51,
      -1,    52,   106,    36,    -1,    52,    -1,    50,    -1,    53,
      -1,   124,   122,    -1,   100,    -1,   109,    -1,   123,    -1,
     127,   120,   122,    -1,   127,   121,   122,    -1,   145,    -1,
     126,    -1,   130,    -1,   124,   128,    -1,   131,   132,   133,
      -1,   131,   132,    -1,   131,   133,    -1,   131,    13,    -1,
     131,    14,    -1,   131,   134,    -1,   129,   122,    -1,   125,
     119,   108,    -1,   135,    -1,   142,   140,   142,    -1,   142,
     141,   142,    -1,   136,    -1,   146,    -1,   147,    -1,   148,
     149,    -1,   152,   153,    -1,   135,    -1,    -1,     4,    -1,
       3,    -1,     4,    99,     3,    -1,     3,    99,     4,    -1,
       4,    98,     3,    -1,     3,    98,     4,    -1,     5,    -1,
       7,    -1,     9,    -1,    10,    -1,     6,    -1,    45,    -1,
      18,    -1,    16,    -1,    17,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,
      26,    -1,    27,    -1,    28,    -1,    29,    -1,    30,    -1,
      31,    -1,    33,    -1,    32,    -1,    57,    -1,    58,    -1,
      59,    -1,    60,    -1,    65,    -1,    66,    -1,    68,    -1,
      69,    -1,    70,    -1,    71,    -1,    72,    -1,    73,    -1,
      75,    -1,    74,    -1,    67,    -1,    76,    -1,    77,    -1,
      78,    -1,    93,    -1,   131,    34,    -1,   131,    35,    -1,
      11,    36,    -1,    12,    36,    -1,    15,    36,   144,    36,
      -1,    37,    -1,    38,    -1,    61,   145,    -1,    61,    -1,
      62,   145,    -1,    62,    -1,    63,    -1,    64,    -1,   137,
      -1,    39,    49,    -1,    40,    49,    -1,    41,    36,    -1,
      42,    36,    -1,    43,   138,    -1,    44,   139,    -1,    36,
      -1,    49,    -1,    49,    -1,   110,    -1,    46,    -1,   111,
      -1,    47,    -1,   112,    -1,    48,    -1,   145,    -1,   143,
      -1,   135,   113,   142,   114,    -1,   135,   113,   142,   115,
      36,   114,    -1,   142,   103,   142,    -1,   142,   104,   142,
      -1,   142,   105,   142,    -1,   142,   106,   142,    -1,   142,
     102,   142,    -1,   142,   101,   142,    -1,   142,    54,   142,
      -1,   142,    55,   142,    -1,   104,   142,    -1,   125,   143,
     108,    -1,    56,    -1,   102,    -1,   101,    -1,   112,    -1,
     110,    -1,   111,    -1,    36,    -1,   125,   145,   108,    -1,
      79,    -1,    80,    -1,    81,    -1,    82,    -1,    85,    -1,
      86,    -1,    83,    -1,    84,    -1,    87,    -1,    88,    -1,
      89,    -1,    90,    -1,    91,    -1,    92,    -1,   150,    -1,
     140,    36,    -1,   141,    36,    -1,   125,   151,   108,    -1,
      36,    -1,   150,    -1,   151,   121,   150,    -1,    94,    -1,
      95,    -1,    96,    -1,    97,    -1,   154,    -1,   140,    36,
      -1,   141,    36,    -1,   125,   155,   108,    -1,    36,    -1,
     154,    -1,   155,   121,   154,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   160,   160,   164,   166,   168,   169,   170,   171,   172,
     174,   176,   178,   179,   181,   183,   184,   186,   188,   193,
     202,   211,   220,   229,   231,   233,   235,   236,   237,   239,
     241,   243,   244,   246,   247,   248,   249,   250,   251,   253,
     254,   255,   256,   258,   260,   261,   262,   263,   264,   267,
     268,   271,   272,   273,   274,   275,   276,   279,   280,   281,
     282,   285,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   342,   343,   344,   345,   346,   347,
     350,   351,   364,   375,   376,   377,   379,   380,   381,   383,
     384,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   400,   401,   402,   403,   404,   406,
     407,   409,   410,   411,   412,   413,   414,   415,   416,   418,
     419,   420,   421,   424,   425,   427,   428,   429,   430,   432,
     439,   440,   443,   444,   445,   446,   448,   449,   450,   451,
     453,   462,   463
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DST", "SRC", "HOST", "GATEWAY", "NET", 
  "NETMASK", "PORT", "PORTRANGE", "LESS", "GREATER", "PROTO", 
  "PROTOCHAIN", "CBYTE", "ARP", "RARP", "IP", "SCTP", "TCP", "UDP", 
  "ICMP", "IGMP", "IGRP", "PIM", "VRRP", "ATALK", "AARP", "DECNET", "LAT", 
  "SCA", "MOPRC", "MOPDL", "TK_BROADCAST", "TK_MULTICAST", "NUM", 
  "INBOUND", "OUTBOUND", "PF_IFNAME", "PF_RSET", "PF_RNR", "PF_SRNR", 
  "PF_REASON", "PF_ACTION", "LINK", "GEQ", "LEQ", "NEQ", "ID", "EID", 
  "HID", "HID6", "AID", "LSH", "RSH", "LEN", "IPV6", "ICMPV6", "AH", 
  "ESP", "VLAN", "MPLS", "PPPOED", "PPPOES", "ISO", "ESIS", "CLNP", 
  "ISIS", "L1", "L2", "IIH", "LSP", "SNP", "CSNP", "PSNP", "STP", "IPX", 
  "NETBEUI", "LANE", "LLC", "METAC", "BCC", "SC", "ILMIC", "OAMF4EC", 
  "OAMF4SC", "OAM", "OAMF4", "CONNECTMSG", "METACONNECT", "VPI", "VCI", 
  "RADIO", "SIO", "OPC", "DPC", "SLS", "AND", "OR", "'!'", "'|'", "'&'", 
  "'+'", "'-'", "'*'", "'/'", "UMINUS", "')'", "'('", "'>'", "'='", "'<'", 
  "'['", "']'", "':'", "$accept", "prog", "null", "expr", "and", "or", 
  "id", "nid", "not", "paren", "pid", "qid", "term", "head", "rterm", 
  "pqual", "dqual", "aqual", "ndaqual", "pname", "other", "pfvar", 
  "reason", "action", "relop", "irelop", "arth", "narth", "byteop", 
  "pnum", "atmtype", "atmmultitype", "atmfield", "atmvalue", 
  "atmfieldvalue", "atmlistvalue", "mtp3field", "mtp3value", 
  "mtp3fieldvalue", "mtp3listvalue", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
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
      33,   124,    38,    43,    45,    42,    47,   355,    41,    40,
      62,    61,    60,    91,    93,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   117,   118,   119,   119,   119,   119,   119,
     120,   121,   122,   122,   122,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   124,   125,   126,   126,   126,   127,
     127,   128,   128,   129,   129,   129,   129,   129,   129,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   131,
     131,   132,   132,   132,   132,   132,   132,   133,   133,   133,
     133,   134,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   137,   137,   137,   137,   137,   137,
     138,   138,   139,   140,   140,   140,   141,   141,   141,   142,
     142,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   144,   144,   144,   145,
     145,   146,   146,   146,   146,   146,   146,   146,   146,   147,
     147,   147,   147,   148,   148,   149,   149,   149,   149,   150,
     151,   151,   152,   152,   152,   152,   153,   153,   153,   153,
     154,   155,   155
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     1,     0,     1,     3,     3,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     2,     1,     1,     1,     3,     3,     1,
       1,     1,     2,     3,     2,     2,     2,     2,     2,     2,
       3,     1,     3,     3,     1,     1,     1,     2,     2,     1,
       0,     1,     1,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     4,     1,     1,     2,     1,     2,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     6,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     1,
       1,     3,     1,     1,     1,     1,     1,     2,     2,     3,
       1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,    50,     1,     0,     0,     0,    64,    65,    63,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    80,    79,   149,   105,   106,     0,     0,
       0,     0,     0,     0,    62,   143,    81,    82,    83,    84,
     108,   110,   111,   112,    85,    86,    95,    87,    88,    89,
      90,    91,    92,    94,    93,    96,    97,    98,   151,   152,
     153,   154,   157,   158,   155,   156,   159,   160,   161,   162,
     163,   164,    99,   172,   173,   174,   175,    24,     0,    25,
       2,    50,    50,     5,     0,    31,     0,    49,    44,   113,
       0,   130,   129,    45,    46,     0,     0,   102,   103,     0,
     114,   115,   116,   117,   120,   121,   118,   122,   119,     0,
     107,   109,     0,     0,   141,    10,    11,    50,    50,    32,
       0,   130,   129,    15,    21,    18,    20,    22,    39,    12,
       0,     0,    13,    52,    51,    57,    61,    58,    59,    60,
      36,    37,   100,   101,    34,    35,    38,     0,   124,   126,
     128,     0,     0,     0,     0,     0,     0,     0,     0,   123,
     125,   127,     0,     0,   169,     0,     0,     0,    47,   165,
     180,     0,     0,     0,    48,   176,   145,   144,   147,   148,
     146,     0,     0,     0,     7,    50,    50,     6,   129,     9,
       8,    40,   142,   150,     0,     0,     0,    23,    26,    30,
       0,    29,     0,     0,     0,     0,    33,     0,   139,   140,
     138,   137,   133,   134,   135,   136,    42,    43,   170,     0,
     166,   167,   181,     0,   177,   178,   104,   129,    17,    16,
      19,    14,     0,     0,    56,    54,    55,    53,   131,     0,
     168,     0,   179,     0,    27,    28,     0,   171,   182,   132
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     1,     2,   120,   117,   118,   197,   129,   130,   112,
     199,   200,    83,    84,    85,    86,   144,   145,   146,   113,
      88,    89,   106,   108,   162,   163,    90,    91,   181,    92,
      93,    94,    95,   168,   169,   219,    96,   174,   175,   223
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -177
static const short yypact[] =
{
    -177,    15,   196,  -177,   -13,    -7,    -3,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,    10,    14,
       0,    17,   -24,    25,  -177,  -177,  -177,  -177,  -177,  -177,
     -14,   -14,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,   480,  -177,
     -67,   386,   386,  -177,   -33,  -177,   556,     4,  -177,  -177,
     475,  -177,  -177,  -177,  -177,   133,   139,  -177,  -177,   -56,
    -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,  -177,   -14,
    -177,  -177,   480,   -36,  -177,  -177,  -177,   291,   291,  -177,
     -71,   -51,   -27,  -177,  -177,     5,   -15,  -177,  -177,  -177,
     -33,   -33,  -177,   -30,   -26,  -177,  -177,  -177,  -177,  -177,
    -177,  -177,  -177,  -177,    55,  -177,  -177,   480,  -177,  -177,
    -177,   480,   480,   480,   480,   480,   480,   480,   480,  -177,
    -177,  -177,   480,   480,  -177,    50,    63,    70,  -177,  -177,
    -177,    72,    94,   105,  -177,  -177,  -177,  -177,  -177,  -177,
    -177,   112,   -27,    99,  -177,   291,   291,  -177,     2,  -177,
    -177,  -177,  -177,  -177,   106,   122,   123,  -177,  -177,    58,
     -67,   -27,   163,   164,   167,   168,  -177,    59,    41,    41,
     335,    16,    -1,    -1,  -177,  -177,    99,    99,  -177,   -78,
    -177,  -177,  -177,   -59,  -177,  -177,  -177,   -47,  -177,  -177,
    -177,  -177,   -33,   -33,  -177,  -177,  -177,  -177,  -177,   136,
    -177,    50,  -177,    72,  -177,  -177,    62,  -177,  -177,  -177
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -177,  -177,  -177,   175,   -22,  -176,   -83,  -120,     6,    -2,
    -177,  -177,   -76,  -177,  -177,  -177,  -177,    38,  -177,     8,
    -177,  -177,  -177,  -177,    44,    60,   -20,   -68,  -177,   -34,
    -177,  -177,  -177,  -177,  -156,  -177,  -177,  -177,  -145,  -177
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -42
static const short yytable[] =
{
      82,   128,   -13,    25,   -41,   119,   110,   111,    81,   218,
      87,   198,   104,   194,   121,     3,   123,   124,   125,   126,
     127,   116,    25,    97,   233,   105,   222,   115,   116,    98,
     240,   115,   116,    99,   184,   189,   102,   191,   109,   109,
     116,   187,   190,   241,   121,   176,   177,   243,   122,   242,
     132,   -29,   -29,   103,   178,   179,   180,   192,   114,   100,
     135,   193,   137,   101,   138,   139,   198,    77,   202,   203,
     151,   152,   204,   205,   107,   182,    79,   147,   122,    82,
      82,   193,   131,   188,   188,   247,   164,    81,    81,    87,
      87,   196,   183,   165,   171,    79,   132,   201,   248,   220,
     -13,   -13,   -41,   -41,   157,   158,   221,   109,   170,   119,
     -13,   195,   -41,   151,   152,   186,   186,   147,   121,   155,
     156,   157,   158,   185,   185,    87,    87,   207,   131,   109,
     224,   208,   209,   210,   211,   212,   213,   214,   215,   166,
     172,   225,   216,   217,   155,   156,   157,   158,   226,   244,
     245,   188,   227,   151,   152,   167,   173,   228,   229,   230,
     153,   154,   155,   156,   157,   158,   231,   234,   235,   164,
     236,   237,   246,   238,   239,   170,   249,    80,   232,   148,
     149,   150,   206,   186,    82,   148,   149,   150,     0,     0,
       0,   185,   185,    87,    87,     0,    -3,     0,   132,   132,
     153,   154,   155,   156,   157,   158,     0,     4,     5,     0,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
     131,   131,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    79,   159,   160,   161,     0,     0,    79,   159,
     160,   161,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,     0,    77,     0,     0,     0,
      78,     0,     4,     5,     0,    79,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,     0,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,     0,     0,     0,
     123,   124,   125,   126,   127,     0,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,   151,
     152,    77,     0,     0,     0,    78,     0,     4,     5,     0,
      79,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,     0,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,     0,     0,     0,     0,   154,   155,   156,
     157,   158,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,     0,    77,     0,     0,     0,
      78,     0,     0,     0,     0,    79,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,     0,    25,     0,     0,     0,
       0,   148,   149,   150,     0,    34,     0,     0,     0,   151,
     152,     0,     0,     0,     0,     0,    35,    36,    37,    38,
      39,     0,     0,     0,     0,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,   133,
     134,   135,   136,   137,     0,   138,   139,     0,     0,   140,
     141,     0,     0,    72,     0,     0,   153,   154,   155,   156,
     157,   158,     0,     0,    78,   159,   160,   161,     0,    79,
     142,   143
};

static const short yycheck[] =
{
       2,    84,     0,    36,     0,    81,    40,    41,     2,   165,
       2,   131,    36,     8,    82,     0,    49,    50,    51,    52,
      53,    99,    36,    36,   200,    49,   171,    98,    99,    36,
     108,    98,    99,    36,   117,   118,    36,   108,    40,    41,
      99,   117,   118,   219,   112,   101,   102,   223,    82,   108,
      84,    98,    99,    36,   110,   111,   112,   108,    78,    49,
       5,   108,     7,    49,     9,    10,   186,   100,    98,    99,
      54,    55,    98,    99,    49,   109,   109,   113,   112,    81,
      82,   108,    84,   117,   118,   241,    36,    81,    82,    81,
      82,   106,   112,    95,    96,   109,   130,   131,   243,    36,
      98,    99,    98,    99,   105,   106,    36,   109,    36,   185,
     108,   106,   108,    54,    55,   117,   118,   113,   186,   103,
     104,   105,   106,   117,   118,   117,   118,   147,   130,   131,
      36,   151,   152,   153,   154,   155,   156,   157,   158,    95,
      96,    36,   162,   163,   103,   104,   105,   106,    36,   232,
     233,   185,   186,    54,    55,    95,    96,    51,    36,    36,
     101,   102,   103,   104,   105,   106,   108,     4,     4,    36,
       3,     3,    36,   114,   115,    36,   114,     2,   200,    46,
      47,    48,   144,   185,   186,    46,    47,    48,    -1,    -1,
      -1,   185,   186,   185,   186,    -1,     0,    -1,   232,   233,
     101,   102,   103,   104,   105,   106,    -1,    11,    12,    -1,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
     232,   233,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,   109,   110,   111,   112,    -1,    -1,   109,   110,
     111,   112,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,    -1,    -1,    -1,
     104,    -1,    11,    12,    -1,   109,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    54,
      55,   100,    -1,    -1,    -1,   104,    -1,    11,    12,    -1,
     109,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,
     105,   106,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,   109,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    46,    47,    48,    -1,    45,    -1,    -1,    -1,    54,
      55,    -1,    -1,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     3,
       4,     5,     6,     7,    -1,     9,    10,    -1,    -1,    13,
      14,    -1,    -1,    93,    -1,    -1,   101,   102,   103,   104,
     105,   106,    -1,    -1,   104,   110,   111,   112,    -1,   109,
      34,    35
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,   117,   118,     0,    11,    12,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,   100,   104,   109,
     119,   124,   125,   128,   129,   130,   131,   135,   136,   137,
     142,   143,   145,   146,   147,   148,   152,    36,    36,    36,
      49,    49,    36,    36,    36,    49,   138,    49,   139,   125,
     145,   145,   125,   135,   142,    98,    99,   120,   121,   128,
     119,   143,   145,    49,    50,    51,    52,    53,   122,   123,
     124,   125,   145,     3,     4,     5,     6,     7,     9,    10,
      13,    14,    34,    35,   132,   133,   134,   113,    46,    47,
      48,    54,    55,   101,   102,   103,   104,   105,   106,   110,
     111,   112,   140,   141,    36,   125,   140,   141,   149,   150,
      36,   125,   140,   141,   153,   154,   101,   102,   110,   111,
     112,   144,   145,   142,   122,   124,   125,   128,   145,   122,
     128,   108,   108,   108,     8,   106,   106,   122,   123,   126,
     127,   145,    98,    99,    98,    99,   133,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   150,   151,
      36,    36,   154,   155,    36,    36,    36,   145,    51,    36,
      36,   108,   120,   121,     4,     4,     3,     3,   114,   115,
     108,   121,   108,   121,   122,   122,    36,   150,   154,   114
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

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
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
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

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
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
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
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
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

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
  register const char *yys = yystr;

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
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



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
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

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
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
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

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



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
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
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
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

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
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

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
        case 2:
#line 161 "grammar.y"
    {
	finish_parse(yyvsp[0].blk.b);
}
    break;

  case 4:
#line 166 "grammar.y"
    { yyval.blk.q = qerr; }
    break;

  case 6:
#line 169 "grammar.y"
    { gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 7:
#line 170 "grammar.y"
    { gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 8:
#line 171 "grammar.y"
    { gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 9:
#line 172 "grammar.y"
    { gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 10:
#line 174 "grammar.y"
    { yyval.blk = yyvsp[-1].blk; }
    break;

  case 11:
#line 176 "grammar.y"
    { yyval.blk = yyvsp[-1].blk; }
    break;

  case 13:
#line 179 "grammar.y"
    { yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
    break;

  case 14:
#line 181 "grammar.y"
    { yyval.blk = yyvsp[-1].blk; }
    break;

  case 15:
#line 183 "grammar.y"
    { yyval.blk.b = gen_scode(yyvsp[0].s, yyval.blk.q = yyvsp[-1].blk.q); }
    break;

  case 16:
#line 184 "grammar.y"
    { yyval.blk.b = gen_mcode(yyvsp[-2].s, NULL, yyvsp[0].i,
				    yyval.blk.q = yyvsp[-3].blk.q); }
    break;

  case 17:
#line 186 "grammar.y"
    { yyval.blk.b = gen_mcode(yyvsp[-2].s, yyvsp[0].s, 0,
				    yyval.blk.q = yyvsp[-3].blk.q); }
    break;

  case 18:
#line 188 "grammar.y"
    {
				  /* Decide how to parse HID based on proto */
				  yyval.blk.q = yyvsp[-1].blk.q;
				  yyval.blk.b = gen_ncode(yyvsp[0].s, 0, yyval.blk.q);
				}
    break;

  case 19:
#line 193 "grammar.y"
    {
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yyvsp[-2].s, NULL, yyvsp[0].i,
				    yyval.blk.q = yyvsp[-3].blk.q);
#else
				  bpf_error("'ip6addr/prefixlen' not supported "
					"in this configuration");
#endif /*INET6*/
				}
    break;

  case 20:
#line 202 "grammar.y"
    {
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yyvsp[0].s, 0, 128,
				    yyval.blk.q = yyvsp[-1].blk.q);
#else
				  bpf_error("'ip6addr' not supported "
					"in this configuration");
#endif /*INET6*/
				}
    break;

  case 21:
#line 211 "grammar.y"
    { 
				  yyval.blk.b = gen_ecode(yyvsp[0].e, yyval.blk.q = yyvsp[-1].blk.q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free(yyvsp[0].e);
				}
    break;

  case 22:
#line 220 "grammar.y"
    {
				  yyval.blk.b = gen_acode(yyvsp[0].e, yyval.blk.q = yyvsp[-1].blk.q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free(yyvsp[0].e);
				}
    break;

  case 23:
#line 229 "grammar.y"
    { gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 24:
#line 231 "grammar.y"
    { yyval.blk = yyvsp[-1].blk; }
    break;

  case 25:
#line 233 "grammar.y"
    { yyval.blk = yyvsp[-1].blk; }
    break;

  case 27:
#line 236 "grammar.y"
    { gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 28:
#line 237 "grammar.y"
    { gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 29:
#line 239 "grammar.y"
    { yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
    break;

  case 32:
#line 244 "grammar.y"
    { gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 33:
#line 246 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-2].i, yyvsp[-1].i, yyvsp[0].i); }
    break;

  case 34:
#line 247 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-1].i, yyvsp[0].i, Q_DEFAULT); }
    break;

  case 35:
#line 248 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
    break;

  case 36:
#line 249 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, Q_PROTO); }
    break;

  case 37:
#line 250 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, Q_PROTOCHAIN); }
    break;

  case 38:
#line 251 "grammar.y"
    { QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
    break;

  case 39:
#line 253 "grammar.y"
    { yyval.blk = yyvsp[0].blk; }
    break;

  case 40:
#line 254 "grammar.y"
    { yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = yyvsp[-2].blk.q; }
    break;

  case 41:
#line 255 "grammar.y"
    { yyval.blk.b = gen_proto_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;

  case 42:
#line 256 "grammar.y"
    { yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 0);
				  yyval.blk.q = qerr; }
    break;

  case 43:
#line 258 "grammar.y"
    { yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 1);
				  yyval.blk.q = qerr; }
    break;

  case 44:
#line 260 "grammar.y"
    { yyval.blk.b = yyvsp[0].rblk; yyval.blk.q = qerr; }
    break;

  case 45:
#line 261 "grammar.y"
    { yyval.blk.b = gen_atmtype_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;

  case 46:
#line 262 "grammar.y"
    { yyval.blk.b = gen_atmmulti_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;

  case 47:
#line 263 "grammar.y"
    { yyval.blk.b = yyvsp[0].blk.b; yyval.blk.q = qerr; }
    break;

  case 48:
#line 264 "grammar.y"
    { yyval.blk.b = yyvsp[0].blk.b; yyval.blk.q = qerr; }
    break;

  case 50:
#line 268 "grammar.y"
    { yyval.i = Q_DEFAULT; }
    break;

  case 51:
#line 271 "grammar.y"
    { yyval.i = Q_SRC; }
    break;

  case 52:
#line 272 "grammar.y"
    { yyval.i = Q_DST; }
    break;

  case 53:
#line 273 "grammar.y"
    { yyval.i = Q_OR; }
    break;

  case 54:
#line 274 "grammar.y"
    { yyval.i = Q_OR; }
    break;

  case 55:
#line 275 "grammar.y"
    { yyval.i = Q_AND; }
    break;

  case 56:
#line 276 "grammar.y"
    { yyval.i = Q_AND; }
    break;

  case 57:
#line 279 "grammar.y"
    { yyval.i = Q_HOST; }
    break;

  case 58:
#line 280 "grammar.y"
    { yyval.i = Q_NET; }
    break;

  case 59:
#line 281 "grammar.y"
    { yyval.i = Q_PORT; }
    break;

  case 60:
#line 282 "grammar.y"
    { yyval.i = Q_PORTRANGE; }
    break;

  case 61:
#line 285 "grammar.y"
    { yyval.i = Q_GATEWAY; }
    break;

  case 62:
#line 287 "grammar.y"
    { yyval.i = Q_LINK; }
    break;

  case 63:
#line 288 "grammar.y"
    { yyval.i = Q_IP; }
    break;

  case 64:
#line 289 "grammar.y"
    { yyval.i = Q_ARP; }
    break;

  case 65:
#line 290 "grammar.y"
    { yyval.i = Q_RARP; }
    break;

  case 66:
#line 291 "grammar.y"
    { yyval.i = Q_SCTP; }
    break;

  case 67:
#line 292 "grammar.y"
    { yyval.i = Q_TCP; }
    break;

  case 68:
#line 293 "grammar.y"
    { yyval.i = Q_UDP; }
    break;

  case 69:
#line 294 "grammar.y"
    { yyval.i = Q_ICMP; }
    break;

  case 70:
#line 295 "grammar.y"
    { yyval.i = Q_IGMP; }
    break;

  case 71:
#line 296 "grammar.y"
    { yyval.i = Q_IGRP; }
    break;

  case 72:
#line 297 "grammar.y"
    { yyval.i = Q_PIM; }
    break;

  case 73:
#line 298 "grammar.y"
    { yyval.i = Q_VRRP; }
    break;

  case 74:
#line 299 "grammar.y"
    { yyval.i = Q_ATALK; }
    break;

  case 75:
#line 300 "grammar.y"
    { yyval.i = Q_AARP; }
    break;

  case 76:
#line 301 "grammar.y"
    { yyval.i = Q_DECNET; }
    break;

  case 77:
#line 302 "grammar.y"
    { yyval.i = Q_LAT; }
    break;

  case 78:
#line 303 "grammar.y"
    { yyval.i = Q_SCA; }
    break;

  case 79:
#line 304 "grammar.y"
    { yyval.i = Q_MOPDL; }
    break;

  case 80:
#line 305 "grammar.y"
    { yyval.i = Q_MOPRC; }
    break;

  case 81:
#line 306 "grammar.y"
    { yyval.i = Q_IPV6; }
    break;

  case 82:
#line 307 "grammar.y"
    { yyval.i = Q_ICMPV6; }
    break;

  case 83:
#line 308 "grammar.y"
    { yyval.i = Q_AH; }
    break;

  case 84:
#line 309 "grammar.y"
    { yyval.i = Q_ESP; }
    break;

  case 85:
#line 310 "grammar.y"
    { yyval.i = Q_ISO; }
    break;

  case 86:
#line 311 "grammar.y"
    { yyval.i = Q_ESIS; }
    break;

  case 87:
#line 312 "grammar.y"
    { yyval.i = Q_ISIS; }
    break;

  case 88:
#line 313 "grammar.y"
    { yyval.i = Q_ISIS_L1; }
    break;

  case 89:
#line 314 "grammar.y"
    { yyval.i = Q_ISIS_L2; }
    break;

  case 90:
#line 315 "grammar.y"
    { yyval.i = Q_ISIS_IIH; }
    break;

  case 91:
#line 316 "grammar.y"
    { yyval.i = Q_ISIS_LSP; }
    break;

  case 92:
#line 317 "grammar.y"
    { yyval.i = Q_ISIS_SNP; }
    break;

  case 93:
#line 318 "grammar.y"
    { yyval.i = Q_ISIS_PSNP; }
    break;

  case 94:
#line 319 "grammar.y"
    { yyval.i = Q_ISIS_CSNP; }
    break;

  case 95:
#line 320 "grammar.y"
    { yyval.i = Q_CLNP; }
    break;

  case 96:
#line 321 "grammar.y"
    { yyval.i = Q_STP; }
    break;

  case 97:
#line 322 "grammar.y"
    { yyval.i = Q_IPX; }
    break;

  case 98:
#line 323 "grammar.y"
    { yyval.i = Q_NETBEUI; }
    break;

  case 99:
#line 324 "grammar.y"
    { yyval.i = Q_RADIO; }
    break;

  case 100:
#line 326 "grammar.y"
    { yyval.rblk = gen_broadcast(yyvsp[-1].i); }
    break;

  case 101:
#line 327 "grammar.y"
    { yyval.rblk = gen_multicast(yyvsp[-1].i); }
    break;

  case 102:
#line 328 "grammar.y"
    { yyval.rblk = gen_less(yyvsp[0].i); }
    break;

  case 103:
#line 329 "grammar.y"
    { yyval.rblk = gen_greater(yyvsp[0].i); }
    break;

  case 104:
#line 330 "grammar.y"
    { yyval.rblk = gen_byteop(yyvsp[-1].i, yyvsp[-2].i, yyvsp[0].i); }
    break;

  case 105:
#line 331 "grammar.y"
    { yyval.rblk = gen_inbound(0); }
    break;

  case 106:
#line 332 "grammar.y"
    { yyval.rblk = gen_inbound(1); }
    break;

  case 107:
#line 333 "grammar.y"
    { yyval.rblk = gen_vlan(yyvsp[0].i); }
    break;

  case 108:
#line 334 "grammar.y"
    { yyval.rblk = gen_vlan(-1); }
    break;

  case 109:
#line 335 "grammar.y"
    { yyval.rblk = gen_mpls(yyvsp[0].i); }
    break;

  case 110:
#line 336 "grammar.y"
    { yyval.rblk = gen_mpls(-1); }
    break;

  case 111:
#line 337 "grammar.y"
    { yyval.rblk = gen_pppoed(); }
    break;

  case 112:
#line 338 "grammar.y"
    { yyval.rblk = gen_pppoes(); }
    break;

  case 113:
#line 339 "grammar.y"
    { yyval.rblk = yyvsp[0].rblk; }
    break;

  case 114:
#line 342 "grammar.y"
    { yyval.rblk = gen_pf_ifname(yyvsp[0].s); }
    break;

  case 115:
#line 343 "grammar.y"
    { yyval.rblk = gen_pf_ruleset(yyvsp[0].s); }
    break;

  case 116:
#line 344 "grammar.y"
    { yyval.rblk = gen_pf_rnr(yyvsp[0].i); }
    break;

  case 117:
#line 345 "grammar.y"
    { yyval.rblk = gen_pf_srnr(yyvsp[0].i); }
    break;

  case 118:
#line 346 "grammar.y"
    { yyval.rblk = gen_pf_reason(yyvsp[0].i); }
    break;

  case 119:
#line 347 "grammar.y"
    { yyval.rblk = gen_pf_action(yyvsp[0].i); }
    break;

  case 120:
#line 350 "grammar.y"
    { yyval.i = yyvsp[0].i; }
    break;

  case 121:
#line 351 "grammar.y"
    { const char *reasons[] = PFRES_NAMES;
				  int i;
				  for (i = 0; reasons[i]; i++) {
					  if (pcap_strcasecmp(yyvsp[0].s, reasons[i]) == 0) {
						  yyval.i = i;
						  break;
					  }
				  }
				  if (reasons[i] == NULL)
					  bpf_error("unknown PF reason");
				}
    break;

  case 122:
#line 364 "grammar.y"
    { if (pcap_strcasecmp(yyvsp[0].s, "pass") == 0 ||
				      pcap_strcasecmp(yyvsp[0].s, "accept") == 0)
					yyval.i = PF_PASS;
				  else if (pcap_strcasecmp(yyvsp[0].s, "drop") == 0 ||
				      pcap_strcasecmp(yyvsp[0].s, "block") == 0)
					yyval.i = PF_DROP;
				  else
					  bpf_error("unknown PF action");
				}
    break;

  case 123:
#line 375 "grammar.y"
    { yyval.i = BPF_JGT; }
    break;

  case 124:
#line 376 "grammar.y"
    { yyval.i = BPF_JGE; }
    break;

  case 125:
#line 377 "grammar.y"
    { yyval.i = BPF_JEQ; }
    break;

  case 126:
#line 379 "grammar.y"
    { yyval.i = BPF_JGT; }
    break;

  case 127:
#line 380 "grammar.y"
    { yyval.i = BPF_JGE; }
    break;

  case 128:
#line 381 "grammar.y"
    { yyval.i = BPF_JEQ; }
    break;

  case 129:
#line 383 "grammar.y"
    { yyval.a = gen_loadi(yyvsp[0].i); }
    break;

  case 131:
#line 386 "grammar.y"
    { yyval.a = gen_load(yyvsp[-3].i, yyvsp[-1].a, 1); }
    break;

  case 132:
#line 387 "grammar.y"
    { yyval.a = gen_load(yyvsp[-5].i, yyvsp[-3].a, yyvsp[-1].i); }
    break;

  case 133:
#line 388 "grammar.y"
    { yyval.a = gen_arth(BPF_ADD, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 134:
#line 389 "grammar.y"
    { yyval.a = gen_arth(BPF_SUB, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 135:
#line 390 "grammar.y"
    { yyval.a = gen_arth(BPF_MUL, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 136:
#line 391 "grammar.y"
    { yyval.a = gen_arth(BPF_DIV, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 137:
#line 392 "grammar.y"
    { yyval.a = gen_arth(BPF_AND, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 138:
#line 393 "grammar.y"
    { yyval.a = gen_arth(BPF_OR, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 139:
#line 394 "grammar.y"
    { yyval.a = gen_arth(BPF_LSH, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 140:
#line 395 "grammar.y"
    { yyval.a = gen_arth(BPF_RSH, yyvsp[-2].a, yyvsp[0].a); }
    break;

  case 141:
#line 396 "grammar.y"
    { yyval.a = gen_neg(yyvsp[0].a); }
    break;

  case 142:
#line 397 "grammar.y"
    { yyval.a = yyvsp[-1].a; }
    break;

  case 143:
#line 398 "grammar.y"
    { yyval.a = gen_loadlen(); }
    break;

  case 144:
#line 400 "grammar.y"
    { yyval.i = '&'; }
    break;

  case 145:
#line 401 "grammar.y"
    { yyval.i = '|'; }
    break;

  case 146:
#line 402 "grammar.y"
    { yyval.i = '<'; }
    break;

  case 147:
#line 403 "grammar.y"
    { yyval.i = '>'; }
    break;

  case 148:
#line 404 "grammar.y"
    { yyval.i = '='; }
    break;

  case 150:
#line 407 "grammar.y"
    { yyval.i = yyvsp[-1].i; }
    break;

  case 151:
#line 409 "grammar.y"
    { yyval.i = A_LANE; }
    break;

  case 152:
#line 410 "grammar.y"
    { yyval.i = A_LLC; }
    break;

  case 153:
#line 411 "grammar.y"
    { yyval.i = A_METAC;	}
    break;

  case 154:
#line 412 "grammar.y"
    { yyval.i = A_BCC; }
    break;

  case 155:
#line 413 "grammar.y"
    { yyval.i = A_OAMF4EC; }
    break;

  case 156:
#line 414 "grammar.y"
    { yyval.i = A_OAMF4SC; }
    break;

  case 157:
#line 415 "grammar.y"
    { yyval.i = A_SC; }
    break;

  case 158:
#line 416 "grammar.y"
    { yyval.i = A_ILMIC; }
    break;

  case 159:
#line 418 "grammar.y"
    { yyval.i = A_OAM; }
    break;

  case 160:
#line 419 "grammar.y"
    { yyval.i = A_OAMF4; }
    break;

  case 161:
#line 420 "grammar.y"
    { yyval.i = A_CONNECTMSG; }
    break;

  case 162:
#line 421 "grammar.y"
    { yyval.i = A_METACONNECT; }
    break;

  case 163:
#line 424 "grammar.y"
    { yyval.blk.atmfieldtype = A_VPI; }
    break;

  case 164:
#line 425 "grammar.y"
    { yyval.blk.atmfieldtype = A_VCI; }
    break;

  case 166:
#line 428 "grammar.y"
    { yyval.blk.b = gen_atmfield_code(yyvsp[-2].blk.atmfieldtype, (bpf_int32)yyvsp[0].i, (bpf_u_int32)yyvsp[-1].i, 0); }
    break;

  case 167:
#line 429 "grammar.y"
    { yyval.blk.b = gen_atmfield_code(yyvsp[-2].blk.atmfieldtype, (bpf_int32)yyvsp[0].i, (bpf_u_int32)yyvsp[-1].i, 1); }
    break;

  case 168:
#line 430 "grammar.y"
    { yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = qerr; }
    break;

  case 169:
#line 432 "grammar.y"
    {
	yyval.blk.atmfieldtype = yyvsp[-1].blk.atmfieldtype;
	if (yyval.blk.atmfieldtype == A_VPI ||
	    yyval.blk.atmfieldtype == A_VCI)
		yyval.blk.b = gen_atmfield_code(yyval.blk.atmfieldtype, (bpf_int32) yyvsp[0].i, BPF_JEQ, 0);
	}
    break;

  case 171:
#line 440 "grammar.y"
    { gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;

  case 172:
#line 443 "grammar.y"
    { yyval.blk.mtp3fieldtype = M_SIO; }
    break;

  case 173:
#line 444 "grammar.y"
    { yyval.blk.mtp3fieldtype = M_OPC; }
    break;

  case 174:
#line 445 "grammar.y"
    { yyval.blk.mtp3fieldtype = M_DPC; }
    break;

  case 175:
#line 446 "grammar.y"
    { yyval.blk.mtp3fieldtype = M_SLS; }
    break;

  case 177:
#line 449 "grammar.y"
    { yyval.blk.b = gen_mtp3field_code(yyvsp[-2].blk.mtp3fieldtype, (u_int)yyvsp[0].i, (u_int)yyvsp[-1].i, 0); }
    break;

  case 178:
#line 450 "grammar.y"
    { yyval.blk.b = gen_mtp3field_code(yyvsp[-2].blk.mtp3fieldtype, (u_int)yyvsp[0].i, (u_int)yyvsp[-1].i, 1); }
    break;

  case 179:
#line 451 "grammar.y"
    { yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = qerr; }
    break;

  case 180:
#line 453 "grammar.y"
    {
	yyval.blk.mtp3fieldtype = yyvsp[-1].blk.mtp3fieldtype;
	if (yyval.blk.mtp3fieldtype == M_SIO ||
	    yyval.blk.mtp3fieldtype == M_OPC ||
	    yyval.blk.mtp3fieldtype == M_DPC ||
	    yyval.blk.mtp3fieldtype == M_SLS )
		yyval.blk.b = gen_mtp3field_code(yyval.blk.mtp3fieldtype, (u_int) yyvsp[0].i, BPF_JEQ, 0);
	}
    break;

  case 182:
#line 463 "grammar.y"
    { gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;


    }

/* Line 991 of yacc.c.  */
#line 2529 "y.tab.c"

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
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 465 "grammar.y"


