/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_PHPDBG_SAPI_PHPDBG_PHPDBG_PARSER_H_INCLUDED
# define YY_PHPDBG_SAPI_PHPDBG_PHPDBG_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef PHPDBG_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PHPDBG_DEBUG 1
#  else
#   define PHPDBG_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PHPDBG_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PHPDBG_DEBUG */
#if PHPDBG_DEBUG
extern int phpdbg_debug;
#endif
/* "%code requires" blocks.  */

#include "phpdbg.h"
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif


/* Token kinds.  */
#ifndef PHPDBG_TOKENTYPE
# define PHPDBG_TOKENTYPE
  enum phpdbg_tokentype
  {
    PHPDBG_EMPTY = -2,
    END = 0,                       /* "end of command"  */
    PHPDBG_error = 256,            /* error  */
    PHPDBG_UNDEF = 257,            /* "invalid token"  */
    T_EVAL = 258,                  /* "eval"  */
    T_RUN = 259,                   /* "run"  */
    T_SHELL = 260,                 /* "shell"  */
    T_IF = 261,                    /* "if (condition)"  */
    T_TRUTHY = 262,                /* "truthy (true, on, yes or enabled)"  */
    T_FALSY = 263,                 /* "falsy (false, off, no or disabled)"  */
    T_STRING = 264,                /* "string (some input, perhaps)"  */
    T_COLON = 265,                 /* ": (colon)"  */
    T_DCOLON = 266,                /* ":: (double colon)"  */
    T_POUND = 267,                 /* "# (pound sign followed by digits)"  */
    T_SEPARATOR = 268,             /* "# (pound sign)"  */
    T_PROTO = 269,                 /* "protocol (file://)"  */
    T_DIGITS = 270,                /* "digits (numbers)"  */
    T_LITERAL = 271,               /* "literal (string)"  */
    T_ADDR = 272,                  /* "address"  */
    T_OPCODE = 273,                /* "opcode"  */
    T_ID = 274,                    /* "identifier (command or function name)"  */
    T_INPUT = 275,                 /* "input (input string or data)"  */
    T_UNEXPECTED = 276,            /* "input"  */
    T_REQ_ID = 277                 /* "request id (-r %d)"  */
  };
  typedef enum phpdbg_tokentype phpdbg_token_kind_t;
#endif

/* Value type.  */
#if ! defined PHPDBG_STYPE && ! defined PHPDBG_STYPE_IS_DECLARED
typedef phpdbg_param_t PHPDBG_STYPE;
# define PHPDBG_STYPE_IS_TRIVIAL 1
# define PHPDBG_STYPE_IS_DECLARED 1
#endif




int phpdbg_parse (void);


#endif /* !YY_PHPDBG_SAPI_PHPDBG_PHPDBG_PARSER_H_INCLUDED  */
