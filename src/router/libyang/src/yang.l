/**
 * @file yang.l
 * @author Pavol Vican
 * @brief YANG parser for libyang (flex grammar)
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

%option noyywrap nounput noinput reentrant bison-bridge bison-locations

%{
#include "parser_yang.h"
#include "parser_yang_bis.h"

#define YY_USER_ACTION yylloc->first_column = yylloc->last_column +1;\
                       yylloc->last_column = yylloc->first_column + yyleng - 1;
%}

U       [\x80-\xbf]
U2     [\xc2-\xdf]{U}
U3     [\xe0][\xa0-\xbf]{U}|[\xe1-\xec]{U}{U}|[\xed][\x80-\x9f]{U}|[\xee-\xef]{U}{U}
U4     [\xf0][\x90-\xbf]{U}{U}|[\xf1-\xf3]{U}{U}{U}|[\xf4][\x80-\x8f]{U}{U}

%x COMMENT1
%x COMMENT2
%x PATH
%x DOUBLEQUOTES
%x SINGLEQUOTES

%%
 int tab_count = 0;
 int size_str = 0;
 int column = 0;
 char *str = NULL;
 int _state = YY_START;
 int i;
 uint32_t value;


"/*" {_state = YY_START; BEGIN COMMENT1; }
<COMMENT1,COMMENT2>[\x00-\x09\x0B-\x7f]|{U2}|{U3}|{U4}
<COMMENT1>\n {yylloc->last_column = 0;}
<COMMENT1>"*/" {BEGIN _state; }
"//" {_state = YY_START; BEGIN COMMENT2;}
<COMMENT2>\n {BEGIN _state; yylloc->last_column = 0; return EOL; }
"anyxml" { return ANYXML_KEYWORD; }
"argument" { return ARGUMENT_KEYWORD; }
"augment" { BEGIN PATH; return AUGMENT_KEYWORD; }
"base" { return BASE_KEYWORD; }
"belongs-to" { return BELONGS_TO_KEYWORD; }
"bit" { return BIT_KEYWORD; }
"case" { return CASE_KEYWORD; }
"choice" { return CHOICE_KEYWORD; }
"config" { return CONFIG_KEYWORD; }
"contact" { return CONTACT_KEYWORD; }
"container" { return CONTAINER_KEYWORD; }
"default" { return DEFAULT_KEYWORD; }
"description" { return DESCRIPTION_KEYWORD; }
"enum" { return ENUM_KEYWORD; }
"error-app-tag" { return ERROR_APP_TAG_KEYWORD; }
"error-message" { return ERROR_MESSAGE_KEYWORD; }
"extension" { return EXTENSION_KEYWORD; }
"deviation" { BEGIN PATH; return DEVIATION_KEYWORD; }
"deviate" { return DEVIATE_KEYWORD; }
"feature" { return FEATURE_KEYWORD; }
"fraction-digits" { return FRACTION_DIGITS_KEYWORD; }
"grouping" { return GROUPING_KEYWORD; }
"identity" { return IDENTITY_KEYWORD; }
"if-feature" { return IF_FEATURE_KEYWORD; }
"import" { return IMPORT_KEYWORD; }
"include" { return INCLUDE_KEYWORD; }
"input" { return INPUT_KEYWORD; }
"key" { return KEY_KEYWORD; }
"leaf" { return LEAF_KEYWORD; }
"leaf-list" { return LEAF_LIST_KEYWORD; }
"length" { return LENGTH_KEYWORD; }
"list" { return LIST_KEYWORD; }
"mandatory" { return MANDATORY_KEYWORD; }
"max-elements" { return MAX_ELEMENTS_KEYWORD; }
"min-elements" { return MIN_ELEMENTS_KEYWORD; }
"module" { return MODULE_KEYWORD; }
"must" { return MUST_KEYWORD; }
"namespace" { return NAMESPACE_KEYWORD; }
"notification" { return NOTIFICATION_KEYWORD; }
"ordered-by" { return ORDERED_BY_KEYWORD; }
"organization" { return ORGANIZATION_KEYWORD; }
"output" { return OUTPUT_KEYWORD; }
"path" { BEGIN PATH; return PATH_KEYWORD; }
"pattern" { return PATTERN_KEYWORD; }
"position" { return POSITION_KEYWORD; }
"prefix" { return PREFIX_KEYWORD; }
"presence" { return PRESENCE_KEYWORD; }
"range" { return RANGE_KEYWORD; }
"reference" { return REFERENCE_KEYWORD; }
"refine" { BEGIN PATH; return REFINE_KEYWORD; }
"require-instance" { return REQUIRE_INSTANCE_KEYWORD; }
"revision" { return REVISION_KEYWORD; }
"revision-date" { return REVISION_DATE_KEYWORD; }
"rpc" { return RPC_KEYWORD; }
"status" { return STATUS_KEYWORD; }
"submodule" { return (yylloc->last_line) ? SUBMODULE_EXT_KEYWORD : SUBMODULE_KEYWORD; }
"type" { return TYPE_KEYWORD; }
"typedef" { return TYPEDEF_KEYWORD; }
"unique" { BEGIN PATH; return UNIQUE_KEYWORD; }
"units" { return UNITS_KEYWORD; }
"uses" { return USES_KEYWORD; }
"value" { return VALUE_KEYWORD; }
"when" { return WHEN_KEYWORD; }
"yang-version" { return YANG_VERSION_KEYWORD; }
"yin-element" { return YIN_ELEMENT_KEYWORD; }
"add" { return ADD_KEYWORD; }
"current" { return CURRENT_KEYWORD; }
<PATH>"current" { return CURRENT_KEYWORD; }
"delete" { return DELETE_KEYWORD; }
"deprecated" { return DEPRECATED_KEYWORD; }
"false" { return FALSE_KEYWORD; }
"not-supported" { return NOT_SUPPORTED_KEYWORD; }
"obsolete" { return OBSOLETE_KEYWORD; }
"replace" { return REPLACE_KEYWORD; }
"system" { return SYSTEM_KEYWORD; }
"true" { return TRUE_KEYWORD; }
"unbounded" { return UNBOUNDED_KEYWORD; }
"user" { return USER_KEYWORD; }
"action" {return ACTION_KEYWORD; }
"modifier" {return MODIFIER_KEYWORD; }
"anydata" {return ANYDATA_KEYWORD; }
"{" |
"}" |
";" |
"+"  { return yytext[0];}  /* unsolved problem with concatenate string '+' */
"\"" {_state = YY_START; BEGIN DOUBLEQUOTES; str = yytext; column = yylloc->first_column; }
<DOUBLEQUOTES>\t|\\t { tab_count++; size_str += yyleng; }
<DOUBLEQUOTES>[\x0D\x20-\x21\x23-\x5b\x5d-\x7f]|{U2} { size_str += yyleng; }
<DOUBLEQUOTES>\\([\x09\x0A\x0D\x20-\x7f]|{U2}|{U3}|{U4}) { size_str += yyleng; }
<DOUBLEQUOTES,SINGLEQUOTES>\n {yylloc->last_column = 0; size_str++; }
<DOUBLEQUOTES,SINGLEQUOTES>{U3} {
    value = ((uint32_t)(yytext[0] & 0xf) << 12) | ((uint32_t)(yytext[1] & 0x3f) << 6) | (yytext[2] & 0x3f);
    if (((value & 0xf800) == 0xd800) ||
        (value >= 0xfdd0 && value <= 0xfdef) ||
        (value & 0xffe) == 0xffe) {
        /* exclude surrogate blocks %xD800-DFFF */
        /* exclude noncharacters %xFDD0-FDEF */
        /* exclude noncharacters %xFFFE-FFFF */
        LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
        yylloc->first_line = -1;
        return ERROR;
    }
    size_str += 3;
}
<DOUBLEQUOTES,SINGLEQUOTES>{U4} {
    value = ((uint32_t)(yytext[0] & 0x7) << 18) | ((uint32_t)(yytext[1] & 0x3f) << 12) | ((uint32_t)(yytext[2] & 0x3f) << 6) | (yytext[3] & 0x3f);
    if ((value & 0xffe) == 0xffe) {
        /* exclude noncharacters %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
         * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
         * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
        LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
        yylloc->first_line = -1;
        return ERROR;
    }
    size_str += 4;
}
<DOUBLEQUOTES>\" { yylval->i = tab_count;
                   BEGIN _state;
                   yytext = str;
                   yyleng = size_str + 2;
                   yylloc->first_column = column;
                   return STRING;
                 }
<DOUBLEQUOTES><<EOF>> { LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Unterminated double-quoted string.");
                        yylloc->first_line = -1;
                        return ERROR;
                      }
<INITIAL,PATH>"'" { _state = YY_START;
                    BEGIN SINGLEQUOTES;
                    str = yytext;
                    column = yylloc->first_column;
                  }
<SINGLEQUOTES>[\x09\x0D\x20-\x26\x28-\x7f]|{U2} { size_str += yyleng; }
<SINGLEQUOTES>"'" { BEGIN _state;
                    yytext = str;
                    yyleng = size_str + 2;
                    yylloc->first_column = column;
                    return STRING;
                  }
<SINGLEQUOTES><<EOF>> { LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Unterminated single-quoted string.");
                        yylloc->first_line = -1;
                        return ERROR;
                      }
"0" {return ZERO;}
"-"[1-9][0-9]* {return INTEGER;}
[1-9][0-9]* {return NON_NEGATIVE_INTEGER;}
<PATH>"/" |
<PATH>"[" |
<PATH>"]" |
<PATH>"=" |
<PATH>"(" |
<PATH>")" |
<PATH>"+" {return yytext[0];}
<PATH>".." {return DOUBLEDOT;}
<PATH>\n  |
<PATH>\r\n { yylloc->last_column = 0; return EOL;}
<PATH>[ \t]+ { return WHITESPACE;}
<PATH>";" |
<PATH>"{" {BEGIN INITIAL; return yytext[0];}
<PATH>"\"" {_state = YY_START; BEGIN DOUBLEQUOTES; str = yytext; column = yylloc->first_column; }
<PATH>"//" {_state = YY_START; BEGIN COMMENT2;}
<PATH>"/*" {_state = YY_START; BEGIN COMMENT1;}
<PATH>[A-Za-z_][-A-Za-z0-9_\.]* {return IDENTIFIER;}
<PATH>[A-Za-z_][-A-Za-z0-9_\.]*:[A-Za-z_][-A-Za-z0-9_\.]*   {return IDENTIFIERPREFIX;}
[0-9]{4}[-][0-9]{2}[-][0-9]{2} {return REVISION_DATE;}
[A-Za-z_][-A-Za-z0-9_\.]* {return IDENTIFIER;}
[A-Za-z_][-A-Za-z0-9_\.]*:[A-Za-z_][-A-Za-z0-9_\.]*   {return IDENTIFIERPREFIX;}
([\x21\x23-\x26\x28-\x3a\x3c-\x7a\x7c\x7e-\x7f]|{U2}|{U3}|{U4})+ {
    if (yytext[0] == '/' && yytext[1] == '/') {
        _state = YY_START;
        yyless(2);
        BEGIN COMMENT2;
    } else if (yytext[0] == '/' && yytext[1] == '*') {
        _state = YY_START;
        yyless(2);
        BEGIN COMMENT1;
    } else {
        i = 0;
        while (i < yyleng) {
            if (!(yytext[i] & 0x80)) {
                /* one byte character */
                if (yytext[i] == '/') {
                    if (yytext[i + 1] == '/') {
                        yyless(i);
                        return STRINGS;
                    } else if (yytext[i + 1] == '*') {
                        yyless(i);
                        return STRINGS;
                    }
                } else if (yytext[i] == '*' && yytext[i + 1] == '/') {
                    if (!i) {
                        yyless(1);
                        return ERROR;
                    } else {
                        yyless(i);
                        return STRINGS;
                    }
                }
                ++i;
            } else if (!(yytext[i] & 0x20)) {
                /* two bytes character */
                i += 2;
            } else if (!(yytext[i] & 0x10)) {
                /* three bytes character */
                value = ((uint32_t)(yytext[i] & 0xf) << 12) | ((uint32_t)(yytext[i + 1] & 0x3f) << 6) | (yytext[i + 2] & 0x3f);
                if (((value & 0xf800) == 0xd800) ||
                    (value >= 0xfdd0 && value <= 0xfdef) ||
                    (value & 0xffe) == 0xffe) {
                    /* exclude surrogate blocks %xD800-DFFF */
                    /* exclude noncharacters %xFDD0-FDEF */
                    /* exclude noncharacters %xFFFE-FFFF */
                    LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
                    yylloc->first_line = -1;
                    return ERROR;
                }
                i += 3;
            } else {
                /* four bytes character */
                value = ((uint32_t)(yytext[i] & 0x7) << 18) | ((uint32_t)(yytext[i + 1] & 0x3f) << 12) | ((uint32_t)(yytext[i + 2] & 0x3f) << 6) | (yytext[i + 3] & 0x3f);
                if ((value & 0xffe) == 0xffe) {
                    /* exclude noncharacters %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
                     * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
                     * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
                    LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
                    yylloc->first_line = -1;
                    return ERROR;
                }
                i += 4;
            }
        }
        return STRINGS;
    }
}
\n |
\r\n { yylloc->last_column = 0; return EOL; }
[ \t]+ { return WHITESPACE;}

<COMMENT1,COMMENT2,DOUBLEQUOTES,SINGLEQUOTES,INITIAL,PATH>[\x00-\xff] {
    LOGVAL(yyget_extra(yyscanner), LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 leading byte 0x%02x", yytext[0]);
    yylloc->first_line = -1;
    return ERROR;
}

%%
