/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FORWARD = 258,
     FORWARD_TCP = 259,
     FORWARD_TLS = 260,
     FORWARD_UDP = 261,
     SEND = 262,
     SEND_TCP = 263,
     DROP = 264,
     LOG_TOK = 265,
     ERROR = 266,
     ROUTE = 267,
     ROUTE_FAILURE = 268,
     ROUTE_ONREPLY = 269,
     EXEC = 270,
     SET_HOST = 271,
     SET_HOSTPORT = 272,
     PREFIX = 273,
     STRIP = 274,
     STRIP_TAIL = 275,
     APPEND_BRANCH = 276,
     SET_USER = 277,
     SET_USERPASS = 278,
     SET_PORT = 279,
     SET_URI = 280,
     REVERT_URI = 281,
     FORCE_RPORT = 282,
     IF = 283,
     ELSE = 284,
     SET_ADV_ADDRESS = 285,
     SET_ADV_PORT = 286,
     URIHOST = 287,
     URIPORT = 288,
     MAX_LEN = 289,
     SETFLAG = 290,
     RESETFLAG = 291,
     ISFLAGSET = 292,
     METHOD = 293,
     URI = 294,
     SRCIP = 295,
     SRCPORT = 296,
     DSTIP = 297,
     DSTPORT = 298,
     PROTO = 299,
     AF = 300,
     MYSELF = 301,
     MSGLEN = 302,
     DEBUG = 303,
     FORK = 304,
     LOGSTDERROR = 305,
     LOGFACILITY = 306,
     LISTEN = 307,
     ALIAS = 308,
     DNS = 309,
     REV_DNS = 310,
     PORT = 311,
     STAT = 312,
     CHILDREN = 313,
     CHECK_VIA = 314,
     SYN_BRANCH = 315,
     MEMLOG = 316,
     SIP_WARNING = 317,
     FIFO = 318,
     FIFO_MODE = 319,
     SERVER_SIGNATURE = 320,
     REPLY_TO_VIA = 321,
     LOADMODULE = 322,
     MODPARAM = 323,
     MAXBUFFER = 324,
     USER = 325,
     GROUP = 326,
     CHROOT = 327,
     WDIR = 328,
     MHOMED = 329,
     DISABLE_TCP = 330,
     TCP_CHILDREN = 331,
     TCP_CONNECT_TIMEOUT = 332,
     TCP_SEND_TIMEOUT = 333,
     DISABLE_TLS = 334,
     TLSLOG = 335,
     TLS_PORT_NO = 336,
     TLS_METHOD = 337,
     TLS_HANDSHAKE_TIMEOUT = 338,
     TLS_SEND_TIMEOUT = 339,
     SSLv23 = 340,
     SSLv2 = 341,
     SSLv3 = 342,
     TLSv1 = 343,
     TLS_VERIFY = 344,
     TLS_REQUIRE_CERTIFICATE = 345,
     TLS_CERTIFICATE = 346,
     TLS_PRIVATE_KEY = 347,
     TLS_CA_LIST = 348,
     ADVERTISED_ADDRESS = 349,
     ADVERTISED_PORT = 350,
     EQUAL = 351,
     EQUAL_T = 352,
     GT = 353,
     LT = 354,
     GTE = 355,
     LTE = 356,
     DIFF = 357,
     MATCH = 358,
     OR = 359,
     AND = 360,
     NOT = 361,
     NUMBER = 362,
     ID = 363,
     STRING = 364,
     IPV6ADDR = 365,
     COMMA = 366,
     SEMICOLON = 367,
     RPAREN = 368,
     LPAREN = 369,
     LBRACE = 370,
     RBRACE = 371,
     LBRACK = 372,
     RBRACK = 373,
     SLASH = 374,
     DOT = 375,
     CR = 376
   };
#endif
#define FORWARD 258
#define FORWARD_TCP 259
#define FORWARD_TLS 260
#define FORWARD_UDP 261
#define SEND 262
#define SEND_TCP 263
#define DROP 264
#define LOG_TOK 265
#define ERROR 266
#define ROUTE 267
#define ROUTE_FAILURE 268
#define ROUTE_ONREPLY 269
#define EXEC 270
#define SET_HOST 271
#define SET_HOSTPORT 272
#define PREFIX 273
#define STRIP 274
#define STRIP_TAIL 275
#define APPEND_BRANCH 276
#define SET_USER 277
#define SET_USERPASS 278
#define SET_PORT 279
#define SET_URI 280
#define REVERT_URI 281
#define FORCE_RPORT 282
#define IF 283
#define ELSE 284
#define SET_ADV_ADDRESS 285
#define SET_ADV_PORT 286
#define URIHOST 287
#define URIPORT 288
#define MAX_LEN 289
#define SETFLAG 290
#define RESETFLAG 291
#define ISFLAGSET 292
#define METHOD 293
#define URI 294
#define SRCIP 295
#define SRCPORT 296
#define DSTIP 297
#define DSTPORT 298
#define PROTO 299
#define AF 300
#define MYSELF 301
#define MSGLEN 302
#define DEBUG 303
#define FORK 304
#define LOGSTDERROR 305
#define LOGFACILITY 306
#define LISTEN 307
#define ALIAS 308
#define DNS 309
#define REV_DNS 310
#define PORT 311
#define STAT 312
#define CHILDREN 313
#define CHECK_VIA 314
#define SYN_BRANCH 315
#define MEMLOG 316
#define SIP_WARNING 317
#define FIFO 318
#define FIFO_MODE 319
#define SERVER_SIGNATURE 320
#define REPLY_TO_VIA 321
#define LOADMODULE 322
#define MODPARAM 323
#define MAXBUFFER 324
#define USER 325
#define GROUP 326
#define CHROOT 327
#define WDIR 328
#define MHOMED 329
#define DISABLE_TCP 330
#define TCP_CHILDREN 331
#define TCP_CONNECT_TIMEOUT 332
#define TCP_SEND_TIMEOUT 333
#define DISABLE_TLS 334
#define TLSLOG 335
#define TLS_PORT_NO 336
#define TLS_METHOD 337
#define TLS_HANDSHAKE_TIMEOUT 338
#define TLS_SEND_TIMEOUT 339
#define SSLv23 340
#define SSLv2 341
#define SSLv3 342
#define TLSv1 343
#define TLS_VERIFY 344
#define TLS_REQUIRE_CERTIFICATE 345
#define TLS_CERTIFICATE 346
#define TLS_PRIVATE_KEY 347
#define TLS_CA_LIST 348
#define ADVERTISED_ADDRESS 349
#define ADVERTISED_PORT 350
#define EQUAL 351
#define EQUAL_T 352
#define GT 353
#define LT 354
#define GTE 355
#define LTE 356
#define DIFF 357
#define MATCH 358
#define OR 359
#define AND 360
#define NOT 361
#define NUMBER 362
#define ID 363
#define STRING 364
#define IPV6ADDR 365
#define COMMA 366
#define SEMICOLON 367
#define RPAREN 368
#define LPAREN 369
#define LBRACE 370
#define RBRACE 371
#define LBRACK 372
#define RBRACK 373
#define SLASH 374
#define DOT 375
#define CR 376




/* Copy the first part of user declarations.  */
#line 55 "cfg.y"


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "route_struct.h"
#include "globals.h"
#include "route.h"
#include "dprint.h"
#include "sr_module.h"
#include "modparam.h"
#include "ip_addr.h"
#include "resolve.h"
#include "name_alias.h"
#include "ut.h"


#include "config.h"
#ifdef USE_TLS
#include "tls/tls_config.h"
#endif

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

/* hack to avoid alloca usage in the generated C file (needed for compiler
 with no built in alloca, like icc*/
#undef _ALLOCA_H

struct id_list{
	char* s;
	struct id_list* next;
};

extern int yylex();
static void yyerror(char* s);
static char* tmp;
static int i_tmp;
static void* f_tmp;
static struct id_list* lst_tmp;
static int rt;  /* Type of route block for find_export */
static str* str_tmp;
static str s_tmp;
static struct ip_addr* ip_tmp;

static void warn(char* s);
 



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
#line 111 "cfg.y"
typedef union YYSTYPE {
	long intval;
	unsigned long uval;
	char* strval;
	struct expr* expr;
	struct action* action;
	struct net* ipnet;
	struct ip_addr* ipaddr;
	struct id_list* idlst;
} YYSTYPE;
/* Line 190 of yacc.c.  */
#line 385 "cfg.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 397 "cfg.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
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
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
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
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  102
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1042

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  122
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  29
/* YYNRULES -- Number of rules. */
#define YYNRULES  326
/* YYNRULES -- Number of states. */
#define YYNSTATES  656

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   376

#define YYTRANSLATE(YYX) 						\
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    13,    15,    17,    18,
      21,    22,    25,    26,    29,    31,    33,    35,    37,    39,
      42,    46,    50,    54,    58,    62,    66,    70,    74,    78,
      82,    86,    90,    94,    98,   102,   106,   110,   114,   118,
     122,   126,   130,   134,   138,   142,   146,   150,   154,   158,
     162,   166,   170,   174,   178,   182,   186,   190,   194,   198,
     202,   206,   210,   214,   218,   222,   226,   230,   234,   238,
     242,   246,   250,   254,   258,   262,   266,   270,   274,   278,
     282,   286,   290,   294,   298,   302,   306,   310,   314,   318,
     322,   326,   330,   334,   338,   342,   346,   350,   354,   358,
     362,   366,   370,   374,   378,   382,   386,   390,   394,   398,
     402,   405,   408,   411,   420,   429,   432,   434,   436,   444,
     446,   451,   459,   462,   470,   473,   481,   484,   488,   492,
     495,   499,   501,   503,   505,   507,   509,   511,   513,   515,
     517,   519,   523,   527,   531,   534,   538,   542,   546,   550,
     553,   557,   561,   564,   568,   572,   575,   579,   583,   586,
     590,   594,   597,   601,   605,   609,   612,   616,   620,   624,
     628,   632,   635,   639,   643,   647,   651,   655,   658,   662,
     666,   670,   674,   677,   679,   681,   685,   689,   691,   695,
     697,   701,   705,   707,   709,   713,   716,   718,   721,   724,
     726,   728,   731,   735,   741,   746,   751,   756,   763,   770,
     777,   784,   791,   796,   799,   804,   809,   814,   819,   826,
     833,   840,   847,   854,   859,   862,   867,   872,   877,   882,
     889,   896,   903,   910,   917,   922,   925,   930,   935,   940,
     945,   952,   959,   966,   973,   980,   985,   988,   993,   998,
    1003,  1008,  1015,  1022,  1029,  1032,  1037,  1042,  1047,  1052,
    1059,  1066,  1073,  1076,  1081,  1085,  1087,  1092,  1099,  1102,
    1107,  1112,  1115,  1120,  1123,  1128,  1131,  1138,  1141,  1146,
    1151,  1154,  1159,  1164,  1169,  1172,  1177,  1182,  1185,  1190,
    1195,  1198,  1203,  1208,  1211,  1216,  1221,  1225,  1227,  1232,
    1235,  1240,  1245,  1248,  1253,  1258,  1261,  1266,  1271,  1274,
    1279,  1284,  1287,  1292,  1296,  1298,  1302,  1304,  1309,  1314,
    1317,  1322,  1327,  1330,  1334,  1339,  1346
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     123,     0,    -1,   124,    -1,   124,   125,    -1,   125,    -1,
     124,     1,    -1,   131,    -1,   132,    -1,    -1,   126,   136,
      -1,    -1,   127,   137,    -1,    -1,   128,   138,    -1,   121,
      -1,   133,    -1,   109,    -1,   145,    -1,   129,    -1,   129,
     130,    -1,    48,    96,   107,    -1,    48,    96,     1,    -1,
      49,    96,   107,    -1,    49,    96,     1,    -1,    50,    96,
     107,    -1,    50,    96,     1,    -1,    51,    96,   108,    -1,
      51,    96,     1,    -1,    54,    96,   107,    -1,    54,    96,
       1,    -1,    55,    96,   107,    -1,    55,    96,     1,    -1,
      56,    96,   107,    -1,    57,    96,   109,    -1,    69,    96,
     107,    -1,    69,    96,     1,    -1,    56,    96,     1,    -1,
      58,    96,   107,    -1,    58,    96,     1,    -1,    59,    96,
     107,    -1,    59,    96,     1,    -1,    60,    96,   107,    -1,
      60,    96,     1,    -1,    61,    96,   107,    -1,    61,    96,
       1,    -1,    62,    96,   107,    -1,    62,    96,     1,    -1,
      63,    96,   109,    -1,    63,    96,     1,    -1,    64,    96,
     107,    -1,    64,    96,     1,    -1,    70,    96,   109,    -1,
      70,    96,   108,    -1,    70,    96,     1,    -1,    71,    96,
     109,    -1,    71,    96,   108,    -1,    71,    96,     1,    -1,
      72,    96,   109,    -1,    72,    96,   108,    -1,    72,    96,
       1,    -1,    73,    96,   109,    -1,    73,    96,   108,    -1,
      73,    96,     1,    -1,    74,    96,   107,    -1,    74,    96,
       1,    -1,    75,    96,   107,    -1,    75,    96,     1,    -1,
      76,    96,   107,    -1,    76,    96,     1,    -1,    77,    96,
     107,    -1,    77,    96,     1,    -1,    78,    96,   107,    -1,
      78,    96,     1,    -1,    79,    96,   107,    -1,    79,    96,
       1,    -1,    80,    96,   107,    -1,    80,    96,     1,    -1,
      81,    96,   107,    -1,    81,    96,     1,    -1,    82,    96,
      85,    -1,    82,    96,    86,    -1,    82,    96,    87,    -1,
      82,    96,    88,    -1,    82,    96,     1,    -1,    89,    96,
     107,    -1,    89,    96,     1,    -1,    90,    96,   107,    -1,
      90,    96,     1,    -1,    91,    96,   109,    -1,    91,    96,
       1,    -1,    92,    96,   109,    -1,    92,    96,     1,    -1,
      93,    96,   109,    -1,    93,    96,     1,    -1,    83,    96,
     107,    -1,    83,    96,     1,    -1,    84,    96,   107,    -1,
      84,    96,     1,    -1,    65,    96,   107,    -1,    65,    96,
       1,    -1,    66,    96,   107,    -1,    66,    96,     1,    -1,
      52,    96,   130,    -1,    52,    96,     1,    -1,    53,    96,
     130,    -1,    53,    96,     1,    -1,    94,    96,   129,    -1,
      94,    96,     1,    -1,    95,    96,   107,    -1,    95,    96,
       1,    -1,     1,    96,    -1,    67,   109,    -1,    67,     1,
      -1,    68,   114,   109,   111,   109,   111,   109,   113,    -1,
      68,   114,   109,   111,   109,   111,   107,   113,    -1,    68,
       1,    -1,   134,    -1,   135,    -1,   107,   120,   107,   120,
     107,   120,   107,    -1,   110,    -1,    12,   115,   147,   116,
      -1,    12,   117,   107,   118,   115,   147,   116,    -1,    12,
       1,    -1,    13,   117,   107,   118,   115,   147,   116,    -1,
      13,     1,    -1,    14,   117,   107,   118,   115,   147,   116,
      -1,    14,     1,    -1,   139,   105,   139,    -1,   139,   104,
     139,    -1,   106,   139,    -1,   114,   139,   113,    -1,   143,
      -1,    97,    -1,   102,    -1,   140,    -1,    98,    -1,    99,
      -1,   100,    -1,   101,    -1,   140,    -1,   103,    -1,    38,
     142,   109,    -1,    38,   142,   108,    -1,    38,   142,     1,
      -1,    38,     1,    -1,    39,   142,   109,    -1,    39,   142,
     145,    -1,    39,   140,    46,    -1,    39,   142,     1,    -1,
      39,     1,    -1,    41,   141,   107,    -1,    41,   141,     1,
      -1,    41,     1,    -1,    43,   141,   107,    -1,    43,   141,
       1,    -1,    43,     1,    -1,    44,   141,   107,    -1,    44,
     141,     1,    -1,    44,     1,    -1,    45,   141,   107,    -1,
      45,   141,     1,    -1,    45,     1,    -1,    47,   141,   107,
      -1,    47,   141,    34,    -1,    47,   141,     1,    -1,    47,
       1,    -1,    40,   140,   144,    -1,    40,   142,   109,    -1,
      40,   142,   145,    -1,    40,   140,    46,    -1,    40,   142,
       1,    -1,    40,     1,    -1,    42,   140,   144,    -1,    42,
     142,   109,    -1,    42,   142,   145,    -1,    42,   140,    46,
      -1,    42,   142,     1,    -1,    42,     1,    -1,    46,   140,
      39,    -1,    46,   140,    40,    -1,    46,   140,    42,    -1,
      46,   140,     1,    -1,    46,     1,    -1,   146,    -1,   107,
      -1,   133,   119,   133,    -1,   133,   119,   107,    -1,   133,
      -1,   133,   119,     1,    -1,   108,    -1,   145,   120,   108,
      -1,   145,   120,     1,    -1,   150,    -1,   149,    -1,   115,
     147,   116,    -1,   147,   148,    -1,   148,    -1,   147,     1,
      -1,   150,   112,    -1,   149,    -1,   112,    -1,   150,     1,
      -1,    28,   139,   146,    -1,    28,   139,   146,    29,   146,
      -1,     3,   114,   145,   113,    -1,     3,   114,   109,   113,
      -1,     3,   114,   133,   113,    -1,     3,   114,   145,   111,
     107,   113,    -1,     3,   114,   109,   111,   107,   113,    -1,
       3,   114,   133,   111,   107,   113,    -1,     3,   114,    32,
     111,    33,   113,    -1,     3,   114,    32,   111,   107,   113,
      -1,     3,   114,    32,   113,    -1,     3,     1,    -1,     3,
     114,     1,   113,    -1,     6,   114,   145,   113,    -1,     6,
     114,   109,   113,    -1,     6,   114,   133,   113,    -1,     6,
     114,   145,   111,   107,   113,    -1,     6,   114,   109,   111,
     107,   113,    -1,     6,   114,   133,   111,   107,   113,    -1,
       6,   114,    32,   111,    33,   113,    -1,     6,   114,    32,
     111,   107,   113,    -1,     6,   114,    32,   113,    -1,     6,
       1,    -1,     6,   114,     1,   113,    -1,     4,   114,   145,
     113,    -1,     4,   114,   109,   113,    -1,     4,   114,   133,
     113,    -1,     4,   114,   145,   111,   107,   113,    -1,     4,
     114,   109,   111,   107,   113,    -1,     4,   114,   133,   111,
     107,   113,    -1,     4,   114,    32,   111,    33,   113,    -1,
       4,   114,    32,   111,   107,   113,    -1,     4,   114,    32,
     113,    -1,     4,     1,    -1,     4,   114,     1,   113,    -1,
       5,   114,   145,   113,    -1,     5,   114,   109,   113,    -1,
       5,   114,   133,   113,    -1,     5,   114,   145,   111,   107,
     113,    -1,     5,   114,   109,   111,   107,   113,    -1,     5,
     114,   133,   111,   107,   113,    -1,     5,   114,    32,   111,
      33,   113,    -1,     5,   114,    32,   111,   107,   113,    -1,
       5,   114,    32,   113,    -1,     5,     1,    -1,     5,   114,
       1,   113,    -1,     7,   114,   145,   113,    -1,     7,   114,
     109,   113,    -1,     7,   114,   133,   113,    -1,     7,   114,
     145,   111,   107,   113,    -1,     7,   114,   109,   111,   107,
     113,    -1,     7,   114,   133,   111,   107,   113,    -1,     7,
       1,    -1,     7,   114,     1,   113,    -1,     8,   114,   145,
     113,    -1,     8,   114,   109,   113,    -1,     8,   114,   133,
     113,    -1,     8,   114,   145,   111,   107,   113,    -1,     8,
     114,   109,   111,   107,   113,    -1,     8,   114,   133,   111,
     107,   113,    -1,     8,     1,    -1,     8,   114,     1,   113,
      -1,     9,   114,   113,    -1,     9,    -1,    10,   114,   109,
     113,    -1,    10,   114,   107,   111,   109,   113,    -1,    10,
       1,    -1,    10,   114,     1,   113,    -1,    35,   114,   107,
     113,    -1,    35,     1,    -1,    36,   114,   107,   113,    -1,
      36,     1,    -1,    37,   114,   107,   113,    -1,    37,     1,
      -1,    11,   114,   109,   111,   109,   113,    -1,    11,     1,
      -1,    11,   114,     1,   113,    -1,    12,   114,   107,   113,
      -1,    12,     1,    -1,    12,   114,     1,   113,    -1,    15,
     114,   109,   113,    -1,    16,   114,   109,   113,    -1,    16,
       1,    -1,    16,   114,     1,   113,    -1,    18,   114,   109,
     113,    -1,    18,     1,    -1,    18,   114,     1,   113,    -1,
      20,   114,   107,   113,    -1,    20,     1,    -1,    20,   114,
       1,   113,    -1,    19,   114,   107,   113,    -1,    19,     1,
      -1,    19,   114,     1,   113,    -1,    21,   114,   109,   113,
      -1,    21,   114,   113,    -1,    21,    -1,    17,   114,   109,
     113,    -1,    17,     1,    -1,    17,   114,     1,   113,    -1,
      24,   114,   109,   113,    -1,    24,     1,    -1,    24,   114,
       1,   113,    -1,    22,   114,   109,   113,    -1,    22,     1,
      -1,    22,   114,     1,   113,    -1,    23,   114,   109,   113,
      -1,    23,     1,    -1,    23,   114,     1,   113,    -1,    25,
     114,   109,   113,    -1,    25,     1,    -1,    25,   114,     1,
     113,    -1,    26,   114,   113,    -1,    26,    -1,    27,   114,
     113,    -1,    27,    -1,    30,   114,   129,   113,    -1,    30,
     114,     1,   113,    -1,    30,     1,    -1,    31,   114,   107,
     113,    -1,    31,   114,     1,   113,    -1,    31,     1,    -1,
     108,   114,   113,    -1,   108,   114,   109,   113,    -1,   108,
     114,   109,   111,   109,   113,    -1,   108,   114,     1,   113,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   277,   277,   280,   281,   282,   285,   286,   287,   287,
     288,   288,   289,   289,   291,   294,   309,   317,   327,   335,
     347,   348,   349,   350,   351,   352,   353,   359,   360,   361,
     362,   363,   364,   368,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   411,   412,   419,   420,
     427,   428,   435,   436,   443,   444,   451,   452,   459,   460,
     467,   474,   481,   488,   497,   504,   505,   512,   514,   521,
     522,   529,   530,   537,   538,   545,   546,   553,   554,   555,
     556,   557,   558,   584,   586,   590,   591,   595,   597,   610,
     612,   615,   620,   621,   626,   631,   635,   636,   639,   673,
     694,   696,   704,   707,   715,   718,   726,   748,   749,   750,
     751,   752,   755,   756,   759,   760,   761,   762,   763,   766,
     767,   770,   773,   776,   777,   780,   783,   786,   789,   790,
     793,   795,   796,   797,   799,   800,   801,   803,   804,   805,
     807,   808,   809,   811,   813,   814,   815,   818,   832,   835,
     838,   840,   842,   845,   859,   862,   865,   867,   869,   872,
     875,   878,   880,   883,   884,   887,   888,   899,   900,   905,
     906,   918,   922,   923,   924,   927,   928,   929,   932,   933,
     934,   935,   938,   946,   956,   962,   968,   974,   980,   986,
     992,  1001,  1008,  1015,  1016,  1018,  1024,  1030,  1036,  1043,
    1050,  1057,  1066,  1073,  1080,  1081,  1083,  1089,  1095,  1101,
    1108,  1115,  1121,  1130,  1137,  1144,  1145,  1147,  1160,  1173,
    1186,  1199,  1212,  1225,  1240,  1253,  1266,  1267,  1270,  1276,
    1282,  1288,  1294,  1300,  1306,  1307,  1309,  1315,  1321,  1327,
    1333,  1339,  1345,  1346,  1348,  1349,  1350,  1353,  1359,  1360,
    1362,  1364,  1365,  1367,  1368,  1370,  1371,  1377,  1378,  1380,
    1383,  1384,  1386,  1389,  1391,  1392,  1395,  1397,  1398,  1400,
    1402,  1403,  1406,  1408,  1409,  1412,  1414,  1416,  1418,  1420,
    1421,  1423,  1425,  1426,  1428,  1430,  1431,  1433,  1435,  1436,
    1438,  1440,  1441,  1443,  1444,  1445,  1446,  1447,  1459,  1461,
    1462,  1480,  1482,  1483,  1501,  1519,  1540
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FORWARD", "FORWARD_TCP", "FORWARD_TLS",
  "FORWARD_UDP", "SEND", "SEND_TCP", "DROP", "LOG_TOK", "ERROR", "ROUTE",
  "ROUTE_FAILURE", "ROUTE_ONREPLY", "EXEC", "SET_HOST", "SET_HOSTPORT",
  "PREFIX", "STRIP", "STRIP_TAIL", "APPEND_BRANCH", "SET_USER",
  "SET_USERPASS", "SET_PORT", "SET_URI", "REVERT_URI", "FORCE_RPORT", "IF",
  "ELSE", "SET_ADV_ADDRESS", "SET_ADV_PORT", "URIHOST", "URIPORT",
  "MAX_LEN", "SETFLAG", "RESETFLAG", "ISFLAGSET", "METHOD", "URI", "SRCIP",
  "SRCPORT", "DSTIP", "DSTPORT", "PROTO", "AF", "MYSELF", "MSGLEN",
  "DEBUG", "FORK", "LOGSTDERROR", "LOGFACILITY", "LISTEN", "ALIAS", "DNS",
  "REV_DNS", "PORT", "STAT", "CHILDREN", "CHECK_VIA", "SYN_BRANCH",
  "MEMLOG", "SIP_WARNING", "FIFO", "FIFO_MODE", "SERVER_SIGNATURE",
  "REPLY_TO_VIA", "LOADMODULE", "MODPARAM", "MAXBUFFER", "USER", "GROUP",
  "CHROOT", "WDIR", "MHOMED", "DISABLE_TCP", "TCP_CHILDREN",
  "TCP_CONNECT_TIMEOUT", "TCP_SEND_TIMEOUT", "DISABLE_TLS", "TLSLOG",
  "TLS_PORT_NO", "TLS_METHOD", "TLS_HANDSHAKE_TIMEOUT", "TLS_SEND_TIMEOUT",
  "SSLv23", "SSLv2", "SSLv3", "TLSv1", "TLS_VERIFY",
  "TLS_REQUIRE_CERTIFICATE", "TLS_CERTIFICATE", "TLS_PRIVATE_KEY",
  "TLS_CA_LIST", "ADVERTISED_ADDRESS", "ADVERTISED_PORT", "EQUAL",
  "EQUAL_T", "GT", "LT", "GTE", "LTE", "DIFF", "MATCH", "OR", "AND", "NOT",
  "NUMBER", "ID", "STRING", "IPV6ADDR", "COMMA", "SEMICOLON", "RPAREN",
  "LPAREN", "LBRACE", "RBRACE", "LBRACK", "RBRACK", "SLASH", "DOT", "CR",
  "$accept", "cfg", "statements", "statement", "@1", "@2", "@3",
  "listen_id", "id_lst", "assign_stm", "module_stm", "ip", "ipv4", "ipv6",
  "route_stm", "failure_route_stm", "onreply_route_stm", "exp", "equalop",
  "intop", "strop", "exp_elem", "ipnet", "host", "stm", "actions",
  "action", "if_cmd", "cmd", 0
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
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   122,   123,   124,   124,   124,   125,   125,   126,   125,
     127,   125,   128,   125,   125,   129,   129,   129,   130,   130,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   132,   132,   132,   132,   132,   133,   133,   134,   135,
     136,   136,   136,   137,   137,   138,   138,   139,   139,   139,
     139,   139,   140,   140,   141,   141,   141,   141,   141,   142,
     142,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   144,   144,   144,   144,   145,
     145,   145,   146,   146,   146,   147,   147,   147,   148,   148,
     148,   148,   149,   149,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     1,     0,     2,
       0,     2,     0,     2,     1,     1,     1,     1,     1,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     8,     8,     2,     1,     1,     7,     1,
       4,     7,     2,     7,     2,     7,     2,     3,     3,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     2,     3,     3,     3,     3,     2,
       3,     3,     2,     3,     3,     2,     3,     3,     2,     3,
       3,     2,     3,     3,     3,     2,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     2,     3,     3,
       3,     3,     2,     1,     1,     3,     3,     1,     3,     1,
       3,     3,     1,     1,     3,     2,     1,     2,     2,     1,
       1,     2,     3,     5,     4,     4,     4,     6,     6,     6,
       6,     6,     4,     2,     4,     4,     4,     4,     6,     6,
       6,     6,     6,     4,     2,     4,     4,     4,     4,     6,
       6,     6,     6,     6,     4,     2,     4,     4,     4,     4,
       6,     6,     6,     6,     6,     4,     2,     4,     4,     4,
       4,     6,     6,     6,     2,     4,     4,     4,     4,     6,
       6,     6,     2,     4,     3,     1,     4,     6,     2,     4,
       4,     2,     4,     2,     4,     2,     6,     2,     4,     4,
       2,     4,     4,     4,     2,     4,     4,     2,     4,     4,
       2,     4,     4,     2,     4,     4,     3,     1,     4,     2,
       4,     4,     2,     4,     4,     2,     4,     4,     2,     4,
       4,     2,     4,     3,     1,     3,     1,     4,     4,     2,
       4,     4,     2,     3,     4,     6,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,     0,     4,
       0,     0,     0,     6,     7,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   112,   111,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     5,     3,     0,     9,     0,    11,     0,
      13,    21,    20,    23,    22,    25,    24,    27,    26,   103,
       0,   189,    16,   119,    18,   102,    15,   116,   117,    17,
     105,   104,    29,    28,    31,    30,    36,    32,    33,    38,
      37,    40,    39,    42,    41,    44,    43,    46,    45,    48,
      47,    50,    49,    99,    98,   101,   100,     0,    35,    34,
      53,    52,    51,    56,    55,    54,    59,    58,    57,    62,
      61,    60,    64,    63,    66,    65,    68,    67,    70,    69,
      72,    71,    74,    73,    76,    75,    78,    77,    83,    79,
      80,    81,    82,    95,    94,    97,    96,    85,    84,    87,
      86,    89,    88,    91,    90,    93,    92,   107,   106,   109,
     108,   122,     0,     0,   124,     0,   126,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   297,     0,
       0,     0,     0,   314,   316,     0,     0,     0,     0,     0,
       0,     0,   200,     0,   196,   199,     0,     0,     0,     0,
       0,   191,   190,     0,   213,     0,   235,     0,   246,     0,
     224,     0,   254,     0,   262,     0,     0,   268,     0,   277,
       0,   280,     0,     0,   284,     0,   299,     0,   287,     0,
     293,     0,   290,     0,     0,   305,     0,   308,     0,   302,
       0,   311,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   184,     0,     0,     0,
     131,   183,   193,   192,   319,     0,   322,     0,   271,     0,
     273,     0,   275,     0,     0,   197,   120,   195,   201,   198,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   264,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   296,     0,     0,     0,     0,     0,     0,
       0,     0,   313,   315,   144,   132,   133,   140,   139,     0,
     149,   139,     0,   171,   139,     0,   152,   135,   136,   137,
     138,   134,     0,   177,   139,     0,   155,     0,   158,     0,
     161,     0,   182,     0,   165,     0,   129,     0,     0,     0,
       0,   202,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   323,     0,     0,     0,     0,     0,     0,   214,     0,
     212,     0,   205,     0,   206,     0,   204,   236,     0,   234,
       0,   227,     0,   228,     0,   226,   247,     0,   245,     0,
     238,     0,   239,     0,   237,   225,     0,   223,     0,   216,
       0,   217,     0,   215,   255,     0,   249,     0,   250,     0,
     248,   263,     0,   257,     0,   258,     0,   256,   269,     0,
     266,   278,     0,   281,   279,   282,   285,   283,   300,   298,
     288,   286,   294,   292,   291,   289,   295,   306,   304,   309,
     307,   303,   301,   312,   310,   143,   142,   141,   147,   148,
     145,   146,   169,   187,   166,   170,   167,   168,   151,   150,
     175,   172,   176,   173,   174,   154,   153,   157,   156,   160,
     159,   181,   178,   179,   180,   164,   163,   162,   130,   194,
     128,   127,     0,   318,   317,   321,   320,   270,   272,   274,
     326,     0,   324,     0,     0,     0,     0,   114,   113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   203,     0,
     121,   123,   125,   118,   210,   211,   208,   209,   207,   232,
     233,   230,   231,   229,   243,   244,   241,   242,   240,   221,
     222,   219,   220,   218,   252,   253,   251,   260,   261,   259,
     267,   276,   188,   186,   185,   325
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    47,    48,    49,    50,    51,    52,   124,   125,    53,
      54,   126,   127,   128,   106,   108,   110,   319,   421,   422,
     409,   320,   544,   129,   321,   253,   254,   255,   256
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -315
static const short int yypact[] =
{
     213,   -73,   -62,   -51,   -38,   -33,   -32,   -27,   -24,   -18,
     103,   126,   137,   139,   146,   154,   159,   242,   329,   330,
     360,   220,    21,   365,   374,   385,   386,   398,   521,   537,
     541,   544,   548,   551,   566,   569,   571,   577,   578,   581,
     594,   614,   615,   685,   709,   714,  -315,    33,    36,  -315,
     112,   188,   453,  -315,  -315,  -315,   233,   334,   370,    73,
      70,    75,   497,   498,   499,   362,   500,   501,   502,   503,
     504,   388,   505,   506,   507,  -315,  -315,  -315,   527,   508,
     201,   219,   336,   366,   509,   512,   513,   517,   520,   522,
     524,   525,   161,   538,   539,   554,   557,   406,   407,   408,
      81,   602,  -315,   -73,  -315,    17,  -315,    19,  -315,    23,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
     515,  -315,  -315,  -315,   680,  -315,  -315,  -315,  -315,   631,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,   510,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,   870,   511,  -315,   519,  -315,   545,   552,  -315,
      79,   658,    27,    28,    29,    30,    31,    37,   651,    38,
      39,    40,   662,    41,    42,    46,    50,    51,   700,    52,
      53,    54,    55,   703,   706,   677,    56,    58,    59,    60,
      61,   707,  -315,   338,  -315,  -315,    64,   701,   704,   710,
     696,  -315,  -315,   712,  -315,   191,  -315,   204,  -315,   208,
    -315,   212,  -315,   121,  -315,   216,   716,  -315,   369,  -315,
     413,  -315,   629,   740,  -315,   450,  -315,   451,  -315,   467,
    -315,   647,  -315,   648,    98,  -315,   468,  -315,   487,  -315,
     488,  -315,   492,   751,   754,   667,   669,   671,   734,   705,
     811,   817,   823,   122,   847,   677,  -315,   677,   870,   722,
    -315,  -315,  -315,  -315,  -315,   350,  -315,   649,  -315,   718,
    -315,   761,  -315,   762,    24,  -315,  -315,  -315,  -315,  -315,
     768,   769,   784,   795,   -63,   790,    83,    99,   105,    95,
     791,   379,   530,   540,   132,   813,   547,   565,   664,   140,
     814,   682,   683,   686,   342,   815,   687,   690,   352,   816,
     693,   698,   353,  -315,   818,   819,   820,   821,   824,   825,
     826,   827,   828,   829,   838,   839,   840,   841,   842,   843,
     844,   845,   846,  -315,   848,   849,   850,   851,   852,   853,
     854,   855,  -315,  -315,  -315,  -315,  -315,  -315,  -315,   371,
    -315,   886,   376,  -315,   147,   387,  -315,  -315,  -315,  -315,
    -315,  -315,   653,  -315,   556,   403,  -315,   654,  -315,   655,
    -315,   656,  -315,   199,  -315,   485,  -315,   -78,   375,   677,
     677,   907,   856,   857,   858,   859,   860,   861,   862,   863,
     702,  -315,   870,   870,   870,   864,   866,   867,  -315,    88,
    -315,   830,  -315,   874,  -315,   876,  -315,  -315,   130,  -315,
     878,  -315,   879,  -315,   880,  -315,  -315,   532,  -315,   881,
    -315,   882,  -315,   883,  -315,  -315,   562,  -315,   884,  -315,
     885,  -315,   887,  -315,  -315,   888,  -315,   889,  -315,   890,
    -315,  -315,   891,  -315,   892,  -315,   893,  -315,  -315,   868,
    -315,  -315,   894,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,   631,  -315,   895,  -315,  -315,  -315,   631,  -315,  -315,
    -315,  -315,  -315,  -315,   631,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
     896,  -315,   835,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,   897,  -315,   412,   526,   563,   898,  -315,  -315,   899,
     900,   902,   903,   904,   905,   906,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   149,  -315,   929,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,  -315,
    -315,  -315,  -315,   515,  -315,  -315
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -315,  -315,  -315,   945,  -315,  -315,  -315,   -93,   567,  -315,
    -315,  -265,  -315,  -315,  -315,  -315,  -315,  -301,   473,  -244,
     316,  -315,   536,  -254,  -314,  -306,  -252,  -242,  -236
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -13
static const short int yytable[] =
{
     348,   337,   353,   322,   358,   441,   363,   208,   367,   323,
     371,   349,   438,   354,   436,   359,   437,   364,   211,   368,
     214,   372,    77,    55,   216,   449,   439,   440,   264,   266,
     268,   270,   272,   102,    56,   568,    -2,   103,   274,   277,
     279,   281,   284,   286,   456,    57,   457,   288,    -8,   -10,
     -12,   290,   292,   295,   297,   299,   301,   324,    58,   326,
     328,   330,   332,    59,    60,   338,   427,   429,   431,    61,
     435,   119,    62,   322,   117,   322,   130,   322,    63,   323,
     261,   323,   207,   323,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,   589,   365,   432,   105,    39,    40,    41,    42,    43,
      44,    45,   212,   450,   213,    78,   215,   451,   570,   571,
     217,   265,   267,   269,   271,   273,   583,   584,   585,   543,
     652,   275,   278,   280,   282,   285,   287,    46,   541,   543,
     289,   547,   188,   594,   291,   293,   296,   298,   300,   302,
     325,   554,   327,   329,   331,   333,   339,   120,   121,   122,
     123,   118,   120,   121,   122,   123,   337,   262,   120,   121,
     122,   123,   345,   542,   459,   590,   460,   322,   322,    64,
     561,   107,   160,   323,   323,   350,   465,   392,   466,   355,
     461,   393,   462,   360,     1,   220,   463,   369,   464,   405,
     163,    75,    65,   346,   406,    -8,   -10,   -12,   120,   121,
     366,   123,   443,    66,   111,    67,   351,   595,   562,   563,
     356,   564,    68,   474,   361,   475,   189,   190,   191,   192,
      69,   483,   220,   484,   120,    70,   653,   123,   618,   123,
     220,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,   120,   121,
     347,   123,    39,    40,    41,    42,    43,    44,    45,   161,
     162,   120,   121,   352,   123,   120,   121,   357,   123,   120,
     121,   362,   123,   120,   121,   370,   123,   164,   165,    76,
     322,   337,   337,   337,    46,   113,   323,   166,    71,   335,
     112,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   442,   654,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   169,   246,   247,
     374,   115,   535,   248,   249,   250,   335,   539,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   545,   149,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   552,   246,   247,   201,   203,   205,
     248,   249,   250,   335,   377,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,    72,    73,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   114,   246,   247,   167,   168,   251,   248,   249,   250,
     252,   382,   384,   492,   336,   493,    74,   120,   121,   122,
     123,    79,   220,   499,   506,   500,   507,   109,   386,   394,
      80,   138,   220,   220,   170,   171,   375,   116,   376,   536,
     537,    81,    82,   251,   121,   540,   565,   252,   396,   398,
     468,   569,   469,   400,    83,   121,   546,   150,   132,   134,
     136,   139,   141,   143,   145,   147,   151,   153,   155,   158,
     172,   121,   553,   174,   176,   202,   204,   206,   178,   566,
     251,   180,   378,   182,   252,   184,   186,   335,   620,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   193,
     195,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   197,   246,   247,   199,   383,
     385,   248,   249,   250,   335,   599,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   387,   395,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   567,   246,   247,   604,   397,   399,   248,   249,
     250,   401,   550,   209,   133,   135,   137,   140,   142,   144,
     146,   148,   152,   154,   156,   159,   173,    84,   257,   175,
     177,   221,   412,   415,   179,   425,   258,   181,   131,   183,
     379,   185,   187,    85,   251,   218,   157,    86,   252,   600,
      87,   470,   621,   471,    88,   194,   196,    89,   388,   390,
     444,   472,   259,   473,   548,   555,   557,   559,   477,   260,
     478,   198,    90,   120,   200,    91,   123,    92,   404,   605,
     410,   251,   413,    93,    94,   252,   479,    95,   480,   622,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
      96,   219,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   423,   246,   247,   210,
      97,    98,   248,   249,   250,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   416,   380,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   220,   246,   247,   389,   391,   445,   248,   249,   250,
     549,   556,   558,   560,   405,   276,   405,   263,   405,   406,
     407,   406,   407,   406,   407,   481,   283,   482,   408,   411,
     414,    99,   424,   315,   316,   251,   433,   120,   121,   122,
     123,   317,   318,   486,   488,   487,   489,   490,   495,   491,
     496,   497,   405,   498,   502,   100,   503,   406,   407,   504,
     101,   505,   426,   581,   294,   582,   343,   303,   428,   340,
     304,   334,   341,   344,   430,   446,   439,   440,   342,   373,
     251,   405,   417,   418,   419,   420,   406,   318,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   434,   381,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   402,   246,   247,   403,   447,   448,
     248,   249,   250,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   452,   453,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   454,
     246,   247,   455,   458,   467,   248,   249,   250,   405,   417,
     418,   419,   420,   406,   405,   417,   418,   419,   420,   406,
     405,   417,   418,   419,   420,   406,   476,   485,   494,   501,
     509,   508,   538,   510,   511,   512,   572,   591,   513,   514,
     515,   516,   517,   251,   405,   417,   418,   419,   420,   406,
     318,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     551,   527,   528,   529,   530,   531,   532,   533,   534,   573,
     574,   575,   576,   577,   578,   579,   580,   615,   251,   587,
     588,   592,   252,   593,   586,   596,   597,   598,   601,   602,
     603,   606,   607,   104,   608,   609,   610,   611,   612,   613,
     614,   440,     0,   616,     0,   623,   619,     0,     0,     0,
       0,     0,   624,   625,   617,   626,   627,   628,   629,   630,
       0,   631,   632,   633,   634,   635,   636,   637,   638,   639,
     640,   641,   642,   643,   644,   645,   646,   647,   648,   649,
     650,   651,   655
};

static const short int yycheck[] =
{
     265,   253,   267,   245,   269,   319,   271,   100,   273,   245,
     275,   265,   318,   267,   315,   269,   317,   271,     1,   273,
       1,   275,     1,    96,     1,     1,   104,   105,     1,     1,
       1,     1,     1,     0,    96,   113,     0,     1,     1,     1,
       1,     1,     1,     1,   107,    96,   109,     1,    12,    13,
      14,     1,     1,     1,     1,     1,     1,     1,    96,     1,
       1,     1,     1,    96,    96,     1,   310,   311,   312,    96,
     314,     1,    96,   315,     1,   317,     1,   319,    96,   315,
       1,   317,     1,   319,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    33,     1,     1,    12,    89,    90,    91,    92,    93,
      94,    95,   115,   109,   117,   114,   117,   113,   439,   440,
     117,   114,   114,   114,   114,   114,   452,   453,   454,   414,
       1,   114,   114,   114,   114,   114,   114,   121,   412,   424,
     114,   415,     1,    33,   114,   114,   114,   114,   114,   114,
     114,   425,   114,   114,   114,   114,   112,   107,   108,   109,
     110,   108,   107,   108,   109,   110,   438,   108,   107,   108,
     109,   110,     1,    46,   111,   107,   113,   439,   440,    96,
       1,    13,     1,   439,   440,     1,   111,   109,   113,     1,
     111,   113,   113,     1,     1,   120,   111,     1,   113,    97,
       1,     1,    96,    32,   102,    12,    13,    14,   107,   108,
     109,   110,   325,    96,     1,    96,    32,   107,    39,    40,
      32,    42,    96,   111,    32,   113,    85,    86,    87,    88,
      96,   111,   120,   113,   107,    96,   107,   110,   572,   110,
     120,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,   107,   108,
     109,   110,    89,    90,    91,    92,    93,    94,    95,   108,
     109,   107,   108,   109,   110,   107,   108,   109,   110,   107,
     108,   109,   110,   107,   108,   109,   110,   108,   109,   109,
     572,   583,   584,   585,   121,     1,   572,     1,    96,     1,
     107,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,     1,   617,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     1,    30,    31,
       1,     1,     1,    35,    36,    37,     1,     1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,     1,     1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     1,    30,    31,     1,     1,     1,
      35,    36,    37,     1,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    96,    96,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   107,    30,    31,   108,   109,   108,    35,    36,    37,
     112,     1,     1,   111,   116,   113,    96,   107,   108,   109,
     110,    96,   120,   111,   111,   113,   113,    14,     1,     1,
      96,   109,   120,   120,   108,   109,   107,   107,   109,   108,
     109,    96,    96,   108,   108,   109,     1,   112,     1,     1,
     111,   116,   113,     1,    96,   108,   109,   109,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,   108,   109,     1,     1,   109,   109,   109,     1,    34,
     108,     1,   109,     1,   112,     1,     1,     1,   116,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     1,
       1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     1,    30,    31,     1,   109,
     109,    35,    36,    37,     1,    33,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,   109,   109,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   107,    30,    31,    33,   109,   109,    35,    36,
      37,   109,    46,     1,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,    96,   107,   107,
     107,   111,   306,   307,   107,   309,   107,   107,    61,   107,
       1,   107,   107,    96,   108,   120,   109,    96,   112,   107,
      96,   111,   116,   113,    96,   107,   107,    96,     1,     1,
       1,   111,   107,   113,     1,     1,     1,     1,   111,   107,
     113,   107,    96,   107,   107,    96,   110,    96,     1,   107,
       1,   108,     1,    96,    96,   112,   111,    96,   113,   116,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      96,   124,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     1,    30,    31,   107,
      96,    96,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,     1,   107,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   120,    30,    31,   107,   107,   107,    35,    36,    37,
     107,   107,   107,   107,    97,   114,    97,   109,    97,   102,
     103,   102,   103,   102,   103,   111,   114,   113,   305,   306,
     307,    96,   309,   106,   107,   108,   313,   107,   108,   109,
     110,   114,   115,   111,   111,   113,   113,   111,   111,   113,
     113,   111,    97,   113,   111,    96,   113,   102,   103,   111,
      96,   113,     1,   111,   114,   113,   120,   114,     1,   118,
     114,   114,   118,   111,     1,   107,   104,   105,   118,   113,
     108,    97,    98,    99,   100,   101,   102,   115,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,     1,   109,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,   113,    30,    31,   113,   107,   107,
      35,    36,    37,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,   115,   115,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,   115,
      30,    31,   107,   113,   113,    35,    36,    37,    97,    98,
      99,   100,   101,   102,    97,    98,    99,   100,   101,   102,
      97,    98,    99,   100,   101,   102,   113,   113,   113,   113,
     111,   113,    46,   113,   113,   111,    29,   107,   113,   113,
     113,   113,   113,   108,    97,    98,    99,   100,   101,   102,
     115,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     424,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   109,   108,   113,
     113,   107,   112,   107,   120,   107,   107,   107,   107,   107,
     107,   107,   107,    48,   107,   107,   107,   107,   107,   107,
     107,   105,    -1,   109,    -1,   107,   109,    -1,    -1,    -1,
      -1,    -1,   113,   113,   119,   113,   113,   113,   113,   113,
      -1,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    89,
      90,    91,    92,    93,    94,    95,   121,   123,   124,   125,
     126,   127,   128,   131,   132,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,     1,   109,     1,   114,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,     0,     1,   125,    12,   136,    13,   137,    14,
     138,     1,   107,     1,   107,     1,   107,     1,   108,     1,
     107,   108,   109,   110,   129,   130,   133,   134,   135,   145,
       1,   130,     1,   107,     1,   107,     1,   107,   109,     1,
     107,     1,   107,     1,   107,     1,   107,     1,   107,     1,
     109,     1,   107,     1,   107,     1,   107,   109,     1,   107,
       1,   108,   109,     1,   108,   109,     1,   108,   109,     1,
     108,   109,     1,   107,     1,   107,     1,   107,     1,   107,
       1,   107,     1,   107,     1,   107,     1,   107,     1,    85,
      86,    87,    88,     1,   107,     1,   107,     1,   107,     1,
     107,     1,   109,     1,   109,     1,   109,     1,   129,     1,
     107,     1,   115,   117,     1,   117,     1,   117,   120,   130,
     120,   111,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    30,    31,    35,    36,
      37,   108,   112,   147,   148,   149,   150,   107,   107,   107,
     107,     1,   108,   109,     1,   114,     1,   114,     1,   114,
       1,   114,     1,   114,     1,   114,   114,     1,   114,     1,
     114,     1,   114,   114,     1,   114,     1,   114,     1,   114,
       1,   114,     1,   114,   114,     1,   114,     1,   114,     1,
     114,     1,   114,   114,   114,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   106,   107,   114,   115,   139,
     143,   146,   149,   150,     1,   114,     1,   114,     1,   114,
       1,   114,     1,   114,   114,     1,   116,   148,     1,   112,
     118,   118,   118,   120,   111,     1,    32,   109,   133,   145,
       1,    32,   109,   133,   145,     1,    32,   109,   133,   145,
       1,    32,   109,   133,   145,     1,   109,   133,   145,     1,
     109,   133,   145,   113,     1,   107,   109,     1,   109,     1,
     107,   109,     1,   109,     1,   109,     1,   109,     1,   107,
       1,   107,   109,   113,     1,   109,     1,   109,     1,   109,
       1,   109,   113,   113,     1,    97,   102,   103,   140,   142,
       1,   140,   142,     1,   140,   142,     1,    98,    99,   100,
     101,   140,   141,     1,   140,   142,     1,   141,     1,   141,
       1,   141,     1,   140,     1,   141,   139,   139,   147,   104,
     105,   146,     1,   129,     1,   107,   107,   107,   107,     1,
     109,   113,   115,   115,   115,   107,   107,   109,   113,   111,
     113,   111,   113,   111,   113,   111,   113,   113,   111,   113,
     111,   113,   111,   113,   111,   113,   113,   111,   113,   111,
     113,   111,   113,   111,   113,   113,   111,   113,   111,   113,
     111,   113,   111,   113,   113,   111,   113,   111,   113,   111,
     113,   113,   111,   113,   111,   113,   111,   113,   113,   111,
     113,   113,   111,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,     1,   108,   109,    46,     1,
     109,   145,    46,   133,   144,     1,   109,   145,     1,   107,
      46,   144,     1,   109,   145,     1,   107,     1,   107,     1,
     107,     1,    39,    40,    42,     1,    34,   107,   113,   116,
     139,   139,    29,   113,   113,   113,   113,   113,   113,   113,
     113,   111,   113,   147,   147,   147,   120,   113,   113,    33,
     107,   107,   107,   107,    33,   107,   107,   107,   107,    33,
     107,   107,   107,   107,    33,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   109,   109,   119,   146,   109,
     116,   116,   116,   107,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,     1,   107,   133,   113
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
    { 								\
      yyerror ("syntax error: cannot back up");\
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
      yysymprint (stderr, 					\
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
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
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

#endif
#endif
{
  
  register int yystate;
  register int yyn;
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
  register short int *yyssp;

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


  yyvsp[0] = yylval;

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
	short int *yyss1 = yyss;
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
        case 3:
#line 280 "cfg.y"
    {;}
    break;

  case 4:
#line 281 "cfg.y"
    {;}
    break;

  case 5:
#line 282 "cfg.y"
    { yyerror(""); YYABORT;;}
    break;

  case 8:
#line 287 "cfg.y"
    {rt=REQUEST_ROUTE;;}
    break;

  case 10:
#line 288 "cfg.y"
    {rt=FAILURE_ROUTE;;}
    break;

  case 12:
#line 289 "cfg.y"
    {rt=ONREPLY_ROUTE;;}
    break;

  case 15:
#line 294 "cfg.y"
    {	tmp=ip_addr2a((yyvsp[0].ipaddr));
		 					if(tmp==0){
								LOG(L_CRIT, "ERROR: cfg. parser: bad ip "
										"addresss.\n");
								(yyval.strval)=0;
							}else{
								(yyval.strval)=pkg_malloc(strlen(tmp)+1);
								if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
								}else{
									strncpy((yyval.strval), tmp, strlen(tmp)+1);
								}
							}
						;}
    break;

  case 16:
#line 309 "cfg.y"
    {	(yyval.strval)=pkg_malloc(strlen((yyvsp[0].strval))+1);
		 					if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
							}else{
									strncpy((yyval.strval), (yyvsp[0].strval), strlen((yyvsp[0].strval))+1);
							}
						;}
    break;

  case 17:
#line 317 "cfg.y"
    {	(yyval.strval)=pkg_malloc(strlen((yyvsp[0].strval))+1);
		 					if ((yyval.strval)==0){
									LOG(L_CRIT, "ERROR: cfg. parser: out of "
											"memory.\n");
							}else{
									strncpy((yyval.strval), (yyvsp[0].strval), strlen((yyvsp[0].strval))+1);
							}
						;}
    break;

  case 18:
#line 327 "cfg.y"
    {	(yyval.idlst)=pkg_malloc(sizeof(struct id_list));
						if ((yyval.idlst)==0){
							LOG(L_CRIT,"ERROR: cfg. parser: out of memory.\n");
						}else{
							(yyval.idlst)->s=(yyvsp[0].strval);
							(yyval.idlst)->next=0;
						}
					;}
    break;

  case 19:
#line 335 "cfg.y"
    {
						(yyval.idlst)=pkg_malloc(sizeof(struct id_list));
						if ((yyval.idlst)==0){
							LOG(L_CRIT,"ERROR: cfg. parser: out of memory.\n");
						}else{
							(yyval.idlst)->s=(yyvsp[-1].strval);
							(yyval.idlst)->next=(yyvsp[0].idlst);
						}
							;}
    break;

  case 20:
#line 347 "cfg.y"
    { debug=(yyvsp[0].intval); ;}
    break;

  case 21:
#line 348 "cfg.y"
    { yyerror("number  expected"); ;}
    break;

  case 22:
#line 349 "cfg.y"
    { dont_fork= ! (yyvsp[0].intval); ;}
    break;

  case 23:
#line 350 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 24:
#line 351 "cfg.y"
    { if (!config_check) log_stderr=(yyvsp[0].intval); ;}
    break;

  case 25:
#line 352 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 26:
#line 353 "cfg.y"
    {
					if ( (i_tmp=str2facility((yyvsp[0].strval)))==-1)
						yyerror("bad facility (see syslog(3) man page)");
					if (!config_check)
						log_facility=i_tmp;
									;}
    break;

  case 27:
#line 359 "cfg.y"
    { yyerror("ID expected"); ;}
    break;

  case 28:
#line 360 "cfg.y"
    { received_dns|= ((yyvsp[0].intval))?DO_DNS:0; ;}
    break;

  case 29:
#line 361 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 30:
#line 362 "cfg.y"
    { received_dns|= ((yyvsp[0].intval))?DO_REV_DNS:0; ;}
    break;

  case 31:
#line 363 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 32:
#line 364 "cfg.y"
    { port_no=(yyvsp[0].intval); 
								if (sock_no>0) 
									sock_info[sock_no-1].port_no=port_no;
							  ;}
    break;

  case 33:
#line 368 "cfg.y"
    {
					#ifdef STATS
							stat_file=(yyvsp[0].strval);
					#endif
							;}
    break;

  case 34:
#line 373 "cfg.y"
    { maxbuffer=(yyvsp[0].intval); ;}
    break;

  case 35:
#line 374 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 36:
#line 375 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 37:
#line 376 "cfg.y"
    { children_no=(yyvsp[0].intval); ;}
    break;

  case 38:
#line 377 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 39:
#line 378 "cfg.y"
    { check_via=(yyvsp[0].intval); ;}
    break;

  case 40:
#line 379 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 41:
#line 380 "cfg.y"
    { syn_branch=(yyvsp[0].intval); ;}
    break;

  case 42:
#line 381 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 43:
#line 382 "cfg.y"
    { memlog=(yyvsp[0].intval); ;}
    break;

  case 44:
#line 383 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 45:
#line 384 "cfg.y"
    { sip_warning=(yyvsp[0].intval); ;}
    break;

  case 46:
#line 385 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 47:
#line 386 "cfg.y"
    { fifo=(yyvsp[0].strval); ;}
    break;

  case 48:
#line 387 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 49:
#line 388 "cfg.y"
    { fifo_mode=(yyvsp[0].intval); ;}
    break;

  case 50:
#line 389 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 51:
#line 390 "cfg.y"
    { user=(yyvsp[0].strval); ;}
    break;

  case 52:
#line 391 "cfg.y"
    { user=(yyvsp[0].strval); ;}
    break;

  case 53:
#line 392 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 54:
#line 393 "cfg.y"
    { group=(yyvsp[0].strval); ;}
    break;

  case 55:
#line 394 "cfg.y"
    { group=(yyvsp[0].strval); ;}
    break;

  case 56:
#line 395 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 57:
#line 396 "cfg.y"
    { chroot_dir=(yyvsp[0].strval); ;}
    break;

  case 58:
#line 397 "cfg.y"
    { chroot_dir=(yyvsp[0].strval); ;}
    break;

  case 59:
#line 398 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 60:
#line 399 "cfg.y"
    { working_dir=(yyvsp[0].strval); ;}
    break;

  case 61:
#line 400 "cfg.y"
    { working_dir=(yyvsp[0].strval); ;}
    break;

  case 62:
#line 401 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 63:
#line 402 "cfg.y"
    { mhomed=(yyvsp[0].intval); ;}
    break;

  case 64:
#line 403 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 65:
#line 404 "cfg.y"
    {
									#ifdef USE_TCP
										tcp_disable=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									;}
    break;

  case 66:
#line 411 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 67:
#line 412 "cfg.y"
    {
									#ifdef USE_TCP
										tcp_children_no=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									;}
    break;

  case 68:
#line 419 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 69:
#line 420 "cfg.y"
    {
									#ifdef USE_TCP
										tcp_connect_timeout=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									;}
    break;

  case 70:
#line 427 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 71:
#line 428 "cfg.y"
    {
									#ifdef USE_TCP
										tcp_send_timeout=(yyvsp[0].intval);
									#else
										warn("tcp support not compiled in");
									#endif
									;}
    break;

  case 72:
#line 435 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 73:
#line 436 "cfg.y"
    {
									#ifdef USE_TLS
										tls_disable=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 74:
#line 443 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 75:
#line 444 "cfg.y"
    { 
									#ifdef USE_TLS
										tls_log=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 76:
#line 451 "cfg.y"
    { yyerror("int value expected"); ;}
    break;

  case 77:
#line 452 "cfg.y"
    {
									#ifdef USE_TLS
										tls_port_no=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 78:
#line 459 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 79:
#line 460 "cfg.y"
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv23;
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 80:
#line 467 "cfg.y"
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv2;
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 81:
#line 474 "cfg.y"
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_SSLv3;
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 82:
#line 481 "cfg.y"
    {
									#ifdef USE_TLS
										tls_method=TLS_USE_TLSv1;
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 83:
#line 488 "cfg.y"
    {
									#ifdef USE_TLS
										yyerror("SSLv23, SSLv2, SSLv3 or TLSv1"
													" expected");
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 84:
#line 497 "cfg.y"
    {
									#ifdef USE_TLS
										tls_verify_cert=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 85:
#line 504 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 86:
#line 505 "cfg.y"
    {
									#ifdef USE_TLS
										tls_require_cert=(yyvsp[0].intval);
									#else
										warn( "tls support not compiled in");
									#endif
									;}
    break;

  case 87:
#line 512 "cfg.y"
    { yyerror("boolean value"
																" expected"); ;}
    break;

  case 88:
#line 514 "cfg.y"
    { 
									#ifdef USE_TLS
											tls_cert_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 89:
#line 521 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 90:
#line 522 "cfg.y"
    { 
									#ifdef USE_TLS
											tls_pkey_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 91:
#line 529 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 92:
#line 530 "cfg.y"
    { 
									#ifdef USE_TLS
											tls_ca_file=(yyvsp[0].strval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 93:
#line 537 "cfg.y"
    { yyerror("string value expected"); ;}
    break;

  case 94:
#line 538 "cfg.y"
    {
									#ifdef USE_TLS
										tls_handshake_timeout=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 95:
#line 545 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 96:
#line 546 "cfg.y"
    {
									#ifdef USE_TLS
										tls_send_timeout=(yyvsp[0].intval);
									#else
										warn("tls support not compiled in");
									#endif
									;}
    break;

  case 97:
#line 553 "cfg.y"
    { yyerror("number expected"); ;}
    break;

  case 98:
#line 554 "cfg.y"
    { server_signature=(yyvsp[0].intval); ;}
    break;

  case 99:
#line 555 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 100:
#line 556 "cfg.y"
    { reply_to_via=(yyvsp[0].intval); ;}
    break;

  case 101:
#line 557 "cfg.y"
    { yyerror("boolean value expected"); ;}
    break;

  case 102:
#line 558 "cfg.y"
    {
							for(lst_tmp=(yyvsp[0].idlst); lst_tmp; lst_tmp=lst_tmp->next){
								if (sock_no < MAX_LISTEN){
									sock_info[sock_no].name.s=(char*)
											pkg_malloc(strlen(lst_tmp->s)+1);
									if (sock_info[sock_no].name.s==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
										break;
									}else{
										strncpy(sock_info[sock_no].name.s,
												lst_tmp->s,
												strlen(lst_tmp->s)+1);
										sock_info[sock_no].name.len=
													strlen(lst_tmp->s);
										sock_info[sock_no].port_no=port_no;
										sock_no++;
									}
								}else{
									LOG(L_CRIT, "ERROR: cfg. parser: "
												"too many listen addresses"
												"(max. %d).\n", MAX_LISTEN);
									break;
								}
							}
							 ;}
    break;

  case 103:
#line 584 "cfg.y"
    { yyerror("ip address or hostname"
						"expected"); ;}
    break;

  case 104:
#line 586 "cfg.y"
    { 
							for(lst_tmp=(yyvsp[0].idlst); lst_tmp; lst_tmp=lst_tmp->next)
								add_alias(lst_tmp->s, strlen(lst_tmp->s), 0);
							  ;}
    break;

  case 105:
#line 590 "cfg.y"
    { yyerror(" hostname expected"); ;}
    break;

  case 106:
#line 591 "cfg.y"
    {
								default_global_address.s=(yyvsp[0].strval);
								default_global_address.len=strlen((yyvsp[0].strval));
								;}
    break;

  case 107:
#line 595 "cfg.y"
    {yyerror("ip address or hostname "
												"expected"); ;}
    break;

  case 108:
#line 597 "cfg.y"
    {
								tmp=int2str((yyvsp[0].intval), &i_tmp);
								if ((default_global_port.s=pkg_malloc(i_tmp))
										==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
										default_global_port.len=0;
								}else{
									default_global_port.len=i_tmp;
									memcpy(default_global_port.s, tmp,
											default_global_port.len);
								};
								;}
    break;

  case 109:
#line 610 "cfg.y"
    {yyerror("ip address or hostname "
												"expected"); ;}
    break;

  case 110:
#line 612 "cfg.y"
    { yyerror("unknown config variable"); ;}
    break;

  case 111:
#line 615 "cfg.y"
    { DBG("loading module %s\n", (yyvsp[0].strval));
		  						  if (load_module((yyvsp[0].strval))!=0){
								  		yyerror("failed to load module");
								  }
								;}
    break;

  case 112:
#line 620 "cfg.y"
    { yyerror("string expected");  ;}
    break;

  case 113:
#line 621 "cfg.y"
    {
			 if (set_mod_param_regex((yyvsp[-5].strval), (yyvsp[-3].strval), STR_PARAM, (yyvsp[-1].strval)) != 0) {
				 yyerror("Can't set module parameter");
			 }
		   ;}
    break;

  case 114:
#line 626 "cfg.y"
    {
			 if (set_mod_param_regex((yyvsp[-5].strval), (yyvsp[-3].strval), INT_PARAM, (void*)(yyvsp[-1].intval)) != 0) {
				 yyerror("Can't set module parameter");
			 }
		   ;}
    break;

  case 115:
#line 631 "cfg.y"
    { yyerror("Invalid arguments"); ;}
    break;

  case 116:
#line 635 "cfg.y"
    { (yyval.ipaddr)=(yyvsp[0].ipaddr); ;}
    break;

  case 117:
#line 636 "cfg.y"
    { (yyval.ipaddr)=(yyvsp[0].ipaddr); ;}
    break;

  case 118:
#line 639 "cfg.y"
    { 
											(yyval.ipaddr)=pkg_malloc(
													sizeof(struct ip_addr));
											if ((yyval.ipaddr)==0){
												LOG(L_CRIT, "ERROR: cfg. "
													"parser: out of memory.\n"
													);
											}else{
												memset((yyval.ipaddr), 0, 
													sizeof(struct ip_addr));
												(yyval.ipaddr)->af=AF_INET;
												(yyval.ipaddr)->len=4;
												if (((yyvsp[-6].intval)>255) || ((yyvsp[-6].intval)<0) ||
													((yyvsp[-4].intval)>255) || ((yyvsp[-4].intval)<0) ||
													((yyvsp[-2].intval)>255) || ((yyvsp[-2].intval)<0) ||
													((yyvsp[0].intval)>255) || ((yyvsp[0].intval)<0)){
													yyerror("invalid ipv4"
															"address");
													(yyval.ipaddr)->u.addr32[0]=0;
													/* $$=0; */
												}else{
													(yyval.ipaddr)->u.addr[0]=(yyvsp[-6].intval);
													(yyval.ipaddr)->u.addr[1]=(yyvsp[-4].intval);
													(yyval.ipaddr)->u.addr[2]=(yyvsp[-2].intval);
													(yyval.ipaddr)->u.addr[3]=(yyvsp[0].intval);
													/*
													$$=htonl( ($1<<24)|
													($3<<16)| ($5<<8)|$7 );
													*/
												}
											}
												;}
    break;

  case 119:
#line 673 "cfg.y"
    {
					(yyval.ipaddr)=pkg_malloc(sizeof(struct ip_addr));
					if ((yyval.ipaddr)==0){
						LOG(L_CRIT, "ERROR: cfg. parser: out of memory.\n");
					}else{
						memset((yyval.ipaddr), 0, sizeof(struct ip_addr));
						(yyval.ipaddr)->af=AF_INET6;
						(yyval.ipaddr)->len=16;
					#ifdef USE_IPV6
						if (inet_pton(AF_INET6, (yyvsp[0].strval), (yyval.ipaddr)->u.addr)<=0){
							yyerror("bad ipv6 address");
						}
					#else
						yyerror("ipv6 address & no ipv6 support compiled in");
						YYABORT;
					#endif
					}
				;}
    break;

  case 120:
#line 694 "cfg.y"
    { push((yyvsp[-1].action), &rlist[DEFAULT_RT]); ;}
    break;

  case 121:
#line 696 "cfg.y"
    { 
										if (((yyvsp[-4].intval)<RT_NO) && ((yyvsp[-4].intval)>=0)){
											push((yyvsp[-1].action), &rlist[(yyvsp[-4].intval)]);
										}else{
											yyerror("invalid routing"
													"table number");
											YYABORT; }
										;}
    break;

  case 122:
#line 704 "cfg.y"
    { yyerror("invalid  route  statement"); ;}
    break;

  case 123:
#line 707 "cfg.y"
    {
										if (((yyvsp[-4].intval)<FAILURE_RT_NO)&&((yyvsp[-4].intval)>=1)){
											push((yyvsp[-1].action), &failure_rlist[(yyvsp[-4].intval)]);
										} else {
											yyerror("invalid reply routing"
												"table number");
											YYABORT; }
										;}
    break;

  case 124:
#line 715 "cfg.y"
    { yyerror("invalid failure_route statement"); ;}
    break;

  case 125:
#line 718 "cfg.y"
    {
										if (((yyvsp[-4].intval)<ONREPLY_RT_NO)&&((yyvsp[-4].intval)>=1)){
											push((yyvsp[-1].action), &onreply_rlist[(yyvsp[-4].intval)]);
										} else {
											yyerror("invalid reply routing"
												"table number");
											YYABORT; }
										;}
    break;

  case 126:
#line 726 "cfg.y"
    { yyerror("invalid failure_route statement"); ;}
    break;

  case 127:
#line 748 "cfg.y"
    { (yyval.expr)=mk_exp(AND_OP, (yyvsp[-2].expr), (yyvsp[0].expr)); ;}
    break;

  case 128:
#line 749 "cfg.y"
    { (yyval.expr)=mk_exp(OR_OP, (yyvsp[-2].expr), (yyvsp[0].expr));  ;}
    break;

  case 129:
#line 750 "cfg.y"
    { (yyval.expr)=mk_exp(NOT_OP, (yyvsp[0].expr), 0);  ;}
    break;

  case 130:
#line 751 "cfg.y"
    { (yyval.expr)=(yyvsp[-1].expr); ;}
    break;

  case 131:
#line 752 "cfg.y"
    { (yyval.expr)=(yyvsp[0].expr); ;}
    break;

  case 132:
#line 755 "cfg.y"
    {(yyval.intval)=EQUAL_OP; ;}
    break;

  case 133:
#line 756 "cfg.y"
    {(yyval.intval)=DIFF_OP; ;}
    break;

  case 134:
#line 759 "cfg.y"
    {(yyval.intval)=(yyvsp[0].intval); ;}
    break;

  case 135:
#line 760 "cfg.y"
    {(yyval.intval)=GT_OP; ;}
    break;

  case 136:
#line 761 "cfg.y"
    {(yyval.intval)=LT_OP; ;}
    break;

  case 137:
#line 762 "cfg.y"
    {(yyval.intval)=GTE_OP; ;}
    break;

  case 138:
#line 763 "cfg.y"
    {(yyval.intval)=LTE_OP; ;}
    break;

  case 139:
#line 766 "cfg.y"
    {(yyval.intval)=(yyvsp[0].intval); ;}
    break;

  case 140:
#line 767 "cfg.y"
    {(yyval.intval)=MATCH_OP; ;}
    break;

  case 141:
#line 770 "cfg.y"
    {(yyval.expr)= mk_elem(	(yyvsp[-1].intval), STRING_ST, 
													METHOD_O, (yyvsp[0].strval));
									;}
    break;

  case 142:
#line 773 "cfg.y"
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
											METHOD_O, (yyvsp[0].strval)); 
				 			;}
    break;

  case 143:
#line 776 "cfg.y"
    { (yyval.expr)=0; yyerror("string expected"); ;}
    break;

  case 144:
#line 777 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator,"
										"== , !=, or =~ expected");
						;}
    break;

  case 145:
#line 780 "cfg.y"
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
												URI_O, (yyvsp[0].strval)); 
				 				;}
    break;

  case 146:
#line 783 "cfg.y"
    {(yyval.expr) = mk_elem(	(yyvsp[-1].intval), STRING_ST,
											URI_O, (yyvsp[0].strval)); 
				 			;}
    break;

  case 147:
#line 786 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												URI_O, 0);
								;}
    break;

  case 148:
#line 789 "cfg.y"
    { (yyval.expr)=0; yyerror("string or MYSELF expected"); ;}
    break;

  case 149:
#line 790 "cfg.y"
    { (yyval.expr)=0; yyerror("invalid operator,"
									" == , != or =~ expected");
					;}
    break;

  case 150:
#line 793 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												SRCPORT_O, (void *) (yyvsp[0].intval) ); ;}
    break;

  case 151:
#line 795 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 152:
#line 796 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 153:
#line 797 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												DSTPORT_O, (void *) (yyvsp[0].intval) ); ;}
    break;

  case 154:
#line 799 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 155:
#line 800 "cfg.y"
    { (yyval.expr)=0; yyerror("==, !=, <,>, >= or <=  expected"); ;}
    break;

  case 156:
#line 801 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												PROTO_O, (void *) (yyvsp[0].intval) ); ;}
    break;

  case 157:
#line 803 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 158:
#line 804 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 159:
#line 805 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												AF_O, (void *) (yyvsp[0].intval) ); ;}
    break;

  case 160:
#line 807 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 161:
#line 808 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 162:
#line 809 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												MSGLEN_O, (void *) (yyvsp[0].intval) ); ;}
    break;

  case 163:
#line 811 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NUMBER_ST,
												MSGLEN_O, (void *) BUF_SIZE); ;}
    break;

  case 164:
#line 813 "cfg.y"
    { (yyval.expr)=0; yyerror("number expected"); ;}
    break;

  case 165:
#line 814 "cfg.y"
    { (yyval.expr)=0; yyerror("equal/!= operator expected"); ;}
    break;

  case 166:
#line 815 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST,
												SRCIP_O, (yyvsp[0].ipnet));
								;}
    break;

  case 167:
#line 818 "cfg.y"
    {	s_tmp.s=(yyvsp[0].strval);
									s_tmp.len=strlen((yyvsp[0].strval));
									ip_tmp=str2ip(&s_tmp);
									if (ip_tmp==0)
										ip_tmp=str2ip6(&s_tmp);
									if (ip_tmp){
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST, SRCIP_O,
												mk_net_bitlen(ip_tmp, 
														ip_tmp->len*8) );
									}else{
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												SRCIP_O, (yyvsp[0].strval));
									}
								;}
    break;

  case 168:
#line 832 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												SRCIP_O, (yyvsp[0].strval));
								;}
    break;

  case 169:
#line 835 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												SRCIP_O, 0);
								;}
    break;

  case 170:
#line 838 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname"
						 "expected" ); ;}
    break;

  case 171:
#line 840 "cfg.y"
    { (yyval.expr)=0; 
						 yyerror("invalid operator, ==, != or =~ expected");;}
    break;

  case 172:
#line 842 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST,
												DSTIP_O, (yyvsp[0].ipnet));
								;}
    break;

  case 173:
#line 845 "cfg.y"
    {	s_tmp.s=(yyvsp[0].strval);
									s_tmp.len=strlen((yyvsp[0].strval));
									ip_tmp=str2ip(&s_tmp);
									if (ip_tmp==0)
										ip_tmp=str2ip6(&s_tmp);
									if (ip_tmp){
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), NET_ST, DSTIP_O,
												mk_net_bitlen(ip_tmp, 
														ip_tmp->len*8) );
									}else{
										(yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												DSTIP_O, (yyvsp[0].strval));
									}
								;}
    break;

  case 174:
#line 859 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), STRING_ST,
												DSTIP_O, (yyvsp[0].strval));
								;}
    break;

  case 175:
#line 862 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												DSTIP_O, 0);
								;}
    break;

  case 176:
#line 865 "cfg.y"
    { (yyval.expr)=0; yyerror( "ip address or hostname"
						 			"expected" ); ;}
    break;

  case 177:
#line 867 "cfg.y"
    { (yyval.expr)=0; 
						yyerror("invalid operator, ==, != or =~ expected");;}
    break;

  case 178:
#line 869 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												URI_O, 0);
								;}
    break;

  case 179:
#line 872 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												SRCIP_O, 0);
								;}
    break;

  case 180:
#line 875 "cfg.y"
    { (yyval.expr)=mk_elem(	(yyvsp[-1].intval), MYSELF_ST,
												DSTIP_O, 0);
								;}
    break;

  case 181:
#line 878 "cfg.y"
    {	(yyval.expr)=0; 
									yyerror(" URI, SRCIP or DSTIP expected"); ;}
    break;

  case 182:
#line 880 "cfg.y"
    { (yyval.expr)=0; 
							yyerror ("invalid operator, == or != expected");
						;}
    break;

  case 183:
#line 883 "cfg.y"
    { (yyval.expr)=mk_elem( NO_OP, ACTIONS_ST, ACTION_O, (yyvsp[0].action) ); ;}
    break;

  case 184:
#line 884 "cfg.y"
    {(yyval.expr)=mk_elem( NO_OP, NUMBER_ST, NUMBER_O, (void*)(yyvsp[0].intval) ); ;}
    break;

  case 185:
#line 887 "cfg.y"
    { (yyval.ipnet)=mk_net((yyvsp[-2].ipaddr), (yyvsp[0].ipaddr)); ;}
    break;

  case 186:
#line 888 "cfg.y"
    {	if (((yyvsp[0].intval)<0) || ((yyvsp[0].intval)>(yyvsp[-2].ipaddr)->len*8)){
								yyerror("invalid bit number in netmask");
								(yyval.ipnet)=0;
							}else{
								(yyval.ipnet)=mk_net_bitlen((yyvsp[-2].ipaddr), (yyvsp[0].intval));
							/*
								$$=mk_net($1, 
										htonl( ($3)?~( (1<<(32-$3))-1 ):0 ) );
							*/
							}
						;}
    break;

  case 187:
#line 899 "cfg.y"
    { (yyval.ipnet)=mk_net_bitlen((yyvsp[0].ipaddr), (yyvsp[0].ipaddr)->len*8); ;}
    break;

  case 188:
#line 900 "cfg.y"
    { (yyval.ipnet)=0;
						 yyerror("netmask (eg:255.0.0.0 or 8) expected");
						;}
    break;

  case 189:
#line 905 "cfg.y"
    { (yyval.strval)=(yyvsp[0].strval); ;}
    break;

  case 190:
#line 906 "cfg.y"
    { (yyval.strval)=(char*)pkg_malloc(strlen((yyvsp[-2].strval))+1+strlen((yyvsp[0].strval))+1);
						  if ((yyval.strval)==0){
						  	LOG(L_CRIT, "ERROR: cfg. parser: memory allocation"
										" failure while parsing host\n");
						  }else{
						  	memcpy((yyval.strval), (yyvsp[-2].strval), strlen((yyvsp[-2].strval)));
						  	(yyval.strval)[strlen((yyvsp[-2].strval))]='.';
						  	memcpy((yyval.strval)+strlen((yyvsp[-2].strval))+1, (yyvsp[0].strval), strlen((yyvsp[0].strval)));
						  	(yyval.strval)[strlen((yyvsp[-2].strval))+1+strlen((yyvsp[0].strval))]=0;
						  }
						  pkg_free((yyvsp[-2].strval)); pkg_free((yyvsp[0].strval));
						;}
    break;

  case 191:
#line 918 "cfg.y"
    { (yyval.strval)=0; pkg_free((yyvsp[-2].strval)); yyerror("invalid hostname"); ;}
    break;

  case 192:
#line 922 "cfg.y"
    { (yyval.action)=(yyvsp[0].action); ;}
    break;

  case 193:
#line 923 "cfg.y"
    { (yyval.action)=(yyvsp[0].action); ;}
    break;

  case 194:
#line 924 "cfg.y"
    { (yyval.action)=(yyvsp[-1].action); ;}
    break;

  case 195:
#line 927 "cfg.y"
    {(yyval.action)=append_action((yyvsp[-1].action), (yyvsp[0].action)); ;}
    break;

  case 196:
#line 928 "cfg.y"
    {(yyval.action)=(yyvsp[0].action);;}
    break;

  case 197:
#line 929 "cfg.y"
    { (yyval.action)=0; yyerror("bad command"); ;}
    break;

  case 198:
#line 932 "cfg.y"
    {(yyval.action)=(yyvsp[-1].action);;}
    break;

  case 199:
#line 933 "cfg.y"
    {(yyval.action)=(yyvsp[0].action);;}
    break;

  case 200:
#line 934 "cfg.y"
    {(yyval.action)=0;;}
    break;

  case 201:
#line 935 "cfg.y"
    { (yyval.action)=0; yyerror("bad command: missing ';'?"); ;}
    break;

  case 202:
#line 938 "cfg.y"
    { (yyval.action)=mk_action3( IF_T,
													 EXPR_ST,
													 ACTIONS_ST,
													 NOSUBTYPE,
													 (yyvsp[-1].expr),
													 (yyvsp[0].action),
													 0);
									;}
    break;

  case 203:
#line 946 "cfg.y"
    { (yyval.action)=mk_action3( IF_T,
													 EXPR_ST,
													 ACTIONS_ST,
													 ACTIONS_ST,
													 (yyvsp[-3].expr),
													 (yyvsp[-2].action),
													 (yyvsp[0].action));
									;}
    break;

  case 204:
#line 956 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 205:
#line 962 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 206:
#line 968 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										;}
    break;

  case 207:
#line 974 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 ;}
    break;

  case 208:
#line 980 "cfg.y"
    {(yyval.action)=mk_action(FORWARD_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 209:
#line 986 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  ;}
    break;

  case 210:
#line 992 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													;}
    break;

  case 211:
#line 1001 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 212:
#line 1008 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										;}
    break;

  case 213:
#line 1015 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 214:
#line 1016 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward"
										"argument"); ;}
    break;

  case 215:
#line 1018 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 216:
#line 1024 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 217:
#line 1030 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_UDP_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										;}
    break;

  case 218:
#line 1036 "cfg.y"
    { (yyval.action)=mk_action(
																FORWARD_UDP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 ;}
    break;

  case 219:
#line 1043 "cfg.y"
    {(yyval.action)=mk_action(
																FORWARD_UDP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 220:
#line 1050 "cfg.y"
    { (yyval.action)=mk_action(
																FORWARD_UDP_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  ;}
    break;

  case 221:
#line 1057 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													;}
    break;

  case 222:
#line 1066 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 223:
#line 1073 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_UDP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										;}
    break;

  case 224:
#line 1080 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 225:
#line 1081 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_udp"
										"argument"); ;}
    break;

  case 226:
#line 1083 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 227:
#line 1089 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										;}
    break;

  case 228:
#line 1095 "cfg.y"
    { (yyval.action)=mk_action(	FORWARD_TCP_T,
														IP_ST,
														NUMBER_ST,
														(void*)(yyvsp[-1].ipaddr),
														0);
										;}
    break;

  case 229:
#line 1101 "cfg.y"
    { (yyval.action)=mk_action(
																FORWARD_TCP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												 ;}
    break;

  case 230:
#line 1108 "cfg.y"
    {(yyval.action)=mk_action(
																FORWARD_TCP_T,
																 STRING_ST,
																 NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 231:
#line 1115 "cfg.y"
    { (yyval.action)=mk_action(FORWARD_TCP_T,
																 IP_ST,
																 NUMBER_ST,
																 (void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
												  ;}
    break;

  case 232:
#line 1121 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 URIPORT_ST,
																0,
																0);
													;}
    break;

  case 233:
#line 1130 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																(void*)(yyvsp[-1].intval));
													;}
    break;

  case 234:
#line 1137 "cfg.y"
    {
													(yyval.action)=mk_action(FORWARD_TCP_T,
																 URIHOST_ST,
																 NUMBER_ST,
																0,
																0);
										;}
    break;

  case 235:
#line 1144 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 236:
#line 1145 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_tcp"
										"argument"); ;}
    break;

  case 237:
#line 1147 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
														STRING_ST,
														NUMBER_ST,
														(yyvsp[-1].strval),
														0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										;}
    break;

  case 238:
#line 1160 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															STRING_ST,
															NUMBER_ST,
															(yyvsp[-1].strval),
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										;}
    break;

  case 239:
#line 1173 "cfg.y"
    { 
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															IP_ST,
															NUMBER_ST,
															(void*)(yyvsp[-1].ipaddr),
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										;}
    break;

  case 240:
#line 1186 "cfg.y"
    { 
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 STRING_ST,
															 NUMBER_ST,
															(yyvsp[-3].strval),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
												 ;}
    break;

  case 241:
#line 1199 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 STRING_ST,
															 NUMBER_ST,
															(yyvsp[-3].strval),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													;}
    break;

  case 242:
#line 1212 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 IP_ST,
															 NUMBER_ST,
															 (void*)(yyvsp[-3].ipaddr),
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
												  ;}
    break;

  case 243:
#line 1225 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 URIPORT_ST,
															0,
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													;}
    break;

  case 244:
#line 1240 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 NUMBER_ST,
															0,
															(void*)(yyvsp[-1].intval));
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
													;}
    break;

  case 245:
#line 1253 "cfg.y"
    {
										#ifdef USE_TLS
											(yyval.action)=mk_action(	FORWARD_TLS_T,
															 URIHOST_ST,
															 NUMBER_ST,
															0,
															0);
										#else
											(yyval.action)=0;
											yyerror("tls support not "
													"compiled in");
										#endif
										;}
    break;

  case 246:
#line 1266 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 247:
#line 1267 "cfg.y"
    { (yyval.action)=0; yyerror("bad forward_tls"
										"argument"); ;}
    break;

  case 248:
#line 1270 "cfg.y"
    { (yyval.action)=mk_action(	SEND_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									;}
    break;

  case 249:
#line 1276 "cfg.y"
    { (yyval.action)=mk_action(	SEND_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									;}
    break;

  case 250:
#line 1282 "cfg.y"
    { (yyval.action)=mk_action(	SEND_T,
													IP_ST,
													NUMBER_ST,
													(void*)(yyvsp[-1].ipaddr),
													0);
									;}
    break;

  case 251:
#line 1288 "cfg.y"
    { (yyval.action)=mk_action(	SEND_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												;}
    break;

  case 252:
#line 1294 "cfg.y"
    {(yyval.action)=mk_action(	SEND_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												;}
    break;

  case 253:
#line 1300 "cfg.y"
    { (yyval.action)=mk_action(	SEND_T,
																IP_ST,
																NUMBER_ST,
																(void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
											   ;}
    break;

  case 254:
#line 1306 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 255:
#line 1307 "cfg.y"
    { (yyval.action)=0; yyerror("bad send"
													"argument"); ;}
    break;

  case 256:
#line 1309 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									;}
    break;

  case 257:
#line 1315 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T,
													STRING_ST,
													NUMBER_ST,
													(yyvsp[-1].strval),
													0);
									;}
    break;

  case 258:
#line 1321 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T,
													IP_ST,
													NUMBER_ST,
													(void*)(yyvsp[-1].ipaddr),
													0);
									;}
    break;

  case 259:
#line 1327 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												;}
    break;

  case 260:
#line 1333 "cfg.y"
    {(yyval.action)=mk_action(	SEND_TCP_T,
																STRING_ST,
																NUMBER_ST,
																(yyvsp[-3].strval),
																(void*)(yyvsp[-1].intval));
												;}
    break;

  case 261:
#line 1339 "cfg.y"
    { (yyval.action)=mk_action(	SEND_TCP_T,
																IP_ST,
																NUMBER_ST,
																(void*)(yyvsp[-3].ipaddr),
																(void*)(yyvsp[-1].intval));
											   ;}
    break;

  case 262:
#line 1345 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 263:
#line 1346 "cfg.y"
    { (yyval.action)=0; yyerror("bad send_tcp"
													"argument"); ;}
    break;

  case 264:
#line 1348 "cfg.y"
    {(yyval.action)=mk_action(DROP_T,0, 0, 0, 0); ;}
    break;

  case 265:
#line 1349 "cfg.y"
    {(yyval.action)=mk_action(DROP_T,0, 0, 0, 0); ;}
    break;

  case 266:
#line 1350 "cfg.y"
    {(yyval.action)=mk_action(	LOG_T, NUMBER_ST, 
													STRING_ST,(void*)4,(yyvsp[-1].strval));
									;}
    break;

  case 267:
#line 1353 "cfg.y"
    {(yyval.action)=mk_action(	LOG_T,
																NUMBER_ST, 
																STRING_ST,
																(void*)(yyvsp[-3].intval),
																(yyvsp[-1].strval));
												;}
    break;

  case 268:
#line 1359 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 269:
#line 1360 "cfg.y"
    { (yyval.action)=0; yyerror("bad log"
									"argument"); ;}
    break;

  case 270:
#line 1362 "cfg.y"
    {(yyval.action)=mk_action( SETFLAG_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); ;}
    break;

  case 271:
#line 1364 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 272:
#line 1365 "cfg.y"
    {(yyval.action)=mk_action(	RESETFLAG_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); ;}
    break;

  case 273:
#line 1367 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 274:
#line 1368 "cfg.y"
    {(yyval.action)=mk_action(	ISFLAGSET_T, NUMBER_ST, 0,
													(void *)(yyvsp[-1].intval), 0 ); ;}
    break;

  case 275:
#line 1370 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')'?"); ;}
    break;

  case 276:
#line 1371 "cfg.y"
    {(yyval.action)=mk_action(ERROR_T,
																STRING_ST, 
																STRING_ST,
																(yyvsp[-3].strval),
																(yyvsp[-1].strval));
												  ;}
    break;

  case 277:
#line 1377 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 278:
#line 1378 "cfg.y"
    { (yyval.action)=0; yyerror("bad error"
														"argument"); ;}
    break;

  case 279:
#line 1380 "cfg.y"
    { (yyval.action)=mk_action(ROUTE_T, NUMBER_ST,
														0, (void*)(yyvsp[-1].intval), 0);
										;}
    break;

  case 280:
#line 1383 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 281:
#line 1384 "cfg.y"
    { (yyval.action)=0; yyerror("bad route"
						"argument"); ;}
    break;

  case 282:
#line 1386 "cfg.y"
    { (yyval.action)=mk_action(	EXEC_T, STRING_ST, 0,
													(yyvsp[-1].strval), 0);
									;}
    break;

  case 283:
#line 1389 "cfg.y"
    { (yyval.action)=mk_action(SET_HOST_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); ;}
    break;

  case 284:
#line 1391 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 285:
#line 1392 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 286:
#line 1395 "cfg.y"
    { (yyval.action)=mk_action(PREFIX_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); ;}
    break;

  case 287:
#line 1397 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 288:
#line 1398 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 289:
#line 1400 "cfg.y"
    { (yyval.action)=mk_action(STRIP_TAIL_T, 
									NUMBER_ST, 0, (void *) (yyvsp[-1].intval), 0); ;}
    break;

  case 290:
#line 1402 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 291:
#line 1403 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"number expected"); ;}
    break;

  case 292:
#line 1406 "cfg.y"
    { (yyval.action)=mk_action(STRIP_T, NUMBER_ST,
														0, (void *) (yyvsp[-1].intval), 0); ;}
    break;

  case 293:
#line 1408 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 294:
#line 1409 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"number expected"); ;}
    break;

  case 295:
#line 1412 "cfg.y"
    { (yyval.action)=mk_action( APPEND_BRANCH_T,
													STRING_ST, 0, (yyvsp[-1].strval), 0) ; ;}
    break;

  case 296:
#line 1414 "cfg.y"
    { (yyval.action)=mk_action( APPEND_BRANCH_T,
													STRING_ST, 0, 0, 0 ) ; ;}
    break;

  case 297:
#line 1416 "cfg.y"
    {  (yyval.action)=mk_action( APPEND_BRANCH_T, STRING_ST, 0, 0, 0 ) ; ;}
    break;

  case 298:
#line 1418 "cfg.y"
    { (yyval.action)=mk_action( SET_HOSTPORT_T, 
														STRING_ST, 0, (yyvsp[-1].strval), 0); ;}
    break;

  case 299:
#line 1420 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 300:
#line 1421 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument,"
												" string expected"); ;}
    break;

  case 301:
#line 1423 "cfg.y"
    { (yyval.action)=mk_action( SET_PORT_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); ;}
    break;

  case 302:
#line 1425 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 303:
#line 1426 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 304:
#line 1428 "cfg.y"
    { (yyval.action)=mk_action( SET_USER_T, STRING_ST,
														0, (yyvsp[-1].strval), 0); ;}
    break;

  case 305:
#line 1430 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 306:
#line 1431 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 307:
#line 1433 "cfg.y"
    { (yyval.action)=mk_action( SET_USERPASS_T, 
														STRING_ST, 0, (yyvsp[-1].strval), 0); ;}
    break;

  case 308:
#line 1435 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 309:
#line 1436 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 310:
#line 1438 "cfg.y"
    { (yyval.action)=mk_action( SET_URI_T, STRING_ST, 
														0, (yyvsp[-1].strval), 0); ;}
    break;

  case 311:
#line 1440 "cfg.y"
    { (yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 312:
#line 1441 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
										"string expected"); ;}
    break;

  case 313:
#line 1443 "cfg.y"
    { (yyval.action)=mk_action( REVERT_URI_T, 0,0,0,0); ;}
    break;

  case 314:
#line 1444 "cfg.y"
    { (yyval.action)=mk_action( REVERT_URI_T, 0,0,0,0); ;}
    break;

  case 315:
#line 1445 "cfg.y"
    {(yyval.action)=mk_action(FORCE_RPORT_T,0, 0, 0, 0); ;}
    break;

  case 316:
#line 1446 "cfg.y"
    {(yyval.action)=mk_action(FORCE_RPORT_T,0, 0, 0, 0); ;}
    break;

  case 317:
#line 1447 "cfg.y"
    {
								(yyval.action)=0;
								if ((str_tmp=pkg_malloc(sizeof(str)))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
								}else{
										str_tmp->s=(yyvsp[-1].strval);
										str_tmp->len=strlen((yyvsp[-1].strval));
										(yyval.action)=mk_action(SET_ADV_ADDR_T, STR_ST,
										             0, str_tmp, 0);
								}
												  ;}
    break;

  case 318:
#line 1459 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 319:
#line 1461 "cfg.y"
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 320:
#line 1462 "cfg.y"
    {
								(yyval.action)=0;
								tmp=int2str((yyvsp[-1].intval), &i_tmp);
								if ((str_tmp=pkg_malloc(sizeof(str)))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
								}else{
									if ((str_tmp->s=pkg_malloc(i_tmp))==0){
										LOG(L_CRIT, "ERROR: cfg. parser:"
													" out of memory.\n");
									}else{
										memcpy(str_tmp->s, tmp, i_tmp);
										str_tmp->len=i_tmp;
										(yyval.action)=mk_action(SET_ADV_PORT_T, STR_ST,
													0, str_tmp, 0);
									}
								}
								            ;}
    break;

  case 321:
#line 1480 "cfg.y"
    { (yyval.action)=0; yyerror("bad argument, "
														"string expected"); ;}
    break;

  case 322:
#line 1482 "cfg.y"
    {(yyval.action)=0; yyerror("missing '(' or ')' ?"); ;}
    break;

  case 323:
#line 1483 "cfg.y"
    { f_tmp=(void*)find_export((yyvsp[-2].strval), 0, rt);
									   if (f_tmp==0){
										   if (find_export((yyvsp[-2].strval), 0, 0)) {
											   yyerror("Command cannot be used in the block\n");
										   } else {
											   yyerror("unknown command, missing"
												   " loadmodule?\n");
										   }
										(yyval.action)=0;
									   }else{
										(yyval.action)=mk_action(	MODULE_T,
														CMDF_ST,
														0,
														f_tmp,
														0
													);
									   }
									;}
    break;

  case 324:
#line 1501 "cfg.y"
    { f_tmp=(void*)find_export((yyvsp[-3].strval), 1, rt);
									if (f_tmp==0){
										if (find_export((yyvsp[-3].strval), 1, 0)) {
											yyerror("Command cannot be used in the block\n");
										} else {
											yyerror("unknown command, missing"
												" loadmodule?\n");
										}
										(yyval.action)=0;
									}else{
										(yyval.action)=mk_action(	MODULE_T,
														CMDF_ST,
														STRING_ST,
														f_tmp,
														(yyvsp[-1].strval)
													);
									}
								  ;}
    break;

  case 325:
#line 1520 "cfg.y"
    { f_tmp=(void*)find_export((yyvsp[-5].strval), 2, rt);
									if (f_tmp==0){
										if (find_export((yyvsp[-5].strval), 2, 0)) {
											yyerror("Command cannot be used in the block\n");
										} else {
											yyerror("unknown command, missing"
												" loadmodule?\n");
										}
										(yyval.action)=0;
									}else{
										(yyval.action)=mk_action3(	MODULE_T,
														CMDF_ST,
														STRING_ST,
														STRING_ST,
														f_tmp,
														(yyvsp[-3].strval),
														(yyvsp[-1].strval)
													);
									}
								  ;}
    break;

  case 326:
#line 1540 "cfg.y"
    { (yyval.action)=0; yyerror("bad arguments"); ;}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 4453 "cfg.tab.c"

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
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
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
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
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


#line 1544 "cfg.y"


extern int line;
extern int column;
extern int startcolumn;
static void warn(char* s)
{
	LOG(L_WARN, "cfg. warning: (%d,%d-%d): %s\n", line, startcolumn, 
			column, s);
	cfg_errors++;
}

static void yyerror(char* s)
{
	LOG(L_CRIT, "parse error (%d,%d-%d): %s\n", line, startcolumn, 
			column, s);
	cfg_errors++;
}

/*
int main(int argc, char ** argv)
{
	if (yyparse()!=0)
		fprintf(stderr, "parsing error\n");
}
*/

