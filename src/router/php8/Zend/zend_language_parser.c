/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */

#include "zend.h"
#include "zend_list.h"
#include "zend_globals.h"
#include "zend_API.h"
#include "zend_constants.h"
#include "zend_language_scanner.h"
#include "zend_exceptions.h"

#define YYSIZE_T size_t
#define yytnamerr zend_yytnamerr
static YYSIZE_T zend_yytnamerr(char*, const char*);

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

/* Substitute the type names.  */
#define YYSTYPE         ZENDSTYPE
/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yydebug         zenddebug
#define yynerrs         zendnerrs


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "zend_language_parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PREC_ARROW_FUNCTION = 3,        /* PREC_ARROW_FUNCTION  */
  YYSYMBOL_4_ = 4,                         /* '='  */
  YYSYMBOL_5_ = 5,                         /* '?'  */
  YYSYMBOL_6_ = 6,                         /* ':'  */
  YYSYMBOL_7_ = 7,                         /* '|'  */
  YYSYMBOL_8_ = 8,                         /* '^'  */
  YYSYMBOL_9_ = 9,                         /* '<'  */
  YYSYMBOL_10_ = 10,                       /* '>'  */
  YYSYMBOL_11_ = 11,                       /* '.'  */
  YYSYMBOL_12_ = 12,                       /* '+'  */
  YYSYMBOL_13_ = 13,                       /* '-'  */
  YYSYMBOL_14_ = 14,                       /* '*'  */
  YYSYMBOL_15_ = 15,                       /* '/'  */
  YYSYMBOL_16_ = 16,                       /* '%'  */
  YYSYMBOL_17_ = 17,                       /* '!'  */
  YYSYMBOL_18_ = 18,                       /* '~'  */
  YYSYMBOL_19_ = 19,                       /* '@'  */
  YYSYMBOL_T_NOELSE = 20,                  /* T_NOELSE  */
  YYSYMBOL_T_LNUMBER = 21,                 /* "integer"  */
  YYSYMBOL_T_DNUMBER = 22,                 /* "floating-point number"  */
  YYSYMBOL_T_STRING = 23,                  /* "identifier"  */
  YYSYMBOL_T_NAME_FULLY_QUALIFIED = 24,    /* "fully qualified name"  */
  YYSYMBOL_T_NAME_RELATIVE = 25,           /* "namespace-relative name"  */
  YYSYMBOL_T_NAME_QUALIFIED = 26,          /* "namespaced name"  */
  YYSYMBOL_T_VARIABLE = 27,                /* "variable"  */
  YYSYMBOL_T_INLINE_HTML = 28,             /* T_INLINE_HTML  */
  YYSYMBOL_T_ENCAPSED_AND_WHITESPACE = 29, /* "string content"  */
  YYSYMBOL_T_CONSTANT_ENCAPSED_STRING = 30, /* "quoted string"  */
  YYSYMBOL_T_STRING_VARNAME = 31,          /* "variable name"  */
  YYSYMBOL_T_NUM_STRING = 32,              /* "number"  */
  YYSYMBOL_T_INCLUDE = 33,                 /* "'include'"  */
  YYSYMBOL_T_INCLUDE_ONCE = 34,            /* "'include_once'"  */
  YYSYMBOL_T_EVAL = 35,                    /* "'eval'"  */
  YYSYMBOL_T_REQUIRE = 36,                 /* "'require'"  */
  YYSYMBOL_T_REQUIRE_ONCE = 37,            /* "'require_once'"  */
  YYSYMBOL_T_LOGICAL_OR = 38,              /* "'or'"  */
  YYSYMBOL_T_LOGICAL_XOR = 39,             /* "'xor'"  */
  YYSYMBOL_T_LOGICAL_AND = 40,             /* "'and'"  */
  YYSYMBOL_T_PRINT = 41,                   /* "'print'"  */
  YYSYMBOL_T_YIELD = 42,                   /* "'yield'"  */
  YYSYMBOL_T_YIELD_FROM = 43,              /* "'yield from'"  */
  YYSYMBOL_T_INSTANCEOF = 44,              /* "'instanceof'"  */
  YYSYMBOL_T_NEW = 45,                     /* "'new'"  */
  YYSYMBOL_T_CLONE = 46,                   /* "'clone'"  */
  YYSYMBOL_T_EXIT = 47,                    /* "'exit'"  */
  YYSYMBOL_T_IF = 48,                      /* "'if'"  */
  YYSYMBOL_T_ELSEIF = 49,                  /* "'elseif'"  */
  YYSYMBOL_T_ELSE = 50,                    /* "'else'"  */
  YYSYMBOL_T_ENDIF = 51,                   /* "'endif'"  */
  YYSYMBOL_T_ECHO = 52,                    /* "'echo'"  */
  YYSYMBOL_T_DO = 53,                      /* "'do'"  */
  YYSYMBOL_T_WHILE = 54,                   /* "'while'"  */
  YYSYMBOL_T_ENDWHILE = 55,                /* "'endwhile'"  */
  YYSYMBOL_T_FOR = 56,                     /* "'for'"  */
  YYSYMBOL_T_ENDFOR = 57,                  /* "'endfor'"  */
  YYSYMBOL_T_FOREACH = 58,                 /* "'foreach'"  */
  YYSYMBOL_T_ENDFOREACH = 59,              /* "'endforeach'"  */
  YYSYMBOL_T_DECLARE = 60,                 /* "'declare'"  */
  YYSYMBOL_T_ENDDECLARE = 61,              /* "'enddeclare'"  */
  YYSYMBOL_T_AS = 62,                      /* "'as'"  */
  YYSYMBOL_T_SWITCH = 63,                  /* "'switch'"  */
  YYSYMBOL_T_ENDSWITCH = 64,               /* "'endswitch'"  */
  YYSYMBOL_T_CASE = 65,                    /* "'case'"  */
  YYSYMBOL_T_DEFAULT = 66,                 /* "'default'"  */
  YYSYMBOL_T_MATCH = 67,                   /* "'match'"  */
  YYSYMBOL_T_BREAK = 68,                   /* "'break'"  */
  YYSYMBOL_T_CONTINUE = 69,                /* "'continue'"  */
  YYSYMBOL_T_GOTO = 70,                    /* "'goto'"  */
  YYSYMBOL_T_FUNCTION = 71,                /* "'function'"  */
  YYSYMBOL_T_FN = 72,                      /* "'fn'"  */
  YYSYMBOL_T_CONST = 73,                   /* "'const'"  */
  YYSYMBOL_T_RETURN = 74,                  /* "'return'"  */
  YYSYMBOL_T_TRY = 75,                     /* "'try'"  */
  YYSYMBOL_T_CATCH = 76,                   /* "'catch'"  */
  YYSYMBOL_T_FINALLY = 77,                 /* "'finally'"  */
  YYSYMBOL_T_THROW = 78,                   /* "'throw'"  */
  YYSYMBOL_T_USE = 79,                     /* "'use'"  */
  YYSYMBOL_T_INSTEADOF = 80,               /* "'insteadof'"  */
  YYSYMBOL_T_GLOBAL = 81,                  /* "'global'"  */
  YYSYMBOL_T_STATIC = 82,                  /* "'static'"  */
  YYSYMBOL_T_ABSTRACT = 83,                /* "'abstract'"  */
  YYSYMBOL_T_FINAL = 84,                   /* "'final'"  */
  YYSYMBOL_T_PRIVATE = 85,                 /* "'private'"  */
  YYSYMBOL_T_PROTECTED = 86,               /* "'protected'"  */
  YYSYMBOL_T_PUBLIC = 87,                  /* "'public'"  */
  YYSYMBOL_T_READONLY = 88,                /* "'readonly'"  */
  YYSYMBOL_T_VAR = 89,                     /* "'var'"  */
  YYSYMBOL_T_UNSET = 90,                   /* "'unset'"  */
  YYSYMBOL_T_ISSET = 91,                   /* "'isset'"  */
  YYSYMBOL_T_EMPTY = 92,                   /* "'empty'"  */
  YYSYMBOL_T_HALT_COMPILER = 93,           /* "'__halt_compiler'"  */
  YYSYMBOL_T_CLASS = 94,                   /* "'class'"  */
  YYSYMBOL_T_TRAIT = 95,                   /* "'trait'"  */
  YYSYMBOL_T_INTERFACE = 96,               /* "'interface'"  */
  YYSYMBOL_T_ENUM = 97,                    /* "'enum'"  */
  YYSYMBOL_T_EXTENDS = 98,                 /* "'extends'"  */
  YYSYMBOL_T_IMPLEMENTS = 99,              /* "'implements'"  */
  YYSYMBOL_T_NAMESPACE = 100,              /* "'namespace'"  */
  YYSYMBOL_T_LIST = 101,                   /* "'list'"  */
  YYSYMBOL_T_ARRAY = 102,                  /* "'array'"  */
  YYSYMBOL_T_CALLABLE = 103,               /* "'callable'"  */
  YYSYMBOL_T_LINE = 104,                   /* "'__LINE__'"  */
  YYSYMBOL_T_FILE = 105,                   /* "'__FILE__'"  */
  YYSYMBOL_T_DIR = 106,                    /* "'__DIR__'"  */
  YYSYMBOL_T_CLASS_C = 107,                /* "'__CLASS__'"  */
  YYSYMBOL_T_TRAIT_C = 108,                /* "'__TRAIT__'"  */
  YYSYMBOL_T_METHOD_C = 109,               /* "'__METHOD__'"  */
  YYSYMBOL_T_FUNC_C = 110,                 /* "'__FUNCTION__'"  */
  YYSYMBOL_T_NS_C = 111,                   /* "'__NAMESPACE__'"  */
  YYSYMBOL_T_ATTRIBUTE = 112,              /* "'#['"  */
  YYSYMBOL_T_PLUS_EQUAL = 113,             /* "'+='"  */
  YYSYMBOL_T_MINUS_EQUAL = 114,            /* "'-='"  */
  YYSYMBOL_T_MUL_EQUAL = 115,              /* "'*='"  */
  YYSYMBOL_T_DIV_EQUAL = 116,              /* "'/='"  */
  YYSYMBOL_T_CONCAT_EQUAL = 117,           /* "'.='"  */
  YYSYMBOL_T_MOD_EQUAL = 118,              /* "'%='"  */
  YYSYMBOL_T_AND_EQUAL = 119,              /* "'&='"  */
  YYSYMBOL_T_OR_EQUAL = 120,               /* "'|='"  */
  YYSYMBOL_T_XOR_EQUAL = 121,              /* "'^='"  */
  YYSYMBOL_T_SL_EQUAL = 122,               /* "'<<='"  */
  YYSYMBOL_T_SR_EQUAL = 123,               /* "'>>='"  */
  YYSYMBOL_T_COALESCE_EQUAL = 124,         /* "'??='"  */
  YYSYMBOL_T_BOOLEAN_OR = 125,             /* "'||'"  */
  YYSYMBOL_T_BOOLEAN_AND = 126,            /* "'&&'"  */
  YYSYMBOL_T_IS_EQUAL = 127,               /* "'=='"  */
  YYSYMBOL_T_IS_NOT_EQUAL = 128,           /* "'!='"  */
  YYSYMBOL_T_IS_IDENTICAL = 129,           /* "'==='"  */
  YYSYMBOL_T_IS_NOT_IDENTICAL = 130,       /* "'!=='"  */
  YYSYMBOL_T_IS_SMALLER_OR_EQUAL = 131,    /* "'<='"  */
  YYSYMBOL_T_IS_GREATER_OR_EQUAL = 132,    /* "'>='"  */
  YYSYMBOL_T_SPACESHIP = 133,              /* "'<=>'"  */
  YYSYMBOL_T_SL = 134,                     /* "'<<'"  */
  YYSYMBOL_T_SR = 135,                     /* "'>>'"  */
  YYSYMBOL_T_INC = 136,                    /* "'++'"  */
  YYSYMBOL_T_DEC = 137,                    /* "'--'"  */
  YYSYMBOL_T_INT_CAST = 138,               /* "'(int)'"  */
  YYSYMBOL_T_DOUBLE_CAST = 139,            /* "'(double)'"  */
  YYSYMBOL_T_STRING_CAST = 140,            /* "'(string)'"  */
  YYSYMBOL_T_ARRAY_CAST = 141,             /* "'(array)'"  */
  YYSYMBOL_T_OBJECT_CAST = 142,            /* "'(object)'"  */
  YYSYMBOL_T_BOOL_CAST = 143,              /* "'(bool)'"  */
  YYSYMBOL_T_UNSET_CAST = 144,             /* "'(unset)'"  */
  YYSYMBOL_T_OBJECT_OPERATOR = 145,        /* "'->'"  */
  YYSYMBOL_T_NULLSAFE_OBJECT_OPERATOR = 146, /* "'?->'"  */
  YYSYMBOL_T_DOUBLE_ARROW = 147,           /* "'=>'"  */
  YYSYMBOL_T_COMMENT = 148,                /* "comment"  */
  YYSYMBOL_T_DOC_COMMENT = 149,            /* "doc comment"  */
  YYSYMBOL_T_OPEN_TAG = 150,               /* "open tag"  */
  YYSYMBOL_T_OPEN_TAG_WITH_ECHO = 151,     /* "'<?='"  */
  YYSYMBOL_T_CLOSE_TAG = 152,              /* "'?>'"  */
  YYSYMBOL_T_WHITESPACE = 153,             /* "whitespace"  */
  YYSYMBOL_T_START_HEREDOC = 154,          /* "heredoc start"  */
  YYSYMBOL_T_END_HEREDOC = 155,            /* "heredoc end"  */
  YYSYMBOL_T_DOLLAR_OPEN_CURLY_BRACES = 156, /* "'${'"  */
  YYSYMBOL_T_CURLY_OPEN = 157,             /* "'{$'"  */
  YYSYMBOL_T_PAAMAYIM_NEKUDOTAYIM = 158,   /* "'::'"  */
  YYSYMBOL_T_NS_SEPARATOR = 159,           /* "'\\'"  */
  YYSYMBOL_T_ELLIPSIS = 160,               /* "'...'"  */
  YYSYMBOL_T_COALESCE = 161,               /* "'??'"  */
  YYSYMBOL_T_POW = 162,                    /* "'**'"  */
  YYSYMBOL_T_POW_EQUAL = 163,              /* "'**='"  */
  YYSYMBOL_T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 164, /* "'&'"  */
  YYSYMBOL_T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 165, /* "amp"  */
  YYSYMBOL_T_BAD_CHARACTER = 166,          /* "invalid character"  */
  YYSYMBOL_T_ERROR = 167,                  /* T_ERROR  */
  YYSYMBOL_168_ = 168,                     /* ','  */
  YYSYMBOL_169_ = 169,                     /* ']'  */
  YYSYMBOL_170_ = 170,                     /* '('  */
  YYSYMBOL_171_ = 171,                     /* ')'  */
  YYSYMBOL_172_ = 172,                     /* ';'  */
  YYSYMBOL_173_ = 173,                     /* '{'  */
  YYSYMBOL_174_ = 174,                     /* '}'  */
  YYSYMBOL_175_ = 175,                     /* '['  */
  YYSYMBOL_176_ = 176,                     /* '`'  */
  YYSYMBOL_177_ = 177,                     /* '"'  */
  YYSYMBOL_178_ = 178,                     /* '$'  */
  YYSYMBOL_YYACCEPT = 179,                 /* $accept  */
  YYSYMBOL_start = 180,                    /* start  */
  YYSYMBOL_reserved_non_modifiers = 181,   /* reserved_non_modifiers  */
  YYSYMBOL_semi_reserved = 182,            /* semi_reserved  */
  YYSYMBOL_ampersand = 183,                /* ampersand  */
  YYSYMBOL_identifier = 184,               /* identifier  */
  YYSYMBOL_top_statement_list = 185,       /* top_statement_list  */
  YYSYMBOL_namespace_declaration_name = 186, /* namespace_declaration_name  */
  YYSYMBOL_namespace_name = 187,           /* namespace_name  */
  YYSYMBOL_legacy_namespace_name = 188,    /* legacy_namespace_name  */
  YYSYMBOL_name = 189,                     /* name  */
  YYSYMBOL_attribute_decl = 190,           /* attribute_decl  */
  YYSYMBOL_attribute_group = 191,          /* attribute_group  */
  YYSYMBOL_attribute = 192,                /* attribute  */
  YYSYMBOL_attributes = 193,               /* attributes  */
  YYSYMBOL_attributed_statement = 194,     /* attributed_statement  */
  YYSYMBOL_top_statement = 195,            /* top_statement  */
  YYSYMBOL_196_1 = 196,                    /* $@1  */
  YYSYMBOL_197_2 = 197,                    /* $@2  */
  YYSYMBOL_use_type = 198,                 /* use_type  */
  YYSYMBOL_group_use_declaration = 199,    /* group_use_declaration  */
  YYSYMBOL_mixed_group_use_declaration = 200, /* mixed_group_use_declaration  */
  YYSYMBOL_possible_comma = 201,           /* possible_comma  */
  YYSYMBOL_inline_use_declarations = 202,  /* inline_use_declarations  */
  YYSYMBOL_unprefixed_use_declarations = 203, /* unprefixed_use_declarations  */
  YYSYMBOL_use_declarations = 204,         /* use_declarations  */
  YYSYMBOL_inline_use_declaration = 205,   /* inline_use_declaration  */
  YYSYMBOL_unprefixed_use_declaration = 206, /* unprefixed_use_declaration  */
  YYSYMBOL_use_declaration = 207,          /* use_declaration  */
  YYSYMBOL_const_list = 208,               /* const_list  */
  YYSYMBOL_inner_statement_list = 209,     /* inner_statement_list  */
  YYSYMBOL_inner_statement = 210,          /* inner_statement  */
  YYSYMBOL_statement = 211,                /* statement  */
  YYSYMBOL_212_3 = 212,                    /* $@3  */
  YYSYMBOL_catch_list = 213,               /* catch_list  */
  YYSYMBOL_catch_name_list = 214,          /* catch_name_list  */
  YYSYMBOL_optional_variable = 215,        /* optional_variable  */
  YYSYMBOL_finally_statement = 216,        /* finally_statement  */
  YYSYMBOL_unset_variables = 217,          /* unset_variables  */
  YYSYMBOL_unset_variable = 218,           /* unset_variable  */
  YYSYMBOL_function_declaration_statement = 219, /* function_declaration_statement  */
  YYSYMBOL_is_reference = 220,             /* is_reference  */
  YYSYMBOL_is_variadic = 221,              /* is_variadic  */
  YYSYMBOL_class_declaration_statement = 222, /* class_declaration_statement  */
  YYSYMBOL_223_4 = 223,                    /* @4  */
  YYSYMBOL_224_5 = 224,                    /* @5  */
  YYSYMBOL_class_modifiers = 225,          /* class_modifiers  */
  YYSYMBOL_class_modifier = 226,           /* class_modifier  */
  YYSYMBOL_trait_declaration_statement = 227, /* trait_declaration_statement  */
  YYSYMBOL_228_6 = 228,                    /* @6  */
  YYSYMBOL_interface_declaration_statement = 229, /* interface_declaration_statement  */
  YYSYMBOL_230_7 = 230,                    /* @7  */
  YYSYMBOL_enum_declaration_statement = 231, /* enum_declaration_statement  */
  YYSYMBOL_232_8 = 232,                    /* @8  */
  YYSYMBOL_enum_backing_type = 233,        /* enum_backing_type  */
  YYSYMBOL_enum_case = 234,                /* enum_case  */
  YYSYMBOL_enum_case_expr = 235,           /* enum_case_expr  */
  YYSYMBOL_extends_from = 236,             /* extends_from  */
  YYSYMBOL_interface_extends_list = 237,   /* interface_extends_list  */
  YYSYMBOL_implements_list = 238,          /* implements_list  */
  YYSYMBOL_foreach_variable = 239,         /* foreach_variable  */
  YYSYMBOL_for_statement = 240,            /* for_statement  */
  YYSYMBOL_foreach_statement = 241,        /* foreach_statement  */
  YYSYMBOL_declare_statement = 242,        /* declare_statement  */
  YYSYMBOL_switch_case_list = 243,         /* switch_case_list  */
  YYSYMBOL_case_list = 244,                /* case_list  */
  YYSYMBOL_case_separator = 245,           /* case_separator  */
  YYSYMBOL_match = 246,                    /* match  */
  YYSYMBOL_match_arm_list = 247,           /* match_arm_list  */
  YYSYMBOL_non_empty_match_arm_list = 248, /* non_empty_match_arm_list  */
  YYSYMBOL_match_arm = 249,                /* match_arm  */
  YYSYMBOL_match_arm_cond_list = 250,      /* match_arm_cond_list  */
  YYSYMBOL_while_statement = 251,          /* while_statement  */
  YYSYMBOL_if_stmt_without_else = 252,     /* if_stmt_without_else  */
  YYSYMBOL_if_stmt = 253,                  /* if_stmt  */
  YYSYMBOL_alt_if_stmt_without_else = 254, /* alt_if_stmt_without_else  */
  YYSYMBOL_alt_if_stmt = 255,              /* alt_if_stmt  */
  YYSYMBOL_parameter_list = 256,           /* parameter_list  */
  YYSYMBOL_non_empty_parameter_list = 257, /* non_empty_parameter_list  */
  YYSYMBOL_attributed_parameter = 258,     /* attributed_parameter  */
  YYSYMBOL_optional_property_modifiers = 259, /* optional_property_modifiers  */
  YYSYMBOL_property_modifier = 260,        /* property_modifier  */
  YYSYMBOL_parameter = 261,                /* parameter  */
  YYSYMBOL_optional_type_without_static = 262, /* optional_type_without_static  */
  YYSYMBOL_type_expr = 263,                /* type_expr  */
  YYSYMBOL_type = 264,                     /* type  */
  YYSYMBOL_union_type = 265,               /* union_type  */
  YYSYMBOL_intersection_type = 266,        /* intersection_type  */
  YYSYMBOL_type_expr_without_static = 267, /* type_expr_without_static  */
  YYSYMBOL_type_without_static = 268,      /* type_without_static  */
  YYSYMBOL_union_type_without_static = 269, /* union_type_without_static  */
  YYSYMBOL_intersection_type_without_static = 270, /* intersection_type_without_static  */
  YYSYMBOL_return_type = 271,              /* return_type  */
  YYSYMBOL_argument_list = 272,            /* argument_list  */
  YYSYMBOL_non_empty_argument_list = 273,  /* non_empty_argument_list  */
  YYSYMBOL_argument = 274,                 /* argument  */
  YYSYMBOL_global_var_list = 275,          /* global_var_list  */
  YYSYMBOL_global_var = 276,               /* global_var  */
  YYSYMBOL_static_var_list = 277,          /* static_var_list  */
  YYSYMBOL_static_var = 278,               /* static_var  */
  YYSYMBOL_class_statement_list = 279,     /* class_statement_list  */
  YYSYMBOL_attributed_class_statement = 280, /* attributed_class_statement  */
  YYSYMBOL_class_statement = 281,          /* class_statement  */
  YYSYMBOL_class_name_list = 282,          /* class_name_list  */
  YYSYMBOL_trait_adaptations = 283,        /* trait_adaptations  */
  YYSYMBOL_trait_adaptation_list = 284,    /* trait_adaptation_list  */
  YYSYMBOL_trait_adaptation = 285,         /* trait_adaptation  */
  YYSYMBOL_trait_precedence = 286,         /* trait_precedence  */
  YYSYMBOL_trait_alias = 287,              /* trait_alias  */
  YYSYMBOL_trait_method_reference = 288,   /* trait_method_reference  */
  YYSYMBOL_absolute_trait_method_reference = 289, /* absolute_trait_method_reference  */
  YYSYMBOL_method_body = 290,              /* method_body  */
  YYSYMBOL_variable_modifiers = 291,       /* variable_modifiers  */
  YYSYMBOL_method_modifiers = 292,         /* method_modifiers  */
  YYSYMBOL_non_empty_member_modifiers = 293, /* non_empty_member_modifiers  */
  YYSYMBOL_member_modifier = 294,          /* member_modifier  */
  YYSYMBOL_property_list = 295,            /* property_list  */
  YYSYMBOL_property = 296,                 /* property  */
  YYSYMBOL_class_const_list = 297,         /* class_const_list  */
  YYSYMBOL_class_const_decl = 298,         /* class_const_decl  */
  YYSYMBOL_const_decl = 299,               /* const_decl  */
  YYSYMBOL_echo_expr_list = 300,           /* echo_expr_list  */
  YYSYMBOL_echo_expr = 301,                /* echo_expr  */
  YYSYMBOL_for_exprs = 302,                /* for_exprs  */
  YYSYMBOL_non_empty_for_exprs = 303,      /* non_empty_for_exprs  */
  YYSYMBOL_anonymous_class = 304,          /* anonymous_class  */
  YYSYMBOL_305_9 = 305,                    /* @9  */
  YYSYMBOL_new_expr = 306,                 /* new_expr  */
  YYSYMBOL_expr = 307,                     /* expr  */
  YYSYMBOL_inline_function = 308,          /* inline_function  */
  YYSYMBOL_fn = 309,                       /* fn  */
  YYSYMBOL_function = 310,                 /* function  */
  YYSYMBOL_backup_doc_comment = 311,       /* backup_doc_comment  */
  YYSYMBOL_backup_fn_flags = 312,          /* backup_fn_flags  */
  YYSYMBOL_backup_lex_pos = 313,           /* backup_lex_pos  */
  YYSYMBOL_returns_ref = 314,              /* returns_ref  */
  YYSYMBOL_lexical_vars = 315,             /* lexical_vars  */
  YYSYMBOL_lexical_var_list = 316,         /* lexical_var_list  */
  YYSYMBOL_lexical_var = 317,              /* lexical_var  */
  YYSYMBOL_function_call = 318,            /* function_call  */
  YYSYMBOL_class_name = 319,               /* class_name  */
  YYSYMBOL_class_name_reference = 320,     /* class_name_reference  */
  YYSYMBOL_exit_expr = 321,                /* exit_expr  */
  YYSYMBOL_backticks_expr = 322,           /* backticks_expr  */
  YYSYMBOL_ctor_arguments = 323,           /* ctor_arguments  */
  YYSYMBOL_dereferenceable_scalar = 324,   /* dereferenceable_scalar  */
  YYSYMBOL_scalar = 325,                   /* scalar  */
  YYSYMBOL_constant = 326,                 /* constant  */
  YYSYMBOL_class_constant = 327,           /* class_constant  */
  YYSYMBOL_optional_expr = 328,            /* optional_expr  */
  YYSYMBOL_variable_class_name = 329,      /* variable_class_name  */
  YYSYMBOL_fully_dereferenceable = 330,    /* fully_dereferenceable  */
  YYSYMBOL_array_object_dereferenceable = 331, /* array_object_dereferenceable  */
  YYSYMBOL_callable_expr = 332,            /* callable_expr  */
  YYSYMBOL_callable_variable = 333,        /* callable_variable  */
  YYSYMBOL_variable = 334,                 /* variable  */
  YYSYMBOL_simple_variable = 335,          /* simple_variable  */
  YYSYMBOL_static_member = 336,            /* static_member  */
  YYSYMBOL_new_variable = 337,             /* new_variable  */
  YYSYMBOL_member_name = 338,              /* member_name  */
  YYSYMBOL_property_name = 339,            /* property_name  */
  YYSYMBOL_array_pair_list = 340,          /* array_pair_list  */
  YYSYMBOL_possible_array_pair = 341,      /* possible_array_pair  */
  YYSYMBOL_non_empty_array_pair_list = 342, /* non_empty_array_pair_list  */
  YYSYMBOL_array_pair = 343,               /* array_pair  */
  YYSYMBOL_encaps_list = 344,              /* encaps_list  */
  YYSYMBOL_encaps_var = 345,               /* encaps_var  */
  YYSYMBOL_encaps_var_offset = 346,        /* encaps_var_offset  */
  YYSYMBOL_internal_functions_in_yacc = 347, /* internal_functions_in_yacc  */
  YYSYMBOL_isset_variables = 348,          /* isset_variables  */
  YYSYMBOL_isset_variable = 349            /* isset_variable  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ZENDSTYPE_IS_TRIVIAL && ZENDSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   9389

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  179
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  171
/* YYNRULES -- Number of rules.  */
#define YYNRULES  581
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1096

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   406


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    17,   177,     2,   178,    16,     2,     2,
     170,   171,    14,    12,   168,    13,    11,    15,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,   172,
       9,     4,    10,     5,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   175,     2,   169,     8,     2,   176,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   173,     7,   174,    18,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167
};

#if ZENDDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   295,   295,   299,   299,   299,   299,   299,   299,   299,
     299,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   303,   304,   304,   304,   304,   304,   304,
     304,   304,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   309,   310,   310,   310,   310,   310,   310,
     310,   314,   315,   319,   320,   328,   329,   334,   335,   340,
     341,   346,   347,   351,   352,   353,   354,   358,   360,   365,
     367,   372,   376,   377,   381,   382,   383,   384,   385,   389,
     390,   391,   392,   396,   399,   399,   402,   402,   405,   406,
     407,   408,   409,   413,   414,   418,   423,   428,   429,   433,
     435,   440,   442,   447,   449,   454,   455,   459,   461,   466,
     468,   473,   474,   478,   480,   486,   487,   488,   489,   496,
     497,   498,   499,   501,   503,   505,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   518,   522,   521,   525,
     526,   528,   529,   533,   535,   540,   541,   545,   546,   550,
     551,   555,   556,   560,   564,   571,   572,   576,   577,   581,
     581,   584,   584,   590,   591,   596,   597,   601,   601,   607,
     607,   613,   613,   619,   620,   624,   629,   630,   634,   635,
     639,   640,   644,   645,   649,   650,   651,   652,   656,   657,
     661,   662,   666,   667,   671,   672,   673,   674,   678,   679,
     681,   686,   687,   692,   697,   698,   702,   703,   707,   709,
     714,   715,   720,   721,   726,   729,   735,   736,   741,   744,
     750,   751,   757,   758,   763,   765,   770,   771,   775,   776,
     780,   781,   782,   783,   787,   791,   799,   800,   804,   805,
     806,   807,   811,   812,   816,   817,   821,   822,   829,   830,
     831,   832,   836,   837,   838,   842,   844,   849,   851,   856,
     857,   861,   862,   863,   867,   869,   874,   875,   877,   881,
     882,   886,   892,   893,   897,   898,   902,   904,   910,   913,
     916,   920,   924,   925,   926,   931,   932,   936,   937,   938,
     942,   944,   949,   950,   954,   959,   961,   965,   967,   972,
     974,   978,   983,   984,   988,   989,   993,   994,   999,  1000,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1015,  1016,  1020,
    1022,  1027,  1028,  1032,  1036,  1040,  1041,  1044,  1048,  1049,
    1053,  1054,  1058,  1058,  1068,  1070,  1072,  1077,  1079,  1081,
    1083,  1085,  1087,  1088,  1090,  1092,  1094,  1096,  1098,  1100,
    1102,  1104,  1106,  1108,  1110,  1112,  1114,  1115,  1116,  1117,
    1118,  1120,  1122,  1124,  1126,  1128,  1129,  1130,  1131,  1132,
    1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,  1142,
    1143,  1144,  1145,  1147,  1149,  1151,  1153,  1155,  1157,  1159,
    1161,  1163,  1165,  1169,  1170,  1172,  1174,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
    1189,  1190,  1191,  1192,  1193,  1194,  1195,  1196,  1197,  1199,
    1204,  1209,  1219,  1223,  1227,  1231,  1235,  1239,  1240,  1244,
    1245,  1249,  1250,  1254,  1255,  1259,  1261,  1263,  1265,  1270,
    1273,  1277,  1278,  1279,  1283,  1284,  1288,  1290,  1291,  1296,
    1297,  1302,  1303,  1304,  1305,  1309,  1310,  1311,  1312,  1314,
    1315,  1316,  1317,  1321,  1322,  1323,  1324,  1325,  1326,  1327,
    1328,  1329,  1333,  1335,  1340,  1341,  1345,  1349,  1350,  1351,
    1352,  1356,  1357,  1361,  1362,  1363,  1367,  1369,  1371,  1373,
    1375,  1377,  1381,  1383,  1385,  1387,  1392,  1393,  1394,  1398,
    1400,  1405,  1407,  1409,  1411,  1413,  1415,  1417,  1422,  1423,
    1424,  1428,  1429,  1430,  1434,  1439,  1440,  1444,  1446,  1451,
    1453,  1455,  1457,  1459,  1461,  1464,  1470,  1472,  1474,  1476,
    1481,  1483,  1486,  1489,  1492,  1494,  1496,  1499,  1503,  1504,
    1505,  1506,  1511,  1512,  1513,  1515,  1517,  1519,  1521,  1526,
    1527,  1532
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PREC_ARROW_FUNCTION",
  "'='", "'?'", "':'", "'|'", "'^'", "'<'", "'>'", "'.'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "'@'", "T_NOELSE", "\"integer\"",
  "\"floating-point number\"", "\"identifier\"",
  "\"fully qualified name\"", "\"namespace-relative name\"",
  "\"namespaced name\"", "\"variable\"", "T_INLINE_HTML",
  "\"string content\"", "\"quoted string\"", "\"variable name\"",
  "\"number\"", "\"'include'\"", "\"'include_once'\"", "\"'eval'\"",
  "\"'require'\"", "\"'require_once'\"", "\"'or'\"", "\"'xor'\"",
  "\"'and'\"", "\"'print'\"", "\"'yield'\"", "\"'yield from'\"",
  "\"'instanceof'\"", "\"'new'\"", "\"'clone'\"", "\"'exit'\"", "\"'if'\"",
  "\"'elseif'\"", "\"'else'\"", "\"'endif'\"", "\"'echo'\"", "\"'do'\"",
  "\"'while'\"", "\"'endwhile'\"", "\"'for'\"", "\"'endfor'\"",
  "\"'foreach'\"", "\"'endforeach'\"", "\"'declare'\"", "\"'enddeclare'\"",
  "\"'as'\"", "\"'switch'\"", "\"'endswitch'\"", "\"'case'\"",
  "\"'default'\"", "\"'match'\"", "\"'break'\"", "\"'continue'\"",
  "\"'goto'\"", "\"'function'\"", "\"'fn'\"", "\"'const'\"",
  "\"'return'\"", "\"'try'\"", "\"'catch'\"", "\"'finally'\"",
  "\"'throw'\"", "\"'use'\"", "\"'insteadof'\"", "\"'global'\"",
  "\"'static'\"", "\"'abstract'\"", "\"'final'\"", "\"'private'\"",
  "\"'protected'\"", "\"'public'\"", "\"'readonly'\"", "\"'var'\"",
  "\"'unset'\"", "\"'isset'\"", "\"'empty'\"", "\"'__halt_compiler'\"",
  "\"'class'\"", "\"'trait'\"", "\"'interface'\"", "\"'enum'\"",
  "\"'extends'\"", "\"'implements'\"", "\"'namespace'\"", "\"'list'\"",
  "\"'array'\"", "\"'callable'\"", "\"'__LINE__'\"", "\"'__FILE__'\"",
  "\"'__DIR__'\"", "\"'__CLASS__'\"", "\"'__TRAIT__'\"",
  "\"'__METHOD__'\"", "\"'__FUNCTION__'\"", "\"'__NAMESPACE__'\"",
  "\"'#['\"", "\"'+='\"", "\"'-='\"", "\"'*='\"", "\"'/='\"", "\"'.='\"",
  "\"'%='\"", "\"'&='\"", "\"'|='\"", "\"'^='\"", "\"'<<='\"", "\"'>>='\"",
  "\"'?""?='\"", "\"'||'\"", "\"'&&'\"", "\"'=='\"", "\"'!='\"",
  "\"'==='\"", "\"'!=='\"", "\"'<='\"", "\"'>='\"", "\"'<=>'\"",
  "\"'<<'\"", "\"'>>'\"", "\"'++'\"", "\"'--'\"", "\"'(int)'\"",
  "\"'(double)'\"", "\"'(string)'\"", "\"'(array)'\"", "\"'(object)'\"",
  "\"'(bool)'\"", "\"'(unset)'\"", "\"'->'\"", "\"'?->'\"", "\"'=>'\"",
  "\"comment\"", "\"doc comment\"", "\"open tag\"", "\"'<?='\"",
  "\"'?>'\"", "\"whitespace\"", "\"heredoc start\"", "\"heredoc end\"",
  "\"'${'\"", "\"'{$'\"", "\"'::'\"", "\"'\\\\'\"", "\"'...'\"",
  "\"'?""?'\"", "\"'**'\"", "\"'**='\"", "\"'&'\"", "\"amp\"",
  "\"invalid character\"", "T_ERROR", "','", "']'", "'('", "')'", "';'",
  "'{'", "'}'", "'['", "'`'", "'\"'", "'$'", "$accept", "start",
  "reserved_non_modifiers", "semi_reserved", "ampersand", "identifier",
  "top_statement_list", "namespace_declaration_name", "namespace_name",
  "legacy_namespace_name", "name", "attribute_decl", "attribute_group",
  "attribute", "attributes", "attributed_statement", "top_statement",
  "$@1", "$@2", "use_type", "group_use_declaration",
  "mixed_group_use_declaration", "possible_comma",
  "inline_use_declarations", "unprefixed_use_declarations",
  "use_declarations", "inline_use_declaration",
  "unprefixed_use_declaration", "use_declaration", "const_list",
  "inner_statement_list", "inner_statement", "statement", "$@3",
  "catch_list", "catch_name_list", "optional_variable",
  "finally_statement", "unset_variables", "unset_variable",
  "function_declaration_statement", "is_reference", "is_variadic",
  "class_declaration_statement", "@4", "@5", "class_modifiers",
  "class_modifier", "trait_declaration_statement", "@6",
  "interface_declaration_statement", "@7", "enum_declaration_statement",
  "@8", "enum_backing_type", "enum_case", "enum_case_expr", "extends_from",
  "interface_extends_list", "implements_list", "foreach_variable",
  "for_statement", "foreach_statement", "declare_statement",
  "switch_case_list", "case_list", "case_separator", "match",
  "match_arm_list", "non_empty_match_arm_list", "match_arm",
  "match_arm_cond_list", "while_statement", "if_stmt_without_else",
  "if_stmt", "alt_if_stmt_without_else", "alt_if_stmt", "parameter_list",
  "non_empty_parameter_list", "attributed_parameter",
  "optional_property_modifiers", "property_modifier", "parameter",
  "optional_type_without_static", "type_expr", "type", "union_type",
  "intersection_type", "type_expr_without_static", "type_without_static",
  "union_type_without_static", "intersection_type_without_static",
  "return_type", "argument_list", "non_empty_argument_list", "argument",
  "global_var_list", "global_var", "static_var_list", "static_var",
  "class_statement_list", "attributed_class_statement", "class_statement",
  "class_name_list", "trait_adaptations", "trait_adaptation_list",
  "trait_adaptation", "trait_precedence", "trait_alias",
  "trait_method_reference", "absolute_trait_method_reference",
  "method_body", "variable_modifiers", "method_modifiers",
  "non_empty_member_modifiers", "member_modifier", "property_list",
  "property", "class_const_list", "class_const_decl", "const_decl",
  "echo_expr_list", "echo_expr", "for_exprs", "non_empty_for_exprs",
  "anonymous_class", "@9", "new_expr", "expr", "inline_function", "fn",
  "function", "backup_doc_comment", "backup_fn_flags", "backup_lex_pos",
  "returns_ref", "lexical_vars", "lexical_var_list", "lexical_var",
  "function_call", "class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "dereferenceable_scalar", "scalar",
  "constant", "class_constant", "optional_expr", "variable_class_name",
  "fully_dereferenceable", "array_object_dereferenceable", "callable_expr",
  "callable_variable", "variable", "simple_variable", "static_member",
  "new_variable", "member_name", "property_name", "array_pair_list",
  "possible_array_pair", "non_empty_array_pair_list", "array_pair",
  "encaps_list", "encaps_var", "encaps_var_offset",
  "internal_functions_in_yacc", "isset_variables", "isset_variable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-912)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-553)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -912,    66,  3537,  -912,  7044,  7044,  7044,  7044,  7044,  -912,
    -912,    68,  -912,  -912,  -912,  -912,  -912,  -912,  7044,  7044,
     -90,  7044,  7044,  7044,  7044,  7044,   133,  7044,   -79,   -43,
    7044,  5875,   -27,     5,    32,    44,    50,    58,  7044,  7044,
     167,  -912,  -912,   228,  7044,    34,  7044,   312,    21,   170,
    -912,  -912,    96,   149,   182,   198,  -912,  -912,  -912,  -912,
    9100,   201,   204,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,   529,  8607,  8607,  7044,  7044,  7044,  7044,  7044,  7044,
    7044,   108,  7044,  -912,  -912,  6042,   134,   140,    16,   -73,
    -912,  1913,  -912,  -912,  -912,  -912,  -912,   279,  -912,  -912,
    -912,  -912,  -912,   249,  -912,   516,  -912,  -912,  2713,  -912,
     417,   417,  -912,   196,   344,  -912,   470,   224,   226,   229,
     559,   219,   230,   164,  -912,  -912,  -912,  -912,   583,   422,
     163,   417,   163,    23,   163,   163,  -912,  8106,  8106,  7044,
    8106,  8106,  8237,  2857,  8237,  -912,  -912,  7044,  -912,   122,
    -912,   240,   219,  -912,   434,  -912,  7044,  -912,  7044,   -85,
    -912,  8106,   377,  7044,  7044,  7044,   228,  7044,  7044,  8106,
     261,   266,   295,   471,   291,  -912,   336,  -912,  8106,  -912,
    -912,  -912,  -912,  -912,  -912,   -12,   521,   339,   294,  -912,
     297,  -912,  -912,   517,   323,  -912,  -912,  8607,  7044,  7044,
     354,   504,   520,   525,   535,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,   363,   367,  6042,
    6042,  -912,   403,   219,  7044,  6042,   415,  -912,  -912,   555,
     555,   163,   163,   163,   163,   163,   163,   163,   130,    99,
    -912,  6543,  8607,   117,  -912,  7152,  3704,   429,  7044,  -912,
    -912,  8607,  7945,   433,  -912,   440,  -912,   150,   436,   184,
     150,    90,  7044,  -912,  2869,  -912,   583,  -912,  -912,  -912,
    -912,  -912,   444,  5875,   462,   604,   467,  1946,  7044,  7044,
    7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,
    7044,    80,  7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,
    7044,  7044,  7044,  7044,  7044,  7044,  7044,  -912,  -912,  -912,
     627,  8508,  8508,    18,    18,  7044,  7044,  -912,  6209,  7044,
    7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,  7044,
    7044,  -912,  -912,  7044,  -912,  7218,  7044,   219,  7230,  -912,
      21,  -912,  -912,    18,    18,    21,  7044,  7044,   492,  7286,
    7044,  -912,   503,  7420,   505,   511,  8106,  7998,  -119,  7467,
    7479,  -912,  -912,  -912,  7044,   228,  -912,  -912,  3871,   661,
     519,    -9,   513,   338,  -912,   521,  -912,    21,  -912,  7044,
     667,  -912,   530,  -912,   121,  8106,   534,  -912,  7526,   531,
     609,  -912,   621,   714,  -912,   548,  -912,   552,   564,   529,
     569,  -912,  7660,   572,   710,   716,   334,  -912,  -912,   452,
    2221,   568,  -912,  -912,  -912,   681,   573,  -912,  1913,  -912,
    -912,  -912,  6042,  8106,   426,  6376,   745,  6042,  -912,  -912,
    2287,    48,  7044,  7044,   -90,  7044,  7044,  7044,  2113,   133,
    7044,    13,    58,   744,   748,  7044,    -7,   149,   182,   201,
     204,   751,   755,   756,   758,   760,   761,   764,   765,  6710,
    -912,   766,   605,  -912,  8106,  -912,   728,  7044,  -912,  7044,
    -912,  -912,  7044,  8043,  8375,  2690,   469,   469,   109,   156,
     156,    23,    23,    23,  8174,  8186,  8237,  -912,  8247,  8315,
    8681,  8681,  8681,  8681,   469,   469,  8681,   562,   562,  2612,
     163,  8719,  8719,   610,  -912,   611,  7044,   612,   617,   219,
     612,   617,   219,  -912,  7044,  -912,   219,   219,  2301,   619,
    8607,  8237,  8237,  8237,  8237,  8237,  8237,  8237,  8237,  8237,
    8237,  8237,  8237,  8237,  8237,  -912,  8237,   609,  -912,  -912,
    -912,  -912,  -912,  2352,   620,  -912,  1106,  -912,  7044,  1274,
    7044,  7044,  8597,  -912,     8,   601,  8106,  -912,  -912,  -912,
     319,   606,  -912,  -912,   730,  -912,  -912,  8106,  -912,  8607,
     630,  7044,   632,  -912,  -912,   529,   705,   633,   529,  -912,
     353,   705,  -912,  3203,   801,  -912,  -912,  -912,   638,  -912,
    -912,  -912,   779,  -912,  -912,  -912,   644,  -912,  7044,  -912,
    -912,   643,  -912,   647,   649,  8607,  8106,  7044,  -912,  -912,
    -912,  8106,  7044,  3036,   650,   609,  7707,  7719,  4038,  2612,
    7044,   -75,   652,   -75,  2483,  -912,  -912,  2533,  -912,  -912,
    -912,  -912,   555,   705,  -912,  -912,  -912,  -912,  7766,  -912,
    -912,  -912,   653,  8106,   654,  6042,  8607,   -33,    78,  1442,
     656,   657,  -912,  6877,  -912,   565,   753,   333,   664,  -912,
    -912,   333,  -912,   662,  -912,  -912,  -912,   529,  -912,  -912,
     665,  -912,   663,   594,  -912,  -912,  -912,  -912,  -912,     9,
     831,   675,  -912,  -912,  3370,  -912,  7044,  -912,  -912,  7900,
     669,   801,  6042,   512,  8237,  8106,  7044,  -912,  -912,   705,
    5875,   838,   673,  2612,   734,   676,   680,  -912,   623,  -912,
     -75,   682,  -912,  -912,  -912,  5708,   685,  4205,  7044,  6042,
     690,   146,  8597,  1610,  -912,  -912,  -912,  -912,   570,  -912,
     158,   692,   687,   696,  -912,   698,  8106,   697,   700,  -912,
     846,  -912,   319,   701,   703,  -912,  -912,   665,   704,   711,
     529,  -912,  -912,   594,   594,   594,   594,   707,  -912,  8237,
     702,  -912,   712,  -912,  -912,  -912,  -912,  -912,   875,   -23,
    -912,   645,  -912,  -912,  -912,  -912,  -912,   718,  -912,    19,
     877,   721,   717,   808,   722,  -912,   719,   723,   725,   155,
     727,  -912,  -912,  -912,  4372,   651,   731,  7044,     7,   172,
    -912,  -912,   742,  -912,  6877,  -912,  7044,   743,   529,  -912,
    -912,  -912,  -912,   333,   726,  -912,  -912,   529,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  1789,  -912,  -912,
    -912,   370,    81,  1012,  -912,  -912,   970,  -912,  -912,  -912,
    -912,  -912,  -912,   801,   729,  5708,   353,   752,  -912,  -912,
    -912,   746,   645,   645,   645,   645,   875,   737,   875,  -912,
    -912,  1778,  -912,  1610,  4539,   736,   738,  -912,  2547,  -912,
    -912,  -912,  -912,  7044,  -912,  8106,  7044,    20,  -912,  4706,
    -912,  -912,  1140,  9189,    89,  -912,   884,  9189,   417,  -912,
    -912,  1308,  -912,  -912,  -912,  -912,   885,  -912,  -912,  -912,
    -912,  -912,     1,  -912,  1476,  -912,  -912,  -912,  -912,   750,
    -912,  -912,  -912,  5708,  8106,  8106,   529,  -912,   749,  -912,
    -912,   911,  -912,  8838,  -912,   912,   348,  -912,   919,   356,
    -912,  9189,  -912,  1644,  -912,  -912,   754,  -912,   897,   757,
    -912,   763,  -912,  4873,  -912,  5708,  -912,   769,  7044,   759,
     768,  -912,  -912,  8969,  -912,   762,   771,   867,   857,   786,
    7044,  -912,   884,  -912,  7044,  9189,  -912,  -912,  -912,  7044,
     942,  -912,  -912,     1,   778,  -912,   780,  -912,  8106,  -912,
    -912,  -912,  -912,  -912,  9278,   529,  9189,  8106,  -912,  8106,
    -912,   781,  8106,  7044,  5040,  -912,  -912,  5207,  -912,  5374,
    -912,  -912,  9189,   665,  -912,  -912,  -912,   -75,  -912,  8106,
    -912,  -912,  -912,  -912,   783,  -912,  -912,   875,  -912,   487,
    -912,  -912,  -912,  5541,  -912,  -912
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      86,     0,     2,     1,     0,     0,     0,     0,     0,   485,
     486,    93,    95,    96,    94,   526,   162,   483,     0,     0,
       0,     0,     0,     0,   440,     0,     0,     0,   474,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   504,   504,
       0,   453,   452,     0,   504,     0,     0,     0,     0,   469,
     195,   196,     0,     0,     0,     0,   191,   197,   199,   201,
     116,     0,     0,   494,   495,   496,   501,   497,   498,   499,
     500,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   169,   144,   545,   476,     0,     0,   493,
     102,     0,   110,    85,   109,   104,   105,     0,   193,   106,
     107,   108,   449,   246,   150,     0,   151,   423,     0,   445,
     457,   457,   521,     0,   490,   437,   491,   492,     0,   511,
       0,     0,   522,   367,   516,   523,   427,    93,   469,     0,
     408,   457,   409,   410,   411,   436,   172,   574,   575,     0,
     577,   578,   439,   441,   443,   469,   362,     0,   470,     0,
     365,   471,   479,   531,   472,   372,   504,   435,     0,     0,
     356,   357,     0,     0,   358,     0,     0,     0,     0,   505,
       0,     0,     0,     0,     0,   142,     0,   144,   444,    89,
      92,    90,   123,   124,    91,   139,     0,     0,     0,   134,
       0,   300,   301,   304,     0,   303,   447,     0,     0,     0,
       0,     0,     0,     0,     0,    83,    88,     3,     4,     5,
       6,     7,     8,     9,    10,    46,    47,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    49,    50,    51,    52,
      71,    53,    41,    42,    43,    70,    44,    45,    30,    31,
      32,    33,    34,    35,    36,    74,    75,    76,    77,    78,
      79,    80,    37,    38,    39,    40,    61,    59,    60,    72,
      56,    57,    58,    48,    54,    55,    66,    67,    68,    62,
      63,    65,    64,    69,    73,    84,    87,   114,     0,   545,
     545,    99,   127,    97,     0,   545,   509,   512,   510,   387,
     389,   428,   429,   430,   431,   432,   433,   434,   560,     0,
     488,     0,     0,     0,   558,     0,     0,     0,     0,    81,
      82,     0,   550,     0,   548,   544,   546,   477,     0,   478,
       0,     0,     0,   528,     0,   465,     0,   103,   111,   446,
     189,   194,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   163,   458,   454,
     454,     0,     0,     0,     0,     0,   504,   468,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   386,   388,     0,   454,     0,     0,   479,     0,   366,
       0,   480,   364,     0,     0,     0,     0,   504,     0,     0,
       0,   161,     0,     0,     0,   359,   361,     0,     0,     0,
       0,   156,   157,   171,     0,     0,   122,   158,     0,     0,
       0,   139,     0,     0,   118,     0,   120,     0,   159,     0,
       0,   160,   127,   181,   507,   581,   127,   579,     0,     0,
     208,   454,   210,   203,   113,     0,    86,     0,     0,   128,
       0,    98,     0,     0,     0,     0,     0,   487,   559,     0,
       0,   507,   557,   489,   556,   422,     0,   149,     0,   146,
     143,   145,   545,   553,   507,     0,   482,   545,   438,   484,
       0,    93,     3,     4,     5,     6,     7,    46,   440,    12,
      13,   474,    71,   453,   452,    33,    74,    39,    40,    48,
      54,   494,   495,   496,   501,   497,   498,   499,   500,     0,
     291,     0,   127,   294,   296,   448,     0,     0,   247,     0,
     144,   250,     0,     0,   395,   398,   416,   418,   399,   400,
     401,   402,   404,   405,   392,   394,   393,   421,   390,   391,
     414,   415,   412,   413,   417,   419,   420,   406,   407,   426,
     403,   397,   396,     0,   454,     0,     0,   502,   529,     0,
     503,   530,     0,   541,     0,   543,   524,   525,     0,     0,
       0,   370,   373,   374,   375,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   376,   576,   442,   208,   473,   536,
     534,   535,   537,     0,     0,   475,     0,   355,     0,     0,
     358,     0,     0,   167,     0,     0,   454,   141,   173,   140,
       0,     0,   119,   121,   139,   133,   299,   305,   302,   128,
       0,   128,     0,   573,   112,     0,   212,     0,     0,   454,
       0,   212,    86,     0,     0,   481,   100,   101,   508,   482,
     562,   563,     0,   568,   571,   569,     0,   565,     0,   564,
     567,     0,   147,     0,     0,     0,   549,     0,   547,   527,
     293,   298,     0,   128,     0,   208,     0,     0,     0,   425,
       0,   258,     0,   258,     0,   466,   467,     0,   519,   520,
     518,   517,   371,   212,   533,   532,   144,   244,     0,   144,
     242,   152,     0,   360,     0,   545,     0,     0,   507,     0,
     228,   228,   155,   234,   354,   179,   137,     0,   127,   130,
     135,     0,   182,     0,   580,   572,   209,     0,   454,   307,
     211,   315,     0,     0,   273,   282,   283,   284,   204,   268,
     270,   271,   272,   454,     0,   117,     0,   570,   561,     0,
       0,   555,   545,   507,   369,   297,     0,   295,   292,   212,
       0,     0,     0,   424,   258,     0,   127,   254,   266,   257,
     258,     0,   539,   542,   454,   248,     0,     0,   358,   545,
       0,   507,     0,     0,   144,   222,   168,   228,     0,   228,
       0,   127,     0,   127,   236,   127,   240,     0,     0,   170,
       0,   136,   128,     0,   127,   132,   164,   213,     0,   336,
       0,   307,   269,     0,     0,     0,     0,     0,   115,   368,
       0,   148,     0,   454,   245,   144,   251,   256,   289,   258,
     252,     0,   262,   261,   260,   263,   259,   185,   267,   278,
     280,   281,     0,   459,     0,   153,     0,     0,     0,   482,
       0,   144,   220,   165,     0,     0,     0,     0,     0,     0,
     224,   128,     0,   233,   128,   235,   128,     0,     0,   144,
     138,   129,   126,   128,     0,   307,   454,     0,   343,   344,
     345,   342,   341,   340,   346,   335,   198,   336,   311,   312,
     306,   266,     0,   334,   338,   316,   336,   274,   276,   275,
     277,   307,   566,   554,     0,   249,     0,     0,   255,   279,
     186,   187,     0,     0,     0,     0,   289,     0,   289,   307,
     243,     0,   216,     0,     0,     0,     0,   226,     0,   231,
     232,   144,   225,     0,   237,   241,     0,   177,   175,     0,
     131,   125,   336,     0,     0,   313,     0,     0,   457,   339,
     200,   336,   307,   290,   455,   188,     0,   285,   287,   286,
     288,   455,     0,   455,   336,   144,   218,   154,   166,     0,
     223,   227,   144,   230,   239,   238,     0,   178,     0,   180,
     192,   206,   317,     0,   314,   454,     0,   348,     0,     0,
     352,     0,   202,   336,   456,   454,     0,   463,     0,   127,
     462,     0,   363,     0,   221,   229,   176,     0,     0,     0,
      74,   318,   329,     0,   320,     0,     0,     0,   330,     0,
       0,   349,     0,   308,     0,     0,   309,   454,   190,     0,
     264,   144,   464,   128,     0,   144,     0,   144,   207,   205,
     319,   321,   322,   323,     0,     0,     0,   454,   347,   454,
     351,     0,   455,     0,     0,   461,   460,     0,   219,     0,
     325,   326,   328,   324,   331,   350,   353,   258,   451,   265,
     455,   455,   174,   327,     0,   184,   450,   289,   455,     0,
     332,   144,   455,     0,   310,   333
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -912,  -912,   -99,  -912,   -80,   -48,  -457,  -912,   -13,  -175,
     115,   488,  -912,   -47,    -2,     4,  -912,  -912,  -912,   913,
    -912,  -912,  -264,  -912,  -912,   772,   152,  -685,   522,   795,
    -173,  -912,    37,  -912,  -912,  -912,  -912,  -912,  -912,   326,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,   865,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -912,  -912,  -572,  -912,  -563,
     174,  -912,    36,  -912,  -912,  -698,    33,  -912,  -912,  -912,
     100,  -912,  -912,  -912,  -912,  -912,  -912,  -686,  -912,   137,
    -912,  -912,   199,    76,    63,  -495,  -912,  -912,  -912,  -692,
    -912,  -912,  -911,   -64,  -912,   300,  -912,   533,  -912,   536,
    -703,    87,  -912,  -732,  -912,  -912,   -35,  -912,  -912,  -912,
    -912,  -912,  -912,  -912,  -912,  -867,  -912,   -42,  -912,   -46,
     556,  -912,   574,  -610,  -912,   843,  -912,  -912,    54,   -20,
    -912,    -1,  -140,  -768,  -912,  -109,  -912,  -912,   -45,  -912,
      -8,   634,  -912,  -912,   589,   -52,  -912,   -17,    39,   -36,
    -912,  -912,  -912,  -912,  -912,    43,   116,  -912,  -912,   622,
    -344,  -257,   502,  -912,  -912,   580,   259,  -912,  -912,  -912,
     362
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   284,   285,   321,   531,     2,   287,   726,   185,
      89,   291,   292,    90,   129,   489,    93,   465,   288,   727,
     442,   187,   470,   728,   814,   188,   729,   730,   189,   174,
     316,   490,   491,   719,   725,   947,   988,   809,   452,   453,
      95,   921,   966,    96,   536,   201,    97,    98,    99,   202,
     100,   203,   101,   204,   651,   898,  1019,   646,   649,   738,
     717,   977,   863,   796,   722,   798,   941,   102,   802,   803,
     804,   805,   711,   103,   104,   105,   106,   775,   776,   777,
     778,   846,   779,   847,   748,   749,   750,   751,   848,   752,
     850,   851,   917,   411,   532,   533,   190,   191,   194,   195,
     819,   899,   900,   740,   994,  1023,  1024,  1025,  1026,  1027,
    1028,  1092,   901,   902,   903,   904,   996,   997,   999,  1000,
     175,   159,   160,   424,   425,   150,   407,   107,   108,   109,
     110,   131,   575,  1004,  1039,   379,   928,  1009,  1010,   112,
     113,   152,   157,   328,   412,   114,   115,   116,   117,   170,
     118,   119,   120,   121,   122,   123,   124,   125,   154,   579,
     586,   323,   324,   325,   326,   313,   314,   666,   126,   456,
     457
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      91,   111,   380,   171,   438,   817,    92,   781,   176,   653,
     712,   441,   286,   939,   720,   971,   823,   973,   151,   -14,
     296,   296,   404,   800,   149,   335,   922,   986,  1007,   196,
     378,   378,   467,   468,   184,   703,   959,    71,   473,    94,
     587,   583,   811,    15,   337,    15,   815,   987,    15,   435,
     439,   378,   623,   439,   -83,   297,   297,   387,   130,   132,
     133,   134,   135,   293,    41,    42,     3,   361,   162,   610,
     611,   339,   137,   138,   136,   140,   141,   142,   143,   144,
     139,   155,   337,   420,   161,  -470,   849,   421,   753,    71,
     111,   156,   169,   169,   852,   338,  -253,   334,   169,   865,
     178,   869,   337,   127,    12,    13,    14,    15,   196,   339,
     -83,   298,   298,   769,   792,   299,   300,   308,   906,   482,
     418,   353,   354,   355,   356,   357,   308,   158,   301,   302,
     303,   304,   305,   306,   307,   308,   315,   309,   793,   322,
     784,   148,   153,   163,   308,   296,   482,   440,  -128,   919,
     631,  -469,    41,   361,   957,   954,   127,    12,    13,    14,
      15,   308,   145,   327,   192,   319,   320,   308,   388,   330,
     355,   356,   357,   184,   824,   164,  1088,   308,   857,   940,
     297,   721,   952,   156,   923,   374,   148,  1072,   640,   332,
     172,   584,   642,   405,    88,   754,    88,   193,   950,    88,
     361,   408,   165,  1006,   333,  1011,   833,   177,   961,   849,
     169,   308,   419,   482,   166,   145,   146,   423,   426,   427,
     167,   429,   430,   867,   868,  -214,   974,   146,   168,   471,
     967,   968,   969,   970,    71,   673,   298,   867,   868,   573,
     454,    41,    42,   371,   372,    71,   311,   312,   822,  -214,
     147,   173,   455,   458,   477,   311,   312,   820,    88,  1003,
     296,   992,   993,   310,   311,   312,   197,   499,   684,   296,
     634,   374,   483,   311,   312,   474,   475,   389,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,  -183,
     311,   312,  -183,  -215,  1078,   297,   311,   312,   342,   343,
     401,   402,  -217,   147,   297,   476,   311,   312,   590,  -507,
    -507,    88,  1085,  1086,   488,   111,   535,  -215,   374,   198,
    1089,   647,  -507,  1073,  1094,   374,  -217,   403,   907,   908,
     909,   910,   870,   577,   580,   179,   180,  -507,   181,  -507,
     311,   312,   179,   322,   322,   181,   942,   662,   472,   322,
     589,   298,   199,   151,   381,   481,   179,   663,   743,   181,
     298,   664,    50,    51,   494,   480,   665,   688,   200,  -510,
    -510,   289,   493,   340,   290,   841,   127,    12,    13,    14,
     538,   614,  -510,   182,   382,   183,   500,  -506,   534,   334,
     182,  1084,   183,   127,    12,    13,    14,  -510,   410,  -510,
    -513,   543,   544,   545,   546,   547,   548,   549,   550,   551,
     552,   553,   554,   555,   556,   675,   558,   559,   560,   561,
     562,   563,   564,   565,   566,   567,   568,   569,   570,   571,
     572,   422,   184,   431,   692,   744,   488,   111,   432,   588,
     169,   337,   591,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,   602,   603,   745,   746,   604,   790,   435,
     606,   293,   445,   436,   813,   447,   446,   433,   339,   448,
     613,   169,   745,   746,   161,   434,   148,   153,  -553,  -553,
     352,   353,   354,   355,   356,   357,   724,   111,   626,  -509,
    -509,   450,   672,    41,    42,   451,   196,   578,   581,   585,
     585,   151,  -509,   637,   336,   832,   445,   149,   437,   742,
     633,   444,   840,   361,  -515,   695,  1032,  -509,   696,  -509,
    1033,   449,   698,   699,  1035,   459,   609,   460,  1036,   585,
     585,   612,   858,   785,    71,   464,   787,   872,   296,   875,
     466,   877,   716,   461,   179,   180,   322,   181,   462,   676,
     884,   322,   127,    12,    13,    14,   137,   138,   463,   140,
     141,   142,   143,   192,   155,   344,   345,   346,   478,   178,
     296,   469,   484,   297,   353,   354,   355,   356,   357,   413,
     414,   319,   320,   681,   148,  -515,   478,   296,   484,   478,
     484,   686,   415,   687,  -552,  -552,   689,  -552,   818,   492,
    -553,  -553,   496,   371,   372,   297,   361,   416,   497,   417,
     540,   145,   498,   827,   537,  -512,  -512,   127,    12,    13,
      14,   864,   297,   296,   148,   153,   667,   668,   841,   298,
     694,   374,   539,   702,   866,   867,   868,   736,   697,   541,
     741,   807,   808,  -512,   854,  -512,   127,    12,    13,    14,
     574,    91,   111,   707,    41,    42,   710,    92,   297,  1090,
    1091,   298,   915,   615,   296,   718,   329,   331,   127,    12,
      13,    14,   708,   618,   426,   713,   744,   620,   298,   621,
    -551,  -551,   454,  -551,   629,   632,   488,   111,   934,   774,
      94,   774,   630,   914,   193,   455,   745,   746,   639,   297,
    -507,  -507,   641,   644,   383,   384,   949,   645,   842,   843,
     844,   845,   716,  -507,   298,   936,   867,   868,   763,   648,
     650,   652,   759,   654,   374,   745,   746,   337,  -507,   741,
    -507,   764,   385,   660,   386,   655,   765,   534,   657,   661,
     296,   659,   670,   671,   773,  1044,   953,   745,   746,   677,
     -43,   685,    91,   111,   -70,   298,   795,   -66,    92,   791,
     148,   -67,   -68,   148,   -62,   747,   -63,   -65,   983,   322,
     -64,   -69,   682,   683,   723,   297,   886,   806,   774,   731,
     691,   693,  -538,   488,   111,   488,   111,  -540,   701,   705,
     887,    94,   439,   888,   889,   890,   891,   892,   893,   894,
     895,   733,  1013,   735,   737,   756,   739,   834,  -514,  1015,
     829,   757,   905,   758,   760,   810,   322,   897,   761,   762,
     681,   768,   780,    71,   789,   788,  -508,  -508,   797,   799,
     862,   298,   812,   820,   816,   718,   821,   774,   825,  -508,
     826,   831,   426,   322,   835,   836,    71,   838,   839,  1001,
     337,  -514,   148,   853,  -508,  1031,  -508,   855,   747,   859,
     871,   873,   488,   111,   874,  1040,   876,   878,  1064,   880,
     948,   883,  1067,   879,  1069,   882,   912,   885,   378,   741,
     911,   916,   920,   913,   924,   896,   925,   927,   926,   943,
     946,   930,  1008,   747,   931,   929,   932,  1061,   933,   964,
     951,   958,   962,   937,   897,   991,   965,   972,   980,   998,
     981,   995,  1005,   488,   111,  1018,  1030,  1075,  1093,  1076,
    1017,   938,  1014,  1034,  1042,  1043,  -469,  1041,   806,  1054,
     945,  1049,   488,   111,  1052,   148,  1045,  1055,   747,   747,
     747,   747,  1047,  1053,  1056,  1022,  1063,   488,   111,  1066,
     897,  1077,  1068,  1037,  1087,  1071,   747,   656,   443,   897,
     186,   428,   341,  1008,   881,   732,   860,   635,   976,   978,
     862,   982,   897,   837,   944,  1022,   918,   956,  1016,   963,
     636,   488,   111,   767,   955,  1029,   638,   998,  1051,  1060,
    1058,   627,   409,   148,   617,   557,   607,   984,  1065,   678,
     985,   897,   148,   734,   582,     0,     0,     0,  1074,     0,
       0,   488,   111,   488,   111,  1029,   747,     0,     0,     0,
       0,     0,     0,     0,  1083,     0,     0,     0,     0,     0,
       0,   747,     0,     0,     0,   886,     0,   747,   747,   747,
     747,     0,     0,     0,     0,     0,     0,   741,     0,   887,
       0,     0,   888,   889,   890,   891,   892,   893,   894,   895,
       0,     0,   488,   111,     0,   488,   111,   488,   111,     0,
       0,     0,  1048,     0,     0,   774,     0,     0,     0,     0,
       0,     0,    71,  -337,  1057,  -337,     0,     0,  1059,     0,
       0,   488,   111,  1062,   888,   889,   890,   891,   892,   893,
     894,   148,     0,     0,     0,     0,     0,     0,   148,     0,
       0,     0,   706,     0,     0,     0,     0,  1079,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,   148,    18,
      19,    20,    21,    22,   960,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
     148,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,     0,
       0,     0,     0,     0,     0,   886,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,    69,    70,    71,   887,
       0,     0,   888,   889,   890,   891,   892,   893,   894,   895,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    71,     0,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,    83,    84,
     709,    85,    86,    87,    88,     0,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,   990,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,     0,     0,     0,
       0,     0,     0,     0,    52,    53,    54,     0,     0,     0,
       0,     0,     0,   886,     0,    61,    62,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,   887,     0,     0,
     888,   889,   890,   891,   892,   893,   894,   895,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      71,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,    83,    84,   794,    85,
      86,    87,    88,     0,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,  1002,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,     0,     0,     0,     0,     0,
       0,   886,     0,    61,    62,     0,    63,    64,    65,    66,
      67,    68,    69,    70,    71,   887,     0,     0,   888,   889,
     890,   891,   892,   893,   894,   895,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    71,     0,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,    83,    84,   861,    85,    86,    87,
      88,     0,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
    1012,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,     0,     0,     0,     0,     0,     0,     0,
      52,    53,    54,     0,     0,     0,     0,     0,     0,   886,
       0,    61,    62,     0,    63,    64,    65,    66,    67,    68,
      69,    70,    71,   887,     0,     0,   888,   889,   890,   891,
     892,   893,   894,   895,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    71,     0,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,    83,    84,   975,    85,    86,    87,    88,     0,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,  1038,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,   886,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,     0,     0,    52,    53,
      54,   888,   889,   890,   891,   892,   893,   894,   895,    61,
      62,     0,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
      83,    84,   542,    85,    86,    87,    88,     0,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   127,
      12,    13,    14,    15,     0,     0,    17,     0,     0,    18,
      19,    20,    21,    22,    41,    42,     0,    23,    24,    25,
       0,    26,    27,    28,     0,   336,    50,    51,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
      59,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,    71,     0,     0,   128,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,     0,
       0,     0,     0,     0,     0,     0,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,   -47,
       0,    85,    86,    87,    88,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   127,    12,    13,    14,
      15,     0,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   128,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    62,     0,    63,    64,    65,
      66,    67,    68,    69,    70,    71,   347,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     0,   358,
     359,   360,     0,     0,     0,   361,     0,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    85,    86,
      87,    88,   347,     0,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,     0,     0,   347,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,   358,   359,   360,     0,     0,
       0,   361,     0,     0,     0,     0,     0,     0,     0,   358,
     359,   360,     0,     0,     0,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   373,   374,     0,   375,   376,     0,     0,     0,
     358,   359,   360,     0,     0,   669,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,     0,     0,     0,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   374,
       0,   375,   376,     0,     0,     0,     0,     0,     0,     0,
       0,   679,   373,   374,     0,   375,   376,     0,     0,     0,
       0,     0,     0,     0,     0,   700,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   347,     0,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   373,   374,     0,   375,   376,     0,     0,
       0,   358,   359,   360,     0,     0,   704,   361,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   347,     0,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
       0,     0,   347,   939,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,   358,   359,   360,     0,     0,     0,   361,     0,     0,
       0,     0,     0,     0,     0,   358,   359,   360,     0,     0,
       0,   361,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   374,     0,   375,   376,     0,
       0,     0,     0,     0,     0,     0,   361,   782,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,     0,
       0,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   374,     0,   375,   376,   350,
     351,   352,   353,   354,   355,   356,   357,   783,   373,   374,
       0,   375,   376,     0,     0,     0,     0,     0,   347,   940,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
       0,     0,     0,     0,   361,     0,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,     0,     0,
       0,   358,   359,   360,     0,     0,     0,   361,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   373,   374,     0,   375,   376,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   364,   365,   366,
     367,   368,   369,   370,   371,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,     0,
       0,     0,   374,     0,   375,   376,     0,     0,     0,     0,
       0,     0,   347,     0,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   373,   374,     0,   375,   376,     0,
       0,     4,     5,     0,     0,   377,     6,     7,     8,     0,
       9,    10,   501,    12,    13,    14,    15,     0,     0,    17,
       0,   361,   502,   503,   504,   505,   506,   212,   213,   214,
     507,   508,    25,   217,   509,   510,   511,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   512,   241,   242,   243,
     513,   514,   246,   247,   248,   249,   250,   515,   252,   253,
     254,   516,   256,   257,   258,   259,   260,   261,   262,   263,
     517,   518,     0,   266,   267,   268,   269,   270,   271,   272,
     519,   520,   275,   521,   522,   523,   524,   525,   526,   527,
     528,    71,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   406,    72,    73,    74,    75,    76,
      77,    78,    79,    80,     0,     0,     0,     0,   373,   374,
       0,   375,   376,    81,     0,     0,     0,     0,     0,   529,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    82,
     530,     0,     0,     0,    85,    86,    87,    88,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   501,
      12,    13,    14,    15,     0,     0,    17,     0,     0,   502,
     503,   504,   505,   506,   212,   213,   214,   507,   508,    25,
     217,   509,   510,   511,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   512,   241,   242,   243,   513,   514,   246,
     247,   248,   249,   250,   515,   252,   253,   254,   516,   256,
     257,   258,   259,   260,   261,   262,   263,   517,   518,     0,
     266,   267,   268,   269,   270,   271,   272,   519,   520,   275,
     521,   522,   523,   524,   525,   526,   527,   528,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,     0,     0,   766,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    85,    86,    87,    88,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,    46,    47,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
      59,     0,     0,    60,    61,    62,     0,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,    83,    84,   755,    85,    86,
      87,    88,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     0,     0,    46,    47,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,    59,     0,     0,
      60,    61,    62,     0,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,    83,    84,   828,    85,    86,    87,    88,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,     0,     0,    46,    47,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,    59,     0,     0,    60,    61,    62,
       0,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,    52,    53,    54,   486,    56,    57,
      58,    59,     0,     0,     0,    61,    62,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,    83,    84,   487,    85,
      86,    87,    88,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,    52,    53,    54,   486,    56,    57,    58,    59,     0,
       0,     0,    61,    62,     0,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,    83,    84,   628,    85,    86,    87,    88,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,   772,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,    52,    53,
      54,   486,    56,    57,    58,    59,     0,     0,     0,    61,
      62,     0,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
      83,    84,     0,    85,    86,    87,    88,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
     856,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,    52,    53,    54,   486,    56,
      57,    58,    59,     0,     0,     0,    61,    62,     0,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,   935,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,    52,    53,    54,   486,    56,    57,    58,    59,
       0,     0,     0,    61,    62,     0,    63,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,   979,    35,
       0,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,    44,    45,     0,     0,    46,     0,     0,
      48,    49,    50,    51,     0,     0,     0,     0,     0,    52,
      53,    54,   486,    56,    57,    58,    59,     0,     0,     0,
      61,    62,     0,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    82,
       0,    83,    84,     0,    85,    86,    87,    88,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,    52,    53,    54,   486,
      56,    57,    58,    59,     0,     0,     0,    61,    62,     0,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,    83,    84,
     989,    85,    86,    87,    88,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
    1046,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,    52,    53,    54,   486,    56,    57,    58,
      59,     0,     0,     0,    61,    62,     0,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,    83,    84,     0,    85,    86,
      87,    88,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
      52,    53,    54,   486,    56,    57,    58,    59,     0,     0,
       0,    61,    62,     0,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,    83,    84,  1080,    85,    86,    87,    88,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,    52,    53,    54,
     486,    56,    57,    58,    59,     0,     0,     0,    61,    62,
       0,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,    83,
      84,  1081,    85,    86,    87,    88,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,    52,    53,    54,   486,    56,    57,
      58,    59,     0,     0,     0,    61,    62,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,    83,    84,  1082,    85,
      86,    87,    88,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,    52,    53,    54,   486,    56,    57,    58,    59,     0,
       0,     0,    61,    62,     0,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,    83,    84,  1095,    85,    86,    87,    88,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,    52,    53,
      54,   486,    56,    57,    58,    59,     0,     0,     0,    61,
      62,     0,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
      83,    84,     0,    85,    86,    87,    88,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,     0,     0,
       0,     0,     0,     0,     0,     0,    61,    62,     0,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,   127,    12,    13,    14,    15,
       0,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
      46,     0,     0,     0,   128,     0,     0,     0,     0,     0,
       0,     0,     0,    53,    54,     0,     0,     0,     0,     0,
       0,     0,     0,   317,    62,     0,    63,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    81,     0,     0,     0,
       0,     0,   318,     0,     0,     0,   319,   320,     0,     0,
       0,     0,    82,     0,     0,     0,     0,    85,    86,    87,
      88,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,   127,    12,    13,    14,    15,     0,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
      41,    42,     0,     0,     0,     0,     0,    46,     0,     0,
       0,   128,     0,     0,     0,     0,     0,     0,     0,     0,
      53,    54,     0,     0,     0,     0,     0,     0,     0,     0,
      61,    62,     0,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   319,   320,     0,     0,     0,     0,    82,
       0,     0,     0,     0,    85,    86,    87,    88,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   127,
      12,    13,    14,    15,     0,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,     0,     0,     0,   128,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,     0,
       0,     0,     0,     0,     0,     0,     0,   674,    62,     0,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     319,   320,     0,     0,     0,     0,    82,     0,     0,     0,
       0,    85,    86,    87,    88,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   127,    12,    13,    14,
      15,     0,     0,    17,   479,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   128,     0,     0,     0,     0,
       0,     0,     0,     0,    53,    54,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    62,     0,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,    85,    86,
      87,    88,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   127,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   128,     0,     0,     0,     0,     0,     0,     0,
       0,    53,    54,     0,     0,     0,     0,     0,     0,     0,
       0,    61,    62,     0,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,   680,     0,     0,     0,    85,    86,    87,    88,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     127,    12,    13,    14,    15,     0,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   801,    37,     0,     0,     0,    41,    42,
       0,     0,     0,     0,     0,    46,     0,     0,     0,   128,
       0,     0,     0,     0,     0,     0,     0,     0,    53,    54,
       0,     0,     0,     0,     0,     0,     0,     0,    61,    62,
       0,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,    85,    86,    87,    88,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   127,    12,    13,
      14,    15,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,    46,     0,     0,     0,   128,     0,     0,     0,
       0,     0,     0,     0,     0,    53,    54,     0,     0,     0,
       0,     0,     0,     0,     0,    61,    62,     0,    63,    64,
      65,    66,    67,    68,    69,    70,    71,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
     358,   359,   360,     0,     0,     0,   361,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,    85,
      86,    87,    88,   347,     0,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   358,   359,   360,     0,
       0,     0,   361,     0,     0,     0,     0,     0,   358,   359,
     360,     0,     0,     0,   361,     0,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   373,   374,     0,   375,   376,     0,     0,
       0,     0,     0,   485,   358,   359,   360,     0,     0,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   373,
     374,     0,   375,   376,     0,     0,     0,     0,     0,   605,
       0,   373,   374,     0,   375,   376,     0,     0,     0,     0,
       0,   608,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,     0,     0,     0,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373,   374,     0,
     375,   376,     0,     0,     0,     0,     0,   616,   358,   359,
     360,     0,     0,     0,   361,     0,     0,     0,     0,     0,
       0,     0,   347,     0,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   347,     0,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   358,   359,   360,     0,     0,
       0,   361,     0,     0,     0,     0,     0,   358,   359,   360,
       0,     0,     0,   361,     0,     0,     0,     0,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,     0,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,     0,     0,     0,     0,
       0,     0,     0,     0,   358,   359,   360,     0,     0,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,   374,     0,   375,   376,     0,     0,     0,     0,
       0,   619,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,     0,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   374,
       0,   375,   376,     0,     0,     0,     0,     0,   624,     0,
     373,   374,     0,   375,   376,     0,     0,     0,     0,     0,
     625,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,     0,     0,     0,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373,   374,     0,
     375,   376,     0,     0,     0,     0,     0,   643,   358,   359,
     360,     0,     0,     0,   361,     0,     0,     0,     0,     0,
       0,     0,   347,     0,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   347,     0,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   358,   359,   360,     0,     0,
       0,   361,     0,     0,     0,     0,     0,   358,   359,   360,
       0,     0,     0,   361,     0,     0,     0,     0,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,     0,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,     0,     0,     0,     0,
       0,     0,     0,     0,   358,   359,   360,     0,     0,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,   374,     0,   375,   376,     0,     0,     0,     0,
       0,   658,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,     0,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   374,
       0,   375,   376,     0,     0,     0,     0,     0,   770,     0,
     373,   374,     0,   375,   376,     0,     0,     0,     0,     0,
     771,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,     0,     0,     0,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373,   374,     0,
     375,   376,     0,     0,     0,     0,     0,   786,   358,   359,
     360,     0,     0,     0,   361,     0,     0,     0,     0,     0,
     347,     0,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   358,   359,   360,     0,     0,     0,   361,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   347,     0,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   358,   359,   360,     0,
       0,     0,   361,     0,     0,     0,     0,     0,   347,   690,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     622,   373,   374,     0,   375,   376,     0,     0,     0,   830,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   358,   359,   360,     0,     0,     0,   361,     0,     0,
       0,     0,   495,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,   374,     0,   375,
     376,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   358,   359,   360,     0,     0,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,   373,
     374,     0,   375,   376,     0,     0,     0,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   347,
       0,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,     0,   373,   374,     0,   375,   376,     0,
       0,     0,     0,   359,   360,     0,     0,     0,   361,     0,
       0,     0,     0,     0,     0,     0,   360,     0,     0,     0,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   347,     0,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,     0,     0,     0,   373,   374,     0,
     375,   376,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   361,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   361,     0,     0,     0,     0,     0,     0,     0,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
       0,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,     0,     0,     0,   373,   374,     0,   375,   376,
       0,     0,     0,     0,     0,     0,     0,   373,   374,     0,
     375,   376,     0,     0,     0,     0,     0,     0,     0,   361,
       0,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   349,   350,   351,   352,   353,   354,   355,
     356,   357,     0,     0,     0,     0,     0,     0,   373,   374,
       0,   375,   376,     0,     0,     0,     0,     0,     0,   374,
       0,   375,   376,     0,     0,     0,     0,     0,     0,   361,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   374,     0,   375,
     376,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   364,   365,   366,   367,   368,   369,   370,   371,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,    15,     0,   374,     0,   375,
     376,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,     0,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,     0,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     127,    12,    13,    14,    15,     0,     0,    17,     0,     0,
     127,    12,    13,    14,    15,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
       0,   576,     0,     0,     0,     0,    88,     0,     0,   145,
     350,   351,   352,   353,   354,   355,   356,   357,   714,    62,
       0,    63,    64,    65,    66,    67,    68,    69,    70,    62,
       0,    63,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,   361,     0,     0,   350,   351,
     352,   353,   354,   355,   356,   357,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   319,   320,   361,     0,     0,     0,   294,     0,     0,
       0,     0,   715,     0,    87,    88,     0,   294,     0,     0,
       0,     0,   295,     0,    87,    88,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -553,  -553,
    -553,  -553,   368,   369,  -553,   371,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   374,     0,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,     0,     0,     0,     0,     0,
       0,   501,    12,    13,    14,     0,     0,     0,     0,     0,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   374,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
    1020,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,     0,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   501,    12,    13,    14,     0,     0,     0,     0,
       0,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,  1021,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,  1020,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,     0,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,     0,   206,     0,     0,     0,
       0,     0,     0,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,  1050,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,     0,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,     0,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,     0,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,  1070,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,     0,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     888,   889,   890,   891,   892,   893,   894,   262,   263,   264,
     265,     0,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283
};

static const yytype_int16 yycheck[] =
{
       2,     2,   111,    39,   177,   737,     2,   693,    44,   466,
     620,   186,    60,     6,     6,   926,     7,   928,    26,     6,
      72,    73,   131,   721,    26,    89,     7,     7,    27,    49,
     110,   111,   289,   290,    47,   607,   903,   112,   295,     2,
     384,    23,   727,    27,    91,    27,   731,    27,    27,   168,
      62,   131,   171,    62,     6,    72,    73,   121,     4,     5,
       6,     7,     8,    71,    71,    72,     0,    44,    31,   413,
     414,    91,    18,    19,     6,    21,    22,    23,    24,    25,
     170,    27,   129,   168,    30,   158,   778,   172,   651,   112,
      91,   170,    38,    39,   780,    91,   171,   170,    44,   797,
      46,   799,   149,    23,    24,    25,    26,    27,   128,   129,
      62,    72,    73,   685,   147,    72,    73,    27,   821,    29,
     156,    12,    13,    14,    15,    16,    27,   170,    74,    75,
      76,    77,    78,    79,    80,    27,    82,    29,   171,    85,
     703,    26,    26,   170,    27,   197,    29,   159,   171,   841,
     159,   158,    71,    44,    73,   887,    23,    24,    25,    26,
      27,    27,    82,    29,    48,   164,   165,    27,     4,    29,
      14,    15,    16,   186,   165,   170,  1087,    27,   788,   172,
     197,   173,   885,   170,   165,   162,    71,  1054,   452,   173,
      23,   173,   456,   139,   178,   652,   178,    27,   883,   178,
      44,   147,   170,   971,    88,   973,   769,   173,   911,   901,
     156,    27,   158,    29,   170,    82,    94,   163,   164,   165,
     170,   167,   168,    65,    66,   147,   929,    94,   170,   293,
     922,   923,   924,   925,   112,   492,   197,    65,    66,   379,
     197,    71,    72,   134,   135,   112,   156,   157,   743,   171,
     170,    23,   198,   199,   155,   156,   157,   168,   178,   962,
     312,   172,   173,   155,   156,   157,   170,   177,   532,   321,
     445,   162,   155,   156,   157,   145,   146,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   168,
     156,   157,   171,   147,  1062,   312,   156,   157,    49,    50,
     136,   137,   147,   170,   321,   175,   156,   157,   388,   145,
     146,   178,  1080,  1081,   316,   316,   336,   171,   162,   170,
    1088,   461,   158,  1055,  1092,   162,   171,   163,   823,   824,
     825,   826,   174,   381,   382,    23,    24,   173,    26,   175,
     156,   157,    23,   289,   290,    26,   174,    13,   294,   295,
     386,   312,   170,   361,   158,   312,    23,    23,     5,    26,
     321,    27,    83,    84,   321,   311,    32,   540,   170,   145,
     146,   170,   318,    94,   170,     5,    23,    24,    25,    26,
     343,   417,   158,    71,   158,    73,   332,   158,   334,   170,
      71,  1077,    73,    23,    24,    25,    26,   173,   158,   175,
     170,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   495,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,    54,   445,   172,   574,    82,   438,   438,   172,   385,
     386,   488,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   102,   103,   403,   715,   168,
     406,   469,   168,   172,   728,   168,   172,   172,   488,   172,
     416,   417,   102,   103,   420,     4,   361,   361,     9,    10,
      11,    12,    13,    14,    15,    16,   626,   488,   434,   145,
     146,   168,   488,    71,    72,   172,   516,   381,   382,   383,
     384,   509,   158,   449,    82,   762,   168,   509,   172,   649,
     172,   172,   776,    44,   170,   579,   168,   173,   582,   175,
     172,     4,   586,   587,   168,   171,   410,    23,   172,   413,
     414,   415,   789,   706,   112,   172,   709,   801,   590,   803,
     173,   805,   622,    23,    23,    24,   492,    26,    23,   495,
     814,   497,    23,    24,    25,    26,   502,   503,    23,   505,
     506,   507,   508,   447,   510,    49,    50,    51,   309,   515,
     622,   168,   313,   590,    12,    13,    14,    15,    16,   145,
     146,   164,   165,   529,   469,   170,   327,   639,   329,   330,
     331,   537,   158,   539,   168,   169,   542,   171,   738,   170,
     131,   132,   169,   134,   135,   622,    44,   173,   168,   175,
       6,    82,   176,   753,   170,   145,   146,    23,    24,    25,
      26,   794,   639,   675,   509,   509,   174,   175,     5,   590,
     576,   162,   170,   590,    64,    65,    66,   645,   584,   172,
     648,    76,    77,   173,   784,   175,    23,    24,    25,    26,
      23,   653,   653,   616,    71,    72,   619,   653,   675,   172,
     173,   622,   835,   171,   716,   622,    86,    87,    23,    24,
      25,    26,   618,   170,   620,   621,    82,   172,   639,   168,
     168,   169,   639,   171,    23,   172,   688,   688,   861,   691,
     653,   693,   173,   833,    27,   641,   102,   103,   168,   716,
     145,   146,   168,   172,   145,   146,   879,    98,    85,    86,
      87,    88,   792,   158,   675,    64,    65,    66,   675,    98,
       6,   173,   668,   171,   162,   102,   103,   774,   173,   737,
     175,   677,   173,    23,   175,   171,   682,   683,   169,    23,
     792,   169,   174,   170,   690,  1009,   886,   102,   103,     4,
       6,    23,   754,   754,     6,   716,   719,     6,   754,   716,
     645,     6,     6,   648,     6,   650,     6,     6,   941,   715,
       6,     6,     6,   168,   173,   792,    65,   723,   780,   173,
     170,   170,   170,   785,   785,   787,   787,   170,   169,   169,
      79,   754,    62,    82,    83,    84,    85,    86,    87,    88,
      89,   171,   975,   171,    99,     4,   173,   770,   170,   982,
     756,    32,   820,   169,   171,    62,   762,   819,   171,   170,
     766,   171,   170,   112,   170,   172,   145,   146,   172,   172,
     793,   792,   168,   168,   172,   792,   173,   839,     7,   158,
     165,   172,   788,   789,     6,   172,   112,   171,   168,   958,
     897,   170,   737,   171,   173,   995,   175,   172,   743,   169,
     168,   174,   864,   864,   168,  1005,   168,   170,  1041,    23,
     878,   168,  1045,   173,  1047,   174,   174,   173,   958,   887,
     173,     6,   164,   171,     7,   174,   165,    79,   171,   147,
     147,   172,   972,   778,   171,   173,   171,  1037,   171,   147,
     174,   902,   173,   172,   906,   953,   160,   170,   172,   957,
     172,    27,    27,   915,   915,     4,     4,  1057,  1091,  1059,
     171,   867,   172,     4,    27,   168,   158,   173,   874,    62,
     876,   172,   934,   934,   172,   820,   173,    80,   823,   824,
     825,   826,   173,   172,   158,   993,     4,   949,   949,   171,
     952,   170,   172,  1001,   171,  1054,   841,   469,   186,   961,
      47,   166,    97,  1043,   812,   639,   792,   445,   931,   933,
     933,   938,   974,   774,   874,  1023,   839,   901,   986,   916,
     447,   983,   983,   683,   897,   993,   450,  1035,  1023,  1035,
    1032,   435,   149,   878,   420,   361,   407,   943,  1043,   497,
     946,  1003,   887,   641,   382,    -1,    -1,    -1,  1056,    -1,
      -1,  1013,  1013,  1015,  1015,  1023,   901,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1072,    -1,    -1,    -1,    -1,    -1,
      -1,   916,    -1,    -1,    -1,    65,    -1,   922,   923,   924,
     925,    -1,    -1,    -1,    -1,    -1,    -1,  1055,    -1,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,  1064,  1064,    -1,  1067,  1067,  1069,  1069,    -1,
      -1,    -1,  1018,    -1,    -1,  1077,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    71,  1030,    73,    -1,    -1,  1034,    -1,
      -1,  1093,  1093,  1039,    82,    83,    84,    85,    86,    87,
      88,   986,    -1,    -1,    -1,    -1,    -1,    -1,   993,    -1,
      -1,    -1,     6,    -1,    -1,    -1,    -1,  1063,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,  1023,    33,
      34,    35,    36,    37,   174,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
    1055,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,   101,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,
       6,   175,   176,   177,   178,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,   174,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,   101,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,   172,   173,     6,   175,
     176,   177,   178,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,   174,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,   101,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    -1,   172,   173,     6,   175,   176,   177,
     178,    -1,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
     174,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,   172,   173,     6,   175,   176,   177,   178,    -1,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,   174,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    65,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    82,    83,    84,    85,    86,    87,    88,    89,   101,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
     172,   173,     6,   175,   176,   177,   178,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    71,    72,    -1,    41,    42,    43,
      -1,    45,    46,    47,    -1,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    -1,    -1,    78,   112,    -1,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,     6,
      -1,   175,   176,   177,   178,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,   102,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,   154,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,    -1,    -1,    -1,   175,   176,
     177,   178,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     5,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,   162,    -1,   164,   165,    -1,    -1,    -1,
      38,    39,    40,    -1,    -1,   174,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,    -1,    -1,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,   161,   162,    -1,   164,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,   162,    -1,   164,   165,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,   174,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,   162,    -1,   164,   165,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,   174,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,    -1,
      -1,    -1,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,   162,    -1,   164,   165,     9,
      10,    11,    12,    13,    14,    15,    16,   174,   161,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,     5,   172,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    44,    -1,    -1,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,   162,    -1,   164,   165,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,    -1,
      -1,    -1,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,
      -1,    -1,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   161,   162,    -1,   164,   165,    -1,
      -1,    12,    13,    -1,    -1,   172,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    30,
      -1,    44,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   136,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,   161,   162,
      -1,   164,   165,   154,    -1,    -1,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
     171,    -1,    -1,    -1,   175,   176,   177,   178,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,
      -1,   175,   176,   177,   178,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,   100,   101,   102,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   172,   173,   174,   175,   176,
     177,   178,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    -1,    -1,    78,    79,
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
     100,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,   172,   173,   174,   175,   176,   177,   178,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    -1,    -1,    78,    79,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,   100,   101,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,
     173,    -1,   175,   176,   177,   178,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,   101,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,   172,   173,   174,   175,
     176,   177,   178,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,   101,   102,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    -1,   172,   173,   174,   175,   176,   177,   178,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    51,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,   101,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
     172,   173,    -1,   175,   176,   177,   178,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      55,    56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    74,
      75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,   101,   102,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   136,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,    -1,
     175,   176,   177,   178,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    61,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,   101,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    -1,   172,   173,    -1,   175,   176,   177,
     178,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    58,    59,    60,
      -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
     101,   102,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,
      -1,   172,   173,    -1,   175,   176,   177,   178,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,    -1,   101,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,
     174,   175,   176,   177,   178,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      57,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    74,    75,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,   101,   102,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,   172,   173,    -1,   175,   176,
     177,   178,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,    -1,   172,   173,   174,   175,   176,   177,   178,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,   101,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,   172,
     173,   174,   175,   176,   177,   178,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,   101,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,   172,   173,   174,   175,
     176,   177,   178,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,   101,   102,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    -1,   172,   173,   174,   175,   176,   177,   178,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,   101,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
     172,   173,    -1,   175,   176,   177,   178,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    74,
      75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    90,    91,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,   102,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   136,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,   172,   173,    -1,
     175,   176,   177,   178,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      -1,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   101,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,
      -1,    -1,   160,    -1,    -1,    -1,   164,   165,    -1,    -1,
      -1,    -1,   170,    -1,    -1,    -1,    -1,   175,   176,   177,
     178,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,   102,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   164,   165,    -1,    -1,    -1,    -1,   170,
      -1,    -1,    -1,    -1,   175,   176,   177,   178,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,    -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,
      -1,   175,   176,   177,   178,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,   102,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    -1,    -1,    -1,    -1,   175,   176,
     177,   178,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     170,   171,    -1,    -1,    -1,   175,   176,   177,   178,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    67,    -1,    -1,    -1,    71,    72,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,
      -1,    -1,   175,   176,   177,   178,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     5,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      38,    39,    40,    -1,    -1,    -1,    44,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,    -1,    -1,    -1,   175,
     176,   177,   178,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,    -1,    -1,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,   162,    -1,   164,   165,    -1,    -1,
      -1,    -1,    -1,   171,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,    -1,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
     162,    -1,   164,   165,    -1,    -1,    -1,    -1,    -1,   171,
      -1,   161,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,    -1,    -1,    -1,    -1,   171,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,
      -1,   171,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
     161,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,    -1,
     171,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,    -1,    -1,    -1,    -1,   171,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,
      -1,   171,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
     161,   162,    -1,   164,   165,    -1,    -1,    -1,    -1,    -1,
     171,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,    -1,    -1,    -1,    -1,   171,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    38,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      62,   161,   162,    -1,   164,   165,    -1,    -1,    -1,   169,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,   164,
     165,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
     162,    -1,   164,   165,    -1,    -1,    -1,    -1,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,   161,   162,    -1,   164,   165,    -1,
      -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      44,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
      -1,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,   161,   162,    -1,   164,   165,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,    -1,
     164,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,    -1,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,   129,   130,   131,   132,   133,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,   164,
     165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,   129,   130,   131,   132,   133,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    -1,    -1,    -1,    27,    -1,   162,    -1,   164,
     165,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,
      -1,   173,    -1,    -1,    -1,    -1,   178,    -1,    -1,    82,
       9,    10,    11,    12,    13,    14,    15,    16,   101,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   164,   165,    44,    -1,    -1,    -1,   170,    -1,    -1,
      -1,    -1,   175,    -1,   177,   178,    -1,   170,    -1,    -1,
      -1,    -1,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
     129,   130,   131,   132,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,    -1,   127,   128,   129,   130,
     131,   132,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,   162,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,   174,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,   174,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    -1,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    -1,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   180,   185,     0,    12,    13,    17,    18,    19,    21,
      22,    23,    24,    25,    26,    27,    28,    30,    33,    34,
      35,    36,    37,    41,    42,    43,    45,    46,    47,    48,
      52,    53,    54,    56,    58,    60,    63,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    78,    79,    81,    82,
      83,    84,    90,    91,    92,    93,    94,    95,    96,    97,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   154,   170,   172,   173,   175,   176,   177,   178,   189,
     192,   193,   194,   195,   211,   219,   222,   225,   226,   227,
     229,   231,   246,   252,   253,   254,   255,   306,   307,   308,
     309,   310,   318,   319,   324,   325,   326,   327,   329,   330,
     331,   332,   333,   334,   335,   336,   347,    23,    82,   193,
     307,   310,   307,   307,   307,   307,     6,   307,   307,   170,
     307,   307,   307,   307,   307,    82,    94,   170,   189,   193,
     304,   319,   320,   335,   337,   307,   170,   321,   170,   300,
     301,   307,   211,   170,   170,   170,   170,   170,   170,   307,
     328,   328,    23,    23,   208,   299,   328,   173,   307,    23,
      24,    26,    71,    73,   187,   188,   198,   200,   204,   207,
     275,   276,   335,    27,   277,   278,   308,   170,   170,   170,
     170,   224,   228,   230,   232,    23,    26,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   181,   182,   184,   186,   197,   170,
     170,   190,   191,   319,   170,   175,   324,   326,   327,   334,
     334,   307,   307,   307,   307,   307,   307,   307,    27,    29,
     155,   156,   157,   344,   345,   307,   209,   101,   160,   164,
     165,   183,   307,   340,   341,   342,   343,    29,   322,   344,
      29,   344,   173,   335,   170,   272,    82,   192,   194,   308,
      94,   226,    49,    50,    49,    50,    51,     5,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    38,    39,
      40,    44,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   161,   162,   164,   165,   172,   183,   314,
     314,   158,   158,   145,   146,   173,   175,   272,     4,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   136,   137,   163,   314,   307,   147,   305,   307,   304,
     158,   272,   323,   145,   146,   158,   173,   175,   328,   307,
     168,   172,    54,   307,   302,   303,   307,   307,   208,   307,
     307,   172,   172,   172,     4,   168,   172,   172,   209,    62,
     159,   188,   199,   204,   172,   168,   172,   168,   172,     4,
     168,   172,   217,   218,   334,   307,   348,   349,   307,   171,
      23,    23,    23,    23,   172,   196,   173,   340,   340,   168,
     201,   272,   307,   340,   145,   146,   175,   155,   345,    31,
     307,   334,    29,   155,   345,   171,    93,   174,   193,   194,
     210,   211,   170,   307,   334,   147,   169,   168,   176,   177,
     307,    23,    33,    34,    35,    36,    37,    41,    42,    45,
      46,    47,    67,    71,    72,    78,    82,    91,    92,   101,
     102,   104,   105,   106,   107,   108,   109,   110,   111,   160,
     171,   184,   273,   274,   307,   308,   223,   170,   211,   170,
       6,   172,     6,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   320,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   311,    23,   311,   173,   184,   335,   338,
     184,   335,   338,    23,   173,   335,   339,   339,   307,   328,
     183,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   171,   307,   323,   171,   335,
     339,   339,   335,   307,   328,   171,   171,   301,   170,   171,
     172,   168,    62,   171,   171,   171,   307,   299,   174,    23,
     173,   159,   172,   172,   188,   207,   276,   307,   278,   168,
     201,   168,   201,   171,   172,    98,   236,   311,    98,   237,
       6,   233,   173,   185,   171,   171,   190,   169,   171,   169,
      23,    23,    13,    23,    27,    32,   346,   174,   175,   174,
     174,   170,   194,   340,   101,   183,   307,     4,   341,   174,
     171,   307,     6,   168,   201,    23,   307,   307,   209,   307,
       6,   170,   311,   170,   307,   272,   272,   307,   272,   272,
     174,   169,   334,   236,   174,   169,     6,   211,   307,     6,
     211,   251,   302,   307,   101,   175,   183,   239,   334,   212,
       6,   173,   243,   173,   311,   213,   187,   198,   202,   205,
     206,   173,   218,   171,   349,   171,   319,    99,   238,   173,
     282,   319,   311,     5,    82,   102,   103,   189,   263,   264,
     265,   266,   268,   238,   185,   174,     4,    32,   169,   307,
     171,   171,   170,   334,   307,   307,   160,   274,   171,   236,
     171,   171,    51,   307,   193,   256,   257,   258,   259,   261,
     170,   256,   174,   174,   238,   209,   171,   209,   172,   170,
     340,   334,   147,   171,     6,   211,   242,   172,   244,   172,
     244,    66,   247,   248,   249,   250,   307,    76,    77,   216,
      62,   206,   168,   201,   203,   206,   172,   282,   311,   279,
     168,   173,   264,     7,   165,     7,   165,   311,   174,   307,
     169,   172,   340,   238,   211,     6,   172,   261,   171,   168,
     201,     5,    85,    86,    87,    88,   260,   262,   267,   268,
     269,   270,   256,   171,   311,   172,    55,   302,   340,   169,
     239,     6,   211,   241,   209,   244,    64,    65,    66,   244,
     174,   168,   201,   174,   168,   201,   168,   201,   170,   173,
      23,   205,   174,   168,   201,   173,    65,    79,    82,    83,
      84,    85,    86,    87,    88,    89,   174,   193,   234,   280,
     281,   291,   292,   293,   294,   319,   279,   264,   264,   264,
     264,   173,   174,   171,   311,   209,     6,   271,   258,   268,
     164,   220,     7,   165,     7,   165,   171,    79,   315,   173,
     172,   171,   171,   171,   209,    61,    64,   172,   307,     6,
     172,   245,   174,   147,   249,   307,   147,   214,   319,   209,
     206,   174,   279,   311,   282,   280,   262,    73,   310,   294,
     174,   279,   173,   263,   147,   160,   221,   268,   268,   268,
     268,   271,   170,   271,   279,     6,   211,   240,   241,    59,
     172,   172,   245,   209,   307,   307,     7,    27,   215,   174,
     174,   184,   172,   173,   283,    27,   295,   296,   184,   297,
     298,   314,   174,   279,   312,    27,   312,    27,   183,   316,
     317,   312,   174,   209,   172,   209,   319,   171,     4,   235,
      82,   174,   184,   284,   285,   286,   287,   288,   289,   319,
       4,   311,   168,   172,     4,   168,   172,   184,   174,   313,
     311,   173,    27,   168,   201,   173,    57,   173,   307,   172,
     174,   285,   172,   172,    62,    80,   158,   307,   296,   307,
     298,   311,   307,     4,   209,   317,   171,   209,   172,   209,
      23,   181,   294,   282,   184,   311,   311,   170,   312,   307,
     174,   174,   174,   184,   256,   312,   312,   171,   271,   312,
     172,   173,   290,   209,   312,   174
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   179,   180,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   182,   182,   182,   182,   182,   182,   182,
     182,   183,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   188,   189,   189,   189,   189,   190,   190,   191,
     191,   192,   193,   193,   194,   194,   194,   194,   194,   195,
     195,   195,   195,   195,   196,   195,   197,   195,   195,   195,
     195,   195,   195,   198,   198,   199,   200,   201,   201,   202,
     202,   203,   203,   204,   204,   205,   205,   206,   206,   207,
     207,   208,   208,   209,   209,   210,   210,   210,   210,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   212,   211,   211,
     211,   211,   211,   213,   213,   214,   214,   215,   215,   216,
     216,   217,   217,   218,   219,   220,   220,   221,   221,   223,
     222,   224,   222,   225,   225,   226,   226,   228,   227,   230,
     229,   232,   231,   233,   233,   234,   235,   235,   236,   236,
     237,   237,   238,   238,   239,   239,   239,   239,   240,   240,
     241,   241,   242,   242,   243,   243,   243,   243,   244,   244,
     244,   245,   245,   246,   247,   247,   248,   248,   249,   249,
     250,   250,   251,   251,   252,   252,   253,   253,   254,   254,
     255,   255,   256,   256,   257,   257,   258,   258,   259,   259,
     260,   260,   260,   260,   261,   261,   262,   262,   263,   263,
     263,   263,   264,   264,   265,   265,   266,   266,   267,   267,
     267,   267,   268,   268,   268,   269,   269,   270,   270,   271,
     271,   272,   272,   272,   273,   273,   274,   274,   274,   275,
     275,   276,   277,   277,   278,   278,   279,   279,   280,   280,
     280,   280,   281,   281,   281,   282,   282,   283,   283,   283,
     284,   284,   285,   285,   286,   287,   287,   287,   287,   288,
     288,   289,   290,   290,   291,   291,   292,   292,   293,   293,
     294,   294,   294,   294,   294,   294,   294,   295,   295,   296,
     296,   297,   297,   298,   299,   300,   300,   301,   302,   302,
     303,   303,   305,   304,   306,   306,   306,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     308,   308,   309,   310,   311,   312,   313,   314,   314,   315,
     315,   316,   316,   317,   317,   318,   318,   318,   318,   319,
     319,   320,   320,   320,   321,   321,   322,   322,   322,   323,
     323,   324,   324,   324,   324,   325,   325,   325,   325,   325,
     325,   325,   325,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   327,   327,   328,   328,   329,   330,   330,   330,
     330,   331,   331,   332,   332,   332,   333,   333,   333,   333,
     333,   333,   334,   334,   334,   334,   335,   335,   335,   336,
     336,   337,   337,   337,   337,   337,   337,   337,   338,   338,
     338,   339,   339,   339,   340,   341,   341,   342,   342,   343,
     343,   343,   343,   343,   343,   343,   344,   344,   344,   344,
     345,   345,   345,   345,   345,   345,   345,   345,   346,   346,
     346,   346,   347,   347,   347,   347,   347,   347,   347,   348,
     348,   349
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       3,     4,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     2,     4,     3,     0,     6,     0,     5,     3,     4,
       3,     4,     3,     1,     1,     6,     6,     0,     1,     3,
       1,     3,     1,     3,     1,     1,     2,     1,     3,     1,
       3,     3,     1,     2,     0,     1,     1,     2,     4,     3,
       1,     1,     5,     7,     9,     5,     3,     3,     3,     3,
       3,     3,     1,     2,     6,     7,     9,     0,     6,     1,
       6,     3,     2,     0,     9,     1,     3,     0,     1,     0,
       4,     1,     3,     1,    13,     0,     1,     0,     1,     0,
      10,     0,     9,     1,     2,     1,     1,     0,     7,     0,
       8,     0,     9,     0,     2,     5,     0,     2,     0,     2,
       0,     2,     0,     2,     1,     2,     4,     3,     1,     4,
       1,     4,     1,     4,     3,     4,     4,     5,     0,     5,
       4,     1,     1,     7,     0,     2,     1,     3,     4,     4,
       1,     3,     1,     4,     5,     6,     1,     3,     6,     7,
       3,     6,     2,     0,     1,     3,     2,     1,     0,     2,
       1,     1,     1,     1,     6,     8,     0,     1,     1,     2,
       1,     1,     1,     1,     3,     3,     3,     3,     1,     2,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     0,
       2,     2,     4,     3,     1,     3,     1,     3,     2,     3,
       1,     1,     3,     1,     1,     3,     2,     0,     4,     4,
      12,     1,     1,     2,     3,     1,     3,     1,     2,     3,
       1,     2,     2,     2,     3,     3,     3,     4,     3,     1,
       1,     3,     1,     3,     1,     1,     0,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     2,
       4,     3,     1,     4,     4,     3,     1,     1,     0,     1,
       3,     1,     0,     9,     3,     2,     3,     1,     6,     5,
       3,     4,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     5,     4,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     3,     2,
       1,     2,     4,     2,     2,     1,     2,     2,     3,     1,
      13,    12,     1,     1,     0,     0,     0,     0,     1,     0,
       5,     3,     1,     1,     2,     2,     4,     4,     2,     1,
       1,     1,     1,     3,     0,     3,     0,     1,     1,     0,
       1,     4,     3,     1,     3,     1,     1,     3,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     0,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     3,     1,     1,     4,     4,     4,
       4,     1,     1,     1,     3,     3,     1,     4,     2,     3,
       3,     1,     4,     4,     3,     3,     3,     3,     1,     3,
       1,     1,     3,     1,     1,     0,     1,     3,     1,     3,
       1,     4,     2,     2,     6,     4,     2,     2,     1,     2,
       1,     4,     3,     3,     3,     3,     6,     3,     1,     1,
       2,     1,     5,     4,     2,     2,     4,     2,     2,     1,
       3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ZENDEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ZENDEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ZENDerror or ZENDUNDEF. */
#define YYERRCODE ZENDUNDEF


/* Enable debugging if requested.  */
#if ZENDDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ZENDDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ZENDDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_T_LNUMBER: /* "integer"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_DNUMBER: /* "floating-point number"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_STRING: /* "identifier"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_FULLY_QUALIFIED: /* "fully qualified name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_RELATIVE: /* "namespace-relative name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NAME_QUALIFIED: /* "namespaced name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_VARIABLE: /* "variable"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_INLINE_HTML: /* T_INLINE_HTML  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_ENCAPSED_AND_WHITESPACE: /* "string content"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_CONSTANT_ENCAPSED_STRING: /* "quoted string"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_STRING_VARNAME: /* "variable name"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_T_NUM_STRING: /* "number"  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_identifier: /* identifier  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_top_statement_list: /* top_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_namespace_declaration_name: /* namespace_declaration_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_namespace_name: /* namespace_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_legacy_namespace_name: /* legacy_namespace_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_name: /* name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute_decl: /* attribute_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute_group: /* attribute_group  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attribute: /* attribute  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributes: /* attributes  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_statement: /* attributed_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_top_statement: /* top_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_group_use_declaration: /* group_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_mixed_group_use_declaration: /* mixed_group_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_use_declarations: /* inline_use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unprefixed_use_declarations: /* unprefixed_use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_use_declarations: /* use_declarations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_use_declaration: /* inline_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unprefixed_use_declaration: /* unprefixed_use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_use_declaration: /* use_declaration  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_const_list: /* const_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inner_statement_list: /* inner_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inner_statement: /* inner_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_statement: /* statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_catch_list: /* catch_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_catch_name_list: /* catch_name_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_variable: /* optional_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_finally_statement: /* finally_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unset_variables: /* unset_variables  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_unset_variable: /* unset_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_function_declaration_statement: /* function_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_declaration_statement: /* class_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_declaration_statement: /* trait_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_interface_declaration_statement: /* interface_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_declaration_statement: /* enum_declaration_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_backing_type: /* enum_backing_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_case: /* enum_case  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_enum_case_expr: /* enum_case_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_extends_from: /* extends_from  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_interface_extends_list: /* interface_extends_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_implements_list: /* implements_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_foreach_variable: /* foreach_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_for_statement: /* for_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_foreach_statement: /* foreach_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_declare_statement: /* declare_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_switch_case_list: /* switch_case_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_case_list: /* case_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match: /* match  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm_list: /* match_arm_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_match_arm_list: /* non_empty_match_arm_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm: /* match_arm  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_match_arm_cond_list: /* match_arm_cond_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_while_statement: /* while_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_if_stmt_without_else: /* if_stmt_without_else  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_if_stmt: /* if_stmt  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_alt_if_stmt_without_else: /* alt_if_stmt_without_else  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_alt_if_stmt: /* alt_if_stmt  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_parameter_list: /* parameter_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_parameter_list: /* non_empty_parameter_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_parameter: /* attributed_parameter  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_parameter: /* parameter  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_type_without_static: /* optional_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_expr: /* type_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type: /* type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type: /* union_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_intersection_type: /* intersection_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_expr_without_static: /* type_expr_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_type_without_static: /* type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_union_type_without_static: /* union_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_intersection_type_without_static: /* intersection_type_without_static  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_return_type: /* return_type  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument_list: /* argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_argument_list: /* non_empty_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument: /* argument  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_global_var_list: /* global_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_global_var: /* global_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_var_list: /* static_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_var: /* static_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_statement_list: /* class_statement_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_attributed_class_statement: /* attributed_class_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_statement: /* class_statement  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name_list: /* class_name_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptations: /* trait_adaptations  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptation_list: /* trait_adaptation_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_adaptation: /* trait_adaptation  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_precedence: /* trait_precedence  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_alias: /* trait_alias  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_trait_method_reference: /* trait_method_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_absolute_trait_method_reference: /* absolute_trait_method_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_method_body: /* method_body  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_list: /* property_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property: /* property  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_const_list: /* class_const_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_const_decl: /* class_const_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_const_decl: /* const_decl  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_echo_expr_list: /* echo_expr_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_echo_expr: /* echo_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_for_exprs: /* for_exprs  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_for_exprs: /* non_empty_for_exprs  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_anonymous_class: /* anonymous_class  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_expr: /* new_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_expr: /* expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_inline_function: /* inline_function  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_backup_doc_comment: /* backup_doc_comment  */
            { if (((*yyvaluep).str)) zend_string_release_ex(((*yyvaluep).str), 0); }
        break;

    case YYSYMBOL_lexical_vars: /* lexical_vars  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_lexical_var_list: /* lexical_var_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_lexical_var: /* lexical_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_function_call: /* function_call  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name: /* class_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_name_reference: /* class_name_reference  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_exit_expr: /* exit_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_backticks_expr: /* backticks_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_ctor_arguments: /* ctor_arguments  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_dereferenceable_scalar: /* dereferenceable_scalar  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_scalar: /* scalar  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_constant: /* constant  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_class_constant: /* class_constant  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_expr: /* optional_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_variable_class_name: /* variable_class_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_fully_dereferenceable: /* fully_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_object_dereferenceable: /* array_object_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_callable_expr: /* callable_expr  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_callable_variable: /* callable_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_variable: /* variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_simple_variable: /* simple_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_static_member: /* static_member  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_variable: /* new_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_member_name: /* member_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_name: /* property_name  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_pair_list: /* array_pair_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_possible_array_pair: /* possible_array_pair  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_array_pair_list: /* non_empty_array_pair_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_array_pair: /* array_pair  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_list: /* encaps_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_var: /* encaps_var  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_encaps_var_offset: /* encaps_var_offset  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_internal_functions_in_yacc: /* internal_functions_in_yacc  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_isset_variables: /* isset_variables  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_isset_variable: /* isset_variable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ZENDEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ZENDEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= END)
    {
      yychar = END;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ZENDerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ZENDUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = ZENDEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: top_statement_list  */
                                { CG(ast) = (yyvsp[0].ast); }
    break;

  case 83: /* identifier: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 84: /* identifier: semi_reserved  */
                               {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 85: /* top_statement_list: top_statement_list top_statement  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 86: /* top_statement_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 87: /* namespace_declaration_name: identifier  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 88: /* namespace_declaration_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 89: /* namespace_name: "identifier"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 90: /* namespace_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 91: /* legacy_namespace_name: namespace_name  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 92: /* legacy_namespace_name: "fully qualified name"  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 93: /* name: "identifier"  */
                                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 94: /* name: "namespaced name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 95: /* name: "fully qualified name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }
    break;

  case 96: /* name: "namespace-relative name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }
    break;

  case 97: /* attribute_decl: class_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[0].ast), NULL); }
    break;

  case 98: /* attribute_decl: class_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 99: /* attribute_group: attribute_decl  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_GROUP, (yyvsp[0].ast)); }
    break;

  case 100: /* attribute_group: attribute_group ',' attribute_decl  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 101: /* attribute: "'#['" attribute_group possible_comma ']'  */
                                                                { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 102: /* attributes: attribute  */
                                                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_LIST, (yyvsp[0].ast)); }
    break;

  case 103: /* attributes: attributes attribute  */
                                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 104: /* attributed_statement: function_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 105: /* attributed_statement: class_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 106: /* attributed_statement: trait_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 107: /* attributed_statement: interface_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 108: /* attributed_statement: enum_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 109: /* top_statement: statement  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 110: /* top_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 111: /* top_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 112: /* top_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }
    break;

  case 113: /* top_statement: "'namespace'" namespace_declaration_name ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }
    break;

  case 114: /* $@1: %empty  */
                                                       { RESET_DOC_COMMENT(); }
    break;

  case 115: /* top_statement: "'namespace'" namespace_declaration_name $@1 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 116: /* $@2: %empty  */
                            { RESET_DOC_COMMENT(); }
    break;

  case 117: /* top_statement: "'namespace'" $@2 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }
    break;

  case 118: /* top_statement: "'use'" mixed_group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 119: /* top_statement: "'use'" use_type group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 120: /* top_statement: "'use'" use_declarations ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 121: /* top_statement: "'use'" use_type use_declarations ';'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 122: /* top_statement: "'const'" const_list ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 123: /* use_type: "'function'"  */
                                        { (yyval.num) = ZEND_SYMBOL_FUNCTION; }
    break;

  case 124: /* use_type: "'const'"  */
                                        { (yyval.num) = ZEND_SYMBOL_CONST; }
    break;

  case 125: /* group_use_declaration: legacy_namespace_name "'\\'" '{' unprefixed_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 126: /* mixed_group_use_declaration: legacy_namespace_name "'\\'" '{' inline_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}
    break;

  case 129: /* inline_use_declarations: inline_use_declarations ',' inline_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 130: /* inline_use_declarations: inline_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 131: /* unprefixed_use_declarations: unprefixed_use_declarations ',' unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 132: /* unprefixed_use_declarations: unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 133: /* use_declarations: use_declarations ',' use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 134: /* use_declarations: use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 135: /* inline_use_declaration: unprefixed_use_declaration  */
                                           { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 136: /* inline_use_declaration: use_type unprefixed_use_declaration  */
                                                    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }
    break;

  case 137: /* unprefixed_use_declaration: namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 138: /* unprefixed_use_declaration: namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 139: /* use_declaration: legacy_namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 140: /* use_declaration: legacy_namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 141: /* const_list: const_list ',' const_decl  */
                                          { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 142: /* const_list: const_decl  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 143: /* inner_statement_list: inner_statement_list inner_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 144: /* inner_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 145: /* inner_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 146: /* inner_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 147: /* inner_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 148: /* inner_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }
    break;

  case 149: /* statement: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 150: /* statement: if_stmt  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 151: /* statement: alt_if_stmt  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 152: /* statement: "'while'" '(' expr ')' while_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 153: /* statement: "'do'" statement "'while'" '(' expr ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 154: /* statement: "'for'" '(' for_exprs ';' for_exprs ';' for_exprs ')' for_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 155: /* statement: "'switch'" '(' expr ')' switch_case_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 156: /* statement: "'break'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }
    break;

  case 157: /* statement: "'continue'" optional_expr ';'  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }
    break;

  case 158: /* statement: "'return'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }
    break;

  case 159: /* statement: "'global'" global_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 160: /* statement: "'static'" static_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 161: /* statement: "'echo'" echo_expr_list ';'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 162: /* statement: T_INLINE_HTML  */
                              { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 163: /* statement: expr ';'  */
                         { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 164: /* statement: "'unset'" '(' unset_variables possible_comma ')' ';'  */
                                                                   { (yyval.ast) = (yyvsp[-3].ast); }
    break;

  case 165: /* statement: "'foreach'" '(' expr "'as'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 166: /* statement: "'foreach'" '(' expr "'as'" foreach_variable "'=>'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }
    break;

  case 167: /* $@3: %empty  */
                        { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }
    break;

  case 168: /* statement: "'declare'" '(' const_list ')' $@3 declare_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 169: /* statement: ';'  */
                                              { (yyval.ast) = NULL; }
    break;

  case 170: /* statement: "'try'" '{' inner_statement_list '}' catch_list finally_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 171: /* statement: "'goto'" "identifier" ';'  */
                                    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }
    break;

  case 172: /* statement: "identifier" ':'  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }
    break;

  case 173: /* catch_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }
    break;

  case 174: /* catch_list: catch_list "'catch'" '(' catch_name_list optional_variable ')' '{' inner_statement_list '}'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }
    break;

  case 175: /* catch_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 176: /* catch_name_list: catch_name_list '|' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 177: /* optional_variable: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 178: /* optional_variable: "variable"  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 179: /* finally_statement: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 180: /* finally_statement: "'finally'" '{' inner_statement_list '}'  */
                                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 181: /* unset_variables: unset_variable  */
                               { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 182: /* unset_variables: unset_variables ',' unset_variable  */
                                                   { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 183: /* unset_variable: variable  */
                         { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }
    break;

  case 184: /* function_declaration_statement: function returns_ref "identifier" backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 185: /* is_reference: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 186: /* is_reference: "'&'"  */
                                                        { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 187: /* is_variadic: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 188: /* is_variadic: "'...'"  */
                            { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 189: /* @4: %empty  */
                                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 190: /* class_declaration_statement: class_modifiers "'class'" @4 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 191: /* @5: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 192: /* class_declaration_statement: "'class'" @5 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 193: /* class_modifiers: class_modifier  */
                                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 194: /* class_modifiers: class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 195: /* class_modifier: "'abstract'"  */
                                        { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 196: /* class_modifier: "'final'"  */
                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 197: /* @6: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 198: /* trait_declaration_statement: "'trait'" @6 "identifier" backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 199: /* @7: %empty  */
                            { (yyval.num) = CG(zend_lineno); }
    break;

  case 200: /* interface_declaration_statement: "'interface'" @7 "identifier" interface_extends_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 201: /* @8: %empty  */
                       { (yyval.num) = CG(zend_lineno); }
    break;

  case 202: /* enum_declaration_statement: "'enum'" @8 "identifier" enum_backing_type implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }
    break;

  case 203: /* enum_backing_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 204: /* enum_backing_type: ':' type_expr  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 205: /* enum_case: "'case'" backup_doc_comment identifier enum_case_expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }
    break;

  case 206: /* enum_case_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 207: /* enum_case_expr: '=' expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 208: /* extends_from: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 209: /* extends_from: "'extends'" class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 210: /* interface_extends_list: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 211: /* interface_extends_list: "'extends'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 212: /* implements_list: %empty  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 213: /* implements_list: "'implements'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 214: /* foreach_variable: variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 215: /* foreach_variable: ampersand variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }
    break;

  case 216: /* foreach_variable: "'list'" '(' array_pair_list ')'  */
                                               { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 217: /* foreach_variable: '[' array_pair_list ']'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 218: /* for_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 219: /* for_statement: ':' inner_statement_list "'endfor'" ';'  */
                                                      { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 220: /* foreach_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 221: /* foreach_statement: ':' inner_statement_list "'endforeach'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 222: /* declare_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 223: /* declare_statement: ':' inner_statement_list "'enddeclare'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 224: /* switch_case_list: '{' case_list '}'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 225: /* switch_case_list: '{' ';' case_list '}'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 226: /* switch_case_list: ':' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 227: /* switch_case_list: ':' ';' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 228: /* case_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 229: /* case_list: case_list "'case'" expr case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 230: /* case_list: case_list "'default'" case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }
    break;

  case 233: /* match: "'match'" '(' expr ')' '{' match_arm_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 234: /* match_arm_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }
    break;

  case 235: /* match_arm_list: non_empty_match_arm_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 236: /* non_empty_match_arm_list: match_arm  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }
    break;

  case 237: /* non_empty_match_arm_list: non_empty_match_arm_list ',' match_arm  */
                                                       { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 238: /* match_arm: match_arm_cond_list possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 239: /* match_arm: "'default'" possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }
    break;

  case 240: /* match_arm_cond_list: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 241: /* match_arm_cond_list: match_arm_cond_list ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 242: /* while_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 243: /* while_statement: ':' inner_statement_list "'endwhile'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 244: /* if_stmt_without_else: "'if'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 245: /* if_stmt_without_else: if_stmt_without_else "'elseif'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 246: /* if_stmt: if_stmt_without_else  */
                                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 247: /* if_stmt: if_stmt_without_else "'else'" statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }
    break;

  case 248: /* alt_if_stmt_without_else: "'if'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 249: /* alt_if_stmt_without_else: alt_if_stmt_without_else "'elseif'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 250: /* alt_if_stmt: alt_if_stmt_without_else "'endif'" ';'  */
                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 251: /* alt_if_stmt: alt_if_stmt_without_else "'else'" ':' inner_statement_list "'endif'" ';'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }
    break;

  case 252: /* parameter_list: non_empty_parameter_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 253: /* parameter_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 254: /* non_empty_parameter_list: attributed_parameter  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }
    break;

  case 255: /* non_empty_parameter_list: non_empty_parameter_list ',' attributed_parameter  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 256: /* attributed_parameter: attributes parameter  */
                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 257: /* attributed_parameter: parameter  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 258: /* optional_property_modifiers: %empty  */
                                                        { (yyval.num) = 0; }
    break;

  case 259: /* optional_property_modifiers: optional_property_modifiers property_modifier  */
                        { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 260: /* property_modifier: "'public'"  */
                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 261: /* property_modifier: "'protected'"  */
                                                        { (yyval.num) = ZEND_ACC_PROTECTED; }
    break;

  case 262: /* property_modifier: "'private'"  */
                                                        { (yyval.num) = ZEND_ACC_PRIVATE; }
    break;

  case 263: /* property_modifier: "'readonly'"  */
                                                        { (yyval.num) = ZEND_ACC_READONLY; }
    break;

  case 264: /* parameter: optional_property_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-5].num) | (yyvsp[-3].num) | (yyvsp[-2].num), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL,
					NULL, (yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL); }
    break;

  case 265: /* parameter: optional_property_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment '=' expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-7].num) | (yyvsp[-5].num) | (yyvsp[-4].num), (yyvsp[-6].ast), (yyvsp[-3].ast), (yyvsp[0].ast),
					NULL, (yyvsp[-2].str) ? zend_ast_create_zval_from_str((yyvsp[-2].str)) : NULL); }
    break;

  case 266: /* optional_type_without_static: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 267: /* optional_type_without_static: type_expr_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 268: /* type_expr: type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 269: /* type_expr: '?' type  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 270: /* type_expr: union_type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 271: /* type_expr: intersection_type  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 272: /* type: type_without_static  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 273: /* type: "'static'"  */
                                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }
    break;

  case 274: /* union_type: type '|' type  */
                                    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 275: /* union_type: union_type '|' type  */
                                    { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 276: /* intersection_type: type "amp" type  */
                                                                          { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 277: /* intersection_type: intersection_type "amp" type  */
                                                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 278: /* type_expr_without_static: type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 279: /* type_expr_without_static: '?' type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 280: /* type_expr_without_static: union_type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 281: /* type_expr_without_static: intersection_type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 282: /* type_without_static: "'array'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 283: /* type_without_static: "'callable'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 284: /* type_without_static: name  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 285: /* union_type_without_static: type_without_static '|' type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 286: /* union_type_without_static: union_type_without_static '|' type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 287: /* intersection_type_without_static: type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 288: /* intersection_type_without_static: intersection_type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 289: /* return_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 290: /* return_type: ':' type_expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 291: /* argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 292: /* argument_list: '(' non_empty_argument_list possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 293: /* argument_list: '(' "'...'" ')'  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_CALLABLE_CONVERT); }
    break;

  case 294: /* non_empty_argument_list: argument  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 295: /* non_empty_argument_list: non_empty_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 296: /* argument: expr  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 297: /* argument: identifier ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 298: /* argument: "'...'" expr  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 299: /* global_var_list: global_var_list ',' global_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 300: /* global_var_list: global_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 301: /* global_var: simple_variable  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }
    break;

  case 302: /* static_var_list: static_var_list ',' static_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 303: /* static_var_list: static_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 304: /* static_var: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }
    break;

  case 305: /* static_var: "variable" '=' expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 306: /* class_statement_list: class_statement_list class_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 307: /* class_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 308: /* attributed_class_statement: variable_modifiers optional_type_without_static property_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 309: /* attributed_class_statement: method_modifiers "'const'" class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 310: /* attributed_class_statement: method_modifiers function returns_ref identifier backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags method_body backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 311: /* attributed_class_statement: enum_case  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 312: /* class_statement: attributed_class_statement  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 313: /* class_statement: attributes attributed_class_statement  */
                                                      { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 314: /* class_statement: "'use'" class_name_list trait_adaptations  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 315: /* class_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 316: /* class_name_list: class_name_list ',' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 317: /* trait_adaptations: ';'  */
                                                                                { (yyval.ast) = NULL; }
    break;

  case 318: /* trait_adaptations: '{' '}'  */
                                                                        { (yyval.ast) = NULL; }
    break;

  case 319: /* trait_adaptations: '{' trait_adaptation_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 320: /* trait_adaptation_list: trait_adaptation  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }
    break;

  case 321: /* trait_adaptation_list: trait_adaptation_list trait_adaptation  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 322: /* trait_adaptation: trait_precedence ';'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 323: /* trait_adaptation: trait_alias ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 324: /* trait_precedence: absolute_trait_method_reference "'insteadof'" class_name_list  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 325: /* trait_alias: trait_method_reference "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 326: /* trait_alias: trait_method_reference "'as'" reserved_non_modifiers  */
                        { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }
    break;

  case 327: /* trait_alias: trait_method_reference "'as'" member_modifier identifier  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[-1].num), (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 328: /* trait_alias: trait_method_reference "'as'" member_modifier  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[0].num), (yyvsp[-2].ast), NULL); }
    break;

  case 329: /* trait_method_reference: identifier  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }
    break;

  case 330: /* trait_method_reference: absolute_trait_method_reference  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 331: /* absolute_trait_method_reference: class_name "'::'" identifier  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 332: /* method_body: ';'  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 333: /* method_body: '{' inner_statement_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 334: /* variable_modifiers: non_empty_member_modifiers  */
                                                        { (yyval.num) = (yyvsp[0].num); }
    break;

  case 335: /* variable_modifiers: "'var'"  */
                                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 336: /* method_modifiers: %empty  */
                                                                { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 337: /* method_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = (yyvsp[0].num); if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 338: /* non_empty_member_modifiers: member_modifier  */
                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 339: /* non_empty_member_modifiers: non_empty_member_modifiers member_modifier  */
                        { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 340: /* member_modifier: "'public'"  */
                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 341: /* member_modifier: "'protected'"  */
                                                        { (yyval.num) = ZEND_ACC_PROTECTED; }
    break;

  case 342: /* member_modifier: "'private'"  */
                                                        { (yyval.num) = ZEND_ACC_PRIVATE; }
    break;

  case 343: /* member_modifier: "'static'"  */
                                                        { (yyval.num) = ZEND_ACC_STATIC; }
    break;

  case 344: /* member_modifier: "'abstract'"  */
                                                        { (yyval.num) = ZEND_ACC_ABSTRACT; }
    break;

  case 345: /* member_modifier: "'final'"  */
                                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 346: /* member_modifier: "'readonly'"  */
                                                        { (yyval.num) = ZEND_ACC_READONLY; }
    break;

  case 347: /* property_list: property_list ',' property  */
                                           { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 348: /* property_list: property  */
                         { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }
    break;

  case 349: /* property: "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 350: /* property: "variable" '=' expr backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 351: /* class_const_list: class_const_list ',' class_const_decl  */
                                                      { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 352: /* class_const_list: class_const_decl  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 353: /* class_const_decl: identifier '=' expr backup_doc_comment  */
                                               { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 354: /* const_decl: "identifier" '=' expr backup_doc_comment  */
                                             { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 355: /* echo_expr_list: echo_expr_list ',' echo_expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 356: /* echo_expr_list: echo_expr  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 357: /* echo_expr: expr  */
             { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 358: /* for_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 359: /* for_exprs: non_empty_for_exprs  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 360: /* non_empty_for_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 361: /* non_empty_for_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 362: /* @9: %empty  */
                { (yyval.num) = CG(zend_lineno); }
    break;

  case 363: /* anonymous_class: "'class'" @9 ctor_arguments extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                                                                                             {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS, (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}
    break;

  case 364: /* new_expr: "'new'" class_name_reference ctor_arguments  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 365: /* new_expr: "'new'" anonymous_class  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 366: /* new_expr: "'new'" attributes anonymous_class  */
                        { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 367: /* expr: variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 368: /* expr: "'list'" '(' array_pair_list ')' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 369: /* expr: '[' array_pair_list ']' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 370: /* expr: variable '=' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 371: /* expr: variable '=' ampersand variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 372: /* expr: "'clone'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }
    break;

  case 373: /* expr: variable "'+='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 374: /* expr: variable "'-='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 375: /* expr: variable "'*='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 376: /* expr: variable "'**='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 377: /* expr: variable "'/='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 378: /* expr: variable "'.='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 379: /* expr: variable "'%='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 380: /* expr: variable "'&='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 381: /* expr: variable "'|='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 382: /* expr: variable "'^='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 383: /* expr: variable "'<<='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 384: /* expr: variable "'>>='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 385: /* expr: variable "'??='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 386: /* expr: variable "'++'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }
    break;

  case 387: /* expr: "'++'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }
    break;

  case 388: /* expr: variable "'--'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }
    break;

  case 389: /* expr: "'--'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }
    break;

  case 390: /* expr: expr "'||'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 391: /* expr: expr "'&&'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 392: /* expr: expr "'or'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 393: /* expr: expr "'and'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 394: /* expr: expr "'xor'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 395: /* expr: expr '|' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 396: /* expr: expr "amp" expr  */
                                                                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 397: /* expr: expr "'&'" expr  */
                                                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 398: /* expr: expr '^' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 399: /* expr: expr '.' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 400: /* expr: expr '+' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 401: /* expr: expr '-' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 402: /* expr: expr '*' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 403: /* expr: expr "'**'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 404: /* expr: expr '/' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 405: /* expr: expr '%' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 406: /* expr: expr "'<<'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 407: /* expr: expr "'>>'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 408: /* expr: '+' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }
    break;

  case 409: /* expr: '-' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }
    break;

  case 410: /* expr: '!' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }
    break;

  case 411: /* expr: '~' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }
    break;

  case 412: /* expr: expr "'==='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 413: /* expr: expr "'!=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 414: /* expr: expr "'=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 415: /* expr: expr "'!='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 416: /* expr: expr '<' expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 417: /* expr: expr "'<='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 418: /* expr: expr '>' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 419: /* expr: expr "'>='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 420: /* expr: expr "'<=>'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 421: /* expr: expr "'instanceof'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 422: /* expr: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
		}
    break;

  case 423: /* expr: new_expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 424: /* expr: expr '?' expr ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 425: /* expr: expr '?' ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 426: /* expr: expr "'??'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 427: /* expr: internal_functions_in_yacc  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 428: /* expr: "'(int)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }
    break;

  case 429: /* expr: "'(double)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }
    break;

  case 430: /* expr: "'(string)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }
    break;

  case 431: /* expr: "'(array)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }
    break;

  case 432: /* expr: "'(object)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }
    break;

  case 433: /* expr: "'(bool)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }
    break;

  case 434: /* expr: "'(unset)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }
    break;

  case 435: /* expr: "'exit'" exit_expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[0].ast)); }
    break;

  case 436: /* expr: '@' expr  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }
    break;

  case 437: /* expr: scalar  */
                       { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 438: /* expr: '`' backticks_expr '`'  */
                                       { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }
    break;

  case 439: /* expr: "'print'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }
    break;

  case 440: /* expr: "'yield'"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 441: /* expr: "'yield'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 442: /* expr: "'yield'" expr "'=>'" expr  */
                                                 { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 443: /* expr: "'yield from'" expr  */
                                  { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 444: /* expr: "'throw'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }
    break;

  case 445: /* expr: inline_function  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 446: /* expr: attributes inline_function  */
                                           { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 447: /* expr: "'static'" inline_function  */
                                         { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 448: /* expr: attributes "'static'" inline_function  */
                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 449: /* expr: match  */
                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 450: /* inline_function: function returns_ref backup_doc_comment '(' parameter_list ')' lexical_vars return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 451: /* inline_function: fn returns_ref backup_doc_comment '(' parameter_list ')' return_type "'=>'" backup_fn_flags backup_lex_pos expr backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0), (yyvsp[-7].ast), NULL,
				  zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)), (yyvsp[-5].ast), NULL);
				  ((zend_ast_decl *) (yyval.ast))->lex_pos = (yyvsp[-2].ptr);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }
    break;

  case 452: /* fn: "'fn'"  */
             { (yyval.num) = CG(zend_lineno); }
    break;

  case 453: /* function: "'function'"  */
                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 454: /* backup_doc_comment: %empty  */
               { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 455: /* backup_fn_flags: %empty  */
                                         { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 456: /* backup_lex_pos: %empty  */
               { (yyval.ptr) = LANG_SCNG(yy_text); }
    break;

  case 457: /* returns_ref: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 458: /* returns_ref: ampersand  */
                                { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 459: /* lexical_vars: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 460: /* lexical_vars: "'use'" '(' lexical_var_list possible_comma ')'  */
                                                              { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 461: /* lexical_var_list: lexical_var_list ',' lexical_var  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 462: /* lexical_var_list: lexical_var  */
                            { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }
    break;

  case 463: /* lexical_var: "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 464: /* lexical_var: ampersand "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }
    break;

  case 465: /* function_call: name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 466: /* function_call: class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 467: /* function_call: variable_class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 468: /* function_call: callable_expr argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 469: /* class_name: "'static'"  */
                        { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 470: /* class_name: name  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 471: /* class_name_reference: class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 472: /* class_name_reference: new_variable  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 473: /* class_name_reference: '(' expr ')'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 474: /* exit_expr: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 475: /* exit_expr: '(' optional_expr ')'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 476: /* backticks_expr: %empty  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 477: /* backticks_expr: "string content"  */
                                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 478: /* backticks_expr: encaps_list  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 479: /* ctor_arguments: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 480: /* ctor_arguments: argument_list  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 481: /* dereferenceable_scalar: "'array'" '(' array_pair_list ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 482: /* dereferenceable_scalar: '[' array_pair_list ']'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 483: /* dereferenceable_scalar: "quoted string"  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 484: /* dereferenceable_scalar: '"' encaps_list '"'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 485: /* scalar: "integer"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 486: /* scalar: "floating-point number"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 487: /* scalar: "heredoc start" "string content" "heredoc end"  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 488: /* scalar: "heredoc start" "heredoc end"  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 489: /* scalar: "heredoc start" encaps_list "heredoc end"  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 490: /* scalar: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 491: /* scalar: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 492: /* scalar: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 493: /* constant: name  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }
    break;

  case 494: /* constant: "'__LINE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 495: /* constant: "'__FILE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 496: /* constant: "'__DIR__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 497: /* constant: "'__TRAIT__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 498: /* constant: "'__METHOD__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 499: /* constant: "'__FUNCTION__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 500: /* constant: "'__NAMESPACE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 501: /* constant: "'__CLASS__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 502: /* class_constant: class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 503: /* class_constant: variable_class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 504: /* optional_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 505: /* optional_expr: expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 506: /* variable_class_name: fully_dereferenceable  */
                                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 507: /* fully_dereferenceable: variable  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 508: /* fully_dereferenceable: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 509: /* fully_dereferenceable: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 510: /* fully_dereferenceable: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 511: /* array_object_dereferenceable: fully_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 512: /* array_object_dereferenceable: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 513: /* callable_expr: callable_variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 514: /* callable_expr: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 515: /* callable_expr: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 516: /* callable_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 517: /* callable_variable: array_object_dereferenceable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 518: /* callable_variable: array_object_dereferenceable '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 519: /* callable_variable: array_object_dereferenceable "'->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 520: /* callable_variable: array_object_dereferenceable "'?->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 521: /* callable_variable: function_call  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 522: /* variable: callable_variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 523: /* variable: static_member  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 524: /* variable: array_object_dereferenceable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 525: /* variable: array_object_dereferenceable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 526: /* simple_variable: "variable"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 527: /* simple_variable: '$' '{' expr '}'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 528: /* simple_variable: '$' simple_variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 529: /* static_member: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 530: /* static_member: variable_class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 531: /* new_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 532: /* new_variable: new_variable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 533: /* new_variable: new_variable '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 534: /* new_variable: new_variable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 535: /* new_variable: new_variable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 536: /* new_variable: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 537: /* new_variable: new_variable "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 538: /* member_name: identifier  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 539: /* member_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 540: /* member_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 541: /* property_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 542: /* property_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 543: /* property_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 544: /* array_pair_list: non_empty_array_pair_list  */
                        { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }
    break;

  case 545: /* possible_array_pair: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 546: /* possible_array_pair: array_pair  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 547: /* non_empty_array_pair_list: non_empty_array_pair_list ',' possible_array_pair  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 548: /* non_empty_array_pair_list: possible_array_pair  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }
    break;

  case 549: /* array_pair: expr "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }
    break;

  case 550: /* array_pair: expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 551: /* array_pair: expr "'=>'" ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }
    break;

  case 552: /* array_pair: ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }
    break;

  case 553: /* array_pair: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 554: /* array_pair: expr "'=>'" "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }
    break;

  case 555: /* array_pair: "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }
    break;

  case 556: /* encaps_list: encaps_list encaps_var  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 557: /* encaps_list: encaps_list "string content"  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 558: /* encaps_list: encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }
    break;

  case 559: /* encaps_list: "string content" encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 560: /* encaps_var: "variable"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 561: /* encaps_var: "variable" '[' encaps_var_offset ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }
    break;

  case 562: /* encaps_var: "variable" "'->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 563: /* encaps_var: "variable" "'?->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 564: /* encaps_var: "'${'" expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }
    break;

  case 565: /* encaps_var: "'${'" "variable name" '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[-1].ast)); }
    break;

  case 566: /* encaps_var: "'${'" "variable name" '[' expr ']' '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }
    break;

  case 567: /* encaps_var: "'{$'" variable '}'  */
                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 568: /* encaps_var_offset: "identifier"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 569: /* encaps_var_offset: "number"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 570: /* encaps_var_offset: '-' "number"  */
                                        { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }
    break;

  case 571: /* encaps_var_offset: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 572: /* internal_functions_in_yacc: "'isset'" '(' isset_variables possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 573: /* internal_functions_in_yacc: "'empty'" '(' expr ')'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }
    break;

  case 574: /* internal_functions_in_yacc: "'include'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }
    break;

  case 575: /* internal_functions_in_yacc: "'include_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }
    break;

  case 576: /* internal_functions_in_yacc: "'eval'" '(' expr ')'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }
    break;

  case 577: /* internal_functions_in_yacc: "'require'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }
    break;

  case 578: /* internal_functions_in_yacc: "'require_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }
    break;

  case 579: /* isset_variables: isset_variable  */
                               { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 580: /* isset_variables: isset_variables ',' isset_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 581: /* isset_variable: expr  */
                     { (yyval.ast) = zend_ast_create(ZEND_AST_ISSET, (yyvsp[0].ast)); }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ZENDEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END)
        {
          /* Return failure if at end of input.  */
          if (yychar == END)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = ZENDEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ZENDEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}



/* Over-ride Bison formatting routine to give better token descriptions.
   Copy to YYRES the contents of YYSTR for use in yyerror.
   YYSTR is taken from yytname, from the %token declaration.
   If YYRES is null, do not copy; instead, return the length of what
   the result would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	const char *toktype = yystr;
	size_t toktype_len = strlen(toktype);

	/* CG(parse_error) states:
	 * 0 => yyres = NULL, yystr is the unexpected token
	 * 1 => yyres = NULL, yystr is one of the expected tokens
	 * 2 => yyres != NULL, yystr is the unexpected token
	 * 3 => yyres != NULL, yystr is one of the expected tokens
	 */
	if (yyres && CG(parse_error) < 2) {
		CG(parse_error) = 2;
	}

	if (CG(parse_error) % 2 == 0) {
		/* The unexpected token */
		char buffer[120];
		const unsigned char *tokcontent, *tokcontent_end;
		size_t tokcontent_len;

		CG(parse_error)++;

		if (LANG_SCNG(yy_text)[0] == 0 &&
			LANG_SCNG(yy_leng) == 1 &&
			strcmp(toktype, "\"end of file\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "end of file");
			}
			return sizeof("end of file")-1;
		}

		/* Prevent the backslash getting doubled in the output (eugh) */
		if (strcmp(toktype, "\"'\\\\'\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"\\\"");
			}
			return sizeof("token \"\\\"")-1;
		}

		/* We used "amp" as a dummy label to avoid a duplicate token literal warning. */
		if (strcmp(toktype, "\"amp\"") == 0) {
			if (yyres) {
				yystpcpy(yyres, "token \"&\"");
			}
			return sizeof("token \"&\"")-1;
		}

		/* Avoid unreadable """ */
		/* "'" would theoretically be just as bad, but is never currently parsed as a separate token */
		if (strcmp(toktype, "'\"'") == 0) {
			if (yyres) {
				yystpcpy(yyres, "double-quote mark");
			}
			return sizeof("double-quote mark")-1;
		}

		/* Strip off the outer quote marks */
		if (toktype_len >= 2 && *toktype == '"') {
			toktype++;
			toktype_len -= 2;
		}

		/* If the token always has one form, the %token line should have a single-quoted name */
		/* The parser rules also include single-character un-named tokens which will be single-quoted here */
		/* We re-format this with double quotes here to ensure everything's consistent */
		if (toktype_len > 0 && *toktype == '\'') {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "token \"%.*s\"", (int)toktype_len-2, toktype+1);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + sizeof("token ")-1;
		}

		/* Fetch the content of the last seen token from global lexer state */
		tokcontent = LANG_SCNG(yy_text);
		tokcontent_len = LANG_SCNG(yy_leng);

		/* For T_BAD_CHARACTER, the content probably won't be a printable char */
		/* Also, "unexpected invalid character" sounds a bit redundant */
		if (tokcontent_len == 1 && strcmp(yystr, "\"invalid character\"") == 0) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "character 0x%02hhX", *tokcontent);
				yystpcpy(yyres, buffer);
			}
			return sizeof("character 0x00")-1;
		}

		/* Truncate at line end to avoid messing up log formats */
		tokcontent_end = memchr(tokcontent, '\n', tokcontent_len);
		if (tokcontent_end != NULL) {
			tokcontent_len = (tokcontent_end - tokcontent);
		}

		/* Try to be helpful about what kind of string was found, before stripping the quotes */
		if (tokcontent_len > 0 && strcmp(yystr, "\"quoted string\"") == 0) {
			if (*tokcontent == '"') {
				toktype = "double-quoted string";
				toktype_len = sizeof("double-quoted string")-1;
			}
			else if (*tokcontent == '\'') {
				toktype = "single-quoted string";
				toktype_len = sizeof("single-quoted string")-1;
			}
		}

		/* For quoted strings, strip off another layer of quotes to avoid putting quotes inside quotes */
		if (tokcontent_len > 0 && (*tokcontent == '\'' || *tokcontent=='"'))  {
			tokcontent++;
			tokcontent_len--;
		}
		if (tokcontent_len > 0 && (tokcontent[tokcontent_len-1] == '\'' || tokcontent[tokcontent_len-1] == '"'))  {
			tokcontent_len--;
		}

		/* Truncate to 30 characters and add a ... */
		if (tokcontent_len > 30 + sizeof("...")-1) {
			if (yyres) {
				snprintf(buffer, sizeof(buffer), "%.*s \"%.*s...\"", (int)toktype_len, toktype, 30, tokcontent);
				yystpcpy(yyres, buffer);
			}
			return toktype_len + 30 + sizeof(" \"...\"")-1;
		}

		if (yyres) {
			snprintf(buffer, sizeof(buffer), "%.*s \"%.*s\"", (int)toktype_len, toktype, (int)tokcontent_len, tokcontent);
			yystpcpy(yyres, buffer);
		}
		return toktype_len + tokcontent_len + sizeof(" \"\"")-1;
	}

	/* One of the expected tokens */

	/* Prevent the backslash getting doubled in the output (eugh) */
	if (strcmp(toktype, "\"'\\\\'\"") == 0) {
		if (yyres) {
			yystpcpy(yyres, "\"\\\"");
		}
		return sizeof("\"\\\"")-1;
	}

	/* Strip off the outer quote marks */
	if (toktype_len >= 2 && *toktype == '"') {
		toktype++;
		toktype_len -= 2;
	}

	if (yyres) {
		YYSIZE_T yyn = 0;

		for (; yyn < toktype_len; ++yyn) {
			/* Replace single quotes with double for consistency */
			if (toktype[yyn] == '\'') {
				yyres[yyn] = '"';
			}
			else {
				yyres[yyn] = toktype[yyn];
			}
		}
		yyres[toktype_len] = '\0';
	}

	return toktype_len;
}
