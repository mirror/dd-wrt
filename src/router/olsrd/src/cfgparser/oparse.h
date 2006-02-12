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
     TOK_LQ_WSIZE = 284,
     TOK_LQ_MULT = 285,
     TOK_CLEAR_SCREEN = 286,
     TOK_PLNAME = 287,
     TOK_PLPARAM = 288,
     TOK_HOSTLABEL = 289,
     TOK_NETLABEL = 290,
     TOK_MAXIPC = 291,
     TOK_IP4BROADCAST = 292,
     TOK_IP6ADDRTYPE = 293,
     TOK_IP6MULTISITE = 294,
     TOK_IP6MULTIGLOBAL = 295,
     TOK_IFWEIGHT = 296,
     TOK_HELLOINT = 297,
     TOK_HELLOVAL = 298,
     TOK_TCINT = 299,
     TOK_TCVAL = 300,
     TOK_MIDINT = 301,
     TOK_MIDVAL = 302,
     TOK_HNAINT = 303,
     TOK_HNAVAL = 304,
     TOK_IP4_ADDR = 305,
     TOK_IP6_ADDR = 306,
     TOK_DEFAULT = 307,
     TOK_COMMENT = 308
   };
#endif
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
#define TOK_LQ_WSIZE 284
#define TOK_LQ_MULT 285
#define TOK_CLEAR_SCREEN 286
#define TOK_PLNAME 287
#define TOK_PLPARAM 288
#define TOK_HOSTLABEL 289
#define TOK_NETLABEL 290
#define TOK_MAXIPC 291
#define TOK_IP4BROADCAST 292
#define TOK_IP6ADDRTYPE 293
#define TOK_IP6MULTISITE 294
#define TOK_IP6MULTIGLOBAL 295
#define TOK_IFWEIGHT 296
#define TOK_HELLOINT 297
#define TOK_HELLOVAL 298
#define TOK_TCINT 299
#define TOK_TCVAL 300
#define TOK_MIDINT 301
#define TOK_MIDVAL 302
#define TOK_HNAINT 303
#define TOK_HNAVAL 304
#define TOK_IP4_ADDR 305
#define TOK_IP6_ADDR 306
#define TOK_DEFAULT 307
#define TOK_COMMENT 308




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



