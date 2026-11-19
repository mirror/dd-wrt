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
  YYSYMBOL_T_PRIVATE_SET = 88,             /* "'private(set)'"  */
  YYSYMBOL_T_PROTECTED_SET = 89,           /* "'protected(set)'"  */
  YYSYMBOL_T_PUBLIC_SET = 90,              /* "'public(set)'"  */
  YYSYMBOL_T_READONLY = 91,                /* "'readonly'"  */
  YYSYMBOL_T_VAR = 92,                     /* "'var'"  */
  YYSYMBOL_T_UNSET = 93,                   /* "'unset'"  */
  YYSYMBOL_T_ISSET = 94,                   /* "'isset'"  */
  YYSYMBOL_T_EMPTY = 95,                   /* "'empty'"  */
  YYSYMBOL_T_HALT_COMPILER = 96,           /* "'__halt_compiler'"  */
  YYSYMBOL_T_CLASS = 97,                   /* "'class'"  */
  YYSYMBOL_T_TRAIT = 98,                   /* "'trait'"  */
  YYSYMBOL_T_INTERFACE = 99,               /* "'interface'"  */
  YYSYMBOL_T_ENUM = 100,                   /* "'enum'"  */
  YYSYMBOL_T_EXTENDS = 101,                /* "'extends'"  */
  YYSYMBOL_T_IMPLEMENTS = 102,             /* "'implements'"  */
  YYSYMBOL_T_NAMESPACE = 103,              /* "'namespace'"  */
  YYSYMBOL_T_LIST = 104,                   /* "'list'"  */
  YYSYMBOL_T_ARRAY = 105,                  /* "'array'"  */
  YYSYMBOL_T_CALLABLE = 106,               /* "'callable'"  */
  YYSYMBOL_T_LINE = 107,                   /* "'__LINE__'"  */
  YYSYMBOL_T_FILE = 108,                   /* "'__FILE__'"  */
  YYSYMBOL_T_DIR = 109,                    /* "'__DIR__'"  */
  YYSYMBOL_T_CLASS_C = 110,                /* "'__CLASS__'"  */
  YYSYMBOL_T_TRAIT_C = 111,                /* "'__TRAIT__'"  */
  YYSYMBOL_T_METHOD_C = 112,               /* "'__METHOD__'"  */
  YYSYMBOL_T_FUNC_C = 113,                 /* "'__FUNCTION__'"  */
  YYSYMBOL_T_PROPERTY_C = 114,             /* "'__PROPERTY__'"  */
  YYSYMBOL_T_NS_C = 115,                   /* "'__NAMESPACE__'"  */
  YYSYMBOL_T_ATTRIBUTE = 116,              /* "'#['"  */
  YYSYMBOL_T_PLUS_EQUAL = 117,             /* "'+='"  */
  YYSYMBOL_T_MINUS_EQUAL = 118,            /* "'-='"  */
  YYSYMBOL_T_MUL_EQUAL = 119,              /* "'*='"  */
  YYSYMBOL_T_DIV_EQUAL = 120,              /* "'/='"  */
  YYSYMBOL_T_CONCAT_EQUAL = 121,           /* "'.='"  */
  YYSYMBOL_T_MOD_EQUAL = 122,              /* "'%='"  */
  YYSYMBOL_T_AND_EQUAL = 123,              /* "'&='"  */
  YYSYMBOL_T_OR_EQUAL = 124,               /* "'|='"  */
  YYSYMBOL_T_XOR_EQUAL = 125,              /* "'^='"  */
  YYSYMBOL_T_SL_EQUAL = 126,               /* "'<<='"  */
  YYSYMBOL_T_SR_EQUAL = 127,               /* "'>>='"  */
  YYSYMBOL_T_COALESCE_EQUAL = 128,         /* "'??='"  */
  YYSYMBOL_T_BOOLEAN_OR = 129,             /* "'||'"  */
  YYSYMBOL_T_BOOLEAN_AND = 130,            /* "'&&'"  */
  YYSYMBOL_T_IS_EQUAL = 131,               /* "'=='"  */
  YYSYMBOL_T_IS_NOT_EQUAL = 132,           /* "'!='"  */
  YYSYMBOL_T_IS_IDENTICAL = 133,           /* "'==='"  */
  YYSYMBOL_T_IS_NOT_IDENTICAL = 134,       /* "'!=='"  */
  YYSYMBOL_T_IS_SMALLER_OR_EQUAL = 135,    /* "'<='"  */
  YYSYMBOL_T_IS_GREATER_OR_EQUAL = 136,    /* "'>='"  */
  YYSYMBOL_T_SPACESHIP = 137,              /* "'<=>'"  */
  YYSYMBOL_T_SL = 138,                     /* "'<<'"  */
  YYSYMBOL_T_SR = 139,                     /* "'>>'"  */
  YYSYMBOL_T_INC = 140,                    /* "'++'"  */
  YYSYMBOL_T_DEC = 141,                    /* "'--'"  */
  YYSYMBOL_T_INT_CAST = 142,               /* "'(int)'"  */
  YYSYMBOL_T_DOUBLE_CAST = 143,            /* "'(float)'"  */
  YYSYMBOL_T_STRING_CAST = 144,            /* "'(string)'"  */
  YYSYMBOL_T_ARRAY_CAST = 145,             /* "'(array)'"  */
  YYSYMBOL_T_OBJECT_CAST = 146,            /* "'(object)'"  */
  YYSYMBOL_T_BOOL_CAST = 147,              /* "'(bool)'"  */
  YYSYMBOL_T_UNSET_CAST = 148,             /* "'(unset)'"  */
  YYSYMBOL_T_VOID_CAST = 149,              /* "'(void)'"  */
  YYSYMBOL_T_OBJECT_OPERATOR = 150,        /* "'->'"  */
  YYSYMBOL_T_NULLSAFE_OBJECT_OPERATOR = 151, /* "'?->'"  */
  YYSYMBOL_T_DOUBLE_ARROW = 152,           /* "'=>'"  */
  YYSYMBOL_T_COMMENT = 153,                /* "comment"  */
  YYSYMBOL_T_DOC_COMMENT = 154,            /* "doc comment"  */
  YYSYMBOL_T_OPEN_TAG = 155,               /* "open tag"  */
  YYSYMBOL_T_OPEN_TAG_WITH_ECHO = 156,     /* "'<?='"  */
  YYSYMBOL_T_CLOSE_TAG = 157,              /* "'?>'"  */
  YYSYMBOL_T_WHITESPACE = 158,             /* "whitespace"  */
  YYSYMBOL_T_START_HEREDOC = 159,          /* "heredoc start"  */
  YYSYMBOL_T_END_HEREDOC = 160,            /* "heredoc end"  */
  YYSYMBOL_T_DOLLAR_OPEN_CURLY_BRACES = 161, /* "'${'"  */
  YYSYMBOL_T_CURLY_OPEN = 162,             /* "'{$'"  */
  YYSYMBOL_T_PAAMAYIM_NEKUDOTAYIM = 163,   /* "'::'"  */
  YYSYMBOL_T_NS_SEPARATOR = 164,           /* "'\\'"  */
  YYSYMBOL_T_ELLIPSIS = 165,               /* "'...'"  */
  YYSYMBOL_T_COALESCE = 166,               /* "'??'"  */
  YYSYMBOL_T_POW = 167,                    /* "'**'"  */
  YYSYMBOL_T_POW_EQUAL = 168,              /* "'**='"  */
  YYSYMBOL_T_PIPE = 169,                   /* "'|>'"  */
  YYSYMBOL_T_AMPERSAND_FOLLOWED_BY_VAR_OR_VARARG = 170, /* "'&'"  */
  YYSYMBOL_T_AMPERSAND_NOT_FOLLOWED_BY_VAR_OR_VARARG = 171, /* "amp"  */
  YYSYMBOL_T_BAD_CHARACTER = 172,          /* "invalid character"  */
  YYSYMBOL_T_ERROR = 173,                  /* T_ERROR  */
  YYSYMBOL_174_ = 174,                     /* ','  */
  YYSYMBOL_175_ = 175,                     /* ']'  */
  YYSYMBOL_176_ = 176,                     /* ';'  */
  YYSYMBOL_177_ = 177,                     /* '('  */
  YYSYMBOL_178_ = 178,                     /* ')'  */
  YYSYMBOL_179_ = 179,                     /* '{'  */
  YYSYMBOL_180_ = 180,                     /* '}'  */
  YYSYMBOL_181_ = 181,                     /* '['  */
  YYSYMBOL_182_ = 182,                     /* '`'  */
  YYSYMBOL_183_ = 183,                     /* '"'  */
  YYSYMBOL_184_ = 184,                     /* '$'  */
  YYSYMBOL_YYACCEPT = 185,                 /* $accept  */
  YYSYMBOL_start = 186,                    /* start  */
  YYSYMBOL_reserved_non_modifiers = 187,   /* reserved_non_modifiers  */
  YYSYMBOL_semi_reserved = 188,            /* semi_reserved  */
  YYSYMBOL_ampersand = 189,                /* ampersand  */
  YYSYMBOL_identifier = 190,               /* identifier  */
  YYSYMBOL_top_statement_list = 191,       /* top_statement_list  */
  YYSYMBOL_namespace_declaration_name = 192, /* namespace_declaration_name  */
  YYSYMBOL_namespace_name = 193,           /* namespace_name  */
  YYSYMBOL_legacy_namespace_name = 194,    /* legacy_namespace_name  */
  YYSYMBOL_name = 195,                     /* name  */
  YYSYMBOL_attribute_decl = 196,           /* attribute_decl  */
  YYSYMBOL_attribute_group = 197,          /* attribute_group  */
  YYSYMBOL_attribute = 198,                /* attribute  */
  YYSYMBOL_attributes = 199,               /* attributes  */
  YYSYMBOL_attributed_statement = 200,     /* attributed_statement  */
  YYSYMBOL_attributed_top_statement = 201, /* attributed_top_statement  */
  YYSYMBOL_top_statement = 202,            /* top_statement  */
  YYSYMBOL_203_1 = 203,                    /* $@1  */
  YYSYMBOL_204_2 = 204,                    /* $@2  */
  YYSYMBOL_use_type = 205,                 /* use_type  */
  YYSYMBOL_group_use_declaration = 206,    /* group_use_declaration  */
  YYSYMBOL_mixed_group_use_declaration = 207, /* mixed_group_use_declaration  */
  YYSYMBOL_possible_comma = 208,           /* possible_comma  */
  YYSYMBOL_inline_use_declarations = 209,  /* inline_use_declarations  */
  YYSYMBOL_unprefixed_use_declarations = 210, /* unprefixed_use_declarations  */
  YYSYMBOL_use_declarations = 211,         /* use_declarations  */
  YYSYMBOL_inline_use_declaration = 212,   /* inline_use_declaration  */
  YYSYMBOL_unprefixed_use_declaration = 213, /* unprefixed_use_declaration  */
  YYSYMBOL_use_declaration = 214,          /* use_declaration  */
  YYSYMBOL_const_list = 215,               /* const_list  */
  YYSYMBOL_inner_statement_list = 216,     /* inner_statement_list  */
  YYSYMBOL_inner_statement = 217,          /* inner_statement  */
  YYSYMBOL_statement = 218,                /* statement  */
  YYSYMBOL_219_3 = 219,                    /* $@3  */
  YYSYMBOL_catch_list = 220,               /* catch_list  */
  YYSYMBOL_catch_name_list = 221,          /* catch_name_list  */
  YYSYMBOL_optional_variable = 222,        /* optional_variable  */
  YYSYMBOL_finally_statement = 223,        /* finally_statement  */
  YYSYMBOL_unset_variables = 224,          /* unset_variables  */
  YYSYMBOL_unset_variable = 225,           /* unset_variable  */
  YYSYMBOL_function_name = 226,            /* function_name  */
  YYSYMBOL_function_declaration_statement = 227, /* function_declaration_statement  */
  YYSYMBOL_is_reference = 228,             /* is_reference  */
  YYSYMBOL_is_variadic = 229,              /* is_variadic  */
  YYSYMBOL_class_declaration_statement = 230, /* class_declaration_statement  */
  YYSYMBOL_231_4 = 231,                    /* @4  */
  YYSYMBOL_232_5 = 232,                    /* @5  */
  YYSYMBOL_class_modifiers = 233,          /* class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers = 234, /* anonymous_class_modifiers  */
  YYSYMBOL_anonymous_class_modifiers_optional = 235, /* anonymous_class_modifiers_optional  */
  YYSYMBOL_class_modifier = 236,           /* class_modifier  */
  YYSYMBOL_trait_declaration_statement = 237, /* trait_declaration_statement  */
  YYSYMBOL_238_6 = 238,                    /* @6  */
  YYSYMBOL_interface_declaration_statement = 239, /* interface_declaration_statement  */
  YYSYMBOL_240_7 = 240,                    /* @7  */
  YYSYMBOL_enum_declaration_statement = 241, /* enum_declaration_statement  */
  YYSYMBOL_242_8 = 242,                    /* @8  */
  YYSYMBOL_enum_backing_type = 243,        /* enum_backing_type  */
  YYSYMBOL_enum_case = 244,                /* enum_case  */
  YYSYMBOL_enum_case_expr = 245,           /* enum_case_expr  */
  YYSYMBOL_extends_from = 246,             /* extends_from  */
  YYSYMBOL_interface_extends_list = 247,   /* interface_extends_list  */
  YYSYMBOL_implements_list = 248,          /* implements_list  */
  YYSYMBOL_foreach_variable = 249,         /* foreach_variable  */
  YYSYMBOL_for_statement = 250,            /* for_statement  */
  YYSYMBOL_foreach_statement = 251,        /* foreach_statement  */
  YYSYMBOL_declare_statement = 252,        /* declare_statement  */
  YYSYMBOL_switch_case_list = 253,         /* switch_case_list  */
  YYSYMBOL_case_list = 254,                /* case_list  */
  YYSYMBOL_match = 255,                    /* match  */
  YYSYMBOL_match_arm_list = 256,           /* match_arm_list  */
  YYSYMBOL_non_empty_match_arm_list = 257, /* non_empty_match_arm_list  */
  YYSYMBOL_match_arm = 258,                /* match_arm  */
  YYSYMBOL_match_arm_cond_list = 259,      /* match_arm_cond_list  */
  YYSYMBOL_while_statement = 260,          /* while_statement  */
  YYSYMBOL_if_stmt_without_else = 261,     /* if_stmt_without_else  */
  YYSYMBOL_if_stmt = 262,                  /* if_stmt  */
  YYSYMBOL_alt_if_stmt_without_else = 263, /* alt_if_stmt_without_else  */
  YYSYMBOL_alt_if_stmt = 264,              /* alt_if_stmt  */
  YYSYMBOL_parameter_list = 265,           /* parameter_list  */
  YYSYMBOL_non_empty_parameter_list = 266, /* non_empty_parameter_list  */
  YYSYMBOL_attributed_parameter = 267,     /* attributed_parameter  */
  YYSYMBOL_optional_cpp_modifiers = 268,   /* optional_cpp_modifiers  */
  YYSYMBOL_parameter = 269,                /* parameter  */
  YYSYMBOL_optional_type_without_static = 270, /* optional_type_without_static  */
  YYSYMBOL_type_expr = 271,                /* type_expr  */
  YYSYMBOL_type = 272,                     /* type  */
  YYSYMBOL_union_type_element = 273,       /* union_type_element  */
  YYSYMBOL_union_type = 274,               /* union_type  */
  YYSYMBOL_intersection_type = 275,        /* intersection_type  */
  YYSYMBOL_type_expr_without_static = 276, /* type_expr_without_static  */
  YYSYMBOL_type_without_static = 277,      /* type_without_static  */
  YYSYMBOL_union_type_without_static_element = 278, /* union_type_without_static_element  */
  YYSYMBOL_union_type_without_static = 279, /* union_type_without_static  */
  YYSYMBOL_intersection_type_without_static = 280, /* intersection_type_without_static  */
  YYSYMBOL_return_type = 281,              /* return_type  */
  YYSYMBOL_argument_list = 282,            /* argument_list  */
  YYSYMBOL_non_empty_argument_list = 283,  /* non_empty_argument_list  */
  YYSYMBOL_clone_argument_list = 284,      /* clone_argument_list  */
  YYSYMBOL_non_empty_clone_argument_list = 285, /* non_empty_clone_argument_list  */
  YYSYMBOL_argument_no_expr = 286,         /* argument_no_expr  */
  YYSYMBOL_argument = 287,                 /* argument  */
  YYSYMBOL_global_var_list = 288,          /* global_var_list  */
  YYSYMBOL_global_var = 289,               /* global_var  */
  YYSYMBOL_static_var_list = 290,          /* static_var_list  */
  YYSYMBOL_static_var = 291,               /* static_var  */
  YYSYMBOL_class_statement_list = 292,     /* class_statement_list  */
  YYSYMBOL_attributed_class_statement = 293, /* attributed_class_statement  */
  YYSYMBOL_class_statement = 294,          /* class_statement  */
  YYSYMBOL_class_name_list = 295,          /* class_name_list  */
  YYSYMBOL_trait_adaptations = 296,        /* trait_adaptations  */
  YYSYMBOL_trait_adaptation_list = 297,    /* trait_adaptation_list  */
  YYSYMBOL_trait_adaptation = 298,         /* trait_adaptation  */
  YYSYMBOL_trait_precedence = 299,         /* trait_precedence  */
  YYSYMBOL_trait_alias = 300,              /* trait_alias  */
  YYSYMBOL_trait_method_reference = 301,   /* trait_method_reference  */
  YYSYMBOL_absolute_trait_method_reference = 302, /* absolute_trait_method_reference  */
  YYSYMBOL_method_body = 303,              /* method_body  */
  YYSYMBOL_property_modifiers = 304,       /* property_modifiers  */
  YYSYMBOL_method_modifiers = 305,         /* method_modifiers  */
  YYSYMBOL_class_const_modifiers = 306,    /* class_const_modifiers  */
  YYSYMBOL_non_empty_member_modifiers = 307, /* non_empty_member_modifiers  */
  YYSYMBOL_member_modifier = 308,          /* member_modifier  */
  YYSYMBOL_property_list = 309,            /* property_list  */
  YYSYMBOL_property = 310,                 /* property  */
  YYSYMBOL_hooked_property = 311,          /* hooked_property  */
  YYSYMBOL_property_hook_list = 312,       /* property_hook_list  */
  YYSYMBOL_optional_property_hook_list = 313, /* optional_property_hook_list  */
  YYSYMBOL_property_hook_modifiers = 314,  /* property_hook_modifiers  */
  YYSYMBOL_property_hook = 315,            /* property_hook  */
  YYSYMBOL_316_9 = 316,                    /* @9  */
  YYSYMBOL_property_hook_body = 317,       /* property_hook_body  */
  YYSYMBOL_optional_parameter_list = 318,  /* optional_parameter_list  */
  YYSYMBOL_class_const_list = 319,         /* class_const_list  */
  YYSYMBOL_class_const_decl = 320,         /* class_const_decl  */
  YYSYMBOL_const_decl = 321,               /* const_decl  */
  YYSYMBOL_echo_expr_list = 322,           /* echo_expr_list  */
  YYSYMBOL_echo_expr = 323,                /* echo_expr  */
  YYSYMBOL_for_cond_exprs = 324,           /* for_cond_exprs  */
  YYSYMBOL_for_exprs = 325,                /* for_exprs  */
  YYSYMBOL_non_empty_for_exprs = 326,      /* non_empty_for_exprs  */
  YYSYMBOL_anonymous_class = 327,          /* anonymous_class  */
  YYSYMBOL_328_10 = 328,                   /* @10  */
  YYSYMBOL_new_dereferenceable = 329,      /* new_dereferenceable  */
  YYSYMBOL_new_non_dereferenceable = 330,  /* new_non_dereferenceable  */
  YYSYMBOL_expr = 331,                     /* expr  */
  YYSYMBOL_inline_function = 332,          /* inline_function  */
  YYSYMBOL_fn = 333,                       /* fn  */
  YYSYMBOL_function = 334,                 /* function  */
  YYSYMBOL_backup_doc_comment = 335,       /* backup_doc_comment  */
  YYSYMBOL_backup_fn_flags = 336,          /* backup_fn_flags  */
  YYSYMBOL_returns_ref = 337,              /* returns_ref  */
  YYSYMBOL_lexical_vars = 338,             /* lexical_vars  */
  YYSYMBOL_lexical_var_list = 339,         /* lexical_var_list  */
  YYSYMBOL_lexical_var = 340,              /* lexical_var  */
  YYSYMBOL_function_call = 341,            /* function_call  */
  YYSYMBOL_342_11 = 342,                   /* @11  */
  YYSYMBOL_class_name = 343,               /* class_name  */
  YYSYMBOL_class_name_reference = 344,     /* class_name_reference  */
  YYSYMBOL_backticks_expr = 345,           /* backticks_expr  */
  YYSYMBOL_ctor_arguments = 346,           /* ctor_arguments  */
  YYSYMBOL_dereferenceable_scalar = 347,   /* dereferenceable_scalar  */
  YYSYMBOL_scalar = 348,                   /* scalar  */
  YYSYMBOL_constant = 349,                 /* constant  */
  YYSYMBOL_class_constant = 350,           /* class_constant  */
  YYSYMBOL_optional_expr = 351,            /* optional_expr  */
  YYSYMBOL_variable_class_name = 352,      /* variable_class_name  */
  YYSYMBOL_fully_dereferenceable = 353,    /* fully_dereferenceable  */
  YYSYMBOL_array_object_dereferenceable = 354, /* array_object_dereferenceable  */
  YYSYMBOL_callable_expr = 355,            /* callable_expr  */
  YYSYMBOL_callable_variable = 356,        /* callable_variable  */
  YYSYMBOL_variable = 357,                 /* variable  */
  YYSYMBOL_simple_variable = 358,          /* simple_variable  */
  YYSYMBOL_static_member = 359,            /* static_member  */
  YYSYMBOL_new_variable = 360,             /* new_variable  */
  YYSYMBOL_member_name = 361,              /* member_name  */
  YYSYMBOL_property_name = 362,            /* property_name  */
  YYSYMBOL_array_pair_list = 363,          /* array_pair_list  */
  YYSYMBOL_possible_array_pair = 364,      /* possible_array_pair  */
  YYSYMBOL_non_empty_array_pair_list = 365, /* non_empty_array_pair_list  */
  YYSYMBOL_array_pair = 366,               /* array_pair  */
  YYSYMBOL_encaps_list = 367,              /* encaps_list  */
  YYSYMBOL_encaps_var = 368,               /* encaps_var  */
  YYSYMBOL_encaps_var_offset = 369,        /* encaps_var_offset  */
  YYSYMBOL_internal_functions_in_yacc = 370, /* internal_functions_in_yacc  */
  YYSYMBOL_isset_variables = 371,          /* isset_variables  */
  YYSYMBOL_isset_variable = 372            /* isset_variable  */
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
#define YYLAST   11085

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  185
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  188
/* YYNRULES -- Number of rules.  */
#define YYNRULES  635
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1203

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   412


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
       2,     2,     2,    17,   183,     2,   184,    16,     2,     2,
     177,   178,    14,    12,   174,    13,    11,    15,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,   176,
       9,     4,    10,     5,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   181,     2,   175,     8,     2,   182,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   179,     7,   180,    18,     2,     2,     2,
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
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173
};

#if ZENDDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   302,   302,   306,   306,   306,   306,   306,   306,   306,
     306,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   308,   308,   308,   308,   308,   308,   308,
     308,   308,   308,   309,   309,   309,   309,   309,   309,   309,
     309,   309,   309,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   311,   311,   311,   311,   311,   311,
     311,   311,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   313,   317,   318,   318,   318,   318,   318,
     318,   318,   322,   323,   327,   328,   336,   337,   342,   343,
     348,   349,   354,   355,   359,   360,   361,   362,   366,   368,
     373,   375,   380,   384,   385,   389,   390,   391,   392,   393,
     397,   398,   402,   403,   404,   405,   409,   412,   412,   415,
     415,   418,   419,   420,   421,   425,   426,   430,   435,   440,
     441,   445,   447,   452,   454,   459,   461,   466,   467,   471,
     473,   478,   480,   485,   486,   490,   492,   498,   499,   500,
     501,   508,   509,   510,   511,   513,   515,   517,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   530,   534,
     533,   537,   538,   540,   541,   542,   546,   548,   553,   554,
     558,   559,   563,   564,   568,   569,   573,   577,   578,   586,
     593,   594,   598,   599,   603,   603,   606,   606,   612,   613,
     618,   620,   625,   626,   630,   631,   632,   636,   636,   642,
     642,   648,   648,   654,   655,   659,   664,   665,   669,   670,
     674,   675,   679,   680,   684,   685,   686,   687,   691,   692,
     696,   697,   701,   702,   706,   707,   708,   709,   713,   714,
     716,   718,   720,   726,   731,   732,   736,   737,   741,   743,
     748,   749,   754,   755,   760,   763,   769,   770,   775,   778,
     784,   785,   791,   792,   797,   799,   804,   805,   809,   811,
     817,   821,   828,   829,   833,   834,   835,   836,   840,   841,
     845,   846,   850,   852,   857,   858,   865,   866,   867,   868,
     872,   873,   874,   878,   879,   883,   885,   890,   892,   897,
     898,   902,   903,   907,   909,   923,   924,   925,   929,   931,
     933,   938,   940,   942,   944,   946,   951,   952,   956,   957,
     961,   967,   968,   972,   973,   977,   979,   985,   988,   991,
     994,   997,  1001,  1005,  1006,  1007,  1012,  1013,  1017,  1018,
    1019,  1023,  1025,  1030,  1031,  1035,  1040,  1042,  1046,  1051,
    1059,  1061,  1065,  1070,  1071,  1075,  1078,  1083,  1085,  1092,
    1094,  1101,  1103,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1121,  1122,  1126,  1128,  1133,  1135,  1140,
    1141,  1142,  1148,  1149,  1153,  1154,  1162,  1161,  1172,  1173,
    1174,  1179,  1180,  1184,  1185,  1189,  1190,  1198,  1202,  1203,
    1206,  1210,  1211,  1212,  1216,  1217,  1221,  1222,  1223,  1224,
    1228,  1228,  1238,  1240,  1242,  1247,  1252,  1254,  1256,  1258,
    1260,  1262,  1267,  1272,  1274,  1276,  1278,  1280,  1282,  1284,
    1286,  1288,  1290,  1292,  1294,  1296,  1298,  1299,  1300,  1301,
    1302,  1304,  1306,  1308,  1310,  1312,  1313,  1314,  1315,  1316,
    1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,
    1327,  1328,  1329,  1331,  1333,  1335,  1337,  1339,  1341,  1343,
    1345,  1347,  1349,  1351,  1356,  1357,  1358,  1360,  1362,  1364,
    1365,  1366,  1367,  1368,  1369,  1370,  1371,  1372,  1377,  1378,
    1379,  1380,  1381,  1382,  1383,  1384,  1385,  1386,  1387,  1388,
    1389,  1391,  1396,  1401,  1409,  1413,  1417,  1421,  1425,  1426,
    1430,  1431,  1435,  1436,  1440,  1441,  1445,  1447,  1452,  1454,
    1456,  1456,  1463,  1466,  1470,  1471,  1472,  1476,  1478,  1479,
    1484,  1485,  1490,  1491,  1492,  1493,  1497,  1498,  1499,  1500,
    1502,  1503,  1504,  1505,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1522,  1524,  1526,  1528,  1533,  1534,
    1538,  1542,  1543,  1547,  1548,  1549,  1553,  1554,  1558,  1559,
    1560,  1561,  1565,  1567,  1569,  1571,  1573,  1577,  1579,  1581,
    1583,  1588,  1589,  1590,  1594,  1596,  1601,  1603,  1605,  1607,
    1609,  1611,  1616,  1617,  1618,  1622,  1623,  1624,  1628,  1633,
    1634,  1638,  1640,  1645,  1647,  1649,  1651,  1653,  1655,  1658,
    1664,  1666,  1668,  1670,  1675,  1677,  1680,  1683,  1686,  1688,
    1690,  1693,  1697,  1698,  1699,  1700,  1705,  1706,  1707,  1709,
    1711,  1713,  1715,  1720,  1721,  1726
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
  "\"'protected'\"", "\"'public'\"", "\"'private(set)'\"",
  "\"'protected(set)'\"", "\"'public(set)'\"", "\"'readonly'\"",
  "\"'var'\"", "\"'unset'\"", "\"'isset'\"", "\"'empty'\"",
  "\"'__halt_compiler'\"", "\"'class'\"", "\"'trait'\"", "\"'interface'\"",
  "\"'enum'\"", "\"'extends'\"", "\"'implements'\"", "\"'namespace'\"",
  "\"'list'\"", "\"'array'\"", "\"'callable'\"", "\"'__LINE__'\"",
  "\"'__FILE__'\"", "\"'__DIR__'\"", "\"'__CLASS__'\"", "\"'__TRAIT__'\"",
  "\"'__METHOD__'\"", "\"'__FUNCTION__'\"", "\"'__PROPERTY__'\"",
  "\"'__NAMESPACE__'\"", "\"'#['\"", "\"'+='\"", "\"'-='\"", "\"'*='\"",
  "\"'/='\"", "\"'.='\"", "\"'%='\"", "\"'&='\"", "\"'|='\"", "\"'^='\"",
  "\"'<<='\"", "\"'>>='\"", "\"'?""?='\"", "\"'||'\"", "\"'&&'\"",
  "\"'=='\"", "\"'!='\"", "\"'==='\"", "\"'!=='\"", "\"'<='\"", "\"'>='\"",
  "\"'<=>'\"", "\"'<<'\"", "\"'>>'\"", "\"'++'\"", "\"'--'\"",
  "\"'(int)'\"", "\"'(float)'\"", "\"'(string)'\"", "\"'(array)'\"",
  "\"'(object)'\"", "\"'(bool)'\"", "\"'(unset)'\"", "\"'(void)'\"",
  "\"'->'\"", "\"'?->'\"", "\"'=>'\"", "\"comment\"", "\"doc comment\"",
  "\"open tag\"", "\"'<?='\"", "\"'?>'\"", "\"whitespace\"",
  "\"heredoc start\"", "\"heredoc end\"", "\"'${'\"", "\"'{$'\"",
  "\"'::'\"", "\"'\\\\'\"", "\"'...'\"", "\"'?""?'\"", "\"'**'\"",
  "\"'**='\"", "\"'|>'\"", "\"'&'\"", "\"amp\"", "\"invalid character\"",
  "T_ERROR", "','", "']'", "';'", "'('", "')'", "'{'", "'}'", "'['", "'`'",
  "'\"'", "'$'", "$accept", "start", "reserved_non_modifiers",
  "semi_reserved", "ampersand", "identifier", "top_statement_list",
  "namespace_declaration_name", "namespace_name", "legacy_namespace_name",
  "name", "attribute_decl", "attribute_group", "attribute", "attributes",
  "attributed_statement", "attributed_top_statement", "top_statement",
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
  "class_modifiers", "anonymous_class_modifiers",
  "anonymous_class_modifiers_optional", "class_modifier",
  "trait_declaration_statement", "@6", "interface_declaration_statement",
  "@7", "enum_declaration_statement", "@8", "enum_backing_type",
  "enum_case", "enum_case_expr", "extends_from", "interface_extends_list",
  "implements_list", "foreach_variable", "for_statement",
  "foreach_statement", "declare_statement", "switch_case_list",
  "case_list", "match", "match_arm_list", "non_empty_match_arm_list",
  "match_arm", "match_arm_cond_list", "while_statement",
  "if_stmt_without_else", "if_stmt", "alt_if_stmt_without_else",
  "alt_if_stmt", "parameter_list", "non_empty_parameter_list",
  "attributed_parameter", "optional_cpp_modifiers", "parameter",
  "optional_type_without_static", "type_expr", "type",
  "union_type_element", "union_type", "intersection_type",
  "type_expr_without_static", "type_without_static",
  "union_type_without_static_element", "union_type_without_static",
  "intersection_type_without_static", "return_type", "argument_list",
  "non_empty_argument_list", "clone_argument_list",
  "non_empty_clone_argument_list", "argument_no_expr", "argument",
  "global_var_list", "global_var", "static_var_list", "static_var",
  "class_statement_list", "attributed_class_statement", "class_statement",
  "class_name_list", "trait_adaptations", "trait_adaptation_list",
  "trait_adaptation", "trait_precedence", "trait_alias",
  "trait_method_reference", "absolute_trait_method_reference",
  "method_body", "property_modifiers", "method_modifiers",
  "class_const_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "hooked_property", "property_hook_list",
  "optional_property_hook_list", "property_hook_modifiers",
  "property_hook", "@9", "property_hook_body", "optional_parameter_list",
  "class_const_list", "class_const_decl", "const_decl", "echo_expr_list",
  "echo_expr", "for_cond_exprs", "for_exprs", "non_empty_for_exprs",
  "anonymous_class", "@10", "new_dereferenceable",
  "new_non_dereferenceable", "expr", "inline_function", "fn", "function",
  "backup_doc_comment", "backup_fn_flags", "returns_ref", "lexical_vars",
  "lexical_var_list", "lexical_var", "function_call", "@11", "class_name",
  "class_name_reference", "backticks_expr", "ctor_arguments",
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

#define YYPACT_NINF (-953)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-607)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -953,    66,  4564,  -953,  8636,  8636,  8636,  8636,  8636,  -953,
    -953,    96,  -953,  -953,  -953,  -953,  -953,  -953,  8636,  8636,
     -36,  8636,  8636,  8636,  8636,  8636,   852,  8809,   -23,    -8,
    8636,  7252,    31,   116,   124,   146,   252,   260,  8636,  8636,
     221,  -953,  -953,   309,  8636,   169,  8636,   257,    55,   171,
    -953,  -953,   -23,   263,   274,   279,   297,  -953,  -953,  -953,
    -953, 10784,   319,   322,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953, 10081, 10081,  8636,  8636,  8636,  8636,
    8636,  8636,  8636,  8636,    86,  -953,  8636,  -953,  7425,    93,
     136,    37,    48,  -953,   707,  -953,  -953,  -953,  -953,  -953,
    -953,   605,  -953,  -953,  -953,  -953,  -953,   165,  -953,   426,
    -953,   385,  -953,  8969,  -953,   341,   341,  -953,   103,   517,
    -953,   155,   205,   204,   345,   320,  -953,   351,  1270,  -953,
    -953,  -953,  -953,   449,   -23,   191,   365,   341,   365,    18,
     365,   365,  -953,  9469,  9469,  8636,  9469,  9469,  9671,  9616,
    9671,  -953,  -953,  8636,  -953,   243,   188,   440,  -953,  -953,
     384,   -23,  -953,   383,  1423,  -953,  -953,  1597,  -953,  -953,
    8636,    60,  -953,  9469,   496,  8636,  7944,  8636,   309,  8636,
    8636,  9469,   377,   401,   404,   590,   312,  -953,   422,  -953,
    9469,  -953,  -953,  -953,  -953,  -953,  -953,    13,   323,   436,
     380,  -953,   421,  -953,  -953,   612,   453,  -953,  -953,  -953,
   10081,  8636,  8636,   448,   615,   621,   625,   632,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,   483,   490,  7425,  7425,   328,   852,  8636,  7425,   495,
     498,  -953,  -953,   500,   500,   365,   365,   365,   365,   365,
     365,   365,  8989,   389,   108,  -953,  8117, 10081,    98,  -953,
    3555,  4737,   505,  8636,  -953,  -953, 10081,  9367,   516,  -953,
     545,  -953,    56,   541,   147,    56,    45,  8636,  -953,  -953,
     449,  -953,  -953,  -953,  -953,  -953,   547,  7252,   554,   728,
     564,  2988,  8636,  8636,  8636,  8636,  8636,  8636,  8636,  8636,
    8636,  8636,  8636,  8636,  8636,    80,  8636,  8636,  8636,  8636,
    8636,  8636,  8636,  8636,  8636,  8636,  8636,  8636,  8636,  8636,
    8636,  8636,  -953,  -953,  -953,    20,  9823,  9976,    40,    40,
    8636,   -23,  7598,  8636,  8636,  8636,  8636,  8636,  8636,  8636,
    8636,  8636,  8636,  8636,  8636,  -953,  -953,  8636,  -953,  3625,
    8636,  3690,  -953,  -953,  -953,    55,  -953,    40,    40,    55,
    8636,  -953,    64,  8636,  8636,   -36,  8636,  8636,  8636,  3161,
     499,  8809,    16,   260,   740,   742,  8636,    67,   -23,   274,
     279,   319,   322,   744,   745,   752,   755,   757,   758,   760,
     769,   770,  8636,  -953,   780,   613,  -953,  3488,  -953,   614,
    -953,  -953,  9469,  3703,  8636,  -953,   617,  3760,  8636,   619,
     622,  9469,  9410,   268,  3773,  3838,  -953,  -953,  -953,  8636,
     309,  -953,  -953,  4910,   777,   623,    19,   633,   523,  -953,
     323,  -953,    55,  -953,  8636,   783,  -953,   637,  -953,   290,
    9469,   638,  -953,  3908,   641,   712,  -953,   713,   809,  -953,
     642,  -953,   640,   644,  -953,   650,   -23,   -23,  3973,   645,
    -953,   802,   803,   639,  -953,  -953,   407,  3273,   647,  -953,
    -953,  -953,   575,   651,  -953,   960,  -953,  -953,  -953,  7425,
    9469,   187,  7771,   826,  7425,  -953,  -953,  3341,  -953,   808,
    8636,  -953,  8636,  -953,  -953,  8636,  9422,  8450,  8623,  1210,
    1210,   466,   129,   129,    18,    18,    18,  9604,  9659,  9671,
    -953,  2105,  8277, 10155, 10155, 10155, 10155,  1210,  1210, 10155,
     276,   276,  9682,   365,   392, 10194, 10194,   656,  -953,  -953,
    -953,   659,  8636,   661,   662,   -23,  8636,   661,   662,   -23,
    -953,  8636,  -953,   -23,   -23,   665,  -953, 10081,  9671,  9671,
    9671,  9671,  9671,  9671,  9671,  9671,  9671,  9671,  9671,  9671,
    9671,  9671,  -953,  9671,  -953,   -23,  -953,  -953,  -953,  -953,
     667,  9469,  1223,  1945,   666,  1771,  1945,   668,  2118,  -953,
    8636,  2292,  9469,  7944,  8290, 10070,  -953,    15,   669,  9469,
    -953,  -953,  -953,   446,   670,  -953,  -953,   785,  -953,  -953,
    9469,  -953, 10081,   675,  8636,   678,  -953,  -953,   328,   743,
     679,   328,  -953,    54,   743,  -953,  4218,   855,  -953,   328,
     687,  -953,   686,  -953,  -953,  -953,   833,  -953,  -953,  -953,
     691,  -953,  8636,  -953,  -953,   690,  -953,   692,   695, 10081,
    9469,  8636,  -953,  -953,   712,  3986,  4043,  5083,  9682,  8636,
    2163,   696,  2163,  3353,  -953,  3408,  -953,  3420,  -953,  -953,
    -953,   500,   712,  -953,  -953,  9469,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  -953,  4056,  -953,  -953,  -953,   693,   716,
    9218,  8636,  9469,   714,  7425, 10081,   134,   143,  2466,   719,
     721,  -953,  8463,  -953,   531,   830,    95,   724,  -953,  -953,
      95,  -953,   723,  -953,  -953,  -953,   328,  -953,  -953,   726,
    -953,   727,   479,  -953,  -953,  -953,   479,  -953,  -953,    24,
     895,   898,   736,  -953,  -953,  4391,  -953,  8636,  -953,  -953,
    -953,  -953,  9171,   732,   855,  7425,   284,  9671,   743,  7252,
     903,   734,  9682,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,  1268,   735,   738,  -953,   107,  -953,  1490,
    -953,  2163,   741,   739,   739,  -953,   743,  6986,   746,  5256,
    7944,  8290,  9469,  7425,   749,   275, 10070,  2640,  -953,  -953,
    -953,  -953,   646,  -953,    87,   747,   748,   751,  -953,   753,
    9469,   754,   750,  -953,   897,  -953,   446,   761,   756,  -953,
    -953,   726,   759,  8952,   328,  -953,  -953,   766,    78,   479,
     595,   595,   479,   765,  -953,  9671,   767,  -953,   771,  -953,
    -953,  -953,  -953,  -953,   912,  2327,  -953,   519,   519,   775,
    -953,    30,   919,   941,   779,  -953,   773,   877,  -953,  -953,
     782,   781,  9230,   786,   283,   787,  -953,  -953,  -953,  5429,
     717,   784,  8636,    21,   102,  -953,  -953,   814,  -953,  8463,
    -953,  8636,   815,   328,  -953,  -953,  -953,  -953,    95,   790,
    -953,  -953,   328,  -953,  -953,  1041,  -953,  -953,  -953,   107,
     890,   899,  1469,  -953,  9884,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -953,   855,   792,  6986,    54,   821,  -953,  -953,
     805,   132,  -953,   812,   519,   469,   469,   519,   912,   797,
     912,   801,  -953,  2814,  -953,  2640,  5602,   807,   810,  -953,
    8921,  -953,  -953,  -953,  8636,  -953,  9469,  8636,    89,  -953,
    5775,  -953,  -953, 10057, 10877,   393,  -953,   958,   341,  7148,
    -953, 10384,  -953,  -953,  -953,  -953,  -953,   961,  -953,  -953,
    -953,  -953,  -953,  -953,    82,  -953,  -953,  -953,  -953,  -953,
    -953,   811,  -953,  -953,  -953,  -953,  6986,  6986,  9469,  9469,
     328,  -953,   816,  -953,  -953,   988,  -953, 10331,  -953,   989,
     540,  -953,  -953, 10877,   992,   993,   995,   998,   999, 10970,
     553,  -953,  -953, 10519,  8636,  1002,   828,  -953,   983,   837,
    -953,   834, 10554,  5948,  -953,  6986,  6986,  -953,   839,  8636,
     840,   856,  -953,  -953, 10468,  -953,   844,   845,   962,   942,
     862,  8636,   848,  1001,  -953,  -953,  8636,  8636,   992,   561,
   10970,  -953,  -953,  9469,  8636,   851,  -953,  -953,    82,   859,
    -953,  -953,   857,  -953,  9469,  -953,  -953,  -953,  -953,  -953,
   10691,   328, 10877,  9469,  -953,  1034,  -953,   868,  9469,  9469,
    -953,  -953,  -953,  9469,  -953,  -953,  6121,  -953,  -953,  6294,
    -953,  6467,  -953,  -953, 10877,   726,  -953,   867,   798,  8636,
    -953,  2163,  -953,  -953,   851,  1982,  -953,  -953,  -953,  -953,
    -953,  -953,  1268,  1490,   341,  -953,  9469,   869,  -953,  -953,
    -953,  -953,  1993,  -953,  1029,  -953,   912,  -953,  -953,  -953,
    -953,   190,   878,  -953,  -953,  -953,  2163,  -953,  6640,  -953,
     876,   164,  -953,  -953,  8636,  -953,  -953,  -953,  9159,  6813,
    -953,  -953,  -953
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      87,     0,     2,     1,     0,     0,     0,     0,     0,   536,
     537,    94,    96,    97,    95,   581,   164,   534,     0,     0,
       0,     0,     0,     0,   492,     0,   202,     0,   530,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   558,   558,
       0,   505,   504,     0,   558,     0,     0,     0,     0,   522,
     204,   205,   206,     0,     0,     0,     0,   196,   207,   209,
     211,   119,     0,     0,   545,   546,   547,   553,   548,   549,
     550,   551,   552,   506,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   171,     0,   146,   599,   527,
       0,     0,   544,   103,     0,   110,   113,    86,   112,   105,
     106,     0,   198,   107,   108,   109,   501,   256,   152,     0,
     153,   474,   475,     0,   497,   508,   508,   576,     0,   541,
     489,   542,   543,     0,   566,     0,   520,   577,   416,   572,
     578,   479,    94,   522,     0,     0,   458,   508,   459,   460,
     461,   488,   174,   628,   629,     0,   631,   632,   491,   493,
     495,   522,   206,     0,   523,   202,   203,     0,   200,   413,
     524,   415,   586,   525,     0,   421,   422,     0,   531,   487,
       0,     0,   399,   400,     0,     0,   404,     0,     0,     0,
       0,   559,     0,     0,     0,     0,     0,   144,     0,   146,
     496,    90,    93,    91,   125,   126,    92,   141,     0,     0,
       0,   136,     0,   319,   320,   323,     0,   322,   499,   517,
       0,     0,     0,     0,     0,     0,     0,     0,    84,    89,
       3,     4,     5,     6,     7,     8,     9,    10,    46,    47,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    49,
      50,    51,    52,    71,    53,    41,    42,    43,    70,    44,
      45,    30,    31,    32,    33,    34,    35,    36,    75,    76,
      77,    78,    79,    80,    81,    37,    38,    39,    40,    61,
      59,    60,    72,    56,    57,    58,    48,    54,    55,    66,
      67,    68,    62,    63,    65,    64,    73,    69,    74,    85,
      88,   117,     0,   599,   599,     0,   202,     0,   599,   565,
     563,   567,   564,   437,   439,   480,   481,   482,   483,   484,
     485,   486,     0,   614,     0,   539,     0,     0,     0,   612,
       0,     0,     0,     0,    82,    83,     0,   604,     0,   602,
     598,   600,   528,     0,   529,     0,     0,     0,   583,   516,
       0,   104,   114,   498,   194,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   165,   509,   506,   506,     0,     0,     0,     0,
     558,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   436,   438,     0,   506,     0,
       0,     0,   414,   201,   410,     0,   412,     0,     0,     0,
     558,   313,    94,     3,     4,     5,     6,     7,    46,   492,
      12,    13,   530,    71,   505,   504,    33,    75,    81,    39,
      40,    48,    54,   545,   546,   547,   553,   548,   549,   550,
     551,   552,   312,   305,     0,   129,   309,     0,   301,   129,
     317,   303,   316,     0,     0,   163,     0,     0,     0,     0,
     405,   409,     0,     0,     0,     0,   158,   159,   173,     0,
       0,   111,   160,     0,     0,     0,   141,     0,     0,   121,
       0,   123,     0,   161,     0,     0,   162,   129,   184,   561,
     635,   129,   633,     0,     0,   218,   506,   220,   213,   116,
       0,    87,     0,     0,   100,   129,    98,     0,     0,     0,
     175,     0,     0,     0,   538,   613,     0,     0,   561,   611,
     540,   610,   473,     0,   151,     0,   148,   145,   147,   599,
     607,   561,     0,   533,   599,   490,   535,     0,   500,     0,
       0,   257,     0,   146,   260,     0,     0,   445,   448,   467,
     469,   449,   450,   451,   452,   454,   455,   442,   444,   443,
     472,   440,   441,   464,   465,   462,   463,   468,   470,   471,
     456,   457,   478,   453,   466,   447,   446,     0,   187,   188,
     506,     0,     0,   554,   584,     0,     0,   555,   585,     0,
     595,     0,   597,   579,   580,     0,   521,     0,   419,   423,
     424,   425,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   426,   630,   494,   526,   530,   590,   588,   589,   591,
       0,   315,     0,   130,     0,     0,   130,     0,     0,   398,
       0,     0,   408,   401,     0,     0,   169,     0,     0,   506,
     143,   176,   142,     0,     0,   122,   124,   141,   135,   318,
     324,   321,   130,     0,   130,     0,   627,   115,     0,   222,
       0,     0,   506,     0,   222,    87,     0,     0,   532,   130,
       0,    99,   562,   533,   616,   617,     0,   622,   625,   623,
       0,   619,     0,   618,   621,     0,   149,     0,     0,     0,
     603,     0,   601,   582,   218,     0,     0,     0,   477,     0,
     268,     0,   268,     0,   518,     0,   519,     0,   574,   575,
     573,   420,   218,   587,   314,   311,   310,   306,   307,   308,
     304,   302,   146,   254,     0,   146,   252,   154,     0,     0,
     403,     0,   406,     0,   599,     0,     0,   561,     0,   238,
     238,   157,   244,   397,   182,   139,     0,   129,   132,   137,
       0,   185,     0,   634,   626,   219,     0,   506,   326,   221,
     336,     0,     0,   279,   290,   291,     0,   292,   214,   274,
       0,   276,   277,   278,   506,     0,   120,     0,   101,   102,
     624,   615,     0,     0,   609,   599,   561,   418,   222,     0,
       0,     0,   476,   369,   370,   371,   365,   364,   363,   368,
     367,   366,   372,   268,     0,   129,   264,   272,   267,   269,
     361,   268,     0,   556,   557,   596,   222,   258,     0,     0,
     404,     0,   407,   599,     0,   561,     0,     0,   146,   232,
     170,   238,     0,   238,     0,   129,     0,   129,   246,   129,
     250,     0,     0,   172,     0,   138,   130,     0,   129,   134,
     166,   223,     0,   357,     0,   326,   275,     0,     0,     0,
       0,     0,     0,     0,   118,   417,     0,   150,     0,   506,
     255,   146,   261,   266,   299,   268,   262,     0,     0,   190,
     273,   286,     0,   288,   289,   362,     0,   510,   506,   155,
       0,     0,   402,     0,   533,     0,   146,   230,   167,     0,
       0,     0,     0,     0,     0,   234,   130,     0,   243,   130,
     245,   130,     0,     0,   146,   140,   131,   128,   130,     0,
     326,   506,     0,   356,   208,   357,   332,   333,   325,   272,
       0,     0,   355,   337,   357,   281,   284,   280,   282,   283,
     285,   326,   620,   608,     0,   259,     0,     0,   265,   287,
       0,     0,   191,   192,     0,     0,     0,     0,   299,     0,
     299,     0,   253,     0,   226,     0,     0,     0,     0,   236,
       0,   146,   146,   235,     0,   247,   251,     0,   180,   178,
       0,   133,   127,   357,     0,     0,   334,     0,   508,     0,
     210,   357,   326,   300,   507,   294,   193,     0,   297,   293,
     295,   296,   298,   507,     0,   507,   326,   146,   228,   156,
     168,     0,   233,   237,   146,   146,   241,   242,   249,   248,
       0,   181,     0,   183,   197,   216,   338,     0,   335,   506,
       0,   374,   328,     0,    94,   279,   290,   291,     0,     0,
       0,   394,   212,   357,     0,   506,     0,   514,     0,   129,
     513,     0,   357,     0,   231,   239,   240,   179,     0,     0,
       0,    75,   339,   350,     0,   341,     0,     0,     0,   351,
       0,     0,   375,     0,   327,   506,     0,     0,     0,     0,
       0,   329,   195,   507,     0,   382,   146,   515,   130,     0,
     146,   411,     0,   146,   217,   215,   340,   342,   343,   344,
       0,     0,     0,   506,   379,   506,   373,     0,   506,   506,
     330,   393,   503,   506,   379,   270,     0,   512,   511,     0,
     229,     0,   346,   347,   349,   345,   352,   376,   384,     0,
     375,   268,   395,   396,   382,   384,   507,   507,   177,   348,
     379,   377,   384,   385,   508,   380,   506,     0,   271,   383,
     189,   502,   384,   381,     0,   376,   299,   378,   506,   507,
     386,     0,   391,   353,   146,   507,   268,   507,     0,   331,
       0,     0,   354,   392,     0,   388,   146,   507,     0,     0,
     387,   390,   389
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -953,  -953,   -64,  -880,   -69,   -58,  -493,  -953,   -40,  -189,
     334,   373,  -953,   -90,    -2,     6,   969,  -953,  -953,  -953,
    1019,  -953,  -953,  -341,  -953,  -953,   870,   201,  -756,   569,
     900,  -131,  -953,   106,  -953,  -953,  -953,  -953,  -953,  -953,
     409,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,    -6,  -953,  -953,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -663,  -953,  -599,   236,  -953,    99,  -953,  -953,  -613,
    -953,  -953,  -953,   154,  -953,  -953,  -953,  -953,  -953,  -953,
    -721,  -953,   193,  -953,   262,   144,  -890,  -137,  -194,  -953,
     306,  -953,  -340,  -272,  -953,   196,  -926,    65,  -953,  -953,
    -953,   932,  -443,  -953,   597,  -953,   592,  -751,   156,  -953,
    -771,  -953,  -953,    25,  -953,  -953,  -953,  -953,  -953,  -953,
    -953,  -953,  -831,  -818,  -953,    10,  -953,  -835,   -54,  -953,
     -51,  -953,  -953,  -953,    53,    17,   628,  -953,   673,  -953,
     273,   468,   964,  -953,   -62,  -953,    11,     7,  -953,     4,
      73,  -952,  -114,  -953,  -953,    27,  -953,  -953,    58,  -220,
    -953,   501,   -49,  -953,   -14,   202,     9,  -953,  -953,  -953,
    -953,  -953,   432,   203,  -953,  -953,   762,   -57,  -264,   594,
    -953,  -953,   618,   425,  -953,  -953,  -953,   470
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   298,   299,   336,   464,     2,   301,   765,   197,
      92,   524,   525,    93,   135,   546,    96,    97,   520,   302,
     766,   497,   199,   644,   767,   868,   200,   768,   769,   201,
     186,   331,   547,   548,   758,   764,   998,  1042,   863,   507,
     508,   600,    99,   973,  1017,   100,   559,   214,   101,   156,
     157,   102,   103,   215,   104,   216,   105,   217,   684,   946,
    1080,   679,   682,   777,   756,  1029,   918,   850,   761,   852,
     106,   856,   857,   858,   859,   747,   107,   108,   109,   110,
     824,   825,   826,   827,   828,   899,   788,   789,   790,   791,
     792,   900,   793,   902,   903,   904,   967,   168,   469,   165,
     465,   470,   471,   202,   203,   206,   207,   873,   947,   948,
     779,  1048,  1084,  1085,  1086,  1087,  1088,  1089,  1185,   949,
     950,   951,   829,   830,  1050,  1051,  1052,  1148,  1135,  1164,
    1165,  1182,  1197,  1187,  1060,  1061,   187,   171,   172,   748,
     479,   480,   159,   635,   111,   112,   113,   114,   115,   137,
     601,  1064,   394,   980,  1069,  1070,   117,   401,   118,   161,
     343,   169,   119,   120,   121,   122,   182,   123,   124,   125,
     126,   127,   128,   129,   130,   163,   605,   613,   338,   339,
     340,   341,   328,   329,   700,   131,   511,   512
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,   832,   395,   300,   351,   871,   116,   196,    95,   496,
     865,   905,   309,   309,   869,   136,   138,   139,   140,   141,
     158,   759,   -14,   418,   155,   310,   310,   991,   686,   143,
     144,  -280,   146,   147,   148,   149,   150,  -293,   166,   522,
     523,   173,   952,   598,   529,   351,   393,   393,   183,   181,
     181,   808,  1023,   188,  1025,   181,   208,   190,   493,   782,
     311,   311,   375,   610,    15,   351,     3,    15,   393,   836,
     -84,  1066,   323,  1071,   539,   494,  1013,   132,    12,    13,
      14,   494,    15,   323,   160,   794,   527,   315,   316,   317,
     318,   319,   320,   321,   322,   355,  1040,   330,   116,   337,
      95,   353,   142,   132,    12,    13,    14,    15,    98,  1067,
     906,   599,   897,   323,   952,   324,  1041,   209,   191,  1059,
     323,   193,   342,   952,   954,   323,   -84,   539,   647,  1058,
     132,    12,    13,    14,   905,   323,   783,   174,    41,    42,
     208,   145,   353,   369,   370,   371,   305,   854,   309,   158,
     423,  1132,   922,   923,   167,   580,   419,   349,   196,   784,
     785,   310,   151,   323,   421,   345,   673,   922,   923,   170,
     675,  1005,   952,   375,   323,   467,   539,   495,   472,  1058,
     952,   473,  1001,   664,   690,   388,   477,   481,   482,  1003,
     484,   485,   795,   167,   760,   879,   311,   992,   205,   209,
     736,   974,   739,   740,  1170,  1171,   326,   327,   175,   889,
    1011,  -523,   784,   785,   356,   357,   347,   326,   327,   611,
    1058,    91,   510,   513,    91,   167,   426,  1181,   556,   162,
    -522,   786,   952,  1189,   474,  1191,   475,   908,   920,    91,
     924,   952,    41,    42,   184,  1200,   325,   326,   327,   882,
    1179,   204,   334,   335,   326,   327,   955,   153,   540,   326,
     327,  1063,    41,    42,    91,   309,   396,   925,   534,   326,
     327,    50,    51,   350,   309,  1072,   312,   312,   310,   152,
     191,   192,   993,   193,   898,   707,   846,   310,   367,   368,
     369,   370,   371,   176,   348,  -224,   388,   326,   327,  1155,
     158,   177,  1144,   977,   155,  -567,  -567,    73,   326,   327,
    1015,   667,   847,   311,   337,   337,  1194,  1163,   528,   337,
     375,  -224,   311,   178,  1163,  1172,    50,    51,   194,   545,
     195,  1163,   185,   617,   152,   116,  -567,   537,   603,   607,
    1195,  1163,   614,  1196,   550,   905,   191,   192,   189,   193,
    1145,   132,    12,    13,    14,  -564,  -564,   558,   557,    73,
     154,  -606,  -606,   526,   160,  -606,  1183,   397,  -564,  1184,
     637,   638,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,  -564,   581,   582,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   366,   367,   368,   369,   370,   371,   615,
     151,   181,   312,   618,   619,   620,   621,   622,   623,   624,
     625,   626,   627,   628,   629,   630,   867,  -225,   631,   179,
    1167,   633,   717,   160,   158,  -227,   375,   180,   155,   640,
     210,   181,   490,   388,   143,   144,   656,   146,   147,   148,
     149,   211,   166,  -225,   208,   351,   212,   190,  -605,  -605,
     196,  -227,  -605,   561,  -186,  1190,   616,   597,  -186,   191,
     398,   399,   193,   641,   213,   358,   359,   360,   367,   368,
     369,   370,   371,   709,   896,   173,   490,   901,   491,   652,
     844,   545,   132,    12,    13,    14,   303,   116,   160,   304,
     659,   400,   132,    12,    13,    14,   313,   314,  -560,   162,
     375,   334,   335,   209,   927,   670,   930,   194,   932,   195,
      41,    42,   132,    12,    13,    14,    15,   939,  -568,   312,
     385,   386,   388,   427,   428,  -565,  -565,   424,   312,   531,
     532,   888,   132,    12,    13,    14,   429,   425,  -565,   116,
     476,   706,   353,   486,   500,   309,   501,   969,   970,   388,
     337,   783,  -571,   710,   430,   337,  -565,   874,   310,  1046,
     533,   715,  1047,   716,   784,   785,   718,   487,   162,   913,
     488,   151,    50,    51,   784,   785,   755,   701,   702,   680,
     152,   691,   426,   309,   489,   502,  -202,   503,   492,   604,
     608,   612,   612,   311,   385,   386,   310,   861,   862,   901,
     309,   837,   499,   723,   839,    73,   504,   725,   132,    12,
      13,    14,   727,   310,   784,   785,   514,   505,   636,   506,
     612,   612,   639,   388,  1018,  1019,  1019,  1022,   515,   154,
     154,   311,   509,   162,   516,   876,   898,   309,   517,   877,
    -561,  -561,   696,   735,   472,   518,   472,   472,   311,   519,
     310,   744,   697,  -561,   750,   752,   698,  -563,  -563,   521,
     724,   699,  -571,   721,   726,  -570,   153,   783,   728,   729,
    -563,  -561,   549,    91,    94,   510,   958,   959,    50,    51,
     116,   553,    95,   309,  -570,   311,   152,   500,  -563,   666,
     784,   785,   354,  1020,  1021,   204,   310,   344,   346,   154,
     921,   922,   923,   802,  1093,   545,  1094,   919,   823,   554,
     823,   116,   807,   555,   560,  -562,  -562,  1100,  1109,  1101,
     812,   562,   763,   351,   563,  1100,   775,  1130,  -562,   780,
     564,   311,   956,   957,   957,   960,   -43,   526,   -70,   535,
     -66,   -67,  -569,   541,   743,   781,  -562,   746,   -68,   538,
     965,   -62,   842,   -63,   -65,   337,   -64,   535,   551,   541,
     535,   541,   786,   860,   154,   -73,   -69,   755,    41,    42,
      43,   988,   922,   923,   309,   986,   642,   643,   646,   350,
      50,    51,    98,    94,   650,   653,   654,   310,   152,   116,
     662,    95,   663,  1000,    57,    58,    59,    60,   885,   665,
     205,   672,   674,   678,   681,   683,   337,   677,   687,   312,
     693,   685,   688,    73,   689,   694,   695,   704,   705,   823,
     711,   714,   311,   720,   780,   545,   722,   545,  -592,  -594,
     730,   116,   733,   116,   737,   776,   741,   494,   762,   770,
     872,   481,   912,   772,   337,   351,   774,   312,   778,   797,
    1036,  1037,   799,  -569,   849,   800,   801,   883,   803,   840,
     804,   945,   805,   831,   312,   132,    12,    13,    14,    15,
     813,   814,   815,   816,   817,   818,   819,   820,   821,   822,
     841,   843,   864,   823,  1053,   851,  1073,   853,   866,   870,
     874,    98,   880,  1075,  1076,   881,   875,   882,   887,   891,
     892,   312,   895,   894,    73,   890,  -593,   545,   966,   907,
     935,   926,   909,   116,   914,   929,   975,   931,   928,   934,
     938,   933,   953,   990,   151,    50,    51,   879,   940,   393,
     860,   937,   996,   152,   961,   972,  1045,   962,   976,   963,
     977,   978,   945,   917,  1008,  1068,   979,   312,   982,   983,
     989,    41,   964,   545,   984,   985,   994,   997,    73,   116,
    1002,  1012,  1009,  1014,  1024,  1136,   974,  1016,  1161,  1139,
    1026,   981,  1141,  1032,   545,  1049,  1033,  1074,  1065,  1083,
     116,   999,  1079,  1091,  1078,  1095,  1096,   -75,   545,   -54,
     780,   945,   -55,  1097,   116,  1038,  1104,  1106,  1039,   945,
    1107,  1108,   154,  1110,  1004,   154,  1115,   787,  1113,  -522,
    1118,  1119,  1121,   154,  1120,  1122,  1083,  1124,  1125,   153,
    1134,    41,    42,  1140,   545,   545,    91,  1138,  1149,  1068,
     116,   116,   350,    50,    51,  1151,  1160,  1176,   312,   731,
    1174,   152,  1178,  1188,  1193,  1186,  1143,    57,    58,    59,
      60,   945,   798,   352,  1146,  1199,   198,   936,   498,   668,
     945,   545,   351,   545,   545,  1103,    73,   116,   483,   116,
     116,   771,   915,   995,  1030,   893,  1159,   757,   968,  1028,
    1114,   917,   878,  1007,   971,   393,   466,   671,  1077,   669,
    1168,  1006,  1123,  1126,   509,  1090,   941,  1128,  1129,  1117,
     154,  1173,  1099,   911,  -359,  1133,   787,  1131,   660,   422,
     787,   749,  1092,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   943,   545,  1137,   732,   545,  1105,   545,
     116,   806,  1090,   116,   773,   116,  1162,   649,   712,   823,
       0,     0,     0,  1162,     0,     0,     0,    73,     0,   609,
    1166,   787,     0,     0,     0,     0,     0,     0,  1127,     0,
    1162,     0,     0,     0,     0,     0,     0,     0,     0,   780,
       0,     0,     0,     0,   823,     0,   545,   845,     0,     0,
       0,     0,   116,     0,     0,     0,  1147,   545,  1150,     0,
       0,  1152,  1153,   116,     0,  1198,  1154,     0,   154,     0,
       0,     0,     0,   787,   787,   787,   787,     0,     0,  -607,
    -607,   366,   367,   368,   369,   370,   371,     0,   734,     0,
       0,   787,   787,     0,     0,     4,     5,     0,     0,  1175,
       6,     7,     8,     0,     9,    10,   132,    12,    13,    14,
      15,  1180,     0,    17,   375,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,   154,    26,    27,
      28,     0,     0,     0,   402,     0,   154,     0,   757,     0,
       0,     0,     0,   787,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
     787,    46,     0,     0,     0,   133,     0,     0,   787,   787,
     787,   787,     0,     0,   134,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,   787,     0,  -607,  -607,     0,   385,   386,
     813,   814,   815,   816,   817,   818,   819,   820,   821,   822,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,   154,     0,     0,   388,     0,   389,
       0,   154,    84,     0,    73,     0,     0,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,     0,
      86,     0,     0,     0,    88,    89,    90,    91,     0,     0,
     415,   416,     0,     0,     0,     0,     0,     0,   154,     0,
    -561,  -561,     0,     0,     0,     0,     0,     0,   431,     0,
       0,     0,     0,  -561,     0,     4,     5,     0,   417,     0,
       6,     7,     8,     0,     9,    10,   432,    12,    13,    14,
      15,  -561,     0,    17,     0,   154,   433,   434,   435,   436,
     437,   225,   226,   227,   438,   439,    25,   230,   440,   441,
     442,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     443,   254,   255,   256,   444,   445,   259,   260,   261,   262,
     263,   446,   265,   266,   267,   447,   269,   270,   271,   272,
     273,     0,     0,     0,   448,   275,   276,   449,   450,     0,
     279,   280,   281,   282,   283,   284,   285,   451,   452,   288,
     453,   454,   455,   456,   457,   458,   459,   460,   461,    73,
    -358,     0,  -360,     0,     0,     0,     0,     0,     0,     0,
       0,   813,   814,   815,   816,   817,   818,   819,   820,   821,
     822,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,   813,   814,   815,   816,   817,   818,   819,   820,
     821,   822,    84,     0,     0,     0,     0,     0,   462,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      86,   463,   431,     0,    88,    89,    90,    91,     0,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     432,    12,    13,    14,    15,     0,     0,    17,     0,     0,
     433,   434,   435,   436,   437,   225,   226,   227,   438,   439,
      25,   230,   440,   441,   442,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   443,   254,   255,   256,   444,   445,
     259,   260,   261,   262,   263,   446,   265,   266,   267,   447,
     269,   270,   271,   272,   273,     0,     0,     0,   448,   275,
     276,   449,   450,     0,   279,   280,   281,   282,   283,   284,
     285,   451,   452,   288,   453,   454,   455,   456,   457,   458,
     459,   460,   461,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,   462,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    86,   468,   431,     0,    88,    89,
      90,    91,     0,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   432,    12,    13,    14,    15,     0,
       0,    17,     0,     0,   433,   434,   435,   436,   437,   225,
     226,   227,   438,   439,    25,   230,   440,   441,   442,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   443,   254,
     255,   256,   444,   445,   259,   260,   261,   262,   263,   446,
     265,   266,   267,   447,   269,   270,   271,   272,   273,     0,
       0,     0,   448,   275,   276,   449,   450,     0,   279,   280,
     281,   282,   283,   284,   285,   451,   452,   288,   453,   454,
     455,   456,   457,   458,   459,   460,   461,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,   462,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,   738,
     431,     0,    88,    89,    90,    91,     0,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,   432,    12,
      13,    14,    15,     0,     0,    17,     0,     0,   433,   434,
     435,   436,   437,   225,   226,   227,   438,   439,    25,   230,
     440,   441,   442,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   443,   254,   255,   256,   444,   445,   259,   260,
     261,   262,   263,   446,   265,   266,   267,   447,   269,   270,
     271,   272,   273,     0,     0,     0,   448,   275,   276,   449,
     450,     0,   279,   280,   281,   282,   283,   284,   285,   451,
     452,   288,   453,   454,   455,   456,   457,   458,   459,   460,
     461,    73,     0,     0,   813,   814,   815,   816,   817,   818,
     819,   820,   821,   822,     0,   813,   814,   815,   816,   817,
     818,   819,   820,   821,   822,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,    73,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,    73,
     462,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,    86,     0,   742,     0,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,   375,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,  1169,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,  1177,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,    53,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   813,   814,   815,   816,   817,
     818,   819,   820,   821,   822,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,   388,     0,   389,   390,   391,    84,     0,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,   745,    88,
      89,    90,    91,     0,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,  -263,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
       0,     0,     0,   134,     0,    53,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,    73,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,   848,    88,    89,    90,    91,     0,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,  -130,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,    53,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,   916,    88,    89,    90,
      91,     0,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,     0,     0,     0,     0,     0,     0,     0,
       0,   134,     0,    53,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
    1027,    88,    89,    90,    91,     0,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,     0,     0,     0,
       0,     0,     0,     0,     0,   134,     0,    53,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,   565,    88,    89,    90,    91,     0,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,   132,    12,    13,    14,    15,     0,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,    46,     0,     0,     0,
     133,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,     0,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    86,     0,   -47,     0,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   132,    12,    13,    14,    15,     0,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    41,    42,     0,     0,     0,     0,     0,    46,
       0,     0,     0,   133,     0,     0,     0,     0,     0,     0,
       0,     0,   134,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,   372,   373,   374,     0,     0,     0,   375,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    88,    89,    90,    91,   361,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   372,
     373,   374,     0,     0,     0,   375,     0,     0,     0,     0,
       0,   372,   373,   374,     0,     0,     0,   375,     0,     0,
       0,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   361,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   361,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,     0,   387,
     388,     0,   389,   390,   391,     0,   372,   373,   374,     0,
       0,     0,   375,   703,     0,     0,     0,     0,   372,   373,
     374,     0,     0,     0,   375,     0,     0,     0,     0,     0,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   361,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,     0,   387,   388,     0,
     389,   390,   391,     0,     0,     0,     0,     0,     0,   387,
     388,   713,   389,   390,   391,     0,   372,   373,   374,     0,
       0,     0,   375,   833,     0,     0,     0,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,     0,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     361,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,     0,     0,   387,   388,     0,   389,   390,   391,
       0,     0,     0,     0,     0,     0,   387,   388,   834,   389,
     390,   391,     0,   372,   373,   374,     0,     0,     0,   375,
     835,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,     0,     0,
     361,     0,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   387,   388,     0,   389,   390,   391,
       0,     0,   645,   372,   373,   374,   542,     0,     0,   375,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   361,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   387,   388,     0,   389,   390,   391,     0,   372,   373,
     374,     0,     0,   542,   375,     0,     0,     0,     0,     0,
       0,   372,   373,   374,     0,     0,     0,   375,     0,     0,
       0,     0,     0,     0,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   361,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   387,   388,     0,   389,   390,   391,     0,   372,   373,
     374,     0,     0,   632,   375,     0,     0,     0,     0,     0,
       0,   372,   373,   374,     0,     0,     0,   375,     0,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
       0,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   361,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   387,   388,     0,   389,
     390,   391,     0,     0,     0,     0,     0,     0,   634,   387,
     388,     0,   389,   390,   391,     0,   372,   373,   374,     0,
       0,   648,   375,     0,     0,     0,     0,     0,     0,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
       0,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   361,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   387,   388,     0,   389,
     390,   391,     0,     0,     0,     0,     0,     0,   651,   387,
     388,     0,   389,   390,   391,     0,   372,   373,   374,     0,
       0,   657,   375,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   361,     0,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   387,   388,     0,   389,   390,   391,
       0,   372,   373,   374,     0,     0,   658,   375,     0,     0,
       0,     0,     0,     0,   372,   373,   374,     0,     0,     0,
     375,     0,     0,     0,     0,     0,     0,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   361,     0,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   361,     0,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   387,   388,     0,   389,   390,   391,
       0,   372,   373,   374,     0,     0,   676,   375,     0,     0,
       0,     0,     0,     0,   372,   373,   374,     0,     0,     0,
     375,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,     0,     0,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   387,
     388,     0,   389,   390,   391,     0,     0,     0,     0,     0,
       0,   692,   387,   388,     0,   389,   390,   391,     0,     0,
       0,     0,     0,     0,   809,     0,     0,     0,     0,     0,
       0,     0,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,     0,     0,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   387,
     388,     0,   389,   390,   391,     0,     0,     0,     0,     0,
       0,   810,   387,   388,     0,   389,   390,   391,     0,     0,
       4,     5,     0,     0,   838,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,     0,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     0,     0,    46,    47,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,    52,
       0,    53,    54,    55,    56,    57,    58,    59,    60,     0,
       0,    61,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,   796,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,    43,    44,    45,     0,     0,    46,
      47,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,     0,     0,    61,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,   884,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       0,     0,    46,    47,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,     0,     0,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
      53,    54,    55,   543,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,   544,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,    52,     0,    53,    54,    55,   543,    57,    58,    59,
      60,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
     661,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,   811,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,   543,
      57,    58,    59,    60,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,     0,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,   910,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,   543,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,     0,    88,    89,    90,
      91,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,    11,    12,    13,    14,    15,    16,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,     0,    33,     0,    34,     0,    35,
     987,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,    42,     0,    44,    45,     0,     0,    46,     0,     0,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
      52,     0,    53,    54,    55,   543,    57,    58,    59,    60,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,    86,     0,    87,     0,
      88,    89,    90,    91,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,  1031,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,     0,    44,    45,     0,     0,
      46,     0,     0,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,    52,     0,    53,    54,    55,   543,    57,
      58,    59,    60,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,     0,    88,    89,    90,    91,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,     0,    34,     0,    35,     0,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,    42,     0,    44,
      45,     0,     0,    46,     0,     0,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,    52,     0,    53,    54,
      55,   543,    57,    58,    59,    60,     0,     0,     0,    62,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,  1043,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,     0,    33,  1112,    34,     0,    35,     0,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    44,    45,     0,     0,    46,     0,     0,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,    52,
       0,    53,    54,    55,   543,    57,    58,    59,    60,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,    87,     0,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,    11,    12,    13,    14,    15,    16,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,     0,    34,
       0,    35,     0,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,    42,     0,    44,    45,     0,     0,    46,
       0,     0,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,    52,     0,    53,    54,    55,   543,    57,    58,
      59,    60,     0,     0,     0,    62,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,     0,
      87,  1156,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,    11,    12,    13,
      14,    15,    16,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,     0,    34,     0,    35,     0,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,    42,     0,    44,    45,
       0,     0,    46,     0,     0,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,    52,     0,    53,    54,    55,
     543,    57,    58,    59,    60,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,    87,  1157,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,     0,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,     0,    34,     0,    35,     0,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,    42,
       0,    44,    45,     0,     0,    46,     0,     0,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,    52,     0,
      53,    54,    55,   543,    57,    58,    59,    60,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,    87,  1158,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,    11,    12,    13,    14,    15,    16,     0,
      17,     0,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,     0,    34,     0,
      35,     0,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,    42,     0,    44,    45,     0,     0,    46,     0,
       0,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,    52,     0,    53,    54,    55,   543,    57,    58,    59,
      60,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
    1192,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,    11,    12,    13,    14,
      15,    16,     0,    17,     0,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
       0,    34,     0,    35,     0,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    44,    45,     0,
       0,    46,     0,     0,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,    52,     0,    53,    54,    55,   543,
      57,    58,    59,    60,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,    87,  1202,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,     0,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,     0,    34,     0,    35,     0,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,    42,     0,
      44,    45,     0,     0,    46,     0,     0,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,    52,     0,    53,
      54,    55,   543,    57,    58,    59,    60,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,     0,     0,     0,     0,
       0,     0,     0,   782,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,    87,     0,    88,    89,    90,
      91,  1054,    12,    13,    14,     0,     0,     0,     0,     0,
       0,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
    1055,   269,   270,   271,   272,   273,     0,     0,     0,   274,
     275,   276,   277,   278,     0,   279,   280,   281,   282,   283,
     284,   285,   286,  1056,  1057,   289,   290,   291,   292,   293,
     294,   295,   296,   297,     4,     5,     0,     0,     0,     6,
       7,     8,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    17,     0,     0,    18,    19,    20,    21,    22,
       0,     0,     0,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,     0,
      34,     0,    35,     0,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,    42,   786,    44,    45,     0,     0,
      46,     0,     0,    48,    49,     0,     0,     0,     0,     0,
       0,     0,     0,   134,     0,    53,    54,    55,     0,     0,
       0,     0,     0,     0,     0,     0,    62,    63,     0,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,    87,     0,    88,    89,    90,    91,     4,     5,     0,
       0,     0,     6,     7,     8,     0,     9,    10,   132,    12,
      13,    14,    15,     0,     0,    17,     0,     0,    18,    19,
      20,    21,    22,     0,     0,     0,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,     0,     0,     0,    41,    42,     0,     0,
       0,     0,     0,    46,     0,     0,     0,   133,     0,     0,
       0,     0,     0,     0,     0,     0,   134,     0,     0,    54,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   332,
      63,     0,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,     0,     0,     0,     0,     0,
     333,     0,     0,     0,     0,   334,   335,     0,     0,     0,
       0,     0,    86,     0,     0,     0,    88,    89,    90,    91,
       4,     5,     0,     0,     0,     6,     7,     8,     0,     9,
      10,   132,    12,    13,    14,    15,     0,     0,    17,     0,
       0,    18,    19,    20,    21,    22,     0,     0,     0,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    37,     0,     0,     0,    41,
      42,     0,     0,     0,     0,     0,    46,     0,     0,     0,
     133,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,     0,    54,    55,     0,     0,     0,     0,     0,     0,
       0,     0,    62,    63,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   334,   335,
       0,     0,     0,     0,     0,    86,     0,     0,     0,    88,
      89,    90,    91,     4,     5,     0,     0,     0,     6,     7,
       8,     0,     9,    10,   132,    12,    13,    14,    15,     0,
       0,    17,     0,     0,    18,    19,    20,    21,    22,     0,
       0,     0,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,    41,    42,     0,     0,     0,     0,     0,    46,
       0,     0,     0,   133,     0,     0,     0,     0,     0,     0,
       0,     0,   134,     0,     0,    54,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   708,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,   335,     0,     0,     0,     0,     0,    86,     0,
       0,     0,    88,    89,    90,    91,     4,     5,     0,     0,
       0,     6,     7,     8,     0,     9,    10,   132,    12,    13,
      14,    15,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,     0,     0,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,     0,    41,    42,     0,     0,     0,
       0,     0,    46,     0,     0,     0,   133,     0,     0,     0,
       0,     0,     0,     0,     0,   134,     0,     0,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,    62,    63,
       0,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,    81,    82,   478,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,     0,     0,     0,    88,    89,    90,    91,     4,
       5,     0,     0,     0,     6,     7,     8,     0,     9,    10,
     132,    12,    13,    14,    15,     0,     0,    17,   536,     0,
      18,    19,    20,    21,    22,     0,     0,     0,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    37,     0,     0,     0,    41,    42,
       0,     0,     0,     0,     0,    46,     0,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,     0,   134,     0,
       0,    54,    55,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     0,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,    86,     0,     0,     0,    88,    89,
      90,    91,     4,     5,     0,     0,     0,     6,     7,     8,
       0,     9,    10,   132,    12,    13,    14,    15,     0,     0,
      17,   375,     0,    18,    19,    20,    21,    22,     0,     0,
       0,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    41,    42,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   133,     0,     0,     0,     0,     0,     0,     0,
       0,   134,     0,     0,    54,    55,     0,     0,     0,     0,
       0,     0,     0,     0,    62,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,     0,   378,   379,
     380,   381,   382,   383,   384,   385,   386,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,    81,    82,   751,
       0,     0,     0,     0,   388,     0,   389,   390,   391,    84,
       0,     0,     0,     0,     0,     0,     0,     0,   363,   364,
     365,   366,   367,   368,   369,   370,   371,    86,     0,     0,
       0,    88,    89,    90,    91,     4,     5,     0,     0,     0,
       6,     7,     8,     0,     9,    10,   132,    12,    13,    14,
      15,     0,     0,    17,   375,     0,    18,    19,    20,    21,
      22,     0,     0,     0,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   855,
      37,     0,     0,     0,    41,    42,     0,     0,     0,     0,
       0,    46,     0,     0,     0,   133,     0,     0,     0,     0,
       0,     0,     0,     0,   134,     0,     0,    54,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    63,     0,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
       0,   378,   379,   380,   381,   382,   383,   384,   385,   386,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,     0,   388,     0,   389,
     390,   391,    84,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   364,   365,   366,   367,   368,   369,   370,   371,
      86,     0,     0,     0,    88,    89,    90,    91,     4,     5,
       0,     0,     0,     6,     7,     8,     0,     9,    10,   132,
      12,    13,    14,    15,     0,     0,    17,   375,     0,    18,
      19,    20,    21,    22,     0,     0,     0,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    37,     0,     0,     0,    41,    42,     0,
       0,     0,     0,     0,    46,     0,     0,     0,   133,     0,
       0,     0,     0,     0,     0,     0,     0,   134,     0,     0,
      54,    55,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    63,     0,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,     0,   378,   379,   380,   381,   382,   383,
     384,   385,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,     0,     0,     0,
     388,     0,   389,   390,   391,    84,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    86,     0,     0,     0,    88,    89,    90,
      91,     4,     5,     0,     0,     0,     6,     7,     8,     0,
       9,    10,   132,    12,    13,    14,    15,     0,     0,    17,
       0,     0,    18,    19,    20,    21,    22,     0,     0,     0,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,     0,
      41,    42,     0,     0,     0,     0,     0,    46,     0,     0,
       0,   133,     0,     0,     0,     0,     0,     0,     0,     0,
     134,     0,     0,    54,    55,     0,     0,     0,     0,     0,
       0,     0,     0,    62,    63,     0,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,   361,  1034,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,   372,
     373,   374,     0,     0,     0,   375,     0,     0,    84,     0,
       0,     0,     0,     0,   361,     0,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   164,     0,     0,     0,
      88,    89,    90,    91,   361,     0,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,   372,   373,   374,
       0,     0,     0,   375,     0,     0,     0,   941,     0,     0,
       0,     0,     0,     0,     0,  -359,     0,   372,   373,   374,
       0,   942,     0,   375,   813,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   943,     0,     0,     0,     0,     0,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,     0,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   387,   388,     0,
     389,   390,   391,     0,     0,     0,     0,  1035,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,     0,
       0,     0,   944,     0,     0,   387,   388,     0,   389,   390,
     391,     0,     0,     0,     0,   392,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,   388,     0,   389,   390,
     391,     0,     0,     0,   361,   530,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   361,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   372,   373,   374,
       0,     0,     0,   375,     0,     0,     0,     0,     0,   372,
     373,   374,     0,     0,     0,   375,     0,     0,     0,     0,
       0,     0,     0,   361,     0,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   361,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   372,   373,   374,     0,
       0,     0,   375,     0,     0,     0,     0,     0,   372,   373,
     374,     0,     0,     0,   375,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,     0,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,   388,     0,   389,   390,
     391,     0,     0,     0,     0,  1201,     0,   387,   388,     0,
     389,   390,   391,     0,     0,     0,   886,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,     0,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
       0,     0,   361,     0,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   387,   388,     0,   389,   390,   391,
       0,     0,  -409,     0,     0,     0,   387,   388,     0,   389,
     390,   391,     0,     0,  -406,   372,   373,   374,     0,     0,
       0,   375,     0,     0,     0,   361,     0,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   361,   719,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   372,   373,
     374,     0,     0,     0,   375,     0,     0,     0,     0,     0,
     372,   373,   374,     0,     0,     0,   375,     0,     0,     0,
       0,     0,   655,     0,   361,     0,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   372,   373,   374,
       0,     0,     0,   375,     0,     0,     0,     0,     0,   552,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   387,   388,     0,   389,   390,   391,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
       0,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   388,     0,   389,
     390,   391,     0,     0,     0,     0,     0,     0,   387,   388,
       0,   389,   390,   391,     0,     0,     0,     0,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   361,
       0,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   361,     0,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,     0,   387,   388,     0,   389,   390,
     391,     0,     0,   373,   374,     0,     0,     0,   375,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     375,     0,     0,     0,   361,     0,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   361,     0,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   374,
       0,     0,     0,   375,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   375,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   375,     0,     0,     0,
       0,     0,     0,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,     0,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   420,     0,
     387,   388,     0,   389,   390,   391,     0,     0,     0,     0,
       0,     0,   387,   388,     0,   389,   390,   391,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,     0,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,     0,     0,     0,   387,   388,     0,   389,   390,
     391,     0,     0,     0,     0,     0,     0,   387,   388,     0,
     389,   390,   391,     0,     0,     0,   218,     0,   387,   388,
      15,   389,   390,   391,     0,     0,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,     0,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,     0,     0,     0,   274,   275,   276,   277,   278,     0,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   941,
       0,     0,     0,     0,     0,     0,     0,  -359,     0,     0,
       0,     0,     0,   942,     0,     0,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   943,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
      73,     0,   602,    15,     0,     0,     0,    91,     0,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,  1010,     0,     0,   274,   275,   276,
     277,   278,     0,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,     0,   132,    12,    13,    14,    15,     0,     0,
      17,     0,     0,     0,   132,    12,    13,    14,    15,     0,
       0,    17,     0,     0,     0,   306,     0,     0,     0,     0,
       0,     0,   941,     0,     0,     0,   306,     0,     0,     0,
    -359,     0,     0,     0,     0,     0,   942,     0,     0,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   943,
       0,     0,   151,     0,     0,   606,     0,     0,     0,     0,
      91,   134,     0,   151,   364,   365,   366,   367,   368,   369,
     370,   371,   134,    73,   753,    63,     0,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    63,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,     0,     0,   375,
       0,     0,     0,   364,   365,   366,   367,   368,   369,   370,
     371,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1044,   375,     0,
     334,   335,     0,     0,     0,     0,     0,   307,     0,     0,
       0,   754,     0,    90,    91,     0,     0,     0,   307,     0,
       0,     0,   308,     0,    90,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -607,  -607,  -607,  -607,
     382,   383,  -607,   385,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   388,     0,   389,   378,   379,   380,   381,   382,
     383,   384,   385,   386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   432,    12,    13,    14,     0,     0,
       0,   388,     0,   389,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,     0,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,  1081,   269,   270,   271,   272,   273,     0,
       0,     0,   274,   275,   276,   277,   278,     0,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,     0,     0,   941,
       0,     0,     0,     0,     0,     0,     0,  -359,     0,     0,
       0,     0,     0,   942,     0,     0,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   943,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   432,    12,    13,    14,     0,     0,     0,     0,     0,
      73,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,  1082,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
    1081,   269,   270,   271,   272,   273,     0,     0,     0,   274,
     275,   276,   277,   278,  1062,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   941,     0,     0,     0,     0,     0,
       0,     0,  -359,     0,     0,     0,     0,     0,   942,     0,
       0,   813,   814,   815,   816,   817,   818,   819,   820,   821,
     822,   943,     0,     0,     0,     0,     0,     0,     0,   941,
       0,     0,     0,     0,     0,     0,     0,  -359,     0,     0,
       0,     0,     0,   942,     0,    73,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   943,     0,  1116,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1142,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,  1111,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   275,   276,   277,   278,     0,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   218,     0,     0,
     219,     0,     0,     0,     0,     0,     0,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,     0,     0,     0,   274,   275,   276,   277,   278,
       0,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
       0,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,     0,     0,     0,   274,   275,
     276,   277,   278,     0,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,  1098,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,     0,     0,
       0,   274,   275,   276,   277,   278,     0,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297
};

static const yytype_int16 yycheck[] =
{
       2,   722,   116,    61,    94,   776,     2,    47,     2,   198,
     766,   829,    74,    75,   770,     4,     5,     6,     7,     8,
      26,     6,     6,   137,    26,    74,    75,     6,   521,    18,
      19,     7,    21,    22,    23,    24,    25,     7,    27,   303,
     304,    30,   873,    23,   308,   135,   115,   116,    39,    38,
      39,   714,   978,    44,   980,    44,    49,    46,   189,     5,
      74,    75,    44,    23,    27,   155,     0,    27,   137,   732,
       6,  1023,    27,  1025,    29,    62,   966,    23,    24,    25,
      26,    62,    27,    27,    26,   684,   306,    76,    77,    78,
      79,    80,    81,    82,    83,   101,     7,    86,    94,    88,
      94,    94,     6,    23,    24,    25,    26,    27,     2,    27,
     831,    91,     5,    27,   945,    29,    27,    52,    23,  1009,
      27,    26,    29,   954,   875,    27,    62,    29,   469,  1009,
      23,    24,    25,    26,   952,    27,    82,    31,    71,    72,
     133,   177,   135,    14,    15,    16,    73,   760,   210,   155,
     156,  1103,    65,    66,   177,   375,   145,    92,   198,   105,
     106,   210,    82,    27,   153,    29,   507,    65,    66,   177,
     511,   942,  1003,    44,    27,   164,    29,   164,   167,  1059,
    1011,   170,   938,   164,   525,   167,   175,   176,   177,   940,
     179,   180,   685,   177,   179,   171,   210,   176,    27,   134,
     643,   171,   645,   646,  1156,  1157,   161,   162,   177,   808,
     961,   163,   105,   106,    49,    50,   179,   161,   162,   179,
    1100,   184,   211,   212,   184,   177,   161,  1179,   183,    26,
     163,   177,  1063,  1185,   174,  1187,   176,   836,   851,   184,
     853,  1072,    71,    72,    23,  1197,   160,   161,   162,   171,
    1176,    48,   170,   171,   161,   162,   178,   177,   160,   161,
     162,  1012,    71,    72,   184,   327,   163,   180,   160,   161,
     162,    83,    84,    82,   336,  1026,    74,    75,   327,    91,
      23,    24,   180,    26,   177,   549,   152,   336,    12,    13,
      14,    15,    16,   177,    91,   152,   167,   161,   162,  1134,
     306,   177,  1120,   171,   306,   150,   151,   116,   161,   162,
     178,   500,   178,   327,   303,   304,   152,  1148,   307,   308,
      44,   178,   336,   177,  1155,  1160,    83,    84,    71,   331,
      73,  1162,    23,   402,    91,   331,   181,   326,   396,   397,
     176,  1172,   399,   179,   333,  1163,    23,    24,   179,    26,
    1121,    23,    24,    25,    26,   150,   151,   350,   347,   116,
      26,   174,   175,   305,   306,   178,   176,   163,   163,   179,
     427,   428,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   181,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,    11,    12,    13,    14,    15,    16,   400,
      82,   400,   210,   402,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   767,   152,   417,   177,
    1151,   420,   563,   375,   440,   152,    44,   177,   440,   430,
     177,   430,   174,   167,   433,   434,   178,   436,   437,   438,
     439,   177,   441,   178,   447,   545,   177,   446,   174,   175,
     500,   178,   178,   357,   174,  1186,   401,   394,   178,    23,
     150,   151,    26,   462,   177,    49,    50,    51,    12,    13,
      14,    15,    16,   552,   825,   474,   174,   827,   176,   478,
     754,   493,    23,    24,    25,    26,   177,   493,   440,   177,
     489,   181,    23,    24,    25,    26,    74,    75,   163,   306,
      44,   170,   171,   448,   855,   504,   857,    71,   859,    73,
      71,    72,    23,    24,    25,    26,    27,   868,   177,   327,
     138,   139,   167,   150,   151,   150,   151,    97,   336,   150,
     151,   805,    23,    24,    25,    26,   163,   163,   163,   545,
      54,   545,   545,   176,   174,   617,   176,   897,   898,   167,
     549,    82,   177,   552,   181,   554,   181,   174,   617,   176,
     181,   560,   179,   562,   105,   106,   565,   176,   375,   843,
     176,    82,    83,    84,   105,   106,   655,   180,   181,   516,
      91,   526,   527,   655,     4,   174,    97,   176,   176,   396,
     397,   398,   399,   617,   138,   139,   655,    76,    77,   949,
     672,   742,   176,   602,   745,   116,     4,   606,    23,    24,
      25,    26,   611,   672,   105,   106,   178,   174,   425,   176,
     427,   428,   429,   167,   974,   975,   976,   977,    23,   305,
     306,   655,   210,   440,    23,   782,   177,   709,    23,   786,
     150,   151,    13,   642,   643,    23,   645,   646,   672,   176,
     709,   650,    23,   163,   653,   654,    27,   150,   151,   179,
     605,    32,   177,   600,   609,   177,   177,    82,   613,   614,
     163,   181,   177,   184,   686,   674,   880,   881,    83,    84,
     686,   175,   686,   755,   177,   709,    91,   174,   181,   176,
     105,   106,    97,   975,   976,   502,   755,    89,    90,   375,
      64,    65,    66,   702,   174,   717,   176,   848,   720,   174,
     722,   717,   711,   182,   177,   150,   151,   174,  1069,   176,
     719,   177,   659,   823,     6,   174,   678,   176,   163,   681,
     176,   755,   879,   880,   881,   882,     6,   689,     6,   324,
       6,     6,   177,   328,   648,   682,   181,   651,     6,   327,
     891,     6,   751,     6,     6,   754,     6,   342,   336,   344,
     345,   346,   177,   762,   440,     6,     6,   846,    71,    72,
      73,    64,    65,    66,   846,   916,     6,   174,   174,    82,
      83,    84,   686,   795,   177,   176,   174,   846,    91,   795,
      23,   795,   179,   934,    97,    98,    99,   100,   797,   176,
      27,   174,   174,   101,   101,     6,   805,   176,   178,   617,
     175,   179,   178,   116,   174,    23,    23,   180,   177,   831,
       4,    23,   846,   177,   776,   837,   177,   839,   177,   177,
     175,   837,   175,   839,   178,   102,   178,    62,   179,   179,
     777,   840,   841,   178,   843,   945,   178,   655,   179,     4,
     991,   992,   175,   177,   758,    32,   175,   794,   178,   176,
     178,   873,   177,   177,   672,    23,    24,    25,    26,    27,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
     174,   177,    62,   895,  1008,   176,  1027,   176,   174,   176,
     174,   795,     7,  1034,  1035,     7,   179,   171,   176,     6,
     176,   709,   174,   178,   116,   809,   177,   919,     6,   178,
      23,   174,   176,   919,   175,   174,     7,   174,   180,   179,
     174,   177,   874,   922,    82,    83,    84,   171,   179,  1008,
     929,   180,   931,    91,   179,   170,  1004,   180,     7,   178,
     171,   178,   954,   847,   950,  1024,    79,   755,   176,   178,
     176,    71,   889,   965,   178,   178,   152,   152,   116,   965,
     180,   179,    73,   152,   177,  1106,   171,   165,   180,  1110,
     179,   908,  1113,   176,   986,    27,   176,   176,    27,  1047,
     986,   933,     4,     4,   178,  1053,     4,     4,  1000,     4,
     942,  1003,     4,     4,  1000,   994,     4,   179,   997,  1011,
      27,   174,   678,   179,   941,   681,   176,   683,   179,   163,
     176,   176,    80,   689,    62,   163,  1084,   179,    27,   177,
     179,    71,    72,   176,  1036,  1037,   184,   178,     4,  1108,
    1036,  1037,    82,    83,    84,   177,   179,   178,   846,   617,
    1164,    91,    23,  1184,   178,   177,  1120,    97,    98,    99,
     100,  1063,   689,    94,  1122,  1196,    47,   866,   198,   500,
    1072,  1073,  1162,  1075,  1076,  1064,   116,  1073,   178,  1075,
    1076,   672,   846,   929,   985,   823,  1144,   655,   895,   983,
    1079,   985,   786,   949,   898,  1164,   164,   505,  1040,   502,
    1154,   945,  1091,  1093,   672,  1047,    65,  1096,  1097,  1084,
     776,  1162,  1059,   840,    73,  1104,   782,  1100,   490,   155,
     786,   653,  1049,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,  1136,  1108,   635,  1139,  1065,  1141,
    1136,   709,  1084,  1139,   674,  1141,  1148,   474,   554,  1151,
      -1,    -1,    -1,  1155,    -1,    -1,    -1,   116,    -1,   397,
    1149,   827,    -1,    -1,    -1,    -1,    -1,    -1,  1095,    -1,
    1172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,
      -1,    -1,    -1,    -1,  1186,    -1,  1188,   755,    -1,    -1,
      -1,    -1,  1188,    -1,    -1,    -1,  1123,  1199,  1125,    -1,
      -1,  1128,  1129,  1199,    -1,  1194,  1133,    -1,   874,    -1,
      -1,    -1,    -1,   879,   880,   881,   882,    -1,    -1,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,     5,    -1,
      -1,   897,   898,    -1,    -1,    12,    13,    -1,    -1,  1166,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,  1178,    -1,    30,    44,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,   933,    45,    46,
      47,    -1,    -1,    -1,     4,    -1,   942,    -1,   846,    -1,
      -1,    -1,    -1,   949,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,
     966,    78,    -1,    -1,    -1,    82,    -1,    -1,   974,   975,
     976,   977,    -1,    -1,    91,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,  1009,    -1,   135,   136,    -1,   138,   139,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    -1,    -1,  1040,    -1,    -1,   167,    -1,   169,
      -1,  1047,   159,    -1,   116,    -1,    -1,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     177,    -1,    -1,    -1,   181,   182,   183,   184,    -1,    -1,
     140,   141,    -1,    -1,    -1,    -1,    -1,    -1,  1084,    -1,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,     5,    -1,
      -1,    -1,    -1,   163,    -1,    12,    13,    -1,   168,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,   181,    -1,    30,    -1,  1121,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      71,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,   159,    -1,    -1,    -1,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     177,   178,     5,    -1,   181,   182,   183,   184,    -1,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,   178,     5,    -1,   181,   182,
     183,   184,    -1,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,   178,
       5,    -1,   181,   182,   183,   184,    -1,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    -1,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    -1,    -1,    -1,   116,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,   116,
     165,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,   177,    -1,     6,    -1,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    44,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,   180,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,   180,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,   167,    -1,   169,   170,   171,   159,    -1,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,     6,   181,
     182,   183,   184,    -1,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,   178,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,    -1,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,     6,   181,   182,   183,   184,    -1,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,   178,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,     6,   181,   182,   183,
     184,    -1,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
       6,   181,   182,   183,   184,    -1,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,     6,   181,   182,   183,   184,    -1,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,
      72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   177,    -1,     6,    -1,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181,   182,   183,   184,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,    -1,    44,   180,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,   180,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,    -1,    44,   180,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,   166,   167,   180,   169,
     170,   171,    -1,    38,    39,    40,    -1,    -1,    -1,    44,
     180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,    -1,
       5,    -1,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    -1,   174,    38,    39,    40,   178,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,   166,   167,    -1,   169,   170,   171,    -1,    38,    39,
      40,    -1,    -1,   178,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,   166,   167,    -1,   169,   170,   171,    -1,    38,    39,
      40,    -1,    -1,   178,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    -1,    -1,    -1,    44,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   178,   166,
     167,    -1,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,   178,    44,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   178,   166,
     167,    -1,   169,   170,   171,    -1,    38,    39,    40,    -1,
      -1,   178,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    38,    39,    40,    -1,    -1,   178,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,     5,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,   166,   167,    -1,   169,   170,   171,
      -1,    38,    39,    40,    -1,    -1,   178,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    -1,    -1,    -1,
      44,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,   166,   167,    -1,   169,   170,   171,    -1,    -1,
      -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,   178,   166,   167,    -1,   169,   170,   171,    -1,    -1,
      12,    13,    -1,    -1,   178,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,   103,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    78,
      79,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,   103,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    78,    79,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,   103,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    51,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    74,    75,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,    -1,   181,   182,   183,   184,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,   183,
     184,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,    60,
      61,    -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
     181,   182,   183,   184,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    59,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,    -1,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,    -1,   181,   182,   183,   184,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    -1,    74,
      75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,    -1,    -1,    -1,   104,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      52,    53,    54,    -1,    56,    57,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    48,
      -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,
      -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    -1,    74,    75,    -1,    -1,    78,
      -1,    -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,
      56,    -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    -1,    74,    75,
      -1,    -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,
      63,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      -1,    74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    48,    -1,
      -1,    -1,    52,    53,    54,    -1,    56,    -1,    58,    -1,
      60,    -1,    -1,    63,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    -1,    74,    75,    -1,    -1,    78,    -1,
      -1,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    -1,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,
      -1,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    -1,    74,    75,    -1,
      -1,    78,    -1,    -1,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    -1,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    -1,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    -1,
      74,    75,    -1,    -1,    78,    -1,    -1,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,   183,
     184,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    -1,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    12,    13,    -1,    -1,    -1,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    -1,    -1,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    -1,
      58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,    67,
      68,    69,    70,    71,    72,   177,    74,    75,    -1,    -1,
      78,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    93,    94,    95,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,    -1,   181,   182,   183,   184,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
     145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
     165,    -1,    -1,    -1,    -1,   170,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,   181,   182,   183,   184,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    30,    -1,
      -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,
      72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,   147,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,   181,
     182,   183,   184,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    42,    43,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,   145,   146,   147,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181,   182,   183,   184,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    -1,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    94,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,   181,   182,   183,   184,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      43,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   177,    -1,    -1,    -1,   181,   182,
     183,   184,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      30,    44,    -1,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,   167,    -1,   169,   170,   171,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     8,     9,
      10,    11,    12,    13,    14,    15,    16,   177,    -1,    -1,
      -1,   181,   182,   183,   184,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    30,    44,    -1,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    94,    95,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
     147,   148,    -1,    -1,    -1,    -1,    -1,   167,    -1,   169,
     170,   171,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    12,    13,    14,    15,    16,
     177,    -1,    -1,    -1,   181,   182,   183,   184,    12,    13,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    30,    44,    -1,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   131,   132,   133,   134,   135,   136,
     137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,   147,   148,    -1,    -1,    -1,    -1,    -1,
     167,    -1,   169,   170,   171,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,   181,   182,   183,
     184,    12,    13,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      71,    72,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    94,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,   147,   148,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,   159,    -1,
      -1,    -1,    -1,    -1,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,   177,    -1,    -1,    -1,
     181,   182,   183,   184,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    38,    39,    40,
      -1,    79,    -1,    44,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,    -1,   176,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
      -1,    -1,   180,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    -1,     5,   176,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     5,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    -1,    -1,   176,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,   175,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,    -1,     5,    -1,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   166,   167,    -1,   169,   170,   171,
      -1,    -1,   174,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,   174,    38,    39,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,     5,    -1,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    62,    -1,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,    38,    39,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   169,   170,   171,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,   171,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,
      -1,   169,   170,   171,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,     5,
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,     5,    -1,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    39,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,     5,    -1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     5,    -1,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    40,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
     166,   167,    -1,   169,   170,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   166,   167,    -1,   169,   170,   171,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,    -1,    -1,   166,   167,    -1,   169,   170,
     171,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,   171,    -1,    -1,    -1,    23,    -1,   166,   167,
      27,   169,   170,   171,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    95,    -1,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
     116,    -1,   179,    27,    -1,    -1,    -1,   184,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    -1,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,   180,    -1,    -1,    91,    92,    93,
      94,    95,    -1,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    -1,    23,    24,    25,    26,    27,    -1,    -1,
      30,    -1,    -1,    -1,    23,    24,    25,    26,    27,    -1,
      -1,    30,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      -1,    -1,    82,    -1,    -1,   179,    -1,    -1,    -1,    -1,
     184,    91,    -1,    82,     9,    10,    11,    12,    13,    14,
      15,    16,    91,   116,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,    -1,    44,
      -1,    -1,    -1,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,    44,    -1,
     170,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,   181,    -1,   183,   184,    -1,    -1,    -1,   177,    -1,
      -1,    -1,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,   133,   134,
     135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   167,    -1,   169,   131,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    24,    25,    26,    -1,    -1,
      -1,   167,    -1,   169,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
     116,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,   180,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,   180,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,   116,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    -1,   180,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,   180,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    23,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    -1,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
      -1,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
      23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      -1,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    -1,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    95,    -1,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   186,   191,     0,    12,    13,    17,    18,    19,    21,
      22,    23,    24,    25,    26,    27,    28,    30,    33,    34,
      35,    36,    37,    41,    42,    43,    45,    46,    47,    48,
      52,    53,    54,    56,    58,    60,    63,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    78,    79,    81,    82,
      83,    84,    91,    93,    94,    95,    96,    97,    98,    99,
     100,   103,   104,   105,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   159,   176,   177,   179,   181,   182,
     183,   184,   195,   198,   199,   200,   201,   202,   218,   227,
     230,   233,   236,   237,   239,   241,   255,   261,   262,   263,
     264,   329,   330,   331,   332,   333,   334,   341,   343,   347,
     348,   349,   350,   352,   353,   354,   355,   356,   357,   358,
     359,   370,    23,    82,    91,   199,   331,   334,   331,   331,
     331,   331,     6,   331,   331,   177,   331,   331,   331,   331,
     331,    82,    91,   177,   195,   199,   234,   235,   236,   327,
     343,   344,   358,   360,   177,   284,   331,   177,   282,   346,
     177,   322,   323,   331,   218,   177,   177,   177,   177,   177,
     177,   331,   351,   351,    23,    23,   215,   321,   351,   179,
     331,    23,    24,    26,    71,    73,   193,   194,   205,   207,
     211,   214,   288,   289,   358,    27,   290,   291,   332,   282,
     177,   177,   177,   177,   232,   238,   240,   242,    23,    26,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    91,    92,    93,    94,    95,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   187,   188,
     190,   192,   204,   177,   177,   335,    45,   177,   181,   329,
     347,   349,   350,   357,   357,   331,   331,   331,   331,   331,
     331,   331,   331,    27,    29,   160,   161,   162,   367,   368,
     331,   216,   104,   165,   170,   171,   189,   331,   363,   364,
     365,   366,    29,   345,   367,    29,   367,   179,   358,   282,
      82,   198,   201,   332,    97,   236,    49,    50,    49,    50,
      51,     5,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    38,    39,    40,    44,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   166,   167,   169,
     170,   171,   176,   189,   337,   337,   163,   163,   150,   151,
     181,   342,     4,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   140,   141,   168,   337,   331,
     152,   331,   327,   236,    97,   163,   282,   150,   151,   163,
     181,     5,    23,    33,    34,    35,    36,    37,    41,    42,
      45,    46,    47,    67,    71,    72,    78,    82,    91,    94,
      95,   104,   105,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   165,   178,   190,   285,   286,   331,   178,   283,
     286,   287,   331,   331,   174,   176,    54,   331,   149,   325,
     326,   331,   331,   215,   331,   331,   176,   176,   176,     4,
     174,   176,   176,   216,    62,   164,   194,   206,   211,   176,
     174,   176,   174,   176,     4,   174,   176,   224,   225,   357,
     331,   371,   372,   331,   178,    23,    23,    23,    23,   176,
     203,   179,   363,   363,   196,   197,   343,   344,   331,   363,
     176,   150,   151,   181,   160,   368,    31,   331,   357,    29,
     160,   368,   178,    96,   180,   199,   200,   217,   218,   177,
     331,   357,   152,   175,   174,   182,   183,   331,   332,   231,
     177,   218,   177,     6,   176,     6,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     344,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   335,    23,    91,
     226,   335,   179,   190,   358,   361,   179,   190,   358,   361,
      23,   179,   358,   362,   362,   351,   282,   189,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   178,   331,   178,   328,   358,   362,   362,   358,
     351,   331,     6,   174,   208,   174,   174,   208,   178,   323,
     177,   178,   331,   176,   174,    62,   178,   178,   178,   331,
     321,   180,    23,   179,   164,   176,   176,   194,   214,   289,
     331,   291,   174,   208,   174,   208,   178,   176,   101,   246,
     335,   101,   247,     6,   243,   179,   191,   178,   178,   174,
     208,   282,   178,   175,    23,    23,    13,    23,    27,    32,
     369,   180,   181,   180,   180,   177,   200,   363,   104,   189,
     331,     4,   364,   180,    23,   331,   331,   216,   331,     6,
     177,   335,   177,   331,   282,   331,   282,   331,   282,   282,
     175,   357,   346,   175,     5,   331,   287,   178,   178,   287,
     287,   178,     6,   218,   331,     6,   218,   260,   324,   326,
     331,   149,   331,   104,   181,   189,   249,   357,   219,     6,
     179,   253,   179,   335,   220,   193,   205,   209,   212,   213,
     179,   225,   178,   372,   178,   343,   102,   248,   179,   295,
     343,   335,     5,    82,   105,   106,   177,   195,   271,   272,
     273,   274,   275,   277,   248,   191,   180,     4,   196,   175,
      32,   175,   331,   178,   178,   177,   357,   331,   246,   178,
     178,    51,   331,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,   199,   265,   266,   267,   268,   269,   307,
     308,   177,   265,   180,   180,   180,   246,   216,   178,   216,
     176,   174,   331,   177,   363,   357,   152,   178,     6,   218,
     252,   176,   254,   176,   254,    66,   256,   257,   258,   259,
     331,    76,    77,   223,    62,   213,   174,   208,   210,   213,
     176,   295,   335,   292,   174,   179,   272,   272,   275,   171,
       7,     7,   171,   335,   180,   331,   175,   176,   363,   248,
     218,     6,   176,   269,   178,   174,   208,     5,   177,   270,
     276,   277,   278,   279,   280,   308,   265,   178,   248,   176,
      55,   325,   331,   363,   175,   249,     6,   218,   251,   216,
     254,    64,    65,    66,   254,   180,   174,   208,   180,   174,
     208,   174,   208,   177,   179,    23,   212,   180,   174,   208,
     179,    65,    79,    92,   180,   199,   244,   293,   294,   304,
     305,   306,   307,   343,   292,   178,   272,   272,   273,   273,
     272,   179,   180,   178,   335,   216,     6,   281,   267,   277,
     277,   280,   170,   228,   171,     7,     7,   171,   178,    79,
     338,   335,   176,   178,   178,   178,   216,    61,    64,   176,
     331,     6,   176,   180,   152,   258,   331,   152,   221,   343,
     216,   213,   180,   292,   335,   295,   293,   270,   334,    73,
     180,   292,   179,   271,   152,   178,   165,   229,   277,   277,
     278,   278,   277,   281,   177,   281,   179,     6,   218,   250,
     251,    59,   176,   176,     6,   176,   216,   216,   331,   331,
       7,    27,   222,   180,   180,   190,   176,   179,   296,    27,
     309,   310,   311,   337,    23,    82,   105,   106,   188,   271,
     319,   320,   180,   292,   336,    27,   336,    27,   189,   339,
     340,   336,   292,   216,   176,   216,   216,   343,   178,     4,
     245,    82,   180,   190,   297,   298,   299,   300,   301,   302,
     343,     4,   335,   174,   176,   190,     4,     4,    23,   319,
     174,   176,   180,   331,     4,   335,   179,    27,   174,   208,
     179,   180,    57,   179,   331,   176,   180,   298,   176,   176,
      62,    80,   163,   331,   179,    27,   310,   335,   331,   331,
     176,   320,   336,   331,   179,   313,   216,   340,   178,   216,
     176,   216,    23,   187,   308,   295,   190,   335,   312,     4,
     335,   177,   335,   335,   335,   312,   180,   180,   180,   190,
     179,   180,   199,   307,   314,   315,   331,   265,   313,   180,
     336,   336,   312,   315,   337,   335,   178,   180,    23,   281,
     335,   336,   316,   176,   179,   303,   177,   318,   216,   336,
     265,   336,   180,   178,   152,   176,   179,   317,   331,   216,
     336,   176,   180
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   185,   186,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   188,   188,   188,   188,   188,   188,
     188,   188,   189,   189,   190,   190,   191,   191,   192,   192,
     193,   193,   194,   194,   195,   195,   195,   195,   196,   196,
     197,   197,   198,   199,   199,   200,   200,   200,   200,   200,
     201,   201,   202,   202,   202,   202,   202,   203,   202,   204,
     202,   202,   202,   202,   202,   205,   205,   206,   207,   208,
     208,   209,   209,   210,   210,   211,   211,   212,   212,   213,
     213,   214,   214,   215,   215,   216,   216,   217,   217,   217,
     217,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   219,
     218,   218,   218,   218,   218,   218,   220,   220,   221,   221,
     222,   222,   223,   223,   224,   224,   225,   226,   226,   227,
     228,   228,   229,   229,   231,   230,   232,   230,   233,   233,
     234,   234,   235,   235,   236,   236,   236,   238,   237,   240,
     239,   242,   241,   243,   243,   244,   245,   245,   246,   246,
     247,   247,   248,   248,   249,   249,   249,   249,   250,   250,
     251,   251,   252,   252,   253,   253,   253,   253,   254,   254,
     254,   254,   254,   255,   256,   256,   257,   257,   258,   258,
     259,   259,   260,   260,   261,   261,   262,   262,   263,   263,
     264,   264,   265,   265,   266,   266,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   276,   276,
     277,   277,   277,   278,   278,   279,   279,   280,   280,   281,
     281,   282,   282,   283,   283,   284,   284,   284,   285,   285,
     285,   286,   286,   286,   286,   286,   287,   287,   288,   288,
     289,   290,   290,   291,   291,   292,   292,   293,   293,   293,
     293,   293,   293,   294,   294,   294,   295,   295,   296,   296,
     296,   297,   297,   298,   298,   299,   300,   300,   300,   300,
     301,   301,   302,   303,   303,   304,   304,   305,   305,   306,
     306,   307,   307,   308,   308,   308,   308,   308,   308,   308,
     308,   308,   308,   309,   309,   310,   310,   311,   311,   312,
     312,   312,   313,   313,   314,   314,   316,   315,   317,   317,
     317,   318,   318,   319,   319,   320,   320,   321,   322,   322,
     323,   324,   324,   324,   325,   325,   326,   326,   326,   326,
     328,   327,   329,   329,   329,   330,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   332,   332,   333,   334,   335,   336,   337,   337,
     338,   338,   339,   339,   340,   340,   341,   341,   341,   341,
     342,   341,   343,   343,   344,   344,   344,   345,   345,   345,
     346,   346,   347,   347,   347,   347,   348,   348,   348,   348,
     348,   348,   348,   348,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   350,   350,   350,   350,   351,   351,
     352,   353,   353,   353,   353,   353,   354,   354,   355,   355,
     355,   355,   356,   356,   356,   356,   356,   357,   357,   357,
     357,   358,   358,   358,   359,   359,   360,   360,   360,   360,
     360,   360,   361,   361,   361,   362,   362,   362,   363,   364,
     364,   365,   365,   366,   366,   366,   366,   366,   366,   366,
     367,   367,   367,   367,   368,   368,   368,   368,   368,   368,
     368,   368,   369,   369,   369,   369,   370,   370,   370,   370,
     370,   370,   370,   371,   371,   372
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
       1,     1,     1,     1,     1,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     3,     5,     1,     2,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     2,     4,     3,     0,     6,     0,
       5,     3,     4,     3,     4,     1,     1,     6,     6,     0,
       1,     3,     1,     3,     1,     3,     1,     1,     2,     1,
       3,     1,     3,     3,     1,     2,     0,     1,     1,     2,
       4,     3,     1,     1,     5,     7,     9,     5,     3,     3,
       3,     3,     3,     3,     1,     2,     6,     7,     9,     0,
       6,     1,     6,     3,     2,     3,     0,     9,     1,     3,
       0,     1,     0,     4,     1,     3,     1,     1,     1,    13,
       0,     1,     0,     1,     0,    10,     0,     9,     1,     2,
       1,     2,     0,     1,     1,     1,     1,     0,     7,     0,
       8,     0,     9,     0,     2,     5,     0,     2,     0,     2,
       0,     2,     0,     2,     1,     2,     4,     3,     1,     4,
       1,     4,     1,     4,     3,     4,     4,     5,     0,     5,
       5,     4,     4,     7,     0,     2,     1,     3,     4,     4,
       1,     3,     1,     4,     5,     6,     1,     3,     6,     7,
       3,     6,     2,     0,     1,     3,     2,     1,     0,     1,
       7,     9,     0,     1,     1,     2,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     0,
       2,     2,     4,     1,     3,     2,     4,     4,     3,     1,
       3,     3,     1,     1,     3,     2,     1,     1,     3,     1,
       1,     3,     1,     1,     3,     2,     0,     4,     3,     4,
       5,    12,     1,     1,     2,     3,     1,     3,     1,     2,
       3,     1,     2,     2,     2,     3,     3,     3,     4,     3,
       1,     1,     3,     1,     3,     1,     1,     0,     1,     0,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     4,     5,     7,     0,
       2,     3,     0,     3,     0,     1,     0,     9,     1,     3,
       3,     0,     3,     3,     1,     4,     4,     4,     3,     1,
       1,     0,     3,     1,     0,     1,     3,     4,     2,     1,
       0,    10,     3,     2,     3,     2,     1,     6,     5,     3,
       4,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     5,     4,     3,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       3,     2,     1,     2,     4,     2,     2,     1,     2,     2,
       3,     1,    13,    11,     1,     1,     0,     0,     0,     1,
       0,     5,     3,     1,     1,     2,     2,     2,     4,     4,
       0,     3,     1,     1,     1,     1,     3,     0,     1,     1,
       0,     1,     4,     3,     1,     3,     1,     1,     3,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     5,     5,     0,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     4,     4,     4,     1,     1,     1,     3,
       3,     1,     4,     2,     3,     3,     1,     4,     3,     3,
       3,     3,     1,     3,     1,     1,     3,     1,     1,     0,
       1,     3,     1,     3,     1,     4,     2,     2,     6,     4,
       2,     2,     1,     2,     1,     4,     3,     3,     3,     3,
       6,     3,     1,     1,     2,     1,     5,     4,     2,     2,
       4,     2,     2,     1,     3,     1
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

    case YYSYMBOL_attributed_top_statement: /* attributed_top_statement  */
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

    case YYSYMBOL_clone_argument_list: /* clone_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_non_empty_clone_argument_list: /* non_empty_clone_argument_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_argument_no_expr: /* argument_no_expr  */
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

    case YYSYMBOL_non_empty_member_modifiers: /* non_empty_member_modifiers  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_list: /* property_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property: /* property  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_hooked_property: /* hooked_property  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook_list: /* property_hook_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_property_hook_list: /* optional_property_hook_list  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook: /* property_hook  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_property_hook_body: /* property_hook_body  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_optional_parameter_list: /* optional_parameter_list  */
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

    case YYSYMBOL_for_cond_exprs: /* for_cond_exprs  */
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

    case YYSYMBOL_new_dereferenceable: /* new_dereferenceable  */
            { zend_ast_destroy(((*yyvaluep).ast)); }
        break;

    case YYSYMBOL_new_non_dereferenceable: /* new_non_dereferenceable  */
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

  case 84: /* identifier: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 85: /* identifier: semi_reserved  */
                               {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 86: /* top_statement_list: top_statement_list top_statement  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 87: /* top_statement_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 88: /* namespace_declaration_name: identifier  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 89: /* namespace_declaration_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 90: /* namespace_name: "identifier"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 91: /* namespace_name: "namespaced name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 92: /* legacy_namespace_name: namespace_name  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 93: /* legacy_namespace_name: "fully qualified name"  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 94: /* name: "identifier"  */
                                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 95: /* name: "namespaced name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 96: /* name: "fully qualified name"  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }
    break;

  case 97: /* name: "namespace-relative name"  */
                                                                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }
    break;

  case 98: /* attribute_decl: class_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[0].ast), NULL); }
    break;

  case 99: /* attribute_decl: class_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ATTRIBUTE, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 100: /* attribute_group: attribute_decl  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_GROUP, (yyvsp[0].ast)); }
    break;

  case 101: /* attribute_group: attribute_group ',' attribute_decl  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 102: /* attribute: "'#['" backup_doc_comment attribute_group possible_comma ']'  */
                                                                                        { (yyval.ast) = (yyvsp[-2].ast); CG(doc_comment) = (yyvsp[-3].str); }
    break;

  case 103: /* attributes: attribute  */
                                                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ATTRIBUTE_LIST, (yyvsp[0].ast)); }
    break;

  case 104: /* attributes: attributes attribute  */
                                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 105: /* attributed_statement: function_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 106: /* attributed_statement: class_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 107: /* attributed_statement: trait_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 108: /* attributed_statement: interface_declaration_statement  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 109: /* attributed_statement: enum_declaration_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 110: /* attributed_top_statement: attributed_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 111: /* attributed_top_statement: "'const'" const_list ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 112: /* top_statement: statement  */
                                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 113: /* top_statement: attributed_top_statement  */
                                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 114: /* top_statement: attributes attributed_top_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 115: /* top_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }
    break;

  case 116: /* top_statement: "'namespace'" namespace_declaration_name ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-1].ast), NULL);
			  RESET_DOC_COMMENT(); }
    break;

  case 117: /* $@1: %empty  */
                                                       { RESET_DOC_COMMENT(); }
    break;

  case 118: /* top_statement: "'namespace'" namespace_declaration_name $@1 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 119: /* $@2: %empty  */
                            { RESET_DOC_COMMENT(); }
    break;

  case 120: /* top_statement: "'namespace'" $@2 '{' top_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[-1].ast)); }
    break;

  case 121: /* top_statement: "'use'" mixed_group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 122: /* top_statement: "'use'" use_type group_use_declaration ';'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 123: /* top_statement: "'use'" use_declarations ';'  */
                                                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 124: /* top_statement: "'use'" use_type use_declarations ';'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 125: /* use_type: "'function'"  */
                                        { (yyval.num) = ZEND_SYMBOL_FUNCTION; }
    break;

  case 126: /* use_type: "'const'"  */
                                        { (yyval.num) = ZEND_SYMBOL_CONST; }
    break;

  case 127: /* group_use_declaration: legacy_namespace_name "'\\'" '{' unprefixed_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 128: /* mixed_group_use_declaration: legacy_namespace_name "'\\'" '{' inline_use_declarations possible_comma '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[-5].ast), (yyvsp[-2].ast));}
    break;

  case 131: /* inline_use_declarations: inline_use_declarations ',' inline_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 132: /* inline_use_declarations: inline_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 133: /* unprefixed_use_declarations: unprefixed_use_declarations ',' unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 134: /* unprefixed_use_declarations: unprefixed_use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 135: /* use_declarations: use_declarations ',' use_declaration  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 136: /* use_declarations: use_declaration  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[0].ast)); }
    break;

  case 137: /* inline_use_declaration: unprefixed_use_declaration  */
                                           { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 138: /* inline_use_declaration: use_type unprefixed_use_declaration  */
                                                    { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = (yyvsp[-1].num); }
    break;

  case 139: /* unprefixed_use_declaration: namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 140: /* unprefixed_use_declaration: namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 141: /* use_declaration: legacy_namespace_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 142: /* use_declaration: legacy_namespace_name "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 143: /* const_list: const_list ',' const_decl  */
                                          { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 144: /* const_list: const_decl  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 145: /* inner_statement_list: inner_statement_list inner_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 146: /* inner_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 147: /* inner_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 148: /* inner_statement: attributed_statement  */
                                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 149: /* inner_statement: attributes attributed_statement  */
                                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 150: /* inner_statement: "'__halt_compiler'" '(' ')' ';'  */
                        { (yyval.ast) = NULL; zend_throw_exception(zend_ce_compile_error,
			      "__HALT_COMPILER() can only be used from the outermost scope", 0); YYERROR; }
    break;

  case 151: /* statement: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 152: /* statement: if_stmt  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 153: /* statement: alt_if_stmt  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 154: /* statement: "'while'" '(' expr ')' while_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 155: /* statement: "'do'" statement "'while'" '(' expr ')' ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[-5].ast), (yyvsp[-2].ast)); }
    break;

  case 156: /* statement: "'for'" '(' for_exprs ';' for_cond_exprs ';' for_exprs ')' for_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[-6].ast), (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 157: /* statement: "'switch'" '(' expr ')' switch_case_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 158: /* statement: "'break'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[-1].ast)); }
    break;

  case 159: /* statement: "'continue'" optional_expr ';'  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[-1].ast)); }
    break;

  case 160: /* statement: "'return'" optional_expr ';'  */
                                                        { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[-1].ast)); }
    break;

  case 161: /* statement: "'global'" global_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 162: /* statement: "'static'" static_var_list ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 163: /* statement: "'echo'" echo_expr_list ';'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 164: /* statement: T_INLINE_HTML  */
                              { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 165: /* statement: expr ';'  */
                         { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 166: /* statement: "'unset'" '(' unset_variables possible_comma ')' ';'  */
                                                                   { (yyval.ast) = (yyvsp[-3].ast); }
    break;

  case 167: /* statement: "'foreach'" '(' expr "'as'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-4].ast), (yyvsp[-2].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 168: /* statement: "'foreach'" '(' expr "'as'" foreach_variable "'=>'" foreach_variable ')' foreach_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-4].ast), (yyvsp[0].ast)); }
    break;

  case 169: /* $@3: %empty  */
                        { if (!zend_handle_encoding_declaration((yyvsp[-1].ast))) { YYERROR; } }
    break;

  case 170: /* statement: "'declare'" '(' const_list ')' $@3 declare_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 171: /* statement: ';'  */
                                              { (yyval.ast) = NULL; }
    break;

  case 172: /* statement: "'try'" '{' inner_statement_list '}' catch_list finally_statement  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 173: /* statement: "'goto'" "identifier" ';'  */
                                    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[-1].ast)); }
    break;

  case 174: /* statement: "identifier" ':'  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[-1].ast)); }
    break;

  case 175: /* statement: "'(void)'" expr ';'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[-1].ast)); }
    break;

  case 176: /* catch_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }
    break;

  case 177: /* catch_list: catch_list "'catch'" '(' catch_name_list optional_variable ')' '{' inner_statement_list '}'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-8].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast))); }
    break;

  case 178: /* catch_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 179: /* catch_name_list: catch_name_list '|' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 180: /* optional_variable: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 181: /* optional_variable: "variable"  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 182: /* finally_statement: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 183: /* finally_statement: "'finally'" '{' inner_statement_list '}'  */
                                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 184: /* unset_variables: unset_variable  */
                               { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 185: /* unset_variables: unset_variables ',' unset_variable  */
                                                   { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 186: /* unset_variable: variable  */
                         { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[0].ast)); }
    break;

  case 187: /* function_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 188: /* function_name: "'readonly'"  */
                           {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 189: /* function_declaration_statement: function returns_ref function_name backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-9].str),
		      zend_ast_get_str((yyvsp[-10].ast)), (yyvsp[-7].ast), NULL, (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 190: /* is_reference: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 191: /* is_reference: "'&'"  */
                                                        { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 192: /* is_variadic: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 193: /* is_variadic: "'...'"  */
                            { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 194: /* @4: %empty  */
                                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 195: /* class_declaration_statement: class_modifiers "'class'" @4 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 196: /* @5: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 197: /* class_declaration_statement: "'class'" @5 "identifier" extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), (yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 198: /* class_modifiers: class_modifier  */
                                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 199: /* class_modifiers: class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 200: /* anonymous_class_modifiers: class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier(0, (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 201: /* anonymous_class_modifiers: anonymous_class_modifiers class_modifier  */
                        { (yyval.num) = zend_add_anonymous_class_modifier((yyvsp[-1].num), (yyvsp[0].num)); if (!(yyval.num)) { YYERROR; } }
    break;

  case 202: /* anonymous_class_modifiers_optional: %empty  */
                                                { (yyval.num) = 0; }
    break;

  case 203: /* anonymous_class_modifiers_optional: anonymous_class_modifiers  */
                                                { (yyval.num) = (yyvsp[0].num); }
    break;

  case 204: /* class_modifier: "'abstract'"  */
                                        { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 205: /* class_modifier: "'final'"  */
                                        { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 206: /* class_modifier: "'readonly'"  */
                                        { (yyval.num) = ZEND_ACC_READONLY_CLASS|ZEND_ACC_NO_DYNAMIC_PROPERTIES; }
    break;

  case 207: /* @6: %empty  */
                        { (yyval.num) = CG(zend_lineno); }
    break;

  case 208: /* trait_declaration_statement: "'trait'" @6 "identifier" backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[-5].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-4].ast)), NULL, NULL, (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 209: /* @7: %empty  */
                            { (yyval.num) = CG(zend_lineno); }
    break;

  case 210: /* interface_declaration_statement: "'interface'" @7 "identifier" interface_extends_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[-6].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-5].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL); }
    break;

  case 211: /* @8: %empty  */
                       { (yyval.num) = CG(zend_lineno); }
    break;

  case 212: /* enum_declaration_statement: "'enum'" @8 "identifier" enum_backing_type implements_list backup_doc_comment '{' class_statement_list '}'  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_ENUM|ZEND_ACC_FINAL, (yyvsp[-7].num), (yyvsp[-3].str), zend_ast_get_str((yyvsp[-6].ast)), NULL, (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, (yyvsp[-5].ast)); }
    break;

  case 213: /* enum_backing_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 214: /* enum_backing_type: ':' type_expr  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 215: /* enum_case: "'case'" backup_doc_comment identifier enum_case_expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ENUM_CASE, (yyvsp[-2].ast), (yyvsp[-1].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), NULL); }
    break;

  case 216: /* enum_case_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 217: /* enum_case_expr: '=' expr  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 218: /* extends_from: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 219: /* extends_from: "'extends'" class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 220: /* interface_extends_list: %empty  */
                                                { (yyval.ast) = NULL; }
    break;

  case 221: /* interface_extends_list: "'extends'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 222: /* implements_list: %empty  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 223: /* implements_list: "'implements'" class_name_list  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 224: /* foreach_variable: variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 225: /* foreach_variable: ampersand variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[0].ast)); }
    break;

  case 226: /* foreach_variable: "'list'" '(' array_pair_list ')'  */
                                               { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 227: /* foreach_variable: '[' array_pair_list ']'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 228: /* for_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 229: /* for_statement: ':' inner_statement_list "'endfor'" ';'  */
                                                      { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 230: /* foreach_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 231: /* foreach_statement: ':' inner_statement_list "'endforeach'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 232: /* declare_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 233: /* declare_statement: ':' inner_statement_list "'enddeclare'" ';'  */
                                                          { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 234: /* switch_case_list: '{' case_list '}'  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 235: /* switch_case_list: '{' ';' case_list '}'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 236: /* switch_case_list: ':' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 237: /* switch_case_list: ':' ';' case_list "'endswitch'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 238: /* case_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 239: /* case_list: case_list "'case'" expr ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 240: /* case_list: case_list "'case'" expr ';' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-4].ast), zend_ast_create_ex(ZEND_AST_SWITCH_CASE, ZEND_ALT_CASE_SYNTAX, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 241: /* case_list: case_list "'default'" ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[0].ast))); }
    break;

  case 242: /* case_list: case_list "'default'" ';' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create_ex(ZEND_AST_SWITCH_CASE, ZEND_ALT_CASE_SYNTAX, NULL, (yyvsp[0].ast))); }
    break;

  case 243: /* match: "'match'" '(' expr ')' '{' match_arm_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 244: /* match_arm_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_MATCH_ARM_LIST); }
    break;

  case 245: /* match_arm_list: non_empty_match_arm_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 246: /* non_empty_match_arm_list: match_arm  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MATCH_ARM_LIST, (yyvsp[0].ast)); }
    break;

  case 247: /* non_empty_match_arm_list: non_empty_match_arm_list ',' match_arm  */
                                                       { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 248: /* match_arm: match_arm_cond_list possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 249: /* match_arm: "'default'" possible_comma "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_MATCH_ARM, NULL, (yyvsp[0].ast)); }
    break;

  case 250: /* match_arm_cond_list: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 251: /* match_arm_cond_list: match_arm_cond_list ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 252: /* while_statement: statement  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 253: /* while_statement: ':' inner_statement_list "'endwhile'" ';'  */
                                                        { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 254: /* if_stmt_without_else: "'if'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 255: /* if_stmt_without_else: if_stmt_without_else "'elseif'" '(' expr ')' statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-2].ast), (yyvsp[0].ast))); }
    break;

  case 256: /* if_stmt: if_stmt_without_else  */
                                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 257: /* if_stmt: if_stmt_without_else "'else'" statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[0].ast))); }
    break;

  case 258: /* alt_if_stmt_without_else: "'if'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 259: /* alt_if_stmt_without_else: alt_if_stmt_without_else "'elseif'" '(' expr ')' ':' inner_statement_list  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-6].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[-3].ast), (yyvsp[0].ast))); }
    break;

  case 260: /* alt_if_stmt: alt_if_stmt_without_else "'endif'" ';'  */
                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 261: /* alt_if_stmt: alt_if_stmt_without_else "'else'" ':' inner_statement_list "'endif'" ';'  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-5].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[-2].ast))); }
    break;

  case 262: /* parameter_list: non_empty_parameter_list possible_comma  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 263: /* parameter_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 264: /* non_empty_parameter_list: attributed_parameter  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[0].ast)); }
    break;

  case 265: /* non_empty_parameter_list: non_empty_parameter_list ',' attributed_parameter  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 266: /* attributed_parameter: attributes parameter  */
                                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 267: /* attributed_parameter: parameter  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 268: /* optional_cpp_modifiers: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 269: /* optional_cpp_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CPP, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 270: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" backup_doc_comment optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-6].num) | (yyvsp[-4].num) | (yyvsp[-3].num), (yyvsp[-5].ast), (yyvsp[-2].ast), NULL,
					NULL, (yyvsp[-1].str) ? zend_ast_create_zval_from_str((yyvsp[-1].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 271: /* parameter: optional_cpp_modifiers optional_type_without_static is_reference is_variadic "variable" '=' expr backup_doc_comment optional_property_hook_list  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[-8].num) | (yyvsp[-6].num) | (yyvsp[-5].num), (yyvsp[-7].ast), (yyvsp[-4].ast), (yyvsp[-2].ast),
					NULL, (yyvsp[-1].str) ? zend_ast_create_zval_from_str((yyvsp[-1].str)) : NULL, (yyvsp[0].ast)); }
    break;

  case 272: /* optional_type_without_static: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 273: /* optional_type_without_static: type_expr_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 274: /* type_expr: type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 275: /* type_expr: '?' type  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 276: /* type_expr: union_type  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 277: /* type_expr: intersection_type  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 278: /* type: type_without_static  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 279: /* type: "'static'"  */
                                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_STATIC); }
    break;

  case 280: /* union_type_element: type  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 281: /* union_type_element: '(' intersection_type ')'  */
                                           { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 282: /* union_type: union_type_element '|' union_type_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 283: /* union_type: union_type '|' union_type_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 284: /* intersection_type: type "amp" type  */
                                                                          { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 285: /* intersection_type: intersection_type "amp" type  */
                                                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 286: /* type_expr_without_static: type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 287: /* type_expr_without_static: '?' type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 288: /* type_expr_without_static: union_type_without_static  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 289: /* type_expr_without_static: intersection_type_without_static  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 290: /* type_without_static: "'array'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 291: /* type_without_static: "'callable'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 292: /* type_without_static: name  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 293: /* union_type_without_static_element: type_without_static  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 294: /* union_type_without_static_element: '(' intersection_type_without_static ')'  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 295: /* union_type_without_static: union_type_without_static_element '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_UNION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 296: /* union_type_without_static: union_type_without_static '|' union_type_without_static_element  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 297: /* intersection_type_without_static: type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_TYPE_INTERSECTION, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 298: /* intersection_type_without_static: intersection_type_without_static "amp" type_without_static  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 299: /* return_type: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 300: /* return_type: ':' type_expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 301: /* argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_arg_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 302: /* argument_list: '(' non_empty_argument_list possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 303: /* non_empty_argument_list: argument  */
                        { (yyval.ast) = zend_ast_create_arg_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 304: /* non_empty_argument_list: non_empty_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_arg_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 305: /* clone_argument_list: '(' ')'  */
                        { (yyval.ast) = zend_ast_create_arg_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 306: /* clone_argument_list: '(' non_empty_clone_argument_list possible_comma ')'  */
                                                                     { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 307: /* clone_argument_list: '(' expr ',' ')'  */
                                 { (yyval.ast) = zend_ast_create_arg_list(1, ZEND_AST_ARG_LIST, (yyvsp[-2].ast)); }
    break;

  case 308: /* non_empty_clone_argument_list: expr ',' argument  */
                        { (yyval.ast) = zend_ast_create_arg_list(2, ZEND_AST_ARG_LIST, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 309: /* non_empty_clone_argument_list: argument_no_expr  */
                        { (yyval.ast) = zend_ast_create_arg_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)); }
    break;

  case 310: /* non_empty_clone_argument_list: non_empty_clone_argument_list ',' argument  */
                        { (yyval.ast) = zend_ast_arg_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 311: /* argument_no_expr: identifier ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 312: /* argument_no_expr: "'...'"  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PLACEHOLDER_ARG, ZEND_PLACEHOLDER_VARIADIC); }
    break;

  case 313: /* argument_no_expr: '?'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PLACEHOLDER_ARG); }
    break;

  case 314: /* argument_no_expr: identifier ':' '?'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NAMED_ARG, (yyvsp[-2].ast), zend_ast_create(ZEND_AST_PLACEHOLDER_ARG)); }
    break;

  case 315: /* argument_no_expr: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 316: /* argument: expr  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 317: /* argument: argument_no_expr  */
                                 { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 318: /* global_var_list: global_var_list ',' global_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 319: /* global_var_list: global_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 320: /* global_var: simple_variable  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast))); }
    break;

  case 321: /* static_var_list: static_var_list ',' static_var  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 322: /* static_var_list: static_var  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 323: /* static_var: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[0].ast), NULL); }
    break;

  case 324: /* static_var: "variable" '=' expr  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 325: /* class_statement_list: class_statement_list class_statement  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 326: /* class_statement_list: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 327: /* attributed_class_statement: property_modifiers optional_type_without_static property_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-2].ast), (yyvsp[-1].ast), NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 328: /* attributed_class_statement: property_modifiers optional_type_without_static hooked_property  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_GROUP, (yyvsp[-1].ast), zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)), NULL);
			  (yyval.ast)->attr = (yyvsp[-2].num); }
    break;

  case 329: /* attributed_class_statement: class_const_modifiers "'const'" class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, NULL);
			  (yyval.ast)->attr = (yyvsp[-3].num); }
    break;

  case 330: /* attributed_class_statement: class_const_modifiers "'const'" type_expr class_const_list ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST_GROUP, (yyvsp[-1].ast), NULL, (yyvsp[-2].ast));
			  (yyval.ast)->attr = (yyvsp[-4].num); }
    break;

  case 331: /* attributed_class_statement: method_modifiers function returns_ref identifier backup_doc_comment '(' parameter_list ')' return_type backup_fn_flags method_body backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[-9].num) | (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-7].str),
				  zend_ast_get_str((yyvsp[-8].ast)), (yyvsp[-5].ast), NULL, (yyvsp[-1].ast), (yyvsp[-3].ast), NULL); CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 332: /* attributed_class_statement: enum_case  */
                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 333: /* class_statement: attributed_class_statement  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 334: /* class_statement: attributes attributed_class_statement  */
                                                      { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 335: /* class_statement: "'use'" class_name_list trait_adaptations  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 336: /* class_name_list: class_name  */
                           { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[0].ast)); }
    break;

  case 337: /* class_name_list: class_name_list ',' class_name  */
                                               { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 338: /* trait_adaptations: ';'  */
                                                                                { (yyval.ast) = NULL; }
    break;

  case 339: /* trait_adaptations: '{' '}'  */
                                                                        { (yyval.ast) = NULL; }
    break;

  case 340: /* trait_adaptations: '{' trait_adaptation_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 341: /* trait_adaptation_list: trait_adaptation  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[0].ast)); }
    break;

  case 342: /* trait_adaptation_list: trait_adaptation_list trait_adaptation  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 343: /* trait_adaptation: trait_precedence ';'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 344: /* trait_adaptation: trait_alias ';'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 345: /* trait_precedence: absolute_trait_method_reference "'insteadof'" class_name_list  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 346: /* trait_alias: trait_method_reference "'as'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 347: /* trait_alias: trait_method_reference "'as'" reserved_non_modifiers  */
                        { zval zv;
			  if (zend_lex_tstring(&zv, (yyvsp[0].ident)) == FAILURE) { YYABORT; }
			  (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_ALIAS, (yyvsp[-2].ast), zend_ast_create_zval(&zv)); }
    break;

  case 348: /* trait_alias: trait_method_reference "'as'" member_modifier identifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[-1].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-3].ast), (yyvsp[0].ast));
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 349: /* trait_alias: trait_method_reference "'as'" member_modifier  */
                        { uint32_t modifiers = zend_modifier_token_to_flag(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].num));
			  (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, modifiers, (yyvsp[-2].ast), NULL);
			  /* identifier nonterminal can cause allocations, so we need to free the node */
			  if (!modifiers) { zend_ast_destroy((yyval.ast)); YYERROR; } }
    break;

  case 350: /* trait_method_reference: identifier  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_METHOD_REFERENCE, NULL, (yyvsp[0].ast)); }
    break;

  case 351: /* trait_method_reference: absolute_trait_method_reference  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 352: /* absolute_trait_method_reference: class_name "'::'" identifier  */
                { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_METHOD_REFERENCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 353: /* method_body: ';'  */
                                                        { (yyval.ast) = NULL; }
    break;

  case 354: /* method_body: '{' inner_statement_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 355: /* property_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; } }
    break;

  case 356: /* property_modifiers: "'var'"  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 357: /* method_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 358: /* method_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_METHOD, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 359: /* class_const_modifiers: %empty  */
                        { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 360: /* class_const_modifiers: non_empty_member_modifiers  */
                        { (yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_CONSTANT, (yyvsp[0].ast));
			  if (!(yyval.num)) { YYERROR; }
			  if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 361: /* non_empty_member_modifiers: member_modifier  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_MODIFIER_LIST, zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 362: /* non_empty_member_modifiers: non_empty_member_modifiers member_modifier  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), zend_ast_create_zval_from_long((yyvsp[0].num))); }
    break;

  case 363: /* member_modifier: "'public'"  */
                                                        { (yyval.num) = T_PUBLIC; }
    break;

  case 364: /* member_modifier: "'protected'"  */
                                                        { (yyval.num) = T_PROTECTED; }
    break;

  case 365: /* member_modifier: "'private'"  */
                                                        { (yyval.num) = T_PRIVATE; }
    break;

  case 366: /* member_modifier: "'public(set)'"  */
                                                { (yyval.num) = T_PUBLIC_SET; }
    break;

  case 367: /* member_modifier: "'protected(set)'"  */
                                                { (yyval.num) = T_PROTECTED_SET; }
    break;

  case 368: /* member_modifier: "'private(set)'"  */
                                                { (yyval.num) = T_PRIVATE_SET; }
    break;

  case 369: /* member_modifier: "'static'"  */
                                                        { (yyval.num) = T_STATIC; }
    break;

  case 370: /* member_modifier: "'abstract'"  */
                                                        { (yyval.num) = T_ABSTRACT; }
    break;

  case 371: /* member_modifier: "'final'"  */
                                                        { (yyval.num) = T_FINAL; }
    break;

  case 372: /* member_modifier: "'readonly'"  */
                                                        { (yyval.num) = T_READONLY; }
    break;

  case 373: /* property_list: property_list ',' property  */
                                           { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 374: /* property_list: property  */
                         { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[0].ast)); }
    break;

  case 375: /* property: "variable" backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-1].ast), NULL, ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 376: /* property: "variable" '=' expr backup_doc_comment  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL), NULL); }
    break;

  case 377: /* hooked_property: "variable" backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-4].ast), NULL, ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 378: /* hooked_property: "variable" '=' expr backup_doc_comment '{' property_hook_list '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[-6].ast), (yyvsp[-4].ast), ((yyvsp[-3].str) ? zend_ast_create_zval_from_str((yyvsp[-3].str)) : NULL), (yyvsp[-1].ast)); }
    break;

  case 379: /* property_hook_list: %empty  */
                       { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 380: /* property_hook_list: property_hook_list property_hook  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 381: /* property_hook_list: property_hook_list attributes property_hook  */
                                                            {
			(yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)));
		}
    break;

  case 382: /* optional_property_hook_list: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 383: /* optional_property_hook_list: '{' property_hook_list '}'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 384: /* property_hook_modifiers: %empty  */
                       { (yyval.num) = 0; }
    break;

  case 385: /* property_hook_modifiers: non_empty_member_modifiers  */
                                           {
			(yyval.num) = zend_modifier_list_to_flags(ZEND_MODIFIER_TARGET_PROPERTY_HOOK, (yyvsp[0].ast));
			if (!(yyval.num)) { YYERROR; }
		}
    break;

  case 386: /* @9: %empty  */
                                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 387: /* property_hook: property_hook_modifiers returns_ref "identifier" backup_doc_comment @9 optional_parameter_list backup_fn_flags property_hook_body backup_fn_flags  */
                                                                                           {
			(yyval.ast) = zend_ast_create_decl(
				ZEND_AST_PROPERTY_HOOK, (yyvsp[-8].num) | (yyvsp[-7].num) | (yyvsp[0].num), (yyvsp[-4].num), (yyvsp[-5].str), zend_ast_get_str((yyvsp[-6].ast)),
				(yyvsp[-3].ast), NULL, (yyvsp[-1].ast), NULL, NULL);
			CG(extra_fn_flags) = (yyvsp[-2].num);
		}
    break;

  case 388: /* property_hook_body: ';'  */
                    { (yyval.ast) = NULL; }
    break;

  case 389: /* property_hook_body: '{' inner_statement_list '}'  */
                                             { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 390: /* property_hook_body: "'=>'" expr ';'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROPERTY_HOOK_SHORT_BODY, (yyvsp[-1].ast)); }
    break;

  case 391: /* optional_parameter_list: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 392: /* optional_parameter_list: '(' parameter_list ')'  */
                                       { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 393: /* class_const_list: class_const_list ',' class_const_decl  */
                                                      { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 394: /* class_const_list: class_const_decl  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[0].ast)); }
    break;

  case 395: /* class_const_decl: "identifier" '=' expr backup_doc_comment  */
                                                     { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 396: /* class_const_decl: semi_reserved '=' expr backup_doc_comment  */
                                                          {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-3].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, zend_ast_create_zval(&zv), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL));
		}
    break;

  case 397: /* const_decl: "identifier" '=' expr backup_doc_comment  */
                                             { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[-3].ast), (yyvsp[-1].ast), ((yyvsp[0].str) ? zend_ast_create_zval_from_str((yyvsp[0].str)) : NULL)); }
    break;

  case 398: /* echo_expr_list: echo_expr_list ',' echo_expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 399: /* echo_expr_list: echo_expr  */
                          { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[0].ast)); }
    break;

  case 400: /* echo_expr: expr  */
             { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[0].ast)); }
    break;

  case 401: /* for_cond_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 402: /* for_cond_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 403: /* for_cond_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 404: /* for_exprs: %empty  */
                                        { (yyval.ast) = NULL; }
    break;

  case 405: /* for_exprs: non_empty_for_exprs  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 406: /* non_empty_for_exprs: non_empty_for_exprs ',' expr  */
                                             { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 407: /* non_empty_for_exprs: non_empty_for_exprs ',' "'(void)'" expr  */
                                                         { (yyval.ast) = zend_ast_list_add((yyvsp[-3].ast), zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[0].ast))); }
    break;

  case 408: /* non_empty_for_exprs: "'(void)'" expr  */
                                 { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, zend_ast_create(ZEND_AST_CAST_VOID, (yyvsp[0].ast))); }
    break;

  case 409: /* non_empty_for_exprs: expr  */
                     { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[0].ast)); }
    break;

  case 410: /* @10: %empty  */
                                                           { (yyval.num) = CG(zend_lineno); }
    break;

  case 411: /* anonymous_class: anonymous_class_modifiers_optional "'class'" @10 ctor_arguments extends_from implements_list backup_doc_comment '{' class_statement_list '}'  */
                                                                                             {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS | (yyvsp[-9].num), (yyvsp[-7].num), (yyvsp[-3].str), NULL,
				(yyvsp[-5].ast), (yyvsp[-4].ast), (yyvsp[-1].ast), NULL, NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[-6].ast));
		}
    break;

  case 412: /* new_dereferenceable: "'new'" class_name_reference argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 413: /* new_dereferenceable: "'new'" anonymous_class  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 414: /* new_dereferenceable: "'new'" attributes anonymous_class  */
                        { zend_ast_with_attributes((yyvsp[0].ast)->child[0], (yyvsp[-1].ast)); (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 415: /* new_non_dereferenceable: "'new'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[0].ast), zend_ast_create_list(0, ZEND_AST_ARG_LIST)); }
    break;

  case 416: /* expr: variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 417: /* expr: "'list'" '(' array_pair_list ')' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 418: /* expr: '[' array_pair_list ']' '=' expr  */
                        { (yyvsp[-3].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 419: /* expr: variable '=' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 420: /* expr: variable '=' ampersand variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[-3].ast), (yyvsp[0].ast)); }
    break;

  case 421: /* expr: "'clone'" clone_argument_list  */
                                            {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_CLONE));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, (yyvsp[0].ast));
		}
    break;

  case 422: /* expr: "'clone'" expr  */
                             {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_CLONE));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[0].ast)));
		}
    break;

  case 423: /* expr: variable "'+='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 424: /* expr: variable "'-='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 425: /* expr: variable "'*='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 426: /* expr: variable "'**='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 427: /* expr: variable "'/='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 428: /* expr: variable "'.='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_CONCAT, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 429: /* expr: variable "'%='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 430: /* expr: variable "'&='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 431: /* expr: variable "'|='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 432: /* expr: variable "'^='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 433: /* expr: variable "'<<='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 434: /* expr: variable "'>>='" expr  */
                        { (yyval.ast) = zend_ast_create_assign_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 435: /* expr: variable "'??='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 436: /* expr: variable "'++'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[-1].ast)); }
    break;

  case 437: /* expr: "'++'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[0].ast)); }
    break;

  case 438: /* expr: variable "'--'"  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[-1].ast)); }
    break;

  case 439: /* expr: "'--'" variable  */
                               { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[0].ast)); }
    break;

  case 440: /* expr: expr "'||'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 441: /* expr: expr "'&&'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 442: /* expr: expr "'or'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 443: /* expr: expr "'and'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 444: /* expr: expr "'xor'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 445: /* expr: expr '|' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 446: /* expr: expr "amp" expr  */
                                                                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 447: /* expr: expr "'&'" expr  */
                                                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 448: /* expr: expr '^' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 449: /* expr: expr '.' expr  */
                                { (yyval.ast) = zend_ast_create_concat_op((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 450: /* expr: expr '+' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 451: /* expr: expr '-' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 452: /* expr: expr '*' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 453: /* expr: expr "'**'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 454: /* expr: expr '/' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 455: /* expr: expr '%' expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 456: /* expr: expr "'<<'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 457: /* expr: expr "'>>'" expr  */
                                { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 458: /* expr: '+' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[0].ast)); }
    break;

  case 459: /* expr: '-' expr  */
                                   { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[0].ast)); }
    break;

  case 460: /* expr: '!' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[0].ast)); }
    break;

  case 461: /* expr: '~' expr  */
                         { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[0].ast)); }
    break;

  case 462: /* expr: expr "'==='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 463: /* expr: expr "'!=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 464: /* expr: expr "'=='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 465: /* expr: expr "'!='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 466: /* expr: expr "'|>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PIPE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 467: /* expr: expr '<' expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 468: /* expr: expr "'<='" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 469: /* expr: expr '>' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 470: /* expr: expr "'>='" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 471: /* expr: expr "'<=>'" expr  */
                        { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 472: /* expr: expr "'instanceof'" class_name_reference  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 473: /* expr: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_CONDITIONAL) (yyval.ast)->attr = ZEND_PARENTHESIZED_CONDITIONAL;
			if ((yyval.ast)->kind == ZEND_AST_ARROW_FUNC) (yyval.ast)->attr = ZEND_PARENTHESIZED_ARROW_FUNC;
		}
    break;

  case 474: /* expr: new_dereferenceable  */
                                    { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 475: /* expr: new_non_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 476: /* expr: expr '?' expr ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 477: /* expr: expr '?' ':' expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[-3].ast), NULL, (yyvsp[0].ast)); }
    break;

  case 478: /* expr: expr "'??'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 479: /* expr: internal_functions_in_yacc  */
                                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 480: /* expr: "'(int)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[0].ast)); }
    break;

  case 481: /* expr: "'(float)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[0].ast)); }
    break;

  case 482: /* expr: "'(string)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[0].ast)); }
    break;

  case 483: /* expr: "'(array)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[0].ast)); }
    break;

  case 484: /* expr: "'(object)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[0].ast)); }
    break;

  case 485: /* expr: "'(bool)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[0].ast)); }
    break;

  case 486: /* expr: "'(unset)'" expr  */
                                        { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[0].ast)); }
    break;

  case 487: /* expr: "'exit'" ctor_arguments  */
                                      {
			zend_ast *name = zend_ast_create_zval_from_str(ZSTR_KNOWN(ZEND_STR_EXIT));
			name->attr = ZEND_NAME_FQ;
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, name, (yyvsp[0].ast));
		}
    break;

  case 488: /* expr: '@' expr  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[0].ast)); }
    break;

  case 489: /* expr: scalar  */
                       { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 490: /* expr: '`' backticks_expr '`'  */
                                       { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[-1].ast)); }
    break;

  case 491: /* expr: "'print'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[0].ast)); }
    break;

  case 492: /* expr: "'yield'"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 493: /* expr: "'yield'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 494: /* expr: "'yield'" expr "'=>'" expr  */
                                                 { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[0].ast), (yyvsp[-2].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 495: /* expr: "'yield from'" expr  */
                                  { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[0].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 496: /* expr: "'throw'" expr  */
                             { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[0].ast)); }
    break;

  case 497: /* expr: inline_function  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 498: /* expr: attributes inline_function  */
                                           { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-1].ast)); }
    break;

  case 499: /* expr: "'static'" inline_function  */
                                         { (yyval.ast) = (yyvsp[0].ast); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 500: /* expr: attributes "'static'" inline_function  */
                        { (yyval.ast) = zend_ast_with_attributes((yyvsp[0].ast), (yyvsp[-2].ast)); ((zend_ast_decl *) (yyval.ast))->flags |= ZEND_ACC_STATIC; }
    break;

  case 501: /* expr: match  */
                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 502: /* inline_function: function returns_ref backup_doc_comment '(' parameter_list ')' lexical_vars return_type backup_fn_flags '{' inner_statement_list '}' backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[-11].num) | (yyvsp[0].num), (yyvsp[-12].num), (yyvsp[-10].str),
				  NULL,
				  (yyvsp[-8].ast), (yyvsp[-6].ast), (yyvsp[-2].ast), (yyvsp[-5].ast), NULL); CG(extra_fn_flags) = (yyvsp[-4].num); }
    break;

  case 503: /* inline_function: fn returns_ref backup_doc_comment '(' parameter_list ')' return_type "'=>'" backup_fn_flags expr backup_fn_flags  */
                        { (yyval.ast) = zend_ast_create_decl(ZEND_AST_ARROW_FUNC, (yyvsp[-9].num) | (yyvsp[0].num), (yyvsp[-10].num), (yyvsp[-8].str),
				  NULL, (yyvsp[-6].ast), NULL, (yyvsp[-1].ast), (yyvsp[-4].ast), NULL);
				  CG(extra_fn_flags) = (yyvsp[-2].num); }
    break;

  case 504: /* fn: "'fn'"  */
             { (yyval.num) = CG(zend_lineno); }
    break;

  case 505: /* function: "'function'"  */
                   { (yyval.num) = CG(zend_lineno); }
    break;

  case 506: /* backup_doc_comment: %empty  */
               { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 507: /* backup_fn_flags: %empty  */
                                         { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 508: /* returns_ref: %empty  */
                        { (yyval.num) = 0; }
    break;

  case 509: /* returns_ref: ampersand  */
                                { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 510: /* lexical_vars: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 511: /* lexical_vars: "'use'" '(' lexical_var_list possible_comma ')'  */
                                                              { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 512: /* lexical_var_list: lexical_var_list ',' lexical_var  */
                                                 { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 513: /* lexical_var_list: lexical_var  */
                            { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[0].ast)); }
    break;

  case 514: /* lexical_var: "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 515: /* lexical_var: ampersand "variable"  */
                                        { (yyval.ast) = (yyvsp[0].ast); (yyval.ast)->attr = ZEND_BIND_REF; }
    break;

  case 516: /* function_call: name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 517: /* function_call: "'readonly'" argument_list  */
                                         {
			zval zv;
			if (zend_lex_tstring(&zv, (yyvsp[-1].ident)) == FAILURE) { YYABORT; }
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, zend_ast_create_zval(&zv), (yyvsp[0].ast));
		}
    break;

  case 518: /* function_call: class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 519: /* function_call: variable_class_name "'::'" member_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 520: /* @11: %empty  */
                              { (yyval.num) = CG(zend_lineno); }
    break;

  case 521: /* function_call: callable_expr @11 argument_list  */
                                                                           {
			(yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[-2].ast), (yyvsp[0].ast));
			(yyval.ast)->lineno = (yyvsp[-1].num);
		}
    break;

  case 522: /* class_name: "'static'"  */
                        { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 523: /* class_name: name  */
                     { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 524: /* class_name_reference: class_name  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 525: /* class_name_reference: new_variable  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 526: /* class_name_reference: '(' expr ')'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 527: /* backticks_expr: %empty  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 528: /* backticks_expr: "string content"  */
                                          { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 529: /* backticks_expr: encaps_list  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 530: /* ctor_arguments: %empty  */
                        { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 531: /* ctor_arguments: argument_list  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 532: /* dereferenceable_scalar: "'array'" '(' array_pair_list ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 533: /* dereferenceable_scalar: '[' array_pair_list ']'  */
                                                        { (yyval.ast) = (yyvsp[-1].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 534: /* dereferenceable_scalar: "quoted string"  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 535: /* dereferenceable_scalar: '"' encaps_list '"'  */
                                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 536: /* scalar: "integer"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 537: /* scalar: "floating-point number"  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 538: /* scalar: "heredoc start" "string content" "heredoc end"  */
                                                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 539: /* scalar: "heredoc start" "heredoc end"  */
                        { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 540: /* scalar: "heredoc start" encaps_list "heredoc end"  */
                                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 541: /* scalar: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 542: /* scalar: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 543: /* scalar: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 544: /* constant: name  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[0].ast)); }
    break;

  case 545: /* constant: "'__LINE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 546: /* constant: "'__FILE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 547: /* constant: "'__DIR__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 548: /* constant: "'__TRAIT__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 549: /* constant: "'__METHOD__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 550: /* constant: "'__FUNCTION__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 551: /* constant: "'__PROPERTY__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_PROPERTY_C); }
    break;

  case 552: /* constant: "'__NAMESPACE__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 553: /* constant: "'__CLASS__'"  */
                                { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 554: /* class_constant: class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 555: /* class_constant: variable_class_name "'::'" identifier  */
                        { (yyval.ast) = zend_ast_create_class_const_or_name((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 556: /* class_constant: class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 557: /* class_constant: variable_class_name "'::'" '{' expr '}'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[-4].ast), (yyvsp[-1].ast)); }
    break;

  case 558: /* optional_expr: %empty  */
                        { (yyval.ast) = NULL; }
    break;

  case 559: /* optional_expr: expr  */
                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 560: /* variable_class_name: fully_dereferenceable  */
                                      { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 561: /* fully_dereferenceable: variable  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 562: /* fully_dereferenceable: '(' expr ')'  */
                             {
			(yyval.ast) = (yyvsp[-1].ast);
			if ((yyval.ast)->kind == ZEND_AST_STATIC_PROP) (yyval.ast)->attr = ZEND_PARENTHESIZED_STATIC_PROP;
		}
    break;

  case 563: /* fully_dereferenceable: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 564: /* fully_dereferenceable: class_constant  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 565: /* fully_dereferenceable: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 566: /* array_object_dereferenceable: fully_dereferenceable  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 567: /* array_object_dereferenceable: constant  */
                                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 568: /* callable_expr: callable_variable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 569: /* callable_expr: '(' expr ')'  */
                                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 570: /* callable_expr: dereferenceable_scalar  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 571: /* callable_expr: new_dereferenceable  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 572: /* callable_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 573: /* callable_variable: array_object_dereferenceable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 574: /* callable_variable: array_object_dereferenceable "'->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 575: /* callable_variable: array_object_dereferenceable "'?->'" property_name argument_list  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_METHOD_CALL, (yyvsp[-3].ast), (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 576: /* callable_variable: function_call  */
                              { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 577: /* variable: callable_variable  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 578: /* variable: static_member  */
                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 579: /* variable: array_object_dereferenceable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 580: /* variable: array_object_dereferenceable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 581: /* simple_variable: "variable"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 582: /* simple_variable: '$' '{' expr '}'  */
                                        { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 583: /* simple_variable: '$' simple_variable  */
                                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 584: /* static_member: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 585: /* static_member: variable_class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 586: /* new_variable: simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 587: /* new_variable: new_variable '[' optional_expr ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
    break;

  case 588: /* new_variable: new_variable "'->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 589: /* new_variable: new_variable "'?->'" property_name  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 590: /* new_variable: class_name "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 591: /* new_variable: new_variable "'::'" simple_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 592: /* member_name: identifier  */
                           { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 593: /* member_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 594: /* member_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 595: /* property_name: "identifier"  */
                         { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 596: /* property_name: '{' expr '}'  */
                                { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 597: /* property_name: simple_variable  */
                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 598: /* array_pair_list: non_empty_array_pair_list  */
                        { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[0].ast)); }
    break;

  case 599: /* possible_array_pair: %empty  */
                       { (yyval.ast) = NULL; }
    break;

  case 600: /* possible_array_pair: array_pair  */
                            { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 601: /* non_empty_array_pair_list: non_empty_array_pair_list ',' possible_array_pair  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 602: /* non_empty_array_pair_list: possible_array_pair  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[0].ast)); }
    break;

  case 603: /* array_pair: expr "'=>'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), (yyvsp[-2].ast)); }
    break;

  case 604: /* array_pair: expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[0].ast), NULL); }
    break;

  case 605: /* array_pair: expr "'=>'" ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), (yyvsp[-3].ast)); }
    break;

  case 606: /* array_pair: ampersand variable  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[0].ast), NULL); }
    break;

  case 607: /* array_pair: "'...'" expr  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[0].ast)); }
    break;

  case 608: /* array_pair: expr "'=>'" "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), (yyvsp[-5].ast)); }
    break;

  case 609: /* array_pair: "'list'" '(' array_pair_list ')'  */
                        { (yyvsp[-1].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[-1].ast), NULL); }
    break;

  case 610: /* encaps_list: encaps_list encaps_var  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 611: /* encaps_list: encaps_list "string content"  */
                        { (yyval.ast) = zend_ast_list_add((yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 612: /* encaps_list: encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[0].ast)); }
    break;

  case 613: /* encaps_list: "string content" encaps_var  */
                        { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[-1].ast), (yyvsp[0].ast)); }
    break;

  case 614: /* encaps_var: "variable"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 615: /* encaps_var: "variable" '[' encaps_var_offset ']'  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-3].ast)), (yyvsp[-1].ast)); }
    break;

  case 616: /* encaps_var: "variable" "'->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 617: /* encaps_var: "variable" "'?->'" "identifier"  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_NULLSAFE_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-2].ast)), (yyvsp[0].ast)); }
    break;

  case 618: /* encaps_var: "'${'" expr '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR, (yyvsp[-1].ast)); }
    break;

  case 619: /* encaps_var: "'${'" "variable name" '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_VAR, ZEND_ENCAPS_VAR_DOLLAR_CURLY, (yyvsp[-1].ast)); }
    break;

  case 620: /* encaps_var: "'${'" "variable name" '[' expr ']' '}'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_DIM, ZEND_ENCAPS_VAR_DOLLAR_CURLY,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[-4].ast)), (yyvsp[-2].ast)); }
    break;

  case 621: /* encaps_var: "'{$'" variable '}'  */
                                          { (yyval.ast) = (yyvsp[-1].ast); }
    break;

  case 622: /* encaps_var_offset: "identifier"  */
                                                { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 623: /* encaps_var_offset: "number"  */
                                        { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 624: /* encaps_var_offset: '-' "number"  */
                                        { (yyval.ast) = zend_negate_num_string((yyvsp[0].ast)); }
    break;

  case 625: /* encaps_var_offset: "variable"  */
                                                { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[0].ast)); }
    break;

  case 626: /* internal_functions_in_yacc: "'isset'" '(' isset_variables possible_comma ')'  */
                                                               { (yyval.ast) = (yyvsp[-2].ast); }
    break;

  case 627: /* internal_functions_in_yacc: "'empty'" '(' expr ')'  */
                                     { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[-1].ast)); }
    break;

  case 628: /* internal_functions_in_yacc: "'include'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[0].ast)); }
    break;

  case 629: /* internal_functions_in_yacc: "'include_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[0].ast)); }
    break;

  case 630: /* internal_functions_in_yacc: "'eval'" '(' expr ')'  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[-1].ast)); }
    break;

  case 631: /* internal_functions_in_yacc: "'require'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[0].ast)); }
    break;

  case 632: /* internal_functions_in_yacc: "'require_once'" expr  */
                        { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[0].ast)); }
    break;

  case 633: /* isset_variables: isset_variable  */
                               { (yyval.ast) = (yyvsp[0].ast); }
    break;

  case 634: /* isset_variables: isset_variables ',' isset_variable  */
                        { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[-2].ast), (yyvsp[0].ast)); }
    break;

  case 635: /* isset_variable: expr  */
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

	/* We used "amp" as a dummy label to avoid a duplicate token literal warning. */
	if (strcmp(toktype, "\"amp\"") == 0) {
		if (yyres) {
			yystpcpy(yyres, "token \"&\"");
		}
		return sizeof("token \"&\"")-1;
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
