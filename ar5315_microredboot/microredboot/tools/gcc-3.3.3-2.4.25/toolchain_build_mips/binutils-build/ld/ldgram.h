
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     NAME = 259,
     LNAME = 260,
     OREQ = 261,
     ANDEQ = 262,
     RSHIFTEQ = 263,
     LSHIFTEQ = 264,
     DIVEQ = 265,
     MULTEQ = 266,
     MINUSEQ = 267,
     PLUSEQ = 268,
     OROR = 269,
     ANDAND = 270,
     NE = 271,
     EQ = 272,
     GE = 273,
     LE = 274,
     RSHIFT = 275,
     LSHIFT = 276,
     UNARY = 277,
     END = 278,
     ALIGN_K = 279,
     BLOCK = 280,
     BIND = 281,
     QUAD = 282,
     SQUAD = 283,
     LONG = 284,
     SHORT = 285,
     BYTE = 286,
     SECTIONS = 287,
     PHDRS = 288,
     SORT = 289,
     DATA_SEGMENT_ALIGN = 290,
     DATA_SEGMENT_END = 291,
     SIZEOF_HEADERS = 292,
     OUTPUT_FORMAT = 293,
     FORCE_COMMON_ALLOCATION = 294,
     OUTPUT_ARCH = 295,
     INHIBIT_COMMON_ALLOCATION = 296,
     INCLUDE = 297,
     MEMORY = 298,
     DEFSYMEND = 299,
     NOLOAD = 300,
     DSECT = 301,
     COPY = 302,
     INFO = 303,
     OVERLAY = 304,
     DEFINED = 305,
     TARGET_K = 306,
     SEARCH_DIR = 307,
     MAP = 308,
     ENTRY = 309,
     NEXT = 310,
     SIZEOF = 311,
     ADDR = 312,
     LOADADDR = 313,
     MAX_K = 314,
     MIN_K = 315,
     STARTUP = 316,
     HLL = 317,
     SYSLIB = 318,
     FLOAT = 319,
     NOFLOAT = 320,
     NOCROSSREFS = 321,
     ORIGIN = 322,
     FILL = 323,
     LENGTH = 324,
     CREATE_OBJECT_SYMBOLS = 325,
     INPUT = 326,
     GROUP = 327,
     OUTPUT = 328,
     CONSTRUCTORS = 329,
     ALIGNMOD = 330,
     AT = 331,
     SUBALIGN = 332,
     PROVIDE = 333,
     CHIP = 334,
     LIST = 335,
     SECT = 336,
     ABSOLUTE = 337,
     LOAD = 338,
     NEWLINE = 339,
     ENDWORD = 340,
     ORDER = 341,
     NAMEWORD = 342,
     ASSERT_K = 343,
     FORMAT = 344,
     PUBLIC = 345,
     BASE = 346,
     ALIAS = 347,
     TRUNCATE = 348,
     REL = 349,
     INPUT_SCRIPT = 350,
     INPUT_MRI_SCRIPT = 351,
     INPUT_DEFSYM = 352,
     CASE = 353,
     EXTERN = 354,
     START = 355,
     VERS_TAG = 356,
     VERS_IDENTIFIER = 357,
     GLOBAL = 358,
     LOCAL = 359,
     VERSIONK = 360,
     INPUT_VERSION_SCRIPT = 361,
     KEEP = 362,
     EXCLUDE_FILE = 363
   };
#endif
/* Tokens.  */
#define INT 258
#define NAME 259
#define LNAME 260
#define OREQ 261
#define ANDEQ 262
#define RSHIFTEQ 263
#define LSHIFTEQ 264
#define DIVEQ 265
#define MULTEQ 266
#define MINUSEQ 267
#define PLUSEQ 268
#define OROR 269
#define ANDAND 270
#define NE 271
#define EQ 272
#define GE 273
#define LE 274
#define RSHIFT 275
#define LSHIFT 276
#define UNARY 277
#define END 278
#define ALIGN_K 279
#define BLOCK 280
#define BIND 281
#define QUAD 282
#define SQUAD 283
#define LONG 284
#define SHORT 285
#define BYTE 286
#define SECTIONS 287
#define PHDRS 288
#define SORT 289
#define DATA_SEGMENT_ALIGN 290
#define DATA_SEGMENT_END 291
#define SIZEOF_HEADERS 292
#define OUTPUT_FORMAT 293
#define FORCE_COMMON_ALLOCATION 294
#define OUTPUT_ARCH 295
#define INHIBIT_COMMON_ALLOCATION 296
#define INCLUDE 297
#define MEMORY 298
#define DEFSYMEND 299
#define NOLOAD 300
#define DSECT 301
#define COPY 302
#define INFO 303
#define OVERLAY 304
#define DEFINED 305
#define TARGET_K 306
#define SEARCH_DIR 307
#define MAP 308
#define ENTRY 309
#define NEXT 310
#define SIZEOF 311
#define ADDR 312
#define LOADADDR 313
#define MAX_K 314
#define MIN_K 315
#define STARTUP 316
#define HLL 317
#define SYSLIB 318
#define FLOAT 319
#define NOFLOAT 320
#define NOCROSSREFS 321
#define ORIGIN 322
#define FILL 323
#define LENGTH 324
#define CREATE_OBJECT_SYMBOLS 325
#define INPUT 326
#define GROUP 327
#define OUTPUT 328
#define CONSTRUCTORS 329
#define ALIGNMOD 330
#define AT 331
#define SUBALIGN 332
#define PROVIDE 333
#define CHIP 334
#define LIST 335
#define SECT 336
#define ABSOLUTE 337
#define LOAD 338
#define NEWLINE 339
#define ENDWORD 340
#define ORDER 341
#define NAMEWORD 342
#define ASSERT_K 343
#define FORMAT 344
#define PUBLIC 345
#define BASE 346
#define ALIAS 347
#define TRUNCATE 348
#define REL 349
#define INPUT_SCRIPT 350
#define INPUT_MRI_SCRIPT 351
#define INPUT_DEFSYM 352
#define CASE 353
#define EXTERN 354
#define START 355
#define VERS_TAG 356
#define VERS_IDENTIFIER 357
#define GLOBAL 358
#define LOCAL 359
#define VERSIONK 360
#define INPUT_VERSION_SCRIPT 361
#define KEEP 362
#define EXCLUDE_FILE 363




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 67 "/home/xfs/projects/AR531X/WLAN/5.3.1.48/tools/gcc-3.3.3-2.4.25/toolchain_build_mips/binutils-2.14.90.0.6/ld/ldgram.y"

  bfd_vma integer;
  struct big_int
    {
      bfd_vma integer;
      char *str;
    } bigint;
  fill_type *fill;
  char *name;
  const char *cname;
  struct wildcard_spec wildcard;
  struct wildcard_list *wildcard_list;
  struct name_list *name_list;
  int token;
  union etree_union *etree;
  struct phdr_info
    {
      bfd_boolean filehdr;
      bfd_boolean phdrs;
      union etree_union *at;
      union etree_union *flags;
    } phdr;
  struct lang_nocrossref *nocrossref;
  struct lang_output_section_phdr_list *section_phdr;
  struct bfd_elf_version_deps *deflist;
  struct bfd_elf_version_expr *versyms;
  struct bfd_elf_version_tree *versnode;



/* Line 1676 of yacc.c  */
#line 299 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


