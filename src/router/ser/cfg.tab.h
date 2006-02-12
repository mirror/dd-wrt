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
/* Line 1318 of yacc.c.  */
#line 290 "cfg.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



