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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



