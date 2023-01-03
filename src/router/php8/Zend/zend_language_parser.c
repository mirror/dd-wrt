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
  YYSYMBOL_function_name = 219,            /* function_name  */
  YYSYMBOL_function_declaration_statement = 220, /* function_declaration_statement  */
  YYSYMBOL_is_reference = 221,             /* is_reference  */
  YYSYMBOL_is_variadic = 222,              /* is_variadic  */
  YYSYMBOL_class_declaration_statement = 223, /* class_declaration_statement  */
  YYSYMBOL_224_4 = 224,                    /* @4  */
  YYSYMBOL_225_5 = 225,                    /* @5  */
  YYSYMBOL_class_modifiers = 226,          /* class_modifiers  */
  YYSYMBOL_class_modifier = 227,           /* class_modifier  */
  YYSYMBOL_trait_declaration_statement = 228, /* trait_declaration_statement  */
  YYSYMBOL_229_6 = 229,                    /* @6  */
  YYSYMBOL_interface_declaration_statement = 230, /* interface_declaration_statement  */
  YYSYMBOL_231_7 = 231,                    /* @7  */
  YYSYMBOL_enum_declaration_statement = 232, /* enum_declaration_statement  */
  YYSYMBOL_233_8 = 233,                    /* @8  */
  YYSYMBOL_enum_backing_type = 234,        /* enum_backing_type  */
  YYSYMBOL_enum_case = 235,                /* enum_case  */
  YYSYMBOL_enum_case_expr = 236,           /* enum_case_expr  */
  YYSYMBOL_extends_from = 237,             /* extends_from  */
  YYSYMBOL_interface_extends_list = 238,   /* interface_extends_list  */
  YYSYMBOL_implements_list = 239,          /* implements_list  */
  YYSYMBOL_foreach_variable = 240,         /* foreach_variable  */
  YYSYMBOL_for_statement = 241,            /* for_statement  */
  YYSYMBOL_foreach_statement = 242,        /* foreach_statement  */
  YYSYMBOL_declare_statement = 243,        /* declare_statement  */
  YYSYMBOL_switch_case_list = 244,         /* switch_case_list  */
  YYSYMBOL_case_list = 245,                /* case_list  */
  YYSYMBOL_case_separator = 246,           /* case_separator  */
  YYSYMBOL_match = 247,                    /* match  */
  YYSYMBOL_match_arm_list = 248,           /* match_arm_list  */
  YYSYMBOL_non_empty_match_arm_list = 249, /* non_empty_match_arm_list  */
  YYSYMBOL_match_arm = 250,                /* match_arm  */
  YYSYMBOL_match_arm_cond_list = 251,      /* match_arm_cond_list  */
  YYSYMBOL_while_statement = 252,          /* while_statement  */
  YYSYMBOL_if_stmt_without_else = 253,     /* if_stmt_without_else  */
  YYSYMBOL_if_stmt = 254,                  /* if_stmt  */
  YYSYMBOL_alt_if_stmt_without_else = 255, /* alt_if_stmt_without_else  */
  YYSYMBOL_alt_if_stmt = 256,              /* alt_if_stmt  */
  YYSYMBOL_parameter_list = 257,           /* parameter_list  */
  YYSYMBOL_non_empty_parameter_list = 258, /* non_empty_parameter_list  */
  YYSYMBOL_attributed_parameter = 259,     /* attributed_parameter  */
  YYSYMBOL_optional_property_modifiers = 260, /* optional_property_modifiers  */
  YYSYMBOL_property_modifier = 261,        /* property_modifier  */
  YYSYMBOL_parameter = 262,                /* parameter  */
  YYSYMBOL_optional_type_without_static = 263, /* optional_type_without_static  */
  YYSYMBOL_type_expr = 264,                /* type_expr  */
  YYSYMBOL_type = 265,                     /* type  */
  YYSYMBOL_union_type_element = 266,       /* union_type_element  */
  YYSYMBOL_union_type = 267,               /* union_type  */
  YYSYMBOL_intersection_type = 268,        /* intersection_type  */
  YYSYMBOL_type_expr_without_static = 269, /* type_expr_without_static  */
  YYSYMBOL_type_without_static = 270,      /* type_without_static  */
  YYSYMBOL_union_type_without_static_element = 271, /* union_type_without_static_element  */
  YYSYMBOL_union_type_without_static = 272, /* union_type_without_static  */
  YYSYMBOL_intersection_type_without_static = 273, /* intersection_type_without_static  */
  YYSYMBOL_return_type = 274,              /* return_type  */
  YYSYMBOL_argument_list = 275,            /* argument_list  */
  YYSYMBOL_non_empty_argument_list = 276,  /* non_empty_argument_list  */
  YYSYMBOL_argument = 277,                 /* argument  */
  YYSYMBOL_global_var_list = 278,          /* global_var_list  */
  YYSYMBOL_global_var = 279,               /* global_var  */
  YYSYMBOL_static_var_list = 280,          /* static_var_list  */
  YYSYMBOL_static_var = 281,               /* static_var  */
  YYSYMBOL_class_statement_list = 282,     /* class_statement_list  */
  YYSYMBOL_attributed_class_statement = 283, /* attributed_class_statement  */
  YYSYMBOL_class_statement = 284,          /* class_statement  */
  YYSYMBOL_class_name_list = 285,          /* class_name_list  */
  YYSYMBOL_trait_adaptations = 286,        /* trait_adaptations  */
  YYSYMBOL_trait_adaptation_list = 287,    /* trait_adaptation_list  */
  YYSYMBOL_trait_adaptation = 288,         /* trait_adaptation  */
  YYSYMBOL_trait_precedence = 289,         /* trait_precedence  */
  YYSYMBOL_trait_alias = 290,              /* trait_alias  */
  YYSYMBOL_trait_method_reference = 291,   /* trait_method_reference  */
  YYSYMBOL_absolute_trait_method_reference = 292, /* absolute_trait_method_reference  */
  YYSYMBOL_method_body = 293,              /* method_body  */
  YYSYMBOL_variable_modifiers = 294,       /* variable_modifiers  */
  YYSYMBOL_method_modifiers = 295,         /* method_modifiers  */
  YYSYMBOL_non_empty_member_modifiers = 296, /* non_empty_member_modifiers  */
  YYSYMBOL_member_modifier = 297,          /* member_modifier  */
  YYSYMBOL_property_list = 298,            /* property_list  */
  YYSYMBOL_property = 299,                 /* property  */
  YYSYMBOL_class_const_list = 300,         /* class_const_list  */
  YYSYMBOL_class_const_decl = 301,         /* class_const_decl  */
  YYSYMBOL_const_decl = 302,               /* const_decl  */
  YYSYMBOL_echo_expr_list = 303,           /* echo_expr_list  */
  YYSYMBOL_echo_expr = 304,                /* echo_expr  */
  YYSYMBOL_for_exprs = 305,                /* for_exprs  */
  YYSYMBOL_non_empty_for_exprs = 306,      /* non_empty_for_exprs  */
  YYSYMBOL_anonymous_class = 307,          /* anonymous_class  */
  YYSYMBOL_308_9 = 308,                    /* @9  */
  YYSYMBOL_new_expr = 309,                 /* new_expr  */
  YYSYMBOL_expr = 310,                     /* expr  */
  YYSYMBOL_inline_function = 311,          /* inline_function  */
  YYSYMBOL_fn = 312,                       /* fn  */
  YYSYMBOL_function = 313,                 /* function  */
  YYSYMBOL_backup_doc_comment = 314,       /* backup_doc_comment  */
  YYSYMBOL_backup_fn_flags = 315,          /* backup_fn_flags  */
  YYSYMBOL_backup_lex_pos = 316,           /* backup_lex_pos  */
  YYSYMBOL_returns_ref = 317,              /* returns_ref  */
  YYSYMBOL_lexical_vars = 318,             /* lexical_vars  */
  YYSYMBOL_lexical_var_list = 319,         /* lexical_var_list  */
  YYSYMBOL_lexical_var = 320,              /* lexical_var  */
  YYSYMBOL_function_call = 321,            /* function_call  */
  YYSYMBOL_322_10 = 322,                   /* @10  */
  YYSYMBOL_class_name = 323,               /* class_name  */
  YYSYMBOL_class_name_reference = 324,     /* class_name_reference  */
  YYSYMBOL_exit_expr = 325,                /* exit_expr  */
  YYSYMBOL_backticks_expr = 326,           /* backticks_expr  */
  YYSYMBOL_ctor_arguments = 327,           /* ctor_arguments  */
  YYSYMBOL_dereferenceable_scalar = 328,   /* dereferenceable_scalar  */
  YYSYMBOL_scalar = 329,                   /* scalar  */
  YYSYMBOL_constant = 330,                 /* constant  */
  YYSYMBOL_class_constant = 331,           /* class_constant  */
  YYSYMBOL_optional_expr = 332,            /* optional_expr  */
  YYSYMBOL_variable_class_name = 333,      /* variable_class_name  */
  YYSYMBOL_fully_dereferenceable = 334,    /* fully_dereferenceable  */
  YYSYMBOL_array_object_dereferenceable = 335, /* array_object_dereferenceable  */
  YYSYMBOL_callable_expr = 336,            /* callable_expr  */
  YYSYMBOL_callable_variable = 337,        /* callable_variable  */
  YYSYMBOL_variable = 338,                 /* variable  */
  YYSYMBOL_simple_variable = 339,          /* simple_variable  */
  YYSYMBOL_static_member = 340,            /* static_member  */
  YYSYMBOL_new_variable = 341,             /* new_variable  */
  YYSYMBOL_member_name = 342,              /* member_name  */
  YYSYMBOL_property_name = 343,            /* property_name  */
  YYSYMBOL_array_pair_list = 344,          /* array_pair_list  */
  YYSYMBOL_possible_array_pair = 345,      /* possible_array_pair  */
  YYSYMBOL_non_empty_array_pair_list = 346, /* non_empty_array_pair_list  */
  YYSYMBOL_array_pair = 347,               /* array_pair  */
  YYSYMBOL_encaps_list = 348,              /* encaps_list  */
  YYSYMBOL_encaps_var = 349,               /* encaps_var  */
  YYSYMBOL_encaps_var_offset = 350,        /* encaps_var_offset  */
  YYSYMBOL_internal_functions_in_yacc = 351, /* internal_functions_in_yacc  */
  YYSYMBOL_isset_variables = 352,          /* isset_variables  */
  YYSYMBOL_isset_variable = 353            /* isset_variable  */
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
#define YYLAST   9309

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  179
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  175
/* YYNRULES -- Number of rules.  */
#define YYNRULES  590
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1116

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
       0,   296,   296,   300,   300,   300,   300,   300,   300,   300,
     300,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   304,   304,   305,   305,   305,   305,   305,   305,
     305,   305,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   310,   311,   311,   311,   311,   311,   311,
     311,   315,   316,   320,   321,   329,   330,   335,   336,   341,
     342,   347,   348,   352,   353,   354,   355,   359,   361,   366,
     368,   373,   377,   378,   382,   383,   384,   385,   386,   390,
     391,   392,   393,   397,   400,   400,   403,   403,   406,   407,
     408,   409,   410,   414,   415,   419,   424,   429,   430,   434,
     436,   441,   443,   448,   450,   455,   456,   460,   462,   467,
     469,   474,   475,   479,   481,   487,   488,   489,   490,   497,
     498,   499,   500,   502,   504,   506,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   519,   523,   522,   526,
     527,   529,   530,   534,   536,   541,   542,   546,   547,   551,
     552,   556,   557,   561,   565,   566,   574,   581,   582,   586,
     587,   591,   591,   594,   594,   600,   601,   606,   607,   608,
     612,   612,   618,   618,   624,   624,   630,   631,   635,   640,
     641,   645,   646,   650,   651,   655,   656,   660,   661,   662,
     663,   667,   668,   672,   673,   677,   678,   682,   683,   684,
     685,   689,   690,   692,   697,   698,   703,   708,   709,   713,
     714,   718,   720,   725,   726,   731,   732,   737,   740,   746,
     747,   752,   755,   761,   762,   768,   769,   774,   776,   781,
     782,   786,   787,   791,   792,   793,   794,   798,   802,   810,
     811,   815,   816,   817,   818,   822,   823,   827,   828,   832,
     834,   839,   840,   847,   848,   849,   850,   854,   855,   856,
     860,   861,   865,   867,   872,   874,   879,   880,   884,   885,
     886,   890,   892,   897,   898,   900,   904,   905,   909,   915,
     916,   920,   921,   925,   927,   933,   936,   939,   943,   947,
     948,   949,   954,   955,   959,   960,   961,   965,   967,   972,
     973,   977,   982,   984,   988,   990,   995,   997,  1001,  1006,
    1007,  1011,  1012,  1016,  1017,  1022,  1023,  1028,  1029,  1030,
    1031,  1032,  1033,  1034,  1038,  1039,  1043,  1045,  1050,  1051,
    1055,  1059,  1063,  1064,  1067,  1071,  1072,  1076,  1077,  1081,
    1081,  1091,  1093,  1095,  1100,  1102,  1104,  1106,  1108,  1110,
    1111,  1113,  1115,  1117,  1119,  1121,  1123,  1125,  1127,  1129,
    1131,  1133,  1135,  1137,  1138,  1139,  1140,  1141,  1143,  1145,
    1147,  1149,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1170,  1172,  1174,  1176,  1178,  1180,  1182,  1184,  1186,  1188,
    1192,  1193,  1195,  1197,  1199,  1200,  1201,  1202,  1203,  1204,
    1205,  1206,  1207,  1208,  1209,  1210,  1211,  1212,  1213,  1214,
    1215,  1216,  1217,  1218,  1219,  1220,  1222,  1227,  1232,  1241,
    1245,  1249,  1253,  1257,  1261,  1262,  1266,  1267,  1271,  1272,
    1276,  1277,  1281,  1283,  1288,  1290,  1292,  1292,  1299,  1302,
    1306,  1307,  1308,  1312,  1313,  1317,  1319,  1320,  1325,  1326,
    1331,  1332,  1333,  1334,  1338,  1339,  1340,  1341,  1343,  1344,
    1345,  1346,  1350,  1351,  1352,  1353,  1354,  1355,  1356,  1357,
    1358,  1362,  1364,  1369,  1370,  1374,  1378,  1379,  1380,  1381,
    1385,  1386,  1390,  1391,  1392,  1396,  1398,  1400,  1402,  1404,
    1406,  1410,  1412,  1414,  1416,  1421,  1422,  1423,  1427,  1429,
    1434,  1436,  1438,  1440,  1442,  1444,  1446,  1451,  1452,  1453,
    1457,  1458,  1459,  1463,  1468,  1469,  1473,  1475,  1480,  1482,
    1484,  1486,  1488,  1490,  1493,  1499,  1501,  1503,  1505,  1510,
    1512,  1515,  1518,  1521,  1523,  1525,  1528,  1532,  1533,  1534,
    1535,  1540,  1541,  1542,  1544,  1546,  1548,  1550,  1555,  1556,
    1561
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
  "function_name", "function_declaration_statement", "is_reference",
  "is_variadic", "class_declaration_statement", "@4", "@5",
  "class_modifiers", "class_modifier", "trait_declaration_statement", "@6",
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
  "optional_type_without_static", "type_expr", "type",
  "union_type_element", "union_type", "intersection_type",
  "type_expr_without_static", "type_without_static",
  "union_type_without_static_element", "union_type_without_static",
  "intersection_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "attributed_class_statement", "class_statement", "class_name_list",
  "trait_adaptations", "trait_adaptation_list", "trait_adaptation",
  "trait_precedence", "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "class_const_list", "class_const_decl",
  "const_decl", "echo_expr_list", "echo_expr", "for_exprs",
  "non_empty_for_exprs", "anonymous_class", "@9", "new_expr", "expr",
  "inline_function", "fn", "function", "backup_doc_comment",
  "backup_fn_flags", "backup_lex_pos", "returns_ref", "lexical_vars",
  "lexical_var_list", "lexical_var", "function_call", "@10", "class_name",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "dereferenceable_scalar", "scalar", "constant", "class_constant",
  "optional_expr", "variable_class_name", "fully_dereferenceable",
  "array_object_dereferenceable", "callable_expr", "callable_variable",
  "variable", "simple_variable", "static_member", "new_variable",
  "member_name", "property_name", "array_pair_list", "possible_array_pair",
  "non_empty_array_pair_list", "array_pair", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions_in_yacc", "isset_variables",
  "isset_variable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-900)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-562)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -900,    96,  3527,  -900,  7034,  7034,  7034,  7034,  7034,  -900,
    -900,   129,  -900,  -900,  -900,  -900,  -900,  -900,  7034,  7034,
     -28,  7034,  7034,  7034,  7034,  7034,   260,  7034,   -13,    46,
    7034,  5865,    72,    89,    93,   107,   139,   178,  7034,  7034,
     160,  -900,  -900,   229,  7034,   177,  7034,   441,    -5,   210,
    -900,  -900,   187,   243,   278,   291,   311,  -900,  -900,  -900,
    -900,  9020,   321,   324,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,   428,  8587,  8587,  7034,  7034,  7034,  7034,  7034,
    7034,  7034,   101,  7034,  -900,  -900,  6032,    47,   143,    62,
     -49,  -900,   864,  -900,  -900,  -900,  -900,  -900,   417,  -900,
    -900,  -900,  -900,  -900,   472,  -900,   254,  -900,  -900,  2703,
    -900,   271,   271,  -900,   330,   616,  -900,   386,   350,   339,
     348,   439,  -900,   357,    32,  -900,  -900,  -900,  -900,   509,
     187,   162,   225,   271,   225,    31,   225,   225,  -900,  8096,
    8096,  7034,  8096,  8096,  8227,  2847,  8227,  -900,  -900,  7034,
    -900,   213,  -900,   351,   187,  -900,   461,  -900,  7034,  -900,
    7034,    82,  -900,  8096,   483,  7034,  7034,  7034,   229,  7034,
    7034,  8096,   368,   370,   378,   558,   197,  -900,   385,  -900,
    8096,  -900,  -900,  -900,  -900,  -900,  -900,    64,   577,   396,
     235,  -900,   318,  -900,  -900,   571,   375,  -900,  -900,  2859,
    -900,  8587,  7034,  7034,   415,   581,   599,   601,   615,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,   469,   470,  6032,  6032,  -900,   477,   187,  7034,  6032,
     491,  -900,  -900,   560,   560,   225,   225,   225,   225,   225,
     225,   225,   127,   167,  -900,  6533,  8587,   110,  -900,  7142,
    3694,   497,  7034,  -900,  -900,  8587,  7935,   514,  -900,   516,
    -900,   137,   524,   199,   137,    91,  7034,  -900,  -900,   509,
    -900,  -900,  -900,  -900,  -900,  -900,   532,  5865,   537,   702,
     541,  1936,  7034,  7034,  7034,  7034,  7034,  7034,  7034,  7034,
    7034,  7034,  7034,  7034,  7034,   547,  7034,  7034,  7034,  7034,
    7034,  7034,  7034,  7034,  7034,  7034,  7034,  7034,  7034,  7034,
    7034,  -900,  -900,  -900,    50,  8498,  8498,    63,    63,  7034,
    7034,   187,  6199,  7034,  7034,  7034,  7034,  7034,  7034,  7034,
    7034,  7034,  7034,  7034,  7034,  -900,  -900,  7034,  -900,  7208,
    7034,   187,  7220,  -900,    -5,  -900,  -900,    63,    63,    -5,
    7034,  7034,   545,  7276,  7034,  -900,   549,  7410,   550,   559,
    8096,  7988,   -61,  7457,  7469,  -900,  -900,  -900,  7034,   229,
    -900,  -900,  3861,   698,   555,    70,   554,   380,  -900,   577,
    -900,    -5,  -900,  7034,   712,  -900,   191,  7034,  7034,   -28,
    7034,  7034,  7034,  2103,   260,  7034,    44,   178,   723,   735,
    7034,     7,   187,   278,   291,   321,   324,   737,   738,   740,
     741,   742,   749,   759,   762,  6700,  -900,   764,   596,  -900,
    8096,   605,  -900,   146,  8096,   607,  -900,  7516,   613,   699,
    -900,   701,   784,  -900,   627,  -900,   630,   639,   428,   644,
    -900,  7650,   645,   792,   799,   598,  -900,  -900,   419,  2211,
     650,  -900,  -900,  -900,   833,   655,  -900,   864,  -900,  -900,
    -900,  6032,  8096,   478,  6366,   826,  6032,  -900,  -900,  2277,
    -900,   809,  7034,  -900,  7034,  -900,  -900,  7034,  8033,  8365,
    2680,  1249,  1249,   576,   315,   315,    31,    31,    31,  8164,
    8176,  8227,  -900,  8237,  8305,  8497,  8497,  8497,  8497,  1249,
    1249,  8497,   100,   100,  2602,   225,  8639,  8639,   663,  -900,
    -900,  -900,   666,  7034,   668,   678,   187,   668,   678,   187,
    -900,  7034,  -900,   187,   187,  2291,   681,  -900,  8587,  8227,
    8227,  8227,  8227,  8227,  8227,  8227,  8227,  8227,  8227,  8227,
    8227,  8227,  8227,  -900,  8227,   699,  -900,  -900,  -900,  -900,
    -900,  2342,   682,  -900,  1096,  -900,  7034,  1264,  7034,  7034,
     779,  -900,    11,   679,  8096,  -900,  -900,  -900,   405,   680,
    -900,  -900,   793,  -900,  -900,  8096,  -900,  -900,  8096,  7034,
    3026,   685,  8587,   686,  7034,   689,  -900,  -900,   428,   763,
     690,   428,  -900,    99,   763,  -900,  3193,   860,  -900,  -900,
    -900,   695,  -900,  -900,  -900,   836,  -900,  -900,  -900,   700,
    -900,  7034,  -900,  -900,   703,  -900,   707,   709,  8587,  8096,
    7034,  -900,  -900,   699,  7697,  7709,  4028,  2602,  7034,   -42,
     722,   -42,  2473,  -900,  -900,  2523,  -900,  -900,  -900,  -900,
     560,   763,  -900,  -900,  -900,  -900,  7756,  -900,  -900,  -900,
     721,  8096,   724,  6032,  8587,   142,   161,  1432,   727,   728,
    -900,  6867,  -900,   533,   839,   345,   730,  -900,  -900,   345,
    8096,  7034,  -900,  -900,  -900,   732,  -900,  -900,  -900,   428,
    -900,  -900,   734,  -900,   736,   726,  -900,  -900,  -900,   726,
    -900,  -900,    14,   888,   898,   706,  -900,  -900,  3360,  -900,
    7034,  -900,  -900,  7890,   739,   860,  6032,   489,  8227,   763,
    5865,   901,   744,  2602,   796,   743,   751,  -900,   432,  -900,
     -42,   746,  -900,  -900,  -900,  5698,   748,  4195,  7034,  6032,
     757,   297,   779,  1600,  -900,  -900,  -900,  -900,   340,  -900,
     154,   760,   753,   765,  -900,   766,  8096,   755,   767,  -900,
     908,  -900,   405,   768,   769,  -900,  -900,   734,   772,   603,
     428,  -900,  -900,   774,   -77,   726,   553,   553,   726,   790,
    -900,  8227,   795,  -900,   770,  -900,  -900,  -900,  -900,  -900,
     958,   -31,  -900,   756,  -900,  -900,  -900,  -900,   756,  -900,
     803,  -900,    57,   963,   964,   807,   802,   896,   811,  -900,
     808,   814,   817,   327,   818,  -900,  -900,  -900,  4362,   614,
     810,  7034,    13,   165,  -900,  -900,   843,  -900,  6867,  -900,
    7034,   846,   428,  -900,  -900,  -900,  -900,   345,   822,  -900,
    -900,   428,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,
    -900,  1275,  -900,  -900,  -900,   136,   217,  1306,  -900,  -900,
     758,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,   860,
     824,  5698,    99,   851,  -900,  -900,   835,    35,  -900,   844,
     756,   572,   572,   756,   958,   837,   958,  -900,  -900,  1768,
    -900,  1600,  4529,   838,   840,  -900,  2537,  -900,  -900,  -900,
    -900,  7034,  -900,  8096,  7034,    60,  -900,  4696,  -900,  -900,
    1130,  9109,   194,  -900,   986,  9109,   271,  -900,  -900,  1466,
    -900,  -900,  -900,  -900,  -900,   987,  -900,  -900,  -900,  -900,
    -900,  -900,    45,  -900,  1634,  -900,  -900,  -900,  -900,   845,
    -900,  -900,  -900,  5698,  8096,  8096,   428,  -900,   848,  -900,
    -900,  1011,  -900,  8758,  -900,  1012,   383,  -900,  1016,   395,
    -900,  9109,  -900,  1802,  -900,  -900,   849,  -900,  1001,   861,
    -900,   857,  -900,  4863,  -900,  5698,  -900,   862,  7034,   866,
     878,  -900,  -900,  8889,  -900,   867,   869,   980,   967,   885,
    7034,  -900,   986,  -900,  7034,  9109,  -900,  -900,  -900,  7034,
    1045,  -900,  -900,    45,   879,  -900,   880,  -900,  8096,  -900,
    -900,  -900,  -900,  -900,  9198,   428,  9109,  8096,  -900,  8096,
    -900,   881,  8096,  7034,  5030,  -900,  -900,  5197,  -900,  5364,
    -900,  -900,  9109,   734,  -900,  -900,  -900,   -42,  -900,  8096,
    -900,  -900,  -900,  -900,   882,  -900,  -900,   958,  -900,   444,
    -900,  -900,  -900,  5531,  -900,  -900
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      86,     0,     2,     1,     0,     0,     0,     0,     0,   494,
     495,    93,    95,    96,    94,   535,   162,   492,     0,     0,
       0,     0,     0,     0,   447,     0,     0,     0,   483,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   513,   513,
       0,   460,   459,     0,   513,     0,     0,     0,     0,   478,
     197,   198,   199,     0,     0,     0,     0,   193,   200,   202,
     204,   116,     0,     0,   503,   504,   505,   510,   506,   507,
     508,   509,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   169,   144,   554,   485,     0,     0,
     502,   102,     0,   110,    85,   109,   104,   105,     0,   195,
     106,   107,   108,   456,   249,   150,     0,   151,   430,     0,
     452,   464,   464,   530,     0,   499,   444,   500,   501,     0,
     520,     0,   476,   531,   374,   525,   532,   434,    93,   478,
       0,     0,   415,   464,   416,   417,   418,   443,   172,   583,
     584,     0,   586,   587,   446,   448,   450,   478,   369,     0,
     479,     0,   372,   480,   488,   540,   481,   379,   513,   442,
       0,     0,   363,   364,     0,     0,   365,     0,     0,     0,
       0,   514,     0,     0,     0,     0,     0,   142,     0,   144,
     451,    89,    92,    90,   123,   124,    91,   139,     0,     0,
       0,   134,     0,   307,   308,   311,     0,   310,   454,     0,
     473,     0,     0,     0,     0,     0,     0,     0,     0,    83,
      88,     3,     4,     5,     6,     7,     8,     9,    10,    46,
      47,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      49,    50,    51,    52,    71,    53,    41,    42,    43,    70,
      44,    45,    30,    31,    32,    33,    34,    35,    36,    74,
      75,    76,    77,    78,    79,    80,    37,    38,    39,    40,
      61,    59,    60,    72,    56,    57,    58,    48,    54,    55,
      66,    67,    68,    62,    63,    65,    64,    69,    73,    84,
      87,   114,     0,   554,   554,    99,   127,    97,     0,   554,
     518,   521,   519,   394,   396,   435,   436,   437,   438,   439,
     440,   441,   569,     0,   497,     0,     0,     0,   567,     0,
       0,     0,     0,    81,    82,     0,   559,     0,   557,   553,
     555,   486,     0,   487,     0,     0,     0,   537,   472,     0,
     199,   103,   111,   453,   191,   196,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   163,   465,   461,   461,     0,     0,     0,     0,     0,
     513,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   393,   395,     0,   461,     0,
       0,   488,     0,   373,     0,   489,   371,     0,     0,     0,
       0,   513,     0,     0,     0,   161,     0,     0,     0,   366,
     368,     0,     0,     0,     0,   156,   157,   171,     0,     0,
     122,   158,     0,     0,     0,   139,     0,     0,   118,     0,
     120,     0,   159,     0,     0,   160,    93,     3,     4,     5,
       6,     7,    46,   447,    12,    13,   483,    71,   460,   459,
      33,    74,    80,    39,    40,    48,    54,   503,   504,   505,
     510,   506,   507,   508,   509,     0,   298,     0,   127,   301,
     303,   127,   181,   516,   590,   127,   588,     0,     0,   211,
     461,   213,   206,   113,     0,    86,     0,     0,   128,     0,
      98,     0,     0,     0,     0,     0,   496,   568,     0,     0,
     516,   566,   498,   565,   429,     0,   149,     0,   146,   143,
     145,   554,   562,   516,     0,   491,   554,   445,   493,     0,
     455,     0,     0,   250,     0,   144,   253,     0,     0,   402,
     405,   423,   425,   406,   407,   408,   409,   411,   412,   399,
     401,   400,   428,   397,   398,   421,   422,   419,   420,   424,
     426,   427,   413,   414,   433,   410,   404,   403,     0,   184,
     185,   461,     0,     0,   511,   538,     0,   512,   539,     0,
     550,     0,   552,   533,   534,     0,     0,   477,     0,   377,
     380,   381,   382,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   383,   585,   449,   211,   482,   545,   543,   544,
     546,     0,     0,   484,     0,   362,     0,     0,   365,     0,
       0,   167,     0,     0,   461,   141,   173,   140,     0,     0,
     119,   121,   139,   133,   306,   312,   309,   300,   305,     0,
     128,     0,   128,     0,   128,     0,   582,   112,     0,   215,
       0,     0,   461,     0,   215,    86,     0,     0,   490,   100,
     101,   517,   491,   571,   572,     0,   577,   580,   578,     0,
     574,     0,   573,   576,     0,   147,     0,     0,     0,   558,
       0,   556,   536,   211,     0,     0,     0,   432,     0,   261,
       0,   261,     0,   474,   475,     0,   528,   529,   527,   526,
     378,   215,   542,   541,   144,   247,     0,   144,   245,   152,
       0,   367,     0,   554,     0,     0,   516,     0,   231,   231,
     155,   237,   361,   179,   137,     0,   127,   130,   135,     0,
     304,     0,   302,   299,   182,     0,   589,   581,   212,     0,
     461,   314,   214,   322,     0,     0,   276,   287,   288,     0,
     289,   207,   271,     0,   273,   274,   275,   461,     0,   117,
       0,   579,   570,     0,     0,   564,   554,   516,   376,   215,
       0,     0,     0,   431,   261,     0,   127,   257,   269,   260,
     261,     0,   548,   551,   461,   251,     0,     0,   365,   554,
       0,   516,     0,     0,   144,   225,   168,   231,     0,   231,
       0,   127,     0,   127,   239,   127,   243,     0,     0,   170,
       0,   136,   128,     0,   127,   132,   164,   216,     0,   343,
       0,   314,   272,     0,     0,     0,     0,     0,     0,     0,
     115,   375,     0,   148,     0,   461,   248,   144,   254,   259,
     296,   261,   255,     0,   265,   264,   263,   266,     0,   262,
     187,   270,   283,     0,   285,   286,     0,   466,     0,   153,
       0,     0,     0,   491,     0,   144,   223,   165,     0,     0,
       0,     0,     0,     0,   227,   128,     0,   236,   128,   238,
     128,     0,     0,   144,   138,   129,   126,   128,     0,   314,
     461,     0,   350,   351,   352,   349,   348,   347,   353,   342,
     201,   343,   318,   319,   313,   269,     0,   341,   345,   323,
     343,   278,   281,   277,   279,   280,   282,   314,   575,   563,
       0,   252,     0,     0,   258,   284,     0,     0,   188,   189,
       0,     0,     0,     0,   296,     0,   296,   314,   246,     0,
     219,     0,     0,     0,     0,   229,     0,   234,   235,   144,
     228,     0,   240,   244,     0,   177,   175,     0,   131,   125,
     343,     0,     0,   320,     0,     0,   464,   346,   203,   343,
     314,   297,   462,   291,   190,     0,   294,   290,   292,   293,
     295,   462,     0,   462,   343,   144,   221,   154,   166,     0,
     226,   230,   144,   233,   242,   241,     0,   178,     0,   180,
     194,   209,   324,     0,   321,   461,     0,   355,     0,     0,
     359,     0,   205,   343,   463,   461,     0,   470,     0,   127,
     469,     0,   370,     0,   224,   232,   176,     0,     0,     0,
      74,   325,   336,     0,   327,     0,     0,     0,   337,     0,
       0,   356,     0,   315,     0,     0,   316,   461,   192,     0,
     267,   144,   471,   128,     0,   144,     0,   144,   210,   208,
     326,   328,   329,   330,     0,     0,     0,   461,   354,   461,
     358,     0,   462,     0,     0,   468,   467,     0,   222,     0,
     332,   333,   335,   331,   338,   357,   360,   261,   458,   268,
     462,   462,   174,   334,     0,   186,   457,   296,   462,     0,
     339,   144,   462,     0,   317,   340
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -900,  -900,   -20,  -900,  -101,    24,  -450,  -900,   -22,  -174,
     -19,   551,  -900,   -80,    -2,     3,  -900,  -900,  -900,  1008,
    -900,  -900,  -475,  -900,  -900,   870,   239,  -670,   608,   894,
     -21,  -900,     4,  -900,  -900,  -900,  -900,  -900,  -900,   411,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,   966,  -900,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -481,  -900,
    -432,   263,  -900,   115,  -900,  -900,  -531,   112,  -900,  -900,
    -900,   181,  -900,  -900,  -900,  -900,  -900,  -900,  -663,  -900,
     219,  -900,  -900,   287,   158,   145,   -78,  -210,  -900,   316,
    -900,  -645,  -309,  -900,   216,  -890,   -48,  -900,   434,  -900,
     638,  -900,   624,  -635,   180,  -900,  -734,  -900,  -900,    49,
    -900,  -900,  -900,  -900,  -900,  -900,  -900,  -900,  -899,  -900,
      41,  -900,    39,   657,  -900,   673,  -580,  -900,   947,  -900,
    -900,    22,   -12,  -900,    -1,  -301,  -748,  -900,  -110,  -900,
    -900,    36,  -900,  -900,     5,   745,  -900,  -900,   693,   -70,
    -900,   -65,   -15,    18,  -900,  -900,  -900,  -900,  -900,    10,
      85,  -900,  -900,   715,  -325,  -260,   567,  -900,  -900,   565,
     692,  -900,  -900,  -900,   452
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   288,   289,   325,   487,     2,   291,   734,   187,
      90,   295,   296,    91,   131,   528,    94,   504,   292,   735,
     446,   189,   509,   736,   824,   190,   737,   738,   191,   176,
     320,   529,   530,   727,   733,   965,  1008,   819,   491,   492,
     581,    96,   939,   985,    97,   541,   205,    98,    99,   100,
     206,   101,   207,   102,   208,   664,   912,  1039,   659,   662,
     750,   725,   997,   877,   806,   730,   808,   959,   103,   812,
     813,   814,   815,   719,   104,   105,   106,   107,   785,   786,
     787,   788,   859,   789,   860,   761,   762,   763,   764,   765,
     861,   766,   863,   864,   865,   933,   200,   488,   489,   192,
     193,   196,   197,   829,   913,   914,   752,  1014,  1043,  1044,
    1045,  1046,  1047,  1048,  1112,   915,   916,   917,   918,  1016,
    1017,  1019,  1020,   177,   161,   162,   428,   429,   152,   411,
     108,   109,   110,   111,   133,   582,  1024,  1059,   383,   946,
    1029,  1030,   113,   391,   114,   154,   159,   332,   416,   115,
     116,   117,   118,   172,   119,   120,   121,   122,   123,   124,
     125,   126,   156,   586,   593,   327,   328,   329,   330,   317,
     318,   679,   127,   495,   496
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      92,   112,   384,   300,   300,    93,    95,   150,   301,   301,
     382,   382,   341,   651,   445,   827,   653,   728,   977,   957,
     655,  -277,    15,   408,   151,   186,   132,   134,   135,   136,
     137,   153,   382,   506,   507,   164,   392,   198,   791,   512,
     139,   140,   338,   142,   143,   144,   145,   146,   720,   157,
     -14,   341,   163,   150,   991,   666,   993,   173,   302,   302,
     171,   171,   178,   594,  -290,   821,   171,  1006,   180,   825,
      72,   341,  1027,   579,   312,   365,   331,   297,    41,    42,
     343,    72,   578,   303,   304,   290,   590,  1007,   838,    15,
      15,   112,   618,   619,   921,   342,     3,   305,   306,   307,
     308,   309,   310,   311,   755,   319,   415,   439,   326,  -479,
     631,   155,   357,   358,   359,   360,   361,   198,   312,   343,
     521,   199,   128,    12,    13,    14,   443,   866,   312,  -256,
     313,   300,   443,   194,   711,   138,   301,   312,   580,   521,
    -128,   853,   141,   862,   365,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   158,   442,   128,
      12,    13,    14,   409,   312,  -478,   186,   972,   405,   406,
     312,   412,   334,    89,   337,  1092,   422,  -516,  -516,   835,
     171,   756,   423,   174,   729,   958,   302,   427,   430,   431,
    -516,   433,   434,   378,   312,   407,   920,   -83,   810,   660,
     943,   757,   758,   315,   316,  -516,   983,  -516,   935,   323,
     324,   493,   779,   936,   158,   768,   160,  1108,   871,   881,
     882,   490,   940,   444,   494,   497,   312,   968,   521,   639,
     881,   882,   767,    41,    42,   336,   591,   195,   757,   758,
      89,    89,   165,  1026,   339,  1031,   300,   315,   316,   510,
     424,   301,   175,   -83,   425,   300,   314,   315,   316,   166,
     301,   823,   378,   167,   970,   522,   315,   316,   538,   759,
     862,   686,   513,   514,    72,   642,   879,   168,   883,   794,
     700,    41,    42,   128,    12,    13,    14,    15,    41,   802,
     975,   598,   979,   315,   316,   986,   987,   987,   990,   315,
     316,   302,   515,   348,   349,   350,   858,   148,  -217,   169,
     302,   852,   994,   803,  -183,   326,   326,  -183,   527,   112,
     511,   326,   516,   315,   316,    72,   520,   540,   884,   359,
     360,   361,  -217,   732,  1098,   533,   886,   519,   889,   960,
     891,  1093,   147,   597,   532,  1023,   150,   845,   170,   898,
     179,   543,  1105,  1106,   148,   315,   316,   199,   539,   365,
    1109,   754,   830,   415,  1114,   439,  1012,  1013,   181,   440,
     153,   183,    72,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   378,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   449,   880,   881,   882,   450,   596,   584,
     587,   595,   171,   201,   599,   600,   601,   602,   603,   604,
     605,   606,   607,   608,   609,   610,   611,   186,   181,   612,
     149,   183,   614,   688,  1104,   323,   324,   853,    89,   622,
     527,   112,   621,   171,  -218,   150,   163,   341,   202,   828,
     155,   128,    12,    13,    14,   128,    12,    13,    14,   198,
     634,   203,   151,   800,   181,   182,   839,   183,  -218,   153,
     585,   588,   592,   592,  -220,   645,   184,   378,   185,   139,
     140,   204,   142,   143,   144,   145,   451,   157,   385,   150,
     452,   293,   180,   868,   294,  -519,  -519,   386,  -220,   617,
      50,    51,   592,   592,   620,   340,  -515,   648,  -519,   414,
     147,   344,   184,   297,   185,   343,   844,   854,   855,   856,
     857,   346,   347,  -519,   696,  -519,   112,  -522,   300,   724,
     685,  -521,  -521,   301,   757,   758,   194,   426,   703,   872,
     435,   704,   436,   454,   930,   706,   707,   455,   449,   155,
     437,  1052,   641,   326,  1064,  1053,   689,   441,   326,  -521,
     300,  -521,   438,  1055,   694,   301,   695,  1056,   448,   697,
     128,    12,    13,    14,    15,   453,   128,    12,    13,    14,
      41,    42,   300,   302,   387,   388,   498,   301,   357,   358,
     359,   360,   361,   680,   681,   128,    12,    13,    14,   971,
     181,   182,   858,   183,   499,   702,   417,   418,   710,   817,
     818,   675,   389,   705,   390,   302,  1110,  1111,   300,   419,
     365,   676,   500,   301,   501,   677,   924,   925,   715,   147,
     678,   718,   988,   989,   420,   756,   421,   302,   502,   150,
     726,   503,   150,   505,   760,   508,  -561,  -561,   716,  -561,
     430,   721,   333,   335,   300,   757,   758,  -560,  -560,   301,
    -560,  -524,   493,   748,    92,   112,   753,   531,   900,    93,
      95,   740,   490,   302,   757,   758,   494,   832,   954,   881,
     882,   833,   901,   535,   536,   902,   903,   904,   905,   906,
     907,   908,   909,   795,   527,   112,   797,   784,   777,   784,
     537,   724,   542,   773,   341,  -516,  -516,   544,   545,   302,
     375,   376,   778,   546,  1051,    72,   623,   149,  -516,   626,
     783,   637,   628,   759,  1060,    89,   640,   629,   638,   -43,
     150,   805,   300,  -516,   801,  -516,   760,   301,   378,   195,
     760,   -70,   858,   -66,   -67,   326,   -68,   -62,   -63,   128,
      12,    13,    14,   816,   753,   -65,  1081,   922,   923,   923,
     926,  -518,  -518,   648,   650,   -64,    92,   112,   -69,   760,
     649,    93,    95,   652,  -518,   654,  1095,   910,  1096,   128,
      12,    13,    14,   878,   846,   657,  -524,   302,   784,  -518,
     663,  -518,   841,   527,   112,   527,   112,   658,   326,   661,
     665,   667,   128,    12,    13,    14,    15,   876,   756,    17,
     668,   150,   726,   670,   672,   673,   760,   760,   760,   760,
     430,   326,   674,   900,   683,   684,   931,   911,   757,   758,
     690,   341,   693,   699,   760,   919,   701,   901,  -547,   760,
     902,   903,   904,   905,   906,   907,   908,   909,  -549,   784,
     709,   713,   731,   739,   952,   443,   743,   745,   757,   758,
     747,   147,   749,   751,   770,  -523,  1021,   130,   771,   772,
      72,   838,   967,   150,   774,   382,   527,   112,   775,   776,
     722,    63,   150,    64,    65,    66,    67,    68,    69,    70,
      71,  1028,   790,   798,   799,   836,   760,   966,   822,   807,
     809,   820,   830,   956,   826,   837,   753,   847,    72,   831,
     816,   843,   963,   760,   850,   976,   848,   867,   911,   851,
     869,   760,   760,   760,   760,   892,   873,   887,   885,   527,
     112,   894,   978,   888,   890,    41,    42,   897,  1003,   835,
     893,   929,   896,   323,   324,   899,   339,    50,    51,   298,
     527,   112,   340,   996,   723,   876,    88,    89,    57,    58,
      59,    60,  1028,   927,   932,   527,   112,   938,   911,   928,
     941,   942,   943,   944,  1033,   945,    72,   911,  -517,  -517,
     948,  1035,   955,  1004,   947,   949,  1005,   150,   950,   951,
     961,  -517,   911,   964,   150,  1011,   969,   980,   982,  1018,
     940,   527,   112,  -523,   984,   517,  -517,   992,  -517,   523,
    1000,  1036,  1001,  1015,  1025,  1038,  1050,  1034,  1049,  1037,
    1054,   911,  1061,   517,   150,   523,   517,   523,  1062,  1063,
    1065,   527,   112,   527,   112,  1067,  -478,  1042,  1069,  1072,
    1084,  1073,  1074,  1076,  1087,  1057,  1089,  1075,  1049,  1083,
    1086,  1097,  1088,  1107,  1091,   188,   150,   643,   447,   669,
    1068,   895,   432,   744,   345,   874,   998,  1042,  1002,   962,
     934,   849,  1077,   974,   937,   834,  1079,   981,   646,  1018,
     753,  1082,   527,   112,   742,   527,   112,   527,   112,   644,
    1113,   973,  1071,  1078,  1080,   784,   635,   625,   413,  1085,
    1094,   589,   714,   691,   615,  1099,   746,     0,     4,     5,
     562,   527,   112,     6,     7,     8,  1103,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,     0,
       0,     0,     0,     0,   130,     0,    53,    54,    55,     0,
       0,     0,     0,     0,     0,   900,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,   901,
       0,     0,   902,   903,   904,   905,   906,   907,   908,   909,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,    72,     0,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,     0,     0,  -562,  -562,
     356,   357,   358,   359,   360,   361,    83,     0,    84,    85,
     717,    86,    87,    88,    89,     0,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,   365,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,  1010,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
     900,     0,    46,     0,     0,    48,    49,     0,     0,     0,
       0,     0,   130,     0,    53,    54,    55,   902,   903,   904,
     905,   906,   907,   908,   909,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,  -344,     0,  -344,
    -562,  -562,     0,   375,   376,     0,     0,    72,   902,   903,
     904,   905,   906,   907,   908,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
       0,   378,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,    84,    85,   804,    86,
      87,    88,    89,     0,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
     130,     0,    53,    54,    55,     0,     0,     0,     0,     0,
       0,   900,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,   901,     0,     0,   902,   903,
     904,   905,   906,   907,   908,   909,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     0,    72,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,    84,    85,   875,    86,    87,    88,
      89,     0,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
    1022,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,     0,     0,     0,     0,     0,   130,     0,
      53,    54,    55,     0,     0,     0,     0,     0,     0,   900,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,   901,     0,     0,   902,   903,   904,   905,
     906,   907,   908,   909,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     0,    72,     0,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,    84,    85,   995,    86,    87,    88,    89,     0,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,  1032,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,   130,     0,    53,    54,
      55,     0,     0,     0,     0,     0,     0,   900,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,   901,     0,     0,   902,   903,   904,   905,   906,   907,
     908,   909,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     0,    72,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
      84,    85,   547,    86,    87,    88,    89,     0,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   128,
      12,    13,    14,    15,     0,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,  1058,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   130,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,   -47,
       0,    86,    87,    88,    89,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   128,    12,    13,    14,
      15,     0,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   129,     0,     0,     0,     0,
       0,   130,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,   351,     0,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     0,   362,
     363,   364,     0,     0,     0,   365,     0,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    86,    87,
      88,    89,   351,     0,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,     0,   351,     0,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,     0,
       0,     0,     0,     0,     0,   362,   363,   364,     0,     0,
       0,   365,     0,     0,     0,     0,     0,     0,     0,   362,
     363,   364,     0,     0,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   351,     0,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   377,   378,     0,   379,   380,     0,     0,     0,
     362,   363,   364,     0,     0,   682,   365,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,     0,     0,     0,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   377,   378,
       0,   379,   380,     0,     0,     0,     0,     0,     0,     0,
       0,   692,   377,   378,     0,   379,   380,     0,     0,     0,
       0,     0,     0,     0,     0,   708,     0,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   351,     0,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   377,   378,     0,   379,   380,     0,     0,
       0,   362,   363,   364,     0,     0,   712,   365,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   351,     0,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,     0,   351,   957,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,     0,     0,     0,     0,     0,
       0,   362,   363,   364,     0,     0,     0,   365,     0,     0,
       0,     0,     0,     0,     0,   362,   363,   364,     0,     0,
       0,   365,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   377,   378,     0,   379,   380,     0,
       0,     0,     0,     0,     0,     0,   365,   792,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,     0,
       0,     0,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   377,   378,     0,   379,   380,   354,
     355,   356,   357,   358,   359,   360,   361,   793,   377,   378,
       0,   379,   380,     0,     0,     0,     0,     0,   351,   958,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,     0,     0,     0,   365,     0,     0,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,     0,     0,
       0,   362,   363,   364,     0,     0,     0,   365,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   377,   378,     0,   379,   380,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   368,   369,   370,
     371,   372,   373,   374,   375,   376,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,     0,
       0,     0,   378,     0,   379,   380,     0,     0,     0,     0,
       0,     0,   351,     0,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   377,   378,     0,   379,   380,     0,
       0,     4,     5,     0,     0,   381,     6,     7,     8,     0,
       9,    10,   456,    12,    13,    14,    15,     0,     0,    17,
       0,   365,   457,   458,   459,   460,   461,   216,   217,   218,
     462,   463,    25,   221,   464,   465,   466,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   467,   245,   246,   247,
     468,   469,   250,   251,   252,   253,   254,   470,   256,   257,
     258,   471,   260,   261,   262,   263,   264,   472,   266,   267,
     473,   474,     0,   270,   271,   272,   273,   274,   275,   276,
     475,   476,   279,   477,   478,   479,   480,   481,   482,   483,
     484,    72,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   410,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     0,     0,     0,     0,   377,   378,
       0,   379,   380,    82,     0,     0,     0,     0,     0,   485,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
     486,     0,     0,     0,    86,    87,    88,    89,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   456,
      12,    13,    14,    15,     0,     0,    17,     0,     0,   457,
     458,   459,   460,   461,   216,   217,   218,   462,   463,    25,
     221,   464,   465,   466,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   467,   245,   246,   247,   468,   469,   250,
     251,   252,   253,   254,   470,   256,   257,   258,   471,   260,
     261,   262,   263,   264,   472,   266,   267,   473,   474,     0,
     270,   271,   272,   273,   274,   275,   276,   475,   476,   279,
     477,   478,   479,   480,   481,   482,   483,   484,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,   741,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    86,    87,    88,    89,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     0,
       0,    46,    47,     0,    48,    49,    50,    51,     0,     0,
       0,    52,     0,    53,    54,    55,    56,    57,    58,    59,
      60,     0,     0,    61,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,    84,    85,   769,    86,    87,
      88,    89,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     0,     0,    46,    47,
       0,    48,    49,    50,    51,     0,     0,     0,    52,     0,
      53,    54,    55,    56,    57,    58,    59,    60,     0,     0,
      61,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,    84,    85,   840,    86,    87,    88,    89,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
      43,    44,    45,     0,     0,    46,    47,     0,    48,    49,
      50,    51,     0,     0,     0,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,     0,     0,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,    84,
      85,     0,    86,    87,    88,    89,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,    52,     0,    53,    54,    55,   525,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,    84,    85,   526,    86,
      87,    88,    89,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,    52,
       0,    53,    54,    55,   525,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,    84,    85,   636,    86,    87,    88,    89,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,   782,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,    52,     0,    53,    54,
      55,   525,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
      84,    85,     0,    86,    87,    88,    89,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
     870,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,    52,     0,    53,    54,    55,   525,    57,
      58,    59,    60,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     0,    84,    85,     0,
      86,    87,    88,    89,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,   953,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
      52,     0,    53,    54,    55,   525,    57,    58,    59,    60,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,    84,    85,     0,    86,    87,    88,
      89,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,   999,    35,
       0,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,    44,    45,     0,     0,    46,     0,     0,
      48,    49,    50,    51,     0,     0,     0,    52,     0,    53,
      54,    55,   525,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
       0,    84,    85,     0,    86,    87,    88,    89,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,    52,     0,    53,    54,    55,   525,
      57,    58,    59,    60,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,     0,    84,    85,
    1009,    86,    87,    88,    89,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
    1066,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,    52,     0,    53,    54,    55,   525,    57,    58,    59,
      60,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,    84,    85,     0,    86,    87,
      88,    89,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,    52,     0,
      53,    54,    55,   525,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,     0,    84,    85,  1100,    86,    87,    88,    89,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,    52,     0,    53,    54,    55,
     525,    57,    58,    59,    60,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,    84,
      85,  1101,    86,    87,    88,    89,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,    52,     0,    53,    54,    55,   525,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,    84,    85,  1102,    86,
      87,    88,    89,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,    52,
       0,    53,    54,    55,   525,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,    74,    75,
      76,    77,    78,    79,    80,    81,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,    84,    85,  1115,    86,    87,    88,    89,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,    52,     0,    53,    54,
      55,   525,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,    74,    75,    76,    77,    78,
      79,    80,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,     0,
      84,    85,     0,    86,    87,    88,    89,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,     0,     0,
       0,     0,     0,   130,     0,    53,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,     0,    84,    85,     0,
      86,    87,    88,    89,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,   128,    12,    13,    14,    15,
       0,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
       0,     0,     0,    41,    42,     0,     0,     0,     0,     0,
      46,     0,     0,     0,   129,     0,     0,     0,     0,     0,
     130,     0,     0,    54,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   321,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
       0,     0,   322,     0,     0,     0,   323,   324,     0,     0,
       0,     0,    83,     0,     0,     0,     0,    86,    87,    88,
      89,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,   128,    12,    13,    14,    15,     0,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
      41,    42,     0,     0,     0,     0,     0,    46,     0,     0,
       0,   129,     0,     0,     0,     0,     0,   130,     0,     0,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    73,    74,    75,    76,    77,
      78,    79,    80,    81,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   323,   324,     0,     0,     0,     0,    83,
       0,     0,     0,     0,    86,    87,    88,    89,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   128,
      12,    13,    14,    15,     0,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   130,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   687,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,    74,    75,    76,    77,    78,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     323,   324,     0,     0,     0,     0,    83,     0,     0,     0,
       0,    86,    87,    88,    89,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   128,    12,    13,    14,
      15,     0,     0,    17,   518,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   129,     0,     0,     0,     0,
       0,   130,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
      74,    75,    76,    77,    78,    79,    80,    81,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    86,    87,
      88,    89,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   128,    12,    13,    14,    15,     0,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   129,     0,     0,     0,     0,     0,   130,     0,
       0,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    73,    74,    75,    76,
      77,    78,    79,    80,    81,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,   647,     0,     0,     0,    86,    87,    88,    89,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     128,    12,    13,    14,    15,     0,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   811,    37,     0,     0,     0,    41,    42,
       0,     0,     0,     0,     0,    46,     0,     0,     0,   129,
       0,     0,     0,     0,     0,   130,     0,     0,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,    74,    75,    76,    77,    78,    79,
      80,    81,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,    86,    87,    88,    89,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   128,    12,    13,
      14,    15,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,    46,     0,     0,     0,   129,     0,     0,     0,
       0,     0,   130,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,   351,     0,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,    74,    75,    76,    77,    78,    79,    80,    81,     0,
     362,   363,   364,     0,     0,     0,   365,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,     0,     0,     0,     0,    86,
      87,    88,    89,   351,     0,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   351,     0,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   362,   363,   364,     0,
       0,     0,   365,     0,     0,     0,     0,     0,   362,   363,
     364,     0,     0,     0,   365,     0,     0,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,     0,     0,
       0,   351,     0,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   377,   378,     0,   379,   380,     0,     0,
       0,     0,     0,   524,   362,   363,   364,     0,     0,     0,
     365,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,     0,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   377,
     378,     0,   379,   380,     0,     0,     0,     0,     0,   613,
       0,   377,   378,     0,   379,   380,     0,     0,     0,     0,
       0,   616,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,     0,     0,     0,   351,     0,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   377,   378,     0,
     379,   380,     0,     0,     0,     0,     0,   624,   362,   363,
     364,     0,     0,     0,   365,     0,     0,     0,     0,     0,
       0,     0,   351,     0,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   351,     0,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   362,   363,   364,     0,     0,
       0,   365,     0,     0,     0,     0,     0,   362,   363,   364,
       0,     0,     0,   365,     0,     0,     0,     0,     0,     0,
       0,   351,     0,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,     0,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,     0,     0,     0,     0,
       0,     0,     0,     0,   362,   363,   364,     0,     0,     0,
     365,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   377,   378,     0,   379,   380,     0,     0,     0,     0,
       0,   627,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,     0,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   377,   378,
       0,   379,   380,     0,     0,     0,     0,     0,   632,     0,
     377,   378,     0,   379,   380,     0,     0,     0,     0,     0,
     633,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,     0,     0,     0,   351,     0,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   377,   378,     0,
     379,   380,     0,     0,     0,     0,     0,   656,   362,   363,
     364,     0,     0,     0,   365,     0,     0,     0,     0,     0,
       0,     0,   351,     0,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   351,     0,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   362,   363,   364,     0,     0,
       0,   365,     0,     0,     0,     0,     0,   362,   363,   364,
       0,     0,     0,   365,     0,     0,     0,     0,     0,     0,
       0,   351,     0,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,     0,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,     0,     0,     0,     0,
       0,     0,     0,     0,   362,   363,   364,     0,     0,     0,
     365,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   377,   378,     0,   379,   380,     0,     0,     0,     0,
       0,   671,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,     0,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   377,   378,
       0,   379,   380,     0,     0,     0,     0,     0,   780,     0,
     377,   378,     0,   379,   380,     0,     0,     0,     0,     0,
     781,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,     0,     0,     0,   351,     0,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   377,   378,     0,
     379,   380,     0,     0,     0,     0,     0,   796,   362,   363,
     364,     0,     0,     0,   365,     0,     0,     0,     0,     0,
     351,     0,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   362,   363,   364,     0,     0,     0,   365,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   351,     0,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   362,   363,   364,     0,
       0,     0,   365,     0,     0,     0,     0,     0,   351,   698,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     630,   377,   378,     0,   379,   380,     0,     0,     0,   842,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   362,   363,   364,     0,     0,     0,   365,     0,     0,
       0,     0,   534,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   377,   378,     0,   379,
     380,   351,     0,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   362,   363,   364,     0,     0,     0,
     365,     0,     0,     0,     0,     0,     0,     0,     0,   377,
     378,     0,   379,   380,     0,     0,     0,     0,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   351,
       0,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   351,     0,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   377,   378,     0,   379,   380,     0,
       0,     0,     0,   363,   364,     0,     0,     0,   365,     0,
       0,     0,     0,     0,     0,     0,   364,     0,     0,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   351,     0,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,     0,     0,   377,   378,     0,
     379,   380,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   365,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   365,     0,     0,     0,     0,     0,     0,     0,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
       0,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,     0,     0,   377,   378,     0,   379,   380,
       0,     0,     0,     0,     0,     0,     0,   377,   378,     0,
     379,   380,     0,     0,     0,     0,     0,     0,     0,   365,
       0,     0,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,     0,     0,     0,     0,     0,   377,   378,
       0,   379,   380,     0,     0,     0,     0,     0,     0,   378,
       0,   379,   380,     0,     0,     0,     0,     0,     0,   365,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   368,   369,   370,   371,   372,   373,   374,   375,
     376,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   378,     0,   379,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   368,   369,   370,   371,   372,   373,   374,   375,
     376,     0,     0,     0,     0,     0,   354,   355,   356,   357,
     358,   359,   360,   361,     0,     0,     0,     0,     0,     0,
       0,   209,     0,     0,     0,    15,     0,   378,     0,   379,
     380,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   365,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     128,    12,    13,    14,    15,     0,     0,    17,     0,     0,
       0,     0,     0,     0,  -562,  -562,  -562,  -562,   372,   373,
    -562,   375,   376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   354,   355,
     356,   357,   358,   359,   360,   361,     0,     0,     0,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147,
       0,   583,     0,     0,     0,   130,    89,     0,     0,     0,
       0,     0,     0,   365,     0,     0,     0,     0,     0,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   298,     0,     0,
       0,     0,   299,     0,    88,    89,   368,   369,   370,   371,
     372,   373,   374,   375,   376,     0,     0,     0,     0,     0,
       0,   456,    12,    13,    14,     0,     0,     0,     0,     0,
       0,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   378,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
    1040,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,    12,    13,    14,     0,     0,     0,     0,
       0,     0,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,  1041,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,  1040,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,     0,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   209,     0,     0,   210,     0,     0,     0,
       0,     0,     0,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,  1070,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,     0,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   209,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,     0,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,   269,     0,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,  1090,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,     0,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     902,   903,   904,   905,   906,   907,   908,   266,   267,   268,
     269,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287
};

static const yytype_int16 yycheck[] =
{
       2,     2,   112,    73,    74,     2,     2,    26,    73,    74,
     111,   112,    92,   488,   188,   749,   491,     6,   917,     6,
     495,     7,    27,   133,    26,    47,     4,     5,     6,     7,
       8,    26,   133,   293,   294,    31,     4,    49,   701,   299,
      18,    19,    90,    21,    22,    23,    24,    25,   628,    27,
       6,   131,    30,    72,   944,   505,   946,    39,    73,    74,
      38,    39,    44,   388,     7,   735,    44,     7,    46,   739,
     112,   151,    27,    23,    27,    44,    29,    72,    71,    72,
      92,   112,   383,    73,    74,    61,    23,    27,   165,    27,
      27,    92,   417,   418,   171,    92,     0,    75,    76,    77,
      78,    79,    80,    81,     5,    83,   154,   168,    86,   158,
     171,    26,    12,    13,    14,    15,    16,   129,    27,   131,
      29,   170,    23,    24,    25,    26,    62,   790,    27,   171,
      29,   201,    62,    48,   615,     6,   201,    27,    88,    29,
     171,     5,   170,   788,    44,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   170,   179,    23,
      24,    25,    26,   141,    27,   158,   188,   901,   136,   137,
      27,   149,    29,   178,    89,  1074,   158,   145,   146,   165,
     158,    82,   160,    23,   173,   172,   201,   165,   166,   167,
     158,   169,   170,   162,    27,   163,   831,     6,   729,   500,
     165,   102,   103,   156,   157,   173,   171,   175,   853,   164,
     165,   201,   693,   858,   170,   665,   170,  1107,   798,    65,
      66,   199,   165,   159,   202,   203,    27,   897,    29,   159,
      65,    66,   664,    71,    72,   173,   173,    27,   102,   103,
     178,   178,   170,   991,    82,   993,   316,   156,   157,   297,
     168,   316,    23,    62,   172,   325,   155,   156,   157,   170,
     325,   736,   162,   170,   899,   155,   156,   157,   177,   170,
     915,   531,   145,   146,   112,   449,   807,   170,   809,   711,
     581,    71,    72,    23,    24,    25,    26,    27,    71,   147,
      73,   392,   927,   156,   157,   940,   941,   942,   943,   156,
     157,   316,   175,    49,    50,    51,   170,    94,   147,   170,
     325,   786,   947,   171,   168,   293,   294,   171,   320,   320,
     298,   299,   155,   156,   157,   112,   316,   339,   174,    14,
      15,    16,   171,   634,  1082,   325,   811,   315,   813,   174,
     815,  1075,    82,   391,   322,   980,   365,   779,   170,   824,
     173,   347,  1100,  1101,    94,   156,   157,   170,   336,    44,
    1108,   662,   168,   411,  1112,   168,   172,   173,    23,   172,
     365,    26,   112,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   162,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   168,    64,    65,    66,   172,   390,   385,
     386,   389,   390,   170,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   404,   449,    23,   407,
     170,    26,   410,   534,  1097,   164,   165,     5,   178,   421,
     442,   442,   420,   421,   147,   464,   424,   527,   170,   750,
     365,    23,    24,    25,    26,    23,    24,    25,    26,   471,
     438,   170,   464,   723,    23,    24,   767,    26,   171,   464,
     385,   386,   387,   388,   147,   453,    71,   162,    73,   457,
     458,   170,   460,   461,   462,   463,   168,   465,   158,   508,
     172,   170,   470,   794,   170,   145,   146,   158,   171,   414,
      83,    84,   417,   418,   419,    88,   158,   485,   158,   158,
      82,    94,    71,   508,    73,   527,   776,    85,    86,    87,
      88,    49,    50,   173,   545,   175,   527,   170,   598,   630,
     527,   145,   146,   598,   102,   103,   451,    54,   586,   799,
     172,   589,   172,   168,   845,   593,   594,   172,   168,   464,
     172,   168,   172,   531,  1029,   172,   534,   172,   536,   173,
     630,   175,     4,   168,   542,   630,   544,   172,   172,   547,
      23,    24,    25,    26,    27,     4,    23,    24,    25,    26,
      71,    72,   652,   598,   145,   146,   171,   652,    12,    13,
      14,    15,    16,   174,   175,    23,    24,    25,    26,   900,
      23,    24,   170,    26,    23,   583,   145,   146,   598,    76,
      77,    13,   173,   591,   175,   630,   172,   173,   688,   158,
      44,    23,    23,   688,    23,    27,   836,   837,   624,    82,
      32,   627,   941,   942,   173,    82,   175,   652,    23,   658,
     630,   172,   661,   173,   663,   168,   168,   169,   626,   171,
     628,   629,    87,    88,   724,   102,   103,   168,   169,   724,
     171,   170,   652,   658,   666,   666,   661,   170,    65,   666,
     666,   649,   650,   688,   102,   103,   654,   755,    64,    65,
      66,   759,    79,   169,   168,    82,    83,    84,    85,    86,
      87,    88,    89,   714,   696,   696,   717,   699,   688,   701,
     176,   802,   170,   681,   784,   145,   146,   170,     6,   724,
     134,   135,   690,   172,  1015,   112,   171,   170,   158,   170,
     698,    23,   172,   170,  1025,   178,   172,   168,   173,     6,
     749,   727,   802,   173,   724,   175,   755,   802,   162,    27,
     759,     6,   170,     6,     6,   723,     6,     6,     6,    23,
      24,    25,    26,   731,   749,     6,  1057,   835,   836,   837,
     838,   145,   146,   741,   168,     6,   768,   768,     6,   788,
       6,   768,   768,   168,   158,   168,  1077,   174,  1079,    23,
      24,    25,    26,   804,   780,   172,   170,   802,   790,   173,
       6,   175,   770,   795,   795,   797,   797,    98,   776,    98,
     173,   171,    23,    24,    25,    26,    27,   803,    82,    30,
     171,   830,   802,   169,   169,    23,   835,   836,   837,   838,
     798,   799,    23,    65,   174,   170,   847,   829,   102,   103,
       4,   911,    23,   170,   853,   830,   170,    79,   170,   858,
      82,    83,    84,    85,    86,    87,    88,    89,   170,   851,
     169,   169,   173,   173,   875,    62,   171,   171,   102,   103,
     171,    82,    99,   173,     4,   170,   976,    88,    32,   169,
     112,   165,   893,   892,   171,   976,   878,   878,   171,   170,
     101,   102,   901,   104,   105,   106,   107,   108,   109,   110,
     111,   992,   170,   172,   170,     7,   915,   892,   168,   172,
     172,    62,   168,   881,   172,     7,   901,     6,   112,   173,
     888,   172,   890,   932,   171,   916,   172,   171,   920,   168,
     172,   940,   941,   942,   943,   170,   169,   174,   168,   931,
     931,    23,   174,   168,   168,    71,    72,   168,   959,   165,
     173,   171,   174,   164,   165,   173,    82,    83,    84,   170,
     952,   952,    88,   949,   175,   951,   177,   178,    94,    95,
      96,    97,  1063,   173,     6,   967,   967,   164,   970,   174,
       7,     7,   165,   171,   995,    79,   112,   979,   145,   146,
     172,  1002,   172,   961,   173,   171,   964,  1006,   171,   171,
     147,   158,   994,   147,  1013,   971,   174,   173,   147,   975,
     165,  1003,  1003,   170,   160,   313,   173,   170,   175,   317,
     172,  1006,   172,    27,    27,     4,     4,   172,  1013,   171,
       4,  1023,   173,   331,  1043,   333,   334,   335,    27,   168,
     173,  1033,  1033,  1035,  1035,   173,   158,  1013,   172,   172,
    1061,   172,    62,   158,  1065,  1021,  1067,    80,  1043,     4,
     171,   170,   172,   171,  1074,    47,  1075,   449,   188,   508,
    1038,   822,   168,   652,    98,   802,   951,  1043,   956,   888,
     851,   784,  1050,   915,   858,   759,  1054,   932,   454,  1055,
    1075,  1059,  1084,  1084,   650,  1087,  1087,  1089,  1089,   451,
    1111,   911,  1043,  1052,  1055,  1097,   439,   424,   151,  1063,
    1076,   386,     6,   536,   411,  1083,   654,    -1,    12,    13,
     365,  1113,  1113,    17,    18,    19,  1092,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    90,    91,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,   101,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,   170,    -1,   172,   173,
       6,   175,   176,   177,   178,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    44,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,   174,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      65,    -1,    78,    -1,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    88,    -1,    90,    91,    92,    82,    83,    84,
      85,    86,    87,    88,    89,   101,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    71,    -1,    73,
     131,   132,    -1,   134,   135,    -1,    -1,   112,    82,    83,
      84,    85,    86,    87,    88,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   170,    -1,   172,   173,     6,   175,
     176,   177,   178,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      88,    -1,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,
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
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    88,    -1,
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
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    88,    -1,    90,    91,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   101,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
     172,   173,     6,   175,   176,   177,   178,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,   174,    41,    42,    43,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    88,    -1,    -1,    91,    92,    -1,
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
      -1,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,
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
      -1,    88,    -1,    90,    91,    92,    93,    94,    95,    96,
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
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,
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
      83,    84,    -1,    -1,    -1,    88,    -1,    90,    91,    92,
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
      -1,    -1,    88,    -1,    90,    91,    92,    93,    94,    95,
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
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    88,
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
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    90,    91,
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
      -1,    -1,    -1,    88,    -1,    90,    91,    92,    93,    94,
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
      88,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
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
      81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,    90,
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
      84,    -1,    -1,    -1,    88,    -1,    90,    91,    92,    93,
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
      -1,    88,    -1,    90,    91,    92,    93,    94,    95,    96,
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
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    88,    -1,
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
      83,    84,    -1,    -1,    -1,    88,    -1,    90,    91,    92,
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
      -1,    -1,    88,    -1,    90,    91,    92,    93,    94,    95,
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
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    88,
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
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    90,    91,
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
      -1,    -1,    -1,    88,    -1,    90,    91,    92,    -1,    -1,
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
      88,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,    -1,
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
      -1,    82,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
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
      -1,    -1,    -1,    -1,    88,    -1,    -1,    91,    92,    -1,
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
      -1,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,    -1,
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
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    88,    -1,
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
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    91,    92,
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
      -1,    -1,    88,    -1,    -1,    91,    92,    -1,    -1,    -1,
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
     135,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    -1,    -1,    -1,    27,    -1,   162,    -1,   164,
     165,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    44,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,   129,   130,   131,   132,
     133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,
      -1,   173,    -1,    -1,    -1,    88,   178,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,
      -1,    -1,   175,    -1,   177,   178,   127,   128,   129,   130,
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
      83,    84,    88,    90,    91,    92,    93,    94,    95,    96,
      97,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   154,   170,   172,   173,   175,   176,   177,   178,
     189,   192,   193,   194,   195,   211,   220,   223,   226,   227,
     228,   230,   232,   247,   253,   254,   255,   256,   309,   310,
     311,   312,   313,   321,   323,   328,   329,   330,   331,   333,
     334,   335,   336,   337,   338,   339,   340,   351,    23,    82,
      88,   193,   310,   313,   310,   310,   310,   310,     6,   310,
     310,   170,   310,   310,   310,   310,   310,    82,    94,   170,
     189,   193,   307,   323,   324,   339,   341,   310,   170,   325,
     170,   303,   304,   310,   211,   170,   170,   170,   170,   170,
     170,   310,   332,   332,    23,    23,   208,   302,   332,   173,
     310,    23,    24,    26,    71,    73,   187,   188,   198,   200,
     204,   207,   278,   279,   339,    27,   280,   281,   311,   170,
     275,   170,   170,   170,   170,   225,   229,   231,   233,    23,
      26,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   181,   182,
     184,   186,   197,   170,   170,   190,   191,   323,   170,   175,
     328,   330,   331,   338,   338,   310,   310,   310,   310,   310,
     310,   310,    27,    29,   155,   156,   157,   348,   349,   310,
     209,   101,   160,   164,   165,   183,   310,   344,   345,   346,
     347,    29,   326,   348,    29,   348,   173,   339,   275,    82,
      88,   192,   194,   311,    94,   227,    49,    50,    49,    50,
      51,     5,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    38,    39,    40,    44,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   161,   162,   164,
     165,   172,   183,   317,   317,   158,   158,   145,   146,   173,
     175,   322,     4,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   136,   137,   163,   317,   310,
     147,   308,   310,   307,   158,   275,   327,   145,   146,   158,
     173,   175,   332,   310,   168,   172,    54,   310,   305,   306,
     310,   310,   208,   310,   310,   172,   172,   172,     4,   168,
     172,   172,   209,    62,   159,   188,   199,   204,   172,   168,
     172,   168,   172,     4,   168,   172,    23,    33,    34,    35,
      36,    37,    41,    42,    45,    46,    47,    67,    71,    72,
      78,    82,    88,    91,    92,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   160,   171,   184,   276,   277,
     310,   217,   218,   338,   310,   352,   353,   310,   171,    23,
      23,    23,    23,   172,   196,   173,   344,   344,   168,   201,
     275,   310,   344,   145,   146,   175,   155,   349,    31,   310,
     338,    29,   155,   349,   171,    93,   174,   193,   194,   210,
     211,   170,   310,   338,   147,   169,   168,   176,   177,   310,
     311,   224,   170,   211,   170,     6,   172,     6,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   324,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   314,    23,
      88,   219,   314,   173,   184,   339,   342,   184,   339,   342,
      23,   173,   339,   343,   343,   310,   332,   275,   183,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   171,   310,   327,   171,   339,   343,   343,
     339,   310,   332,   171,   171,   304,   170,   171,   172,   168,
      62,   171,   171,   171,   310,   302,   174,    23,   173,   159,
     172,   172,   188,   207,   279,   310,   281,   171,   310,     6,
     168,   201,   168,   201,   168,   201,   171,   172,    98,   237,
     314,    98,   238,     6,   234,   173,   185,   171,   171,   190,
     169,   171,   169,    23,    23,    13,    23,    27,    32,   350,
     174,   175,   174,   174,   170,   194,   344,   101,   183,   310,
       4,   345,   174,    23,   310,   310,   209,   310,     6,   170,
     314,   170,   310,   275,   275,   310,   275,   275,   174,   169,
     338,   237,   174,   169,     6,   211,   310,     6,   211,   252,
     305,   310,   101,   175,   183,   240,   338,   212,     6,   173,
     244,   173,   314,   213,   187,   198,   202,   205,   206,   173,
     310,   160,   277,   171,   218,   171,   353,   171,   323,    99,
     239,   173,   285,   323,   314,     5,    82,   102,   103,   170,
     189,   264,   265,   266,   267,   268,   270,   239,   185,   174,
       4,    32,   169,   310,   171,   171,   170,   338,   310,   237,
     171,   171,    51,   310,   193,   257,   258,   259,   260,   262,
     170,   257,   174,   174,   239,   209,   171,   209,   172,   170,
     344,   338,   147,   171,     6,   211,   243,   172,   245,   172,
     245,    66,   248,   249,   250,   251,   310,    76,    77,   216,
      62,   206,   168,   201,   203,   206,   172,   285,   314,   282,
     168,   173,   265,   265,   268,   165,     7,     7,   165,   314,
     174,   310,   169,   172,   344,   239,   211,     6,   172,   262,
     171,   168,   201,     5,    85,    86,    87,    88,   170,   261,
     263,   269,   270,   271,   272,   273,   257,   171,   314,   172,
      55,   305,   344,   169,   240,     6,   211,   242,   209,   245,
      64,    65,    66,   245,   174,   168,   201,   174,   168,   201,
     168,   201,   170,   173,    23,   205,   174,   168,   201,   173,
      65,    79,    82,    83,    84,    85,    86,    87,    88,    89,
     174,   193,   235,   283,   284,   294,   295,   296,   297,   323,
     282,   171,   265,   265,   266,   266,   265,   173,   174,   171,
     314,   209,     6,   274,   259,   270,   270,   273,   164,   221,
     165,     7,     7,   165,   171,    79,   318,   173,   172,   171,
     171,   171,   209,    61,    64,   172,   310,     6,   172,   246,
     174,   147,   250,   310,   147,   214,   323,   209,   206,   174,
     282,   314,   285,   283,   263,    73,   313,   297,   174,   282,
     173,   264,   147,   171,   160,   222,   270,   270,   271,   271,
     270,   274,   170,   274,   282,     6,   211,   241,   242,    59,
     172,   172,   246,   209,   310,   310,     7,    27,   215,   174,
     174,   184,   172,   173,   286,    27,   298,   299,   184,   300,
     301,   317,   174,   282,   315,    27,   315,    27,   183,   319,
     320,   315,   174,   209,   172,   209,   323,   171,     4,   236,
      82,   174,   184,   287,   288,   289,   290,   291,   292,   323,
       4,   314,   168,   172,     4,   168,   172,   184,   174,   316,
     314,   173,    27,   168,   201,   173,    57,   173,   310,   172,
     174,   288,   172,   172,    62,    80,   158,   310,   299,   310,
     301,   314,   310,     4,   209,   320,   171,   209,   172,   209,
      23,   181,   297,   285,   184,   314,   314,   170,   315,   310,
     174,   174,   174,   184,   257,   315,   315,   171,   274,   315,
     172,   173,   293,   209,   315,   174
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
     216,   217,   217,   218,   219,   219,   220,   221,   221,   222,
     222,   224,   223,   225,   223,   226,   226,   227,   227,   227,
     229,   228,   231,   230,   233,   232,   234,   234,   235,   236,
     236,   237,   237,   238,   238,   239,   239,   240,   240,   240,
     240,   241,   241,   242,   242,   243,   243,   244,   244,   244,
     244,   245,   245,   245,   246,   246,   247,   248,   248,   249,
     249,   250,   250,   251,   251,   252,   252,   253,   253,   254,
     254,   255,   255,   256,   256,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   261,   261,   262,   262,   263,
     263,   264,   264,   264,   264,   265,   265,   266,   266,   267,
     267,   268,   268,   269,   269,   269,   269,   270,   270,   270,
     271,   271,   272,   272,   273,   273,   274,   274,   275,   275,
     275,   276,   276,   277,   277,   277,   278,   278,   279,   280,
     280,   281,   281,   282,   282,   283,   283,   283,   283,   284,
     284,   284,   285,   285,   286,   286,   286,   287,   287,   288,
     288,   289,   290,   290,   290,   290,   291,   291,   292,   293,
     293,   294,   294,   295,   295,   296,   296,   297,   297,   297,
     297,   297,   297,   297,   298,   298,   299,   299,   300,   300,
     301,   302,   303,   303,   304,   305,   305,   306,   306,   308,
     307,   309,   309,   309,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   311,   311,   312,
     313,   314,   315,   316,   317,   317,   318,   318,   319,   319,
     320,   320,   321,   321,   321,   321,   322,   321,   323,   323,
     324,   324,   324,   325,   325,   326,   326,   326,   327,   327,
     328,   328,   328,   328,   329,   329,   329,   329,   329,   329,
     329,   329,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   331,   331,   332,   332,   333,   334,   334,   334,   334,
     335,   335,   336,   336,   336,   337,   337,   337,   337,   337,
     337,   338,   338,   338,   338,   339,   339,   339,   340,   340,
     341,   341,   341,   341,   341,   341,   341,   342,   342,   342,
     343,   343,   343,   344,   345,   345,   346,   346,   347,   347,
     347,   347,   347,   347,   347,   348,   348,   348,   348,   349,
     349,   349,   349,   349,   349,   349,   349,   350,   350,   350,
     350,   351,   351,   351,   351,   351,   351,   351,   352,   352,
     353
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
       4,     1,     3,     1,     1,     1,    13,     0,     1,     0,
       1,     0,    10,     0,     9,     1,     2,     1,     1,     1,
       0,     7,     0,     8,     0,     9,     0,     2,     5,     0,
       2,     0,     2,     0,     2,     0,     2,     1,     2,     4,
       3,     1,     4,     1,     4,     1,     4,     3,     4,     4,
       5,     0,     5,     4,     1,     1,     7,     0,     2,     1,
       3,     4,     4,     1,     3,     1,     4,     5,     6,     1,
       3,     6,     7,     3,     6,     2,     0,     1,     3,     2,
       1,     0,     2,     1,     1,     1,     1,     6,     8,     0,
       1,     1,     2,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     1,     2,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     0,     2,     2,     4,
       3,     1,     3,     1,     3,     2,     3,     1,     1,     3,
       1,     1,     3,     2,     0,     4,     4,    12,     1,     1,
       2,     3,     1,     3,     1,     2,     3,     1,     2,     2,
       2,     3,     3,     3,     4,     3,     1,     1,     3,     1,
       3,     1,     1,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     4,     3,     1,
       4,     4,     3,     1,     1,     0,     1,     3,     1,     0,
       9,     3,     2,     3,     1,     6,     5,     3,     4,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     5,     4,     3,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     3,     2,     1,     2,     4,
       2,     2,     1,     2,     2,     3,     1,    13,    12,     1,
       1,     0,     0,     0,     0,     1,     0,     5,     3,     1,
       1,     2,     2,     2,     4,     4,     0,     3,     1,     1,
       1,     1,     3,     0,     3,     0,     1,     1,     0,     1,
       4,     3,     1,     3,     1,     1,     3,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     3,     1,     1,     4,     4,     4,     4,
       1,     1,     1,     3,     3,     1,     4,     2,     3,     3,
       1,     4,     4,     3,     3,     3,     3,     1,     3,     1,
       1,     3,     1,     1,     0,     1,     3,     1,     3,     1,
       4,     2,     2,     6,     4,     2,     2,     1,     2,     1,
       4,     3,     3,     3,     3,     6,     3,     1,     1,     2,
       1,     5,     4,     2,     2,     4,     2,     2,     1,     3,
       1
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

    case YYSYMBOL_function_name: /* function_name  */
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

    case YYSYMBOL_union_type_element: /* union_type_element  */
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

    case YYSYMBOL_union_type_without_static_element: /* union_type_without_static_element  */
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
                                { CG(ast) = (yyvsp[0].ast); (void) zendnerrs; }
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

  case 184: /* function_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 185: /* function_name: "'readonly'"  */
                           {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 186: /* function_declaration_statement: function returns_ref function_name backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 187: /* is_reference: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 188: /* is_reference: "'&'"  */
                                                        { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 189: /* is_variadic: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 190: /* is_variadic: "'...'"  */
                            { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 191: /* @4: %empty  */
                                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 192: /* class_declaration_statement: class_modifiers "'class'" @4 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 193: /* @5: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 194: /* class_declaration_statement: "'class'" @5 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 195: /* class_modifiers: class_modifier  */
                                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 196: /* class_modifiers: class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 197: /* class_modifier: "'abstract'"  */
                                        { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 198: /* class_modifier: "'final'"  */
                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 199: /* class_modifier: "'readonly'"  */
                                        { (yyval.num) = ZEND_ACC_READONLY_CLASS|ZEND_ACC_NO_DYNAMIC_PROPERTIES; }
    break;

  case 200: /* @6: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 201: /* trait_declaration_statement: "'trait'" @6 "identifier" backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 202: /* @7: %empty  */
                            { (yyval.num) = CG(zend_lineno); }
    break;

  case 203: /* interface_declaration_statement: "'interface'" @7 "identifier" interface_extends_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 204: /* @8: %empty  */
                       { (yyval.num) = CG(zend_lineno); }
    break;

  case 205: /* enum_declaration_statement: "'enum'" @8 "identifier" enum_backing_type implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }
    break;

  case 206: /* enum_backing_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 207: /* enum_backing_type: ':' type_expr  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 208: /* enum_case: "'case'" backup_doc_comment identifier enum_case_expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }
    break;

  case 209: /* enum_case_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 210: /* enum_case_expr: '=' expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 211: /* extends_from: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 212: /* extends_from: "'extends'" class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 213: /* interface_extends_list: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 214: /* interface_extends_list: "'extends'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 215: /* implements_list: %empty  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 216: /* implements_list: "'implements'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 217: /* foreach_variable: variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 218: /* foreach_variable: ampersand variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }
    break;

  case 219: /* foreach_variable: "'list'" '(' array_pair_list ')'  */
                                               { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 220: /* foreach_variable: '[' array_pair_list ']'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 221: /* for_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 222: /* for_statement: ':' inner_statement_list "'endfor'" ';'  */
                                                      { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 223: /* foreach_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 224: /* foreach_statement: ':' inner_statement_list "'endforeach'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 225: /* declare_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 226: /* declare_statement: ':' inner_statement_list "'enddeclare'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 227: /* switch_case_list: '{' case_list '}'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 228: /* switch_case_list: '{' ';' case_list '}'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 229: /* switch_case_list: ':' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 230: /* switch_case_list: ':' ';' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 231: /* case_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 232: /* case_list: case_list "'case'" expr case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 233: /* case_list: case_list "'default'" case_separator inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }
    break;

  case 236: /* match: "'match'" '(' expr ')' '{' match_arm_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 237: /* match_arm_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }
    break;

  case 238: /* match_arm_list: non_empty_match_arm_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 239: /* non_empty_match_arm_list: match_arm  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }
    break;

  case 240: /* non_empty_match_arm_list: non_empty_match_arm_list ',' match_arm  */
                                                       { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 241: /* match_arm: match_arm_cond_list possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 242: /* match_arm: "'default'" possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }
    break;

  case 243: /* match_arm_cond_list: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 244: /* match_arm_cond_list: match_arm_cond_list ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 245: /* while_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 246: /* while_statement: ':' inner_statement_list "'endwhile'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 247: /* if_stmt_without_else: "'if'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 248: /* if_stmt_without_else: if_stmt_without_else "'elseif'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 249: /* if_stmt: if_stmt_without_else  */
                                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 250: /* if_stmt: if_stmt_without_else "'else'" statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }
    break;

  case 251: /* alt_if_stmt_without_else: "'if'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 252: /* alt_if_stmt_without_else: alt_if_stmt_without_else "'elseif'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 253: /* alt_if_stmt: alt_if_stmt_without_else "'endif'" ';'  */
                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 254: /* alt_if_stmt: alt_if_stmt_without_else "'else'" ':' inner_statement_list "'endif'" ';'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }
    break;

  case 255: /* parameter_list: non_empty_parameter_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 256: /* parameter_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 257: /* non_empty_parameter_list: attributed_parameter  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }
    break;

  case 258: /* non_empty_parameter_list: non_empty_parameter_list ',' attributed_parameter  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 259: /* attributed_parameter: attributes parameter  */
                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 260: /* attributed_parameter: parameter  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 261: /* optional_property_modifiers: %empty  */
                                                        { (yyval.num) = 0; }
    break;

  case 262: /* optional_property_modifiers: optional_property_modifiers property_modifier  */
                        { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 263: /* property_modifier: "'public'"  */
                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 264: /* property_modifier: "'protected'"  */
                                                        { (yyval.num) = ZEND_ACC_PROTECTED; }
    break;

  case 265: /* property_modifier: "'private'"  */
                                                        { (yyval.num) = ZEND_ACC_PRIVATE; }
    break;

  case 266: /* property_modifier: "'readonly'"  */
                                                        { (yyval.num) = ZEND_ACC_READONLY; }
    break;

  case 267: /* parameter: optional_property_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-5].num) | (yyvsp[-3].num) | (yyvsp[-2].num), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL,
					NULL, (yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL); }
    break;

  case 268: /* parameter: optional_property_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment '=' expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-7].num) | (yyvsp[-5].num) | (yyvsp[-4].num), (yyvsp[-6].ast), (yyvsp[-3].ast), (yyvsp[0].ast),
					NULL, (yyvsp[-2].str) ? zend_ast_create_zval_from_str((yyvsp[-2].str)) : NULL); }
    break;

  case 269: /* optional_type_without_static: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 270: /* optional_type_without_static: type_expr_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 271: /* type_expr: type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 272: /* type_expr: '?' type  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 273: /* type_expr: union_type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 274: /* type_expr: intersection_type  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 275: /* type: type_without_static  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 276: /* type: "'static'"  */
                                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }
    break;

  case 277: /* union_type_element: type  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 278: /* union_type_element: '(' intersection_type ')'  */
                                           { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 279: /* union_type: union_type_element '|' union_type_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 280: /* union_type: union_type '|' union_type_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 281: /* intersection_type: type "amp" type  */
                                                                          { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 282: /* intersection_type: intersection_type "amp" type  */
                                                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 283: /* type_expr_without_static: type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 284: /* type_expr_without_static: '?' type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 285: /* type_expr_without_static: union_type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 286: /* type_expr_without_static: intersection_type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 287: /* type_without_static: "'array'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 288: /* type_without_static: "'callable'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 289: /* type_without_static: name  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 290: /* union_type_without_static_element: type_without_static  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 291: /* union_type_without_static_element: '(' intersection_type_without_static ')'  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 292: /* union_type_without_static: union_type_without_static_element '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 293: /* union_type_without_static: union_type_without_static '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 294: /* intersection_type_without_static: type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 295: /* intersection_type_without_static: intersection_type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 296: /* return_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 297: /* return_type: ':' type_expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 298: /* argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 299: /* argument_list: '(' non_empty_argument_list possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 300: /* argument_list: '(' "'...'" ')'  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_CALLABLE_CONVERT); }
    break;

  case 301: /* non_empty_argument_list: argument  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 302: /* non_empty_argument_list: non_empty_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 303: /* argument: expr  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 304: /* argument: identifier ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 305: /* argument: "'...'" expr  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 306: /* global_var_list: global_var_list ',' global_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 307: /* global_var_list: global_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 308: /* global_var: simple_variable  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }
    break;

  case 309: /* static_var_list: static_var_list ',' static_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 310: /* static_var_list: static_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 311: /* static_var: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }
    break;

  case 312: /* static_var: "variable" '=' expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 313: /* class_statement_list: class_statement_list class_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 314: /* class_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 315: /* attributed_class_statement: variable_modifiers optional_type_without_static property_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 316: /* attributed_class_statement: method_modifiers "'const'" class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 317: /* attributed_class_statement: method_modifiers function returns_ref identifier backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags method_body backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 318: /* attributed_class_statement: enum_case  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 319: /* class_statement: attributed_class_statement  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 320: /* class_statement: attributes attributed_class_statement  */
                                                      { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 321: /* class_statement: "'use'" class_name_list trait_adaptations  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 322: /* class_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 323: /* class_name_list: class_name_list ',' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 324: /* trait_adaptations: ';'  */
                                                                                { (yyval.ast) = NULL; }
    break;

  case 325: /* trait_adaptations: '{' '}'  */
                                                                        { (yyval.ast) = NULL; }
    break;

  case 326: /* trait_adaptations: '{' trait_adaptation_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 327: /* trait_adaptation_list: trait_adaptation  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }
    break;

  case 328: /* trait_adaptation_list: trait_adaptation_list trait_adaptation  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 329: /* trait_adaptation: trait_precedence ';'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 330: /* trait_adaptation: trait_alias ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 331: /* trait_precedence: absolute_trait_method_reference "'insteadof'" class_name_list  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 332: /* trait_alias: trait_method_reference "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 333: /* trait_alias: trait_method_reference "'as'" reserved_non_modifiers  */
                        { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }
    break;

  case 334: /* trait_alias: trait_method_reference "'as'" member_modifier identifier  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[-1].num), (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 335: /* trait_alias: trait_method_reference "'as'" member_modifier  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[0].num), (yyvsp[-2].ast), NULL); }
    break;

  case 336: /* trait_method_reference: identifier  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }
    break;

  case 337: /* trait_method_reference: absolute_trait_method_reference  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 338: /* absolute_trait_method_reference: class_name "'::'" identifier  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 339: /* method_body: ';'  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 340: /* method_body: '{' inner_statement_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 341: /* variable_modifiers: non_empty_member_modifiers  */
                                                        { (yyval.num) = (yyvsp[0].num); }
    break;

  case 342: /* variable_modifiers: "'var'"  */
                                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 343: /* method_modifiers: %empty  */
                                                                { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 344: /* method_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = (yyvsp[0].num); if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 345: /* non_empty_member_modifiers: member_modifier  */
                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 346: /* non_empty_member_modifiers: non_empty_member_modifiers member_modifier  */
                        { (yyval.num) = zend_add_member_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 347: /* member_modifier: "'public'"  */
                                                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 348: /* member_modifier: "'protected'"  */
                                                        { (yyval.num) = ZEND_ACC_PROTECTED; }
    break;

  case 349: /* member_modifier: "'private'"  */
                                                        { (yyval.num) = ZEND_ACC_PRIVATE; }
    break;

  case 350: /* member_modifier: "'static'"  */
                                                        { (yyval.num) = ZEND_ACC_STATIC; }
    break;

  case 351: /* member_modifier: "'abstract'"  */
                                                        { (yyval.num) = ZEND_ACC_ABSTRACT; }
    break;

  case 352: /* member_modifier: "'final'"  */
                                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 353: /* member_modifier: "'readonly'"  */
                                                        { (yyval.num) = ZEND_ACC_READONLY; }
    break;

  case 354: /* property_list: property_list ',' property  */
                                           { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 355: /* property_list: property  */
                         { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }
    break;

  case 356: /* property: "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 357: /* property: "variable" '=' expr backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 358: /* class_const_list: class_const_list ',' class_const_decl  */
                                                      { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 359: /* class_const_list: class_const_decl  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 360: /* class_const_decl: identifier '=' expr backup_doc_comment  */
                                               { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 361: /* const_decl: "identifier" '=' expr backup_doc_comment  */
                                             { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 362: /* echo_expr_list: echo_expr_list ',' echo_expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 363: /* echo_expr_list: echo_expr  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 364: /* echo_expr: expr  */
             { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 365: /* for_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 366: /* for_exprs: non_empty_for_exprs  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 367: /* non_empty_for_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 368: /* non_empty_for_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 369: /* @9: %empty  */
                { (yyval.num) = CG(zend_lineno); }
    break;

  case 370: /* anonymous_class: "'class'" @9 ctor_arguments extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                                                                                             {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS, (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}
    break;

  case 371: /* new_expr: "'new'" class_name_reference ctor_arguments  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 372: /* new_expr: "'new'" anonymous_class  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 373: /* new_expr: "'new'" attributes anonymous_class  */
                        { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 374: /* expr: variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 375: /* expr: "'list'" '(' array_pair_list ')' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 376: /* expr: '[' array_pair_list ']' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 377: /* expr: variable '=' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 378: /* expr: variable '=' ampersand variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 379: /* expr: "'clone'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[0].ast)); }
    break;

  case 380: /* expr: variable "'+='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 381: /* expr: variable "'-='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 382: /* expr: variable "'*='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 383: /* expr: variable "'**='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 384: /* expr: variable "'/='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 385: /* expr: variable "'.='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 386: /* expr: variable "'%='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 387: /* expr: variable "'&='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 388: /* expr: variable "'|='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 389: /* expr: variable "'^='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 390: /* expr: variable "'<<='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 391: /* expr: variable "'>>='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 392: /* expr: variable "'??='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 393: /* expr: variable "'++'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }
    break;

  case 394: /* expr: "'++'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }
    break;

  case 395: /* expr: variable "'--'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }
    break;

  case 396: /* expr: "'--'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }
    break;

  case 397: /* expr: expr "'||'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 398: /* expr: expr "'&&'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 399: /* expr: expr "'or'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 400: /* expr: expr "'and'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 401: /* expr: expr "'xor'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 402: /* expr: expr '|' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 403: /* expr: expr "amp" expr  */
                                                                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 404: /* expr: expr "'&'" expr  */
                                                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 405: /* expr: expr '^' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 406: /* expr: expr '.' expr  */
                                { (yyval.ast) = zend_ast_create_concat_op((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 407: /* expr: expr '+' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 408: /* expr: expr '-' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 409: /* expr: expr '*' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 410: /* expr: expr "'**'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 411: /* expr: expr '/' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 412: /* expr: expr '%' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 413: /* expr: expr "'<<'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 414: /* expr: expr "'>>'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 415: /* expr: '+' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }
    break;

  case 416: /* expr: '-' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }
    break;

  case 417: /* expr: '!' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }
    break;

  case 418: /* expr: '~' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }
    break;

  case 419: /* expr: expr "'==='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 420: /* expr: expr "'!=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 421: /* expr: expr "'=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 422: /* expr: expr "'!='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 423: /* expr: expr '<' expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 424: /* expr: expr "'<='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 425: /* expr: expr '>' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 426: /* expr: expr "'>='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 427: /* expr: expr "'<=>'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 428: /* expr: expr "'instanceof'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 429: /* expr: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
		}
    break;

  case 430: /* expr: new_expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 431: /* expr: expr '?' expr ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 432: /* expr: expr '?' ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 433: /* expr: expr "'??'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 434: /* expr: internal_functions_in_yacc  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 435: /* expr: "'(int)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }
    break;

  case 436: /* expr: "'(double)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }
    break;

  case 437: /* expr: "'(string)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }
    break;

  case 438: /* expr: "'(array)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }
    break;

  case 439: /* expr: "'(object)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }
    break;

  case 440: /* expr: "'(bool)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }
    break;

  case 441: /* expr: "'(unset)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }
    break;

  case 442: /* expr: "'exit'" exit_expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[0].ast)); }
    break;

  case 443: /* expr: '@' expr  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }
    break;

  case 444: /* expr: scalar  */
                       { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 445: /* expr: '`' backticks_expr '`'  */
                                       { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }
    break;

  case 446: /* expr: "'print'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }
    break;

  case 447: /* expr: "'yield'"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 448: /* expr: "'yield'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 449: /* expr: "'yield'" expr "'=>'" expr  */
                                                 { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 450: /* expr: "'yield from'" expr  */
                                  { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 451: /* expr: "'throw'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }
    break;

  case 452: /* expr: inline_function  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 453: /* expr: attributes inline_function  */
                                           { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 454: /* expr: "'static'" inline_function  */
                                         { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 455: /* expr: attributes "'static'" inline_function  */
                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 456: /* expr: match  */
                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 457: /* inline_function: function returns_ref backup_doc_comment '(' parameter_list ')' lexical_vars return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 458: /* inline_function: fn returns_ref backup_doc_comment '(' parameter_list ')' return_type "'=>'" backup_fn_flags backup_lex_pos expr backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-10].num) | (yyvsp[0].num), (yyvsp[-11].num), (yyvsp[-9].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0), (yyvsp[-7].ast), NULL, (yyvsp[-1].ast), (yyvsp[-5].ast), NULL);
				  ((zend_ast_decl *) (yyval.ast))->lex_pos = (yyvsp[-2].ptr);
				  CG(extra_fn_flags) = (yyvsp[-3].num); }
    break;

  case 459: /* fn: "'fn'"  */
             { (yyval.num) = CG(zend_lineno); }
    break;

  case 460: /* function: "'function'"  */
                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 461: /* backup_doc_comment: %empty  */
               { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 462: /* backup_fn_flags: %empty  */
                                         { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 463: /* backup_lex_pos: %empty  */
               { (yyval.ptr) = LANG_SCNG(yy_text); }
    break;

  case 464: /* returns_ref: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 465: /* returns_ref: ampersand  */
                                { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 466: /* lexical_vars: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 467: /* lexical_vars: "'use'" '(' lexical_var_list possible_comma ')'  */
                                                              { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 468: /* lexical_var_list: lexical_var_list ',' lexical_var  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 469: /* lexical_var_list: lexical_var  */
                            { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }
    break;

  case 470: /* lexical_var: "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 471: /* lexical_var: ampersand "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }
    break;

  case 472: /* function_call: name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 473: /* function_call: "'readonly'" argument_list  */
                                         {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-1].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, zend_ast_create_zval(&zv), (yyvsp[0].ast));
		}
    break;

  case 474: /* function_call: class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 475: /* function_call: variable_class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 476: /* @10: %empty  */
                              { (yyval.num) = CG(zend_lineno); }
    break;

  case 477: /* function_call: callable_expr @10 argument_list  */
                                                                           { 
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-2].ast), (yyvsp[0].ast)); 
			(yyval.ast)->lineno = (yyvsp[-1].num);
		}
    break;

  case 478: /* class_name: "'static'"  */
                        { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 479: /* class_name: name  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 480: /* class_name_reference: class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 481: /* class_name_reference: new_variable  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 482: /* class_name_reference: '(' expr ')'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 483: /* exit_expr: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 484: /* exit_expr: '(' optional_expr ')'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 485: /* backticks_expr: %empty  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 486: /* backticks_expr: "string content"  */
                                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 487: /* backticks_expr: encaps_list  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 488: /* ctor_arguments: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 489: /* ctor_arguments: argument_list  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 490: /* dereferenceable_scalar: "'array'" '(' array_pair_list ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 491: /* dereferenceable_scalar: '[' array_pair_list ']'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 492: /* dereferenceable_scalar: "quoted string"  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 493: /* dereferenceable_scalar: '"' encaps_list '"'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 494: /* scalar: "integer"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 495: /* scalar: "floating-point number"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 496: /* scalar: "heredoc start" "string content" "heredoc end"  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 497: /* scalar: "heredoc start" "heredoc end"  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 498: /* scalar: "heredoc start" encaps_list "heredoc end"  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 499: /* scalar: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 500: /* scalar: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 501: /* scalar: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 502: /* constant: name  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }
    break;

  case 503: /* constant: "'__LINE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 504: /* constant: "'__FILE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 505: /* constant: "'__DIR__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 506: /* constant: "'__TRAIT__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 507: /* constant: "'__METHOD__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 508: /* constant: "'__FUNCTION__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 509: /* constant: "'__NAMESPACE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 510: /* constant: "'__CLASS__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 511: /* class_constant: class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 512: /* class_constant: variable_class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 513: /* optional_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 514: /* optional_expr: expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 515: /* variable_class_name: fully_dereferenceable  */
                                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 516: /* fully_dereferenceable: variable  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 517: /* fully_dereferenceable: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 518: /* fully_dereferenceable: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 519: /* fully_dereferenceable: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 520: /* array_object_dereferenceable: fully_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 521: /* array_object_dereferenceable: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 522: /* callable_expr: callable_variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 523: /* callable_expr: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 524: /* callable_expr: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 525: /* callable_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 526: /* callable_variable: array_object_dereferenceable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 527: /* callable_variable: array_object_dereferenceable '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 528: /* callable_variable: array_object_dereferenceable "'->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 529: /* callable_variable: array_object_dereferenceable "'?->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 530: /* callable_variable: function_call  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 531: /* variable: callable_variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 532: /* variable: static_member  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 533: /* variable: array_object_dereferenceable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 534: /* variable: array_object_dereferenceable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 535: /* simple_variable: "variable"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 536: /* simple_variable: '$' '{' expr '}'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 537: /* simple_variable: '$' simple_variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 538: /* static_member: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 539: /* static_member: variable_class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 540: /* new_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 541: /* new_variable: new_variable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 542: /* new_variable: new_variable '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_DIM_ALTERNATIVE_SYNTAX, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 543: /* new_variable: new_variable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 544: /* new_variable: new_variable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 545: /* new_variable: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 546: /* new_variable: new_variable "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 547: /* member_name: identifier  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 548: /* member_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 549: /* member_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 550: /* property_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 551: /* property_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 552: /* property_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 553: /* array_pair_list: non_empty_array_pair_list  */
                        { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }
    break;

  case 554: /* possible_array_pair: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 555: /* possible_array_pair: array_pair  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 556: /* non_empty_array_pair_list: non_empty_array_pair_list ',' possible_array_pair  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 557: /* non_empty_array_pair_list: possible_array_pair  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }
    break;

  case 558: /* array_pair: expr "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }
    break;

  case 559: /* array_pair: expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 560: /* array_pair: expr "'=>'" ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }
    break;

  case 561: /* array_pair: ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }
    break;

  case 562: /* array_pair: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 563: /* array_pair: expr "'=>'" "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }
    break;

  case 564: /* array_pair: "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }
    break;

  case 565: /* encaps_list: encaps_list encaps_var  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 566: /* encaps_list: encaps_list "string content"  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 567: /* encaps_list: encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }
    break;

  case 568: /* encaps_list: "string content" encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 569: /* encaps_var: "variable"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 570: /* encaps_var: "variable" '[' encaps_var_offset ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }
    break;

  case 571: /* encaps_var: "variable" "'->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 572: /* encaps_var: "variable" "'?->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 573: /* encaps_var: "'${'" expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR, (yyvsp[-1].ast)); }
    break;

  case 574: /* encaps_var: "'${'" "variable name" '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY, (yyvsp[-1].ast)); }
    break;

  case 575: /* encaps_var: "'${'" "variable name" '[' expr ']' '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_ENCAPS_VAR_DOLLAR_CURLY,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }
    break;

  case 576: /* encaps_var: "'{$'" variable '}'  */
                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 577: /* encaps_var_offset: "identifier"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 578: /* encaps_var_offset: "number"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 579: /* encaps_var_offset: '-' "number"  */
                                        { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }
    break;

  case 580: /* encaps_var_offset: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 581: /* internal_functions_in_yacc: "'isset'" '(' isset_variables possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 582: /* internal_functions_in_yacc: "'empty'" '(' expr ')'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }
    break;

  case 583: /* internal_functions_in_yacc: "'include'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }
    break;

  case 584: /* internal_functions_in_yacc: "'include_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }
    break;

  case 585: /* internal_functions_in_yacc: "'eval'" '(' expr ')'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }
    break;

  case 586: /* internal_functions_in_yacc: "'require'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }
    break;

  case 587: /* internal_functions_in_yacc: "'require_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }
    break;

  case 588: /* isset_variables: isset_variable  */
                               { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 589: /* isset_variables: isset_variables ',' isset_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 590: /* isset_variable: expr  */
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
