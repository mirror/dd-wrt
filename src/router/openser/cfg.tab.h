/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
     EXIT = 265,
     RETURN = 266,
     RETCODE = 267,
     LOG_TOK = 268,
     ERROR = 269,
     ROUTE = 270,
     ROUTE_FAILURE = 271,
     ROUTE_ONREPLY = 272,
     ROUTE_BRANCH = 273,
     EXEC = 274,
     SET_HOST = 275,
     SET_HOSTPORT = 276,
     PREFIX = 277,
     STRIP = 278,
     STRIP_TAIL = 279,
     APPEND_BRANCH = 280,
     SET_USER = 281,
     SET_USERPASS = 282,
     SET_PORT = 283,
     SET_URI = 284,
     REVERT_URI = 285,
     SET_DSTURI = 286,
     RESET_DSTURI = 287,
     ISDSTURISET = 288,
     FORCE_RPORT = 289,
     FORCE_LOCAL_RPORT = 290,
     FORCE_TCP_ALIAS = 291,
     IF = 292,
     ELSE = 293,
     SWITCH = 294,
     CASE = 295,
     DEFAULT = 296,
     SBREAK = 297,
     SET_ADV_ADDRESS = 298,
     SET_ADV_PORT = 299,
     FORCE_SEND_SOCKET = 300,
     URIHOST = 301,
     URIPORT = 302,
     MAX_LEN = 303,
     SETFLAG = 304,
     RESETFLAG = 305,
     ISFLAGSET = 306,
     METHOD = 307,
     URI = 308,
     FROM_URI = 309,
     TO_URI = 310,
     SRCIP = 311,
     SRCPORT = 312,
     DSTIP = 313,
     DSTPORT = 314,
     PROTO = 315,
     AF = 316,
     MYSELF = 317,
     MSGLEN = 318,
     UDP = 319,
     TCP = 320,
     TLS = 321,
     DEBUG = 322,
     FORK = 323,
     LOGSTDERROR = 324,
     LOGFACILITY = 325,
     LOGNAME = 326,
     LISTEN = 327,
     ALIAS = 328,
     DNS = 329,
     REV_DNS = 330,
     DNS_TRY_IPV6 = 331,
     DNS_RETR_TIME = 332,
     DNS_RETR_NO = 333,
     DNS_SERVERS_NO = 334,
     DNS_USE_SEARCH = 335,
     PORT = 336,
     STAT = 337,
     CHILDREN = 338,
     CHECK_VIA = 339,
     SYN_BRANCH = 340,
     MEMLOG = 341,
     SIP_WARNING = 342,
     FIFO = 343,
     FIFO_DIR = 344,
     SOCK_MODE = 345,
     SOCK_USER = 346,
     SOCK_GROUP = 347,
     FIFO_DB_URL = 348,
     UNIX_SOCK = 349,
     UNIX_SOCK_CHILDREN = 350,
     UNIX_TX_TIMEOUT = 351,
     SERVER_SIGNATURE = 352,
     REPLY_TO_VIA = 353,
     LOADMODULE = 354,
     MPATH = 355,
     MODPARAM = 356,
     MAXBUFFER = 357,
     USER = 358,
     GROUP = 359,
     CHROOT = 360,
     WDIR = 361,
     MHOMED = 362,
     DISABLE_TCP = 363,
     TCP_ACCEPT_ALIASES = 364,
     TCP_CHILDREN = 365,
     TCP_CONNECT_TIMEOUT = 366,
     TCP_SEND_TIMEOUT = 367,
     DISABLE_TLS = 368,
     TLSLOG = 369,
     TLS_PORT_NO = 370,
     TLS_METHOD = 371,
     TLS_HANDSHAKE_TIMEOUT = 372,
     TLS_SEND_TIMEOUT = 373,
     SSLv23 = 374,
     SSLv2 = 375,
     SSLv3 = 376,
     TLSv1 = 377,
     TLS_VERIFY = 378,
     TLS_REQUIRE_CERTIFICATE = 379,
     TLS_CERTIFICATE = 380,
     TLS_PRIVATE_KEY = 381,
     TLS_CA_LIST = 382,
     TLS_CIPHERS_LIST = 383,
     ADVERTISED_ADDRESS = 384,
     ADVERTISED_PORT = 385,
     DISABLE_CORE = 386,
     OPEN_FD_LIMIT = 387,
     MCAST_LOOPBACK = 388,
     MCAST_TTL = 389,
     TLS_DOMAIN = 390,
     EQUAL = 391,
     EQUAL_T = 392,
     GT = 393,
     LT = 394,
     GTE = 395,
     LTE = 396,
     DIFF = 397,
     MATCH = 398,
     OR = 399,
     AND = 400,
     NOT = 401,
     PLUS = 402,
     MINUS = 403,
     NUMBER = 404,
     ID = 405,
     STRING = 406,
     IPV6ADDR = 407,
     COMMA = 408,
     SEMICOLON = 409,
     RPAREN = 410,
     LPAREN = 411,
     LBRACE = 412,
     RBRACE = 413,
     LBRACK = 414,
     RBRACK = 415,
     SLASH = 416,
     DOT = 417,
     CR = 418,
     COLON = 419,
     STAR = 420
   };
#endif
/* Tokens.  */
#define FORWARD 258
#define FORWARD_TCP 259
#define FORWARD_TLS 260
#define FORWARD_UDP 261
#define SEND 262
#define SEND_TCP 263
#define DROP 264
#define EXIT 265
#define RETURN 266
#define RETCODE 267
#define LOG_TOK 268
#define ERROR 269
#define ROUTE 270
#define ROUTE_FAILURE 271
#define ROUTE_ONREPLY 272
#define ROUTE_BRANCH 273
#define EXEC 274
#define SET_HOST 275
#define SET_HOSTPORT 276
#define PREFIX 277
#define STRIP 278
#define STRIP_TAIL 279
#define APPEND_BRANCH 280
#define SET_USER 281
#define SET_USERPASS 282
#define SET_PORT 283
#define SET_URI 284
#define REVERT_URI 285
#define SET_DSTURI 286
#define RESET_DSTURI 287
#define ISDSTURISET 288
#define FORCE_RPORT 289
#define FORCE_LOCAL_RPORT 290
#define FORCE_TCP_ALIAS 291
#define IF 292
#define ELSE 293
#define SWITCH 294
#define CASE 295
#define DEFAULT 296
#define SBREAK 297
#define SET_ADV_ADDRESS 298
#define SET_ADV_PORT 299
#define FORCE_SEND_SOCKET 300
#define URIHOST 301
#define URIPORT 302
#define MAX_LEN 303
#define SETFLAG 304
#define RESETFLAG 305
#define ISFLAGSET 306
#define METHOD 307
#define URI 308
#define FROM_URI 309
#define TO_URI 310
#define SRCIP 311
#define SRCPORT 312
#define DSTIP 313
#define DSTPORT 314
#define PROTO 315
#define AF 316
#define MYSELF 317
#define MSGLEN 318
#define UDP 319
#define TCP 320
#define TLS 321
#define DEBUG 322
#define FORK 323
#define LOGSTDERROR 324
#define LOGFACILITY 325
#define LOGNAME 326
#define LISTEN 327
#define ALIAS 328
#define DNS 329
#define REV_DNS 330
#define DNS_TRY_IPV6 331
#define DNS_RETR_TIME 332
#define DNS_RETR_NO 333
#define DNS_SERVERS_NO 334
#define DNS_USE_SEARCH 335
#define PORT 336
#define STAT 337
#define CHILDREN 338
#define CHECK_VIA 339
#define SYN_BRANCH 340
#define MEMLOG 341
#define SIP_WARNING 342
#define FIFO 343
#define FIFO_DIR 344
#define SOCK_MODE 345
#define SOCK_USER 346
#define SOCK_GROUP 347
#define FIFO_DB_URL 348
#define UNIX_SOCK 349
#define UNIX_SOCK_CHILDREN 350
#define UNIX_TX_TIMEOUT 351
#define SERVER_SIGNATURE 352
#define REPLY_TO_VIA 353
#define LOADMODULE 354
#define MPATH 355
#define MODPARAM 356
#define MAXBUFFER 357
#define USER 358
#define GROUP 359
#define CHROOT 360
#define WDIR 361
#define MHOMED 362
#define DISABLE_TCP 363
#define TCP_ACCEPT_ALIASES 364
#define TCP_CHILDREN 365
#define TCP_CONNECT_TIMEOUT 366
#define TCP_SEND_TIMEOUT 367
#define DISABLE_TLS 368
#define TLSLOG 369
#define TLS_PORT_NO 370
#define TLS_METHOD 371
#define TLS_HANDSHAKE_TIMEOUT 372
#define TLS_SEND_TIMEOUT 373
#define SSLv23 374
#define SSLv2 375
#define SSLv3 376
#define TLSv1 377
#define TLS_VERIFY 378
#define TLS_REQUIRE_CERTIFICATE 379
#define TLS_CERTIFICATE 380
#define TLS_PRIVATE_KEY 381
#define TLS_CA_LIST 382
#define TLS_CIPHERS_LIST 383
#define ADVERTISED_ADDRESS 384
#define ADVERTISED_PORT 385
#define DISABLE_CORE 386
#define OPEN_FD_LIMIT 387
#define MCAST_LOOPBACK 388
#define MCAST_TTL 389
#define TLS_DOMAIN 390
#define EQUAL 391
#define EQUAL_T 392
#define GT 393
#define LT 394
#define GTE 395
#define LTE 396
#define DIFF 397
#define MATCH 398
#define OR 399
#define AND 400
#define NOT 401
#define PLUS 402
#define MINUS 403
#define NUMBER 404
#define ID 405
#define STRING 406
#define IPV6ADDR 407
#define COMMA 408
#define SEMICOLON 409
#define RPAREN 410
#define LPAREN 411
#define LBRACE 412
#define RBRACE 413
#define LBRACK 414
#define RBRACK 415
#define SLASH 416
#define DOT 417
#define CR 418
#define COLON 419
#define STAR 420




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 119 "cfg.y"
{
	long intval;
	unsigned long uval;
	char* strval;
	struct expr* expr;
	struct action* action;
	struct net* ipnet;
	struct ip_addr* ipaddr;
	struct socket_id* sockid;
}
/* Line 1529 of yacc.c.  */
#line 390 "cfg.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

