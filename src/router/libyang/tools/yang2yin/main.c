/**
 * @file main.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yang2yin converter tool
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

#define INDENT(x) (x ? " " : "")
#define LEVEL(x) (x * 2)

enum yang_arg {
    Y_NO_ARG,
    Y_IDENTIF_ARG,
    Y_STR_ARG
};

enum yang_token {
    YANG_UNKNOWN = 0,
    YANG_ACTION,
    YANG_ANYDATA,
    YANG_ANYXML,
    YANG_ARGUMENT,
    YANG_AUGMENT,
    YANG_BASE,
    YANG_BELONGS_TO,
    YANG_BIT,
    YANG_CASE,
    YANG_CHOICE,
    YANG_CONFIG,
    YANG_CONTACT,
    YANG_CONTAINER,
    YANG_DEFAULT,
    YANG_DESCRIPTION,
    YANG_DEVIATE,
    YANG_DEVIATION,
    YANG_ENUM,
    YANG_ERROR_APP_TAG,
    YANG_ERROR_MESSAGE,
    YANG_EXTENSION,
    YANG_FEATURE,
    YANG_FRACTION_DIGITS,
    YANG_GROUPING,
    YANG_IDENTITY,
    YANG_IF_FEATURE,
    YANG_IMPORT,
    YANG_INCLUDE,
    YANG_INPUT,
    YANG_KEY,
    YANG_LEAF,
    YANG_LEAF_LIST,
    YANG_LENGTH,
    YANG_LIST,
    YANG_MANDATORY,
    YANG_MAX_ELEMENTS,
    YANG_MIN_ELEMENTS,
    YANG_MODIFIER,
    YANG_MODULE,
    YANG_MUST,
    YANG_NAMESPACE,
    YANG_NOTIFICATION,
    YANG_ORDERED_BY,
    YANG_ORGANIZATION,
    YANG_OUTPUT,
    YANG_PATH,
    YANG_PATTERN,
    YANG_POSITION,
    YANG_PREFIX,
    YANG_PRESENCE,
    YANG_RANGE,
    YANG_REFERENCE,
    YANG_REFINE,
    YANG_REQUIRE_INSTANCE,
    YANG_REVISION,
    YANG_REVISION_DATE,
    YANG_RPC,
    YANG_STATUS,
    YANG_SUBMODULE,
    YANG_TYPE,
    YANG_TYPEDEF,
    YANG_UNIQUE,
    YANG_UNITS,
    YANG_USES,
    YANG_VALUE,
    YANG_WHEN,
    YANG_YANG_VERSION,
    YANG_YIN_ELEMENT,

    YANG_SEMICOLON,
    YANG_LEFT_BRACE,
    YANG_RIGHT_BRACE,
    YANG_CUSTOM             /* simple extension with prefix */
};

static const char *
keyword2str(enum yang_token keyword)
{
    switch (keyword) {
    case YANG_ACTION:
        return "action";
    case YANG_ANYDATA:
        return "anydata";
    case YANG_ANYXML:
        return "anyxml";
    case YANG_ARGUMENT:
        return "argument";
    case YANG_AUGMENT:
        return "augment";
    case YANG_BASE:
        return "base";
    case YANG_BELONGS_TO:
        return "belongs-to";
    case YANG_BIT:
        return "bit";
    case YANG_CASE:
        return "case";
    case YANG_CHOICE:
        return "choice";
    case YANG_CONFIG:
        return "config";
    case YANG_CONTACT:
        return "contact";
    case YANG_CONTAINER:
        return "container";
    case YANG_DEFAULT:
        return "default";
    case YANG_DESCRIPTION:
        return "description";
    case YANG_DEVIATE:
        return "deviate";
    case YANG_DEVIATION:
        return "deviation";
    case YANG_ENUM:
        return "enum";
    case YANG_ERROR_APP_TAG:
        return "error-app-tag";
    case YANG_ERROR_MESSAGE:
        return "error-message";
    case YANG_EXTENSION:
        return "extension";
    case YANG_FEATURE:
        return "feature";
    case YANG_FRACTION_DIGITS:
        return "fraction-digits";
    case YANG_GROUPING:
        return "grouping";
    case YANG_IDENTITY:
        return "identitiy";
    case YANG_IF_FEATURE:
        return "if-feature";
    case YANG_IMPORT:
        return "import";
    case YANG_INCLUDE:
        return "include";
    case YANG_INPUT:
        return "input";
    case YANG_KEY:
        return "key";
    case YANG_LEAF:
        return "leaf";
    case YANG_LEAF_LIST:
        return "leaf-list";
    case YANG_LENGTH:
        return "length";
    case YANG_LIST:
        return "list";
    case YANG_MANDATORY:
        return "mandatory";
    case YANG_MAX_ELEMENTS:
        return "max-elements";
    case YANG_MIN_ELEMENTS:
        return "min-elements";
    case YANG_MODIFIER:
        return "modifier";
    case YANG_MODULE:
        return "module";
    case YANG_MUST:
        return "must";
    case YANG_NAMESPACE:
        return "namespace";
    case YANG_NOTIFICATION:
        return "notification";
    case YANG_ORDERED_BY:
        return "ordered-by";
    case YANG_ORGANIZATION:
        return "organization";
    case YANG_OUTPUT:
        return "output";
    case YANG_PATH:
        return "path";
    case YANG_PATTERN:
        return "pattern";
    case YANG_POSITION:
        return "position";
    case YANG_PREFIX:
        return "prefix";
    case YANG_PRESENCE:
        return "presence";
    case YANG_RANGE:
        return "range";
    case YANG_REFERENCE:
        return "reference";
    case YANG_REFINE:
        return "refine";
    case YANG_REQUIRE_INSTANCE:
        return "require-instance";
    case YANG_REVISION:
        return "revision";
    case YANG_REVISION_DATE:
        return "revision-date";
    case YANG_RPC:
        return "rpc";
    case YANG_STATUS:
        return "status";
    case YANG_SUBMODULE:
        return "submodule";
    case YANG_TYPE:
        return "type";
    case YANG_TYPEDEF:
        return "typedef";
    case YANG_UNIQUE:
        return "unique";
    case YANG_UNITS:
        return "units";
    case YANG_USES:
        return "uses";
    case YANG_VALUE:
        return "value";
    case YANG_WHEN:
        return "when";
    case YANG_YANG_VERSION:
        return "yang-version";
    case YANG_YIN_ELEMENT:
        return "yin-element";
    case YANG_SEMICOLON:
        return ";";
    case YANG_LEFT_BRACE:
        return "{";
    case YANG_RIGHT_BRACE:
        return "}";
    default:
        return "";
    }
}

static int
store_char(char c, char **buf, int *buf_len, int *used)
{
    char *new_buf;

    if (*used == *buf_len) {
        *buf_len += 16;
        new_buf = realloc(*buf, *buf_len * sizeof *new_buf);
        if (!new_buf) {
            fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
            return -1;
        }
        *buf = new_buf;
    }

    (*buf)[*used] = c;
    ++(*used);
    return 0;
}

static FILE *
open_module(const char *name, const char *search_dir)
{
    DIR *dir;
    struct dirent *dirent;
    char *path;
    FILE *ret = NULL;

    dir = opendir(search_dir);
    if (!dir) {
        fprintf(stderr, "Opening the directory \"%s\" failed (%s).\n", search_dir, strerror(errno));
        return NULL;
    }

    while ((dirent = readdir(dir))) {
        if (!strncmp(dirent->d_name, name, strlen(name))
                && ((dirent->d_name[strlen(name)] == '.') || (dirent->d_name[strlen(name)] == '@'))
                && !strcmp(dirent->d_name + strlen(dirent->d_name) - 5, ".yang")) {
            path = malloc(strlen(search_dir) + 1 + strlen(dirent->d_name) + 1);
            if (!path) {
                fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                break;
            }
            sprintf(path, "%s/%s", search_dir, dirent->d_name);
            ret = fopen(path, "r");
            free(path);

            if (!ret) {
                fprintf(stderr, "Opening file \"%s\" failed (%s).\n", dirent->d_name, strerror(errno));
            }
            break;
        }
    }

    closedir(dir);
    return ret;
}

/* read another word - character sequence without whitespaces (logs directly)
 * - words are basically YANG tokens
 * - there can be whitespaces if they are a part of string
 * - strings are always returned separately even if not separated by whitespaces
 * - strings are returned without ' or "
 * - strings divided by + are returned concatenated
 * - comments are skipped
 */
static char *
get_word(FILE *in, char **buf, int *buf_len)
{
    char c;
    /* comment: 0 - nothing, 1 - in line comment, 2 - in block comment, 3 - in block comment with last read character '*'
     * slash: 0 - nothing, 1 - last character was '/'
     * string: 0 - nothing, 1 - string with ', 2 - string with ", 3 - string with " with last character \,
     *         4 - string finished, now skipping whitespaces looking for +,
     *         5 - string continues after +, skipping whitespaces
     * indent: number of spaces of the indent, needed for double-quoted string, if following
     * str_nl_indent: can be non-zero only if string == 2, number of spaces to skip as the indent
     */
    int used = 0, ret, slash = 0, comment = 0, string = 0, indent = 0, str_nl_indent = 0;

    do {
        ret = fread(&c, sizeof c, 1, in);
        if (ret < 1) {
            if (feof(in)) {
                break;
            } else if (ferror(in)) {
                fprintf(stderr, "Read error (%s).\n", strerror(errno));
            } else {
                fprintf(stderr, "Int error (%d).\n", __LINE__);
            }
            return NULL;
        }

        if (string == 4) {
            if (c == '+') {
                /* string continues */
                string = 5;
            } else if (!isspace(c)) {
                /* nope, string is finished */
                if (ungetc(c, in) != c) {
                    fprintf(stderr, "ungetc() failed.\n");
                    return NULL;
                }
                break;
            }
        } else if (string == 5) {
            if (!isspace(c)) {
                if (c == '\'') {
                    string = 1;
                } else if (c == '\"') {
                    string = 2;
                } else {
                    /* it must be quoted again */
                    fprintf(stderr, "Both string parts divided by '+' must be quoted.\n");
                    return NULL;
                }
            }
        } else if (((c == '\'') || (c == '\"')) && !string && !comment) {
            if (used) {
                /* we want strings always in a separate word, leave it */
                if (ungetc(c, in) != c) {
                    fprintf(stderr, "ungetc() failed.\n");
                    return NULL;
                }
                break;
            }
            if (c == '\'') {
                string = 1;
            } else {
                string = 2;
                /* for the " itself */
                ++indent;
            }
        } else if (comment) {
            switch (comment) {
            case 1:
                if (c == '\n') {
                    comment = 0;
                }
                break;
            case 2:
                if (c == '*') {
                    comment = 3;
                }
                break;
            case 3:
                if (c == '/') {
                    comment = 0;
                } else {
                    comment = 2;
                }
                break;
            default:
                fprintf(stderr, "Int error (%d).\n", __LINE__);
                return NULL;
            }
        } else if (slash) {
            if (c == '/') {
                comment = 1;
            } else if (c == '*') {
                comment = 2;
            } else {
                if (store_char('/', buf, buf_len, &used) || store_char(c, buf, buf_len, &used)) {
                    return NULL;
                }
            }
            slash = 0;
        } else if ((c == '/') && !string) {
            slash = 1;
        } else if (((string == 1) && (c == '\'')) || ((string == 2) && (c == '\"'))) {
            /* string may be finished, but check for + */
            string = 4;
        } else if ((string == 2) && (c == '\\')) {
            /* special character following */
            string = 3;
        } else if (((c == ';') || (c == '{') || (c == '}')) && !string && used) {
            /* another keyword, leave it for later */
            if (ungetc(c, in) != c) {
                fprintf(stderr, "ungetc() failed.\n");
                return NULL;
            }
            break;
        } else if (!string && !used && ((c == ' ') || (c == '\t'))) {
            if (c == ' ') {
                ++indent;
            } else {
                indent += 8;
            }
        } else if (!isspace(c) || string) {
            if (string == 2) {
                if (isspace(c)) {
                    /* skipping indent spaces */
                    if (str_nl_indent && (c != '\n')) {
                        if (c == ' ') {
                            --str_nl_indent;
                        } else if (c == '\t') {
                            if (str_nl_indent < 9) {
                                str_nl_indent = 0;
                            } else {
                                str_nl_indent -= 8;
                            }
                        }
                        /* we just ignore other whitespace characters */
                        continue;
                    } else if (c == '\n') {
                        str_nl_indent = indent;
                    }
                } else if (str_nl_indent) {
                    /* first non-whitespace character, clear current indent */
                    str_nl_indent = 0;
                }
            } else if (string == 3) {
                /* string encoded characters */
                switch (c) {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case '\"':
                    /* c is correct */
                    break;
                case '\\':
                    /* c is correct */
                    break;
                default:
                    /* so let this one slide just like pyang and yanglint >:-|
                    fprintf(stderr, "Double-quoted string unknown special character '\\%c'.\n", c);
                    return NULL;*/
                    if (store_char('\\', buf, buf_len, &used)) {
                        return NULL;
                    }
                }
                string = 2;
            }
            if (store_char(c, buf, buf_len, &used)) {
                return NULL;
            }
            /* this is a keyword */
            if (((c == ';') || (c == '{') || (c == '}')) && !string) {
                break;
            }
        }
    } while (!isspace(c) || !used || string);

    if (store_char('\0', buf, buf_len, &used)) {
        return NULL;
    }
    return *buf;
}

/* (does not log) */
static enum yang_token
get_keyword(char *word, enum yang_arg *arg)
{
    enum yang_token ret = YANG_UNKNOWN;

    switch (word[0]) {
    case 'a':
        ++word;
        if (!strncmp(word, "ction", 5)) {
            word += 5;
            ret = YANG_ACTION;
            *arg = Y_STR_ARG;
	} else if (!strncmp(word, "nydata", 6)) {
            word += 6;
            ret = YANG_ANYDATA;
            *arg = Y_IDENTIF_ARG;
	} else if (!strncmp(word, "nyxml", 5)) {
            word += 5;
            ret = YANG_ANYXML;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "rgument", 7)) {
            word += 7;
            ret = YANG_ARGUMENT;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "ugment", 6)) {
            word += 6;
            ret = YANG_AUGMENT;
            *arg = Y_STR_ARG;
        }
        break;
    case 'b':
        ++word;
        if (!strncmp(word, "ase", 3)) {
            word += 3;
            ret = YANG_BASE;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "elongs-to", 9)) {
            word += 9;
            ret = YANG_BELONGS_TO;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "it", 2)) {
            word += 2;
            ret = YANG_BIT;
            *arg = Y_STR_ARG;
        }
        break;
    case 'c':
        ++word;
        if (!strncmp(word, "ase", 3)) {
            word += 3;
            ret = YANG_CASE;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "hoice", 5)) {
            word += 5;
            ret = YANG_CHOICE;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "on", 2)) {
            word += 2;
            if (!strncmp(word, "fig", 3)) {
                word += 3;
                ret = YANG_CONFIG;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "ta", 2)) {
                word += 2;
                if (!strncmp(word, "ct", 2)) {
                    word += 2;
                    ret = YANG_CONTACT;
                    *arg = Y_STR_ARG;
                } else if (!strncmp(word, "iner", 4)) {
                    word += 4;
                    ret = YANG_CONTAINER;
                    *arg = Y_IDENTIF_ARG;
                }
            }
        }
        break;
    case 'd':
        ++word;
        if (word[0] == 'e') {
            ++word;
            if (!strncmp(word, "fault", 5)) {
                word += 5;
                ret = YANG_DEFAULT;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "scription", 9)) {
                word += 9;
                ret = YANG_DESCRIPTION;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "viat", 4)) {
                word += 4;
                if (word[0] == 'e') {
                    ++word;
                    ret = YANG_DEVIATE;
                    *arg = Y_STR_ARG;
                } else if (!strncmp(word, "ion", 3)) {
                    word += 3;
                    ret = YANG_DEVIATION;
                    *arg = Y_STR_ARG;
                }
            }
        }
        break;
    case 'e':
        ++word;
        if (!strncmp(word, "num", 3)) {
            word += 3;
            ret = YANG_ENUM;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "rror-", 5)) {
            word += 5;
            if (!strncmp(word, "app-tag", 7)) {
                word += 7;
                ret = YANG_ERROR_APP_TAG;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "message", 7)) {
                word += 7;
                ret = YANG_ERROR_MESSAGE;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "xtension", 8)) {
            word += 8;
            ret = YANG_EXTENSION;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 'f':
        ++word;
        if (!strncmp(word, "eature", 6)) {
            word += 6;
            ret = YANG_FEATURE;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "raction-digits", 14)) {
            word += 14;
            ret = YANG_FRACTION_DIGITS;
            *arg = Y_STR_ARG;
        }
        break;
    case 'g':
        ++word;
        if (!strncmp(word, "rouping", 7)) {
            word += 7;
            ret = YANG_GROUPING;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 'i':
        ++word;
        if (!strncmp(word, "dentity", 7)) {
            word += 7;
            ret = YANG_IDENTITY;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "f-feature", 9)) {
            word += 9;
            ret = YANG_IF_FEATURE;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "mport", 5)) {
            word += 5;
            ret = YANG_IMPORT;
            *arg = Y_IDENTIF_ARG;
        } else if (word[0] == 'n') {
            ++word;
            if (!strncmp(word, "clude", 5)) {
                word += 5;
                ret = YANG_INCLUDE;
                *arg = Y_IDENTIF_ARG;
            } else if (!strncmp(word, "put", 3)) {
                word += 3;
                ret = YANG_INPUT;
                *arg = Y_NO_ARG;
            }
        }
        break;
    case 'k':
        ++word;
        if (!strncmp(word, "ey", 2)) {
            word += 2;
            ret = YANG_KEY;
            *arg = Y_STR_ARG;
        }
        break;
    case 'l':
        ++word;
        if (word[0] == 'e') {
            ++word;
            if (!strncmp(word, "af", 2)) {
                word += 2;
                if (word[0] != '-') {
                    ret = YANG_LEAF;
                    *arg = Y_IDENTIF_ARG;
                } else if (!strncmp(word, "-list", 5)) {
                    word += 5;
                    ret = YANG_LEAF_LIST;
                    *arg = Y_IDENTIF_ARG;
                }
            } else if (!strncmp(word, "ngth", 4)) {
                word += 4;
                ret = YANG_LENGTH;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "ist", 3)) {
            word += 3;
            ret = YANG_LIST;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 'm':
        ++word;
        if (word[0] == 'a') {
            ++word;
            if (!strncmp(word, "ndatory", 7)) {
                word += 7;
                ret = YANG_MANDATORY;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "x-elements", 10)) {
                word += 10;
                ret = YANG_MAX_ELEMENTS;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "in-elements", 11)) {
            word += 11;
            ret = YANG_MIN_ELEMENTS;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "odifier", 7)) {
            word += 7;
            ret = YANG_MODIFIER;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "odule", 5)) {
            word += 5;
            ret = YANG_MODULE;
            *arg = Y_IDENTIF_ARG;
        } else if (!strncmp(word, "ust", 3)) {
            word += 3;
            ret = YANG_MUST;
            *arg = Y_STR_ARG;
        }
        break;
    case 'n':
        ++word;
        if (!strncmp(word, "amespace", 8)) {
            word += 8;
            ret = YANG_NAMESPACE;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "otification", 11)) {
            word += 11;
            ret = YANG_NOTIFICATION;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 'o':
        ++word;
        if (word[0] == 'r') {
            ++word;
            if (!strncmp(word, "dered-by", 8)) {
                word += 8;
                ret = YANG_ORDERED_BY;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "ganization", 10)) {
                word += 10;
                ret = YANG_ORGANIZATION;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "utput", 5)) {
            word += 5;
            ret = YANG_OUTPUT;
            *arg = Y_NO_ARG;
        }
        break;
    case 'p':
        ++word;
        if (!strncmp(word, "at", 2)) {
            word += 2;
            if (word[0] == 'h') {
                ++word;
                ret = YANG_PATH;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "tern", 4)) {
                word += 4;
                ret = YANG_PATTERN;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "osition", 7)) {
            word += 7;
            ret = YANG_POSITION;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "re", 2)) {
            word += 2;
            if (!strncmp(word, "fix", 3)) {
                word += 3;
                ret = YANG_PREFIX;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "sence", 5)) {
                word += 5;
                ret = YANG_PRESENCE;
                *arg = Y_STR_ARG;
            }
        }
        break;
    case 'r':
        ++word;
        if (!strncmp(word, "ange", 4)) {
            word += 4;
            ret = YANG_RANGE;
            *arg = Y_STR_ARG;
        } else if (word[0] == 'e') {
            ++word;
            if (word[0] == 'f') {
                ++word;
                if (!strncmp(word, "erence", 6)) {
                    word += 6;
                    ret = YANG_REFERENCE;
                    *arg = Y_STR_ARG;
                } else if (!strncmp(word, "ine", 3)) {
                    word += 3;
                    ret = YANG_REFINE;
                    *arg = Y_STR_ARG;
                }
            } else if (!strncmp(word, "quire-instance", 14)) {
                word += 14;
                ret = YANG_REQUIRE_INSTANCE;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "vision", 6)) {
                word += 6;
                if (word[0] != '-') {
                    ret = YANG_REVISION;
                    *arg = Y_STR_ARG;
                } else if (!strncmp(word, "-date", 5)) {
                    word += 5;
                    ret = YANG_REVISION_DATE;
                    *arg = Y_STR_ARG;
                }
            }
        } else if (!strncmp(word, "pc", 2)) {
            word += 2;
            ret = YANG_RPC;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 's':
        ++word;
        if (!strncmp(word, "tatus", 5)) {
            word += 5;
            ret = YANG_STATUS;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "ubmodule", 8)) {
            word += 8;
            ret = YANG_SUBMODULE;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 't':
        ++word;
        if (!strncmp(word, "ype", 3)) {
            word += 3;
            if (word[0] != 'd') {
                ret = YANG_TYPE;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "def", 3)) {
                word += 3;
                ret = YANG_TYPEDEF;
                *arg = Y_IDENTIF_ARG;
            }
        }
        break;
    case 'u':
        ++word;
        if (!strncmp(word, "ni", 2)) {
            word += 2;
            if (!strncmp(word, "que", 3)) {
                word += 3;
                ret = YANG_UNIQUE;
                *arg = Y_STR_ARG;
            } else if (!strncmp(word, "ts", 2)) {
                word += 2;
                ret = YANG_UNITS;
                *arg = Y_STR_ARG;
            }
        } else if (!strncmp(word, "ses", 3)) {
            word += 3;
            ret = YANG_USES;
            *arg = Y_IDENTIF_ARG;
        }
        break;
    case 'v':
        ++word;
        if (!strncmp(word, "alue", 4)) {
            word += 4;
            ret = YANG_VALUE;
            *arg = Y_STR_ARG;
        }
        break;
    case 'w':
        ++word;
        if (!strncmp(word, "hen", 3)) {
            word += 3;
            ret = YANG_WHEN;
            *arg = Y_STR_ARG;
        }
        break;
    case 'y':
        ++word;
        if (!strncmp(word, "ang-version", 11)) {
            word += 11;
            ret = YANG_YANG_VERSION;
            *arg = Y_STR_ARG;
        } else if (!strncmp(word, "in-element", 10)) {
            word += 10;
            ret = YANG_YIN_ELEMENT;
            *arg = Y_STR_ARG;
        }
        break;
    case ';':
        ++word;
        ret = YANG_SEMICOLON;
        *arg = Y_NO_ARG;
        break;
    case '{':
        ++word;
        ret = YANG_LEFT_BRACE;
        *arg = Y_NO_ARG;
        break;
    case '}':
        ++word;
        ret = YANG_RIGHT_BRACE;
        *arg = Y_NO_ARG;
        break;
    default:
        break;
    }

    if ((ret == YANG_UNKNOWN) && strchr(word, ':') && (strchr(word, ':') == strrchr(word, ':'))) {
        /* some string with one ':', let's say it's an extension */
        word += strlen(word);
        ret = YANG_CUSTOM;
        *arg = Y_NO_ARG;
    }

    /* not a keyword after all */
    if ((ret != YANG_UNKNOWN) && word[0]) {
        ret = YANG_UNKNOWN;
    }
    return ret;
}

static void
print_text_xml_encode(FILE *out, const char *word)
{
    const char *ptr;

    for (ptr = word; ptr[0]; ++ptr) {
        switch (ptr[0]) {
        case '<':
            fputs("&lt;", out);
            break;
        case '>':
            fputs("&gt;", out);
            break;
        case '&':
            fputs("&amp;", out);
            break;
        /* not really necessary, I guess... */
        /*case '\'':
            fputs("&apos;", out);
            break;
        case '\"':
            fputs("&quot;", out);
            break;*/
        default:
            fputc(ptr[0], out);
            break;
        }
    }
}

static void
print_attr_val_xml_encode(FILE *out, const char *word)
{
    const char *ptr;

    for (ptr = word; ptr[0]; ++ptr) {
        switch (ptr[0]) {
        case '<':
            fputs("&lt;", out);
            break;
        case '>':
            fputs("&gt;", out);
            break;
        case '&':
            fputs("&amp;", out);
            break;
        /* we always use ", so ' is fine */
        /*case '\'':
            fputs("&apos;", out);
            break;*/
        case '\"':
            fputs("&quot;", out);
            break;
        default:
            fputc(ptr[0], out);
            break;
        }
    }
}

static int
print_keyword(enum yang_token keyword, enum yang_arg arg, FILE *out, int level, FILE *in, char **buf, int *buf_len)
{
    char *word;
    const char *yin_element = NULL, *close_tag;

    switch (keyword) {
    case YANG_ACTION:
        fprintf(out, "%*s<action name=\"", LEVEL(level), INDENT(level));
        close_tag = "action";
        break;
    case YANG_ANYDATA:
        fprintf(out, "%*s<anydata name=\"", LEVEL(level), INDENT(level));
        close_tag = "anydata";
        break;
    case YANG_ANYXML:
        fprintf(out, "%*s<anyxml name=\"", LEVEL(level), INDENT(level));
        close_tag = "anyxml";
        break;
    case YANG_ARGUMENT:
        fprintf(out, "%*s<argument name=\"", LEVEL(level), INDENT(level));
        close_tag = "argument";
        break;
    case YANG_AUGMENT:
        fprintf(out, "%*s<augment target-node=\"", LEVEL(level), INDENT(level));
        close_tag = "augment";
        break;
    case YANG_BASE:
        fprintf(out, "%*s<base name=\"", LEVEL(level), INDENT(level));
        close_tag = "base";
        break;
    case YANG_BELONGS_TO:
        fprintf(out, "%*s<belongs-to module=\"", LEVEL(level), INDENT(level));
        close_tag = "belongs-to";
        break;
    case YANG_BIT:
        fprintf(out, "%*s<bit name=\"", LEVEL(level), INDENT(level));
        close_tag = "bit";
        break;
    case YANG_CASE:
        fprintf(out, "%*s<case name=\"", LEVEL(level), INDENT(level));
        close_tag = "case";
        break;
    case YANG_CHOICE:
        fprintf(out, "%*s<choice name=\"", LEVEL(level), INDENT(level));
        close_tag = "choice";
        break;
    case YANG_CONFIG:
        fprintf(out, "%*s<config value=\"", LEVEL(level), INDENT(level));
        close_tag = "config";
        break;
    case YANG_CONTACT:
        fprintf(out, "%*s<contact>\n", LEVEL(level), INDENT(level));
        yin_element = "text";
        close_tag = "contact";
        break;
    case YANG_CONTAINER:
        fprintf(out, "%*s<container name=\"", LEVEL(level), INDENT(level));
        close_tag = "container";
        break;
    case YANG_DEFAULT:
        fprintf(out, "%*s<default value=\"", LEVEL(level), INDENT(level));
        close_tag = "default";
        break;
    case YANG_DESCRIPTION:
        fprintf(out, "%*s<description>\n", LEVEL(level), INDENT(level));
        yin_element = "text";
        close_tag = "description";
        break;
    case YANG_DEVIATE:
        fprintf(out, "%*s<deviate value=\"", LEVEL(level), INDENT(level));
        close_tag = "deviate";
        break;
    case YANG_DEVIATION:
        fprintf(out, "%*s<deviation target-node=\"", LEVEL(level), INDENT(level));
        close_tag = "deviation";
        break;
    case YANG_ENUM:
        fprintf(out, "%*s<enum name=\"", LEVEL(level), INDENT(level));
        close_tag = "enum";
        break;
    case YANG_ERROR_APP_TAG:
        fprintf(out, "%*s<error-app-tag value=\"", LEVEL(level), INDENT(level));
        close_tag = "error-app-tag";
        break;
    case YANG_ERROR_MESSAGE:
        fprintf(out, "%*s<error-message>\n", LEVEL(level), INDENT(level));
        yin_element = "value";
        close_tag = "error-message";
        break;
    case YANG_EXTENSION:
        fprintf(out, "%*s<extension name=\"", LEVEL(level), INDENT(level));
        close_tag = "extension";
        break;
    case YANG_FEATURE:
        fprintf(out, "%*s<feature name=\"", LEVEL(level), INDENT(level));
        close_tag = "feature";
        break;
    case YANG_FRACTION_DIGITS:
        fprintf(out, "%*s<fraction-digits value=\"", LEVEL(level), INDENT(level));
        close_tag = "fraction-digits";
        break;
    case YANG_GROUPING:
        fprintf(out, "%*s<grouping name=\"", LEVEL(level), INDENT(level));
        close_tag = "grouping";
        break;
    case YANG_IDENTITY:
        fprintf(out, "%*s<identity name=\"", LEVEL(level), INDENT(level));
        close_tag = "identity";
        break;
    case YANG_IF_FEATURE:
        fprintf(out, "%*s<if-feature name=\"", LEVEL(level), INDENT(level));
        close_tag = "if-feature";
        break;
    case YANG_IMPORT:
        fprintf(out, "%*s<import module=\"", LEVEL(level), INDENT(level));
        close_tag = "import";
        break;
    case YANG_INCLUDE:
        fprintf(out, "%*s<include module=\"", LEVEL(level), INDENT(level));
        close_tag = "include";
        break;
    case YANG_INPUT:
        fprintf(out, "%*s<input", LEVEL(level), INDENT(level));
        close_tag = "input";
        break;
    case YANG_KEY:
        fprintf(out, "%*s<key value=\"", LEVEL(level), INDENT(level));
        close_tag = "key";
        break;
    case YANG_LEAF:
        fprintf(out, "%*s<leaf name=\"", LEVEL(level), INDENT(level));
        close_tag = "leaf";
        break;
    case YANG_LEAF_LIST:
        fprintf(out, "%*s<leaf-list name=\"", LEVEL(level), INDENT(level));
        close_tag = "leaf-list";
        break;
    case YANG_LENGTH:
        fprintf(out, "%*s<length value=\"", LEVEL(level), INDENT(level));
        close_tag = "length";
        break;
    case YANG_LIST:
        fprintf(out, "%*s<list name=\"", LEVEL(level), INDENT(level));
        close_tag = "list";
        break;
    case YANG_MANDATORY:
        fprintf(out, "%*s<mandatory value=\"", LEVEL(level), INDENT(level));
        close_tag = "mandatory";
        break;
    case YANG_MAX_ELEMENTS:
        fprintf(out, "%*s<max-elements value=\"", LEVEL(level), INDENT(level));
        close_tag = "max-elements";
        break;
    case YANG_MIN_ELEMENTS:
        fprintf(out, "%*s<min-elements value=\"", LEVEL(level), INDENT(level));
        close_tag = "min-elements";
        break;
    case YANG_MODIFIER:
        fprintf(out, "%*s<modifier value=\"", LEVEL(level), INDENT(level));
        close_tag = "modifier";
        break;
    case YANG_MUST:
        fprintf(out, "%*s<must condition=\"", LEVEL(level), INDENT(level));
        close_tag = "must";
        break;
    case YANG_NAMESPACE:
        fprintf(out, "%*s<namespace uri=\"", LEVEL(level), INDENT(level));
        close_tag = "namespace";
        break;
    case YANG_NOTIFICATION:
        fprintf(out, "%*s<notification name=\"", LEVEL(level), INDENT(level));
        close_tag = "notification";
        break;
    case YANG_ORDERED_BY:
        fprintf(out, "%*s<ordered-by value=\"", LEVEL(level), INDENT(level));
        close_tag = "ordered-by";
        break;
    case YANG_ORGANIZATION:
        fprintf(out, "%*s<organization>\n", LEVEL(level), INDENT(level));
        yin_element = "text";
        close_tag = "organization";
        break;
    case YANG_OUTPUT:
        fprintf(out, "%*s<output", LEVEL(level), INDENT(level));
        close_tag = "output";
        break;
    case YANG_PATH:
        fprintf(out, "%*s<path value=\"", LEVEL(level), INDENT(level));
        close_tag = "path";
        break;
    case YANG_PATTERN:
        fprintf(out, "%*s<pattern value=\"", LEVEL(level), INDENT(level));
        close_tag = "pattern";
        break;
    case YANG_POSITION:
        fprintf(out, "%*s<position value=\"", LEVEL(level), INDENT(level));
        close_tag = "value";
        break;
    case YANG_PREFIX:
        fprintf(out, "%*s<prefix value=\"", LEVEL(level), INDENT(level));
        close_tag = "prefix";
        break;
    case YANG_PRESENCE:
        fprintf(out, "%*s<presence value=\"", LEVEL(level), INDENT(level));
        close_tag = "presence";
        break;
    case YANG_RANGE:
        fprintf(out, "%*s<range value=\"", LEVEL(level), INDENT(level));
        close_tag = "range";
        break;
    case YANG_REFERENCE:
        fprintf(out, "%*s<reference>\n", LEVEL(level), INDENT(level));
        yin_element = "text";
        close_tag = "reference";
        break;
    case YANG_REFINE:
        fprintf(out, "%*s<refine target-node=\"", LEVEL(level), INDENT(level));
        close_tag = "refine";
        break;
    case YANG_REQUIRE_INSTANCE:
        fprintf(out, "%*s<require-instance value=\"", LEVEL(level), INDENT(level));
        close_tag = "require-instance";
        break;
    case YANG_REVISION:
        fprintf(out, "%*s<revision date=\"", LEVEL(level), INDENT(level));
        close_tag = "revision";
        break;
    case YANG_REVISION_DATE:
        fprintf(out, "%*s<revision-date date=\"", LEVEL(level), INDENT(level));
        close_tag = "revision-date";
        break;
    case YANG_RPC:
        fprintf(out, "%*s<rpc name=\"", LEVEL(level), INDENT(level));
        close_tag = "rpc";
        break;
    case YANG_STATUS:
        fprintf(out, "%*s<status value=\"", LEVEL(level), INDENT(level));
        close_tag = "status";
        break;
    case YANG_TYPE:
        fprintf(out, "%*s<type name=\"", LEVEL(level), INDENT(level));
        close_tag = "type";
        break;
    case YANG_TYPEDEF:
        fprintf(out, "%*s<typedef name=\"", LEVEL(level), INDENT(level));
        close_tag = "typedef";
        break;
    case YANG_UNIQUE:
        fprintf(out, "%*s<unique tag=\"", LEVEL(level), INDENT(level));
        close_tag = "unique";
        break;
    case YANG_UNITS:
        fprintf(out, "%*s<units name=\"", LEVEL(level), INDENT(level));
        close_tag = "units";
        break;
    case YANG_USES:
        fprintf(out, "%*s<uses name=\"", LEVEL(level), INDENT(level));
        close_tag = "uses";
        break;
    case YANG_VALUE:
        fprintf(out, "%*s<value value=\"", LEVEL(level), INDENT(level));
        close_tag = "value";
        break;
    case YANG_WHEN:
        fprintf(out, "%*s<when condition=\"", LEVEL(level), INDENT(level));
        close_tag = "when";
        break;
    case YANG_YANG_VERSION:
        fprintf(out, "%*s<yang-version value=\"", LEVEL(level), INDENT(level));
        close_tag = "yang-version";
        break;
    case YANG_YIN_ELEMENT:
        fprintf(out, "%*s<yin-element value=\"", LEVEL(level), INDENT(level));
        close_tag = "yin-element";
        break;
    case YANG_CUSTOM:
        /* it must still be stored in buf */
        fprintf(out, "%*s<%s", LEVEL(level), INDENT(level), *buf);
        /* will be printed in case of some nested extension elements, we don't support that */
        close_tag = "!!error!!";
        break;
    default:
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        return -1;
    }

    /* keyword followed by a child element */
    if (yin_element) {
        ++level;
        fprintf(out, "%*s<%s>", LEVEL(level), INDENT(level), yin_element);
    }

    /* keyword argument */
    switch (arg) {
    case Y_IDENTIF_ARG:
        word = get_word(in, buf, buf_len);
        if (!word) {
            return -1;
        }

        /* word is the identifier (after some changes) */
        fprintf(out, "%s%s", word, (yin_element ? "" : "\""));
        break;

    case Y_STR_ARG:
        word = get_word(in, buf, buf_len);
        if (!word) {
            return -1;
        }
        /* word is the string */
        if (yin_element) {
            print_text_xml_encode(out, word);
        } else {
            print_attr_val_xml_encode(out, word);
        }

        if (!yin_element) {
            fprintf(out, "\"");
        }
        break;

    case Y_NO_ARG:
        break;
    }

    if (yin_element) {
        fprintf(out, "</%s>\n", yin_element);
        --level;
    }

    word = get_word(in, buf, buf_len);
    if (!word) {
        return -1;
    }
    keyword = get_keyword(word, &arg);
    if (!keyword) {
        fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
        return -1;
    }

    if (keyword == YANG_LEFT_BRACE) {
        if (!yin_element) {
            fprintf(out, ">\n");
        }

        while (1) {
            word = get_word(in, buf, buf_len);
            if (!word) {
                return -1;
            }
            keyword = get_keyword(word, &arg);
            if (!keyword) {
                fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
                return -1;
            }

            if (keyword == YANG_RIGHT_BRACE) {
                break;
            }

            if (print_keyword(keyword, arg, out, level + 1, in, buf, buf_len)) {
                return -1;
            }
        }

        fprintf(out, "%*s</%s>\n", LEVEL(level), INDENT(level), close_tag);
    }
    if ((keyword != YANG_SEMICOLON) && (keyword != YANG_RIGHT_BRACE)) {
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        return -1;
    }
    if (keyword == YANG_SEMICOLON) {
        if (yin_element) {
            fprintf(out, "%*s</%s>\n", LEVEL(level), INDENT(level), close_tag);
        } else {
            fprintf(out, "/>\n");
        }
    }

    return 0;
}

static int
print_sub_module(enum yang_token keyword, enum yang_arg arg, FILE *out, FILE *in, char **buf, int *buf_len,
                 const char *namespace, const char *prefix, char **import_namespaces, char **import_prefixes, int import_count)
{
    char *word;
    const char *close_tag;
    int i;

    switch (keyword) {
    case YANG_MODULE:
        fprintf(out, "<module name=\"");
        close_tag = "module";
        break;
    case YANG_SUBMODULE:
        fprintf(out, "<submodule name=\"");
        close_tag = "submodule";
        break;
    default:
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        return -1;
    }

    /* (sub)module name */
    switch (arg) {
    case Y_IDENTIF_ARG:
        word = get_word(in, buf, buf_len);
        if (!word) {
            return -1;
        }

        fprintf(out, "%s\"", word);
        break;

    default:
        fprintf(stderr, "Int error (%d).\n", __LINE__);
        return -1;
    }

    /* namespaces */
    fprintf(out, "\n%*sxmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"", (keyword == YANG_MODULE ? 8 : 11), " ");
    fprintf(out, "\n%*sxmlns:%s=\"%s\"", (keyword == YANG_MODULE ? 8 : 11), " ", prefix, namespace);
    for (i = 0; i < import_count; ++i) {
        fprintf(out, "\n%*sxmlns:%s=\"%s\"", (keyword == YANG_MODULE ? 8 : 11), " ", import_prefixes[i], import_namespaces[i]);
    }

    word = get_word(in, buf, buf_len);
    if (!word) {
        return -1;
    }
    keyword = get_keyword(word, &arg);
    if (!keyword) {
        fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
        return -1;
    }

    if (keyword != YANG_LEFT_BRACE) {
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        return -1;
    }

    fprintf(out, ">\n");

    while (1) {
        word = get_word(in, buf, buf_len);
        if (!word) {
            return -1;
        }
        keyword = get_keyword(word, &arg);
        if (!keyword) {
            fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
            return -1;
        }

        if (keyword == YANG_RIGHT_BRACE) {
            break;
        }

        if (print_keyword(keyword, arg, out, 1, in, buf, buf_len)) {
            return -1;
        }
    }

    fprintf(out, "</%s>\n", close_tag);

    return 0;
}

static int
find_namespace_imports(FILE *in, char **buf, int *buf_len, char **name_space, char **prefix, char ***import_modules,
                       char ***import_prefixes, int *import_count)
{
    char *word;
    void *new_buf;
    enum yang_token keyword;
    enum yang_arg arg;
    int i, want_prefix = 0;

    *name_space = NULL;
    if (prefix) {
        *prefix = NULL;
    }
    if (import_modules && import_prefixes && import_count) {
        *import_modules = NULL;
        *import_prefixes = NULL;
        *import_count = 0;
    }

    while (1) {
        word = get_word(in, buf, buf_len);
        if (!word) {
            goto error;
        } else if (!word[0]) {
            /* EOF */
            goto success;
        }

        keyword = get_keyword(word, &arg);
        if (!keyword) {
            fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
            goto error;
        }

        switch (arg) {
        case Y_IDENTIF_ARG:
        case Y_STR_ARG:
            word = get_word(in, buf, buf_len);
            if (!word) {
                goto error;
            }
            break;
        case Y_NO_ARG:
            break;
        }

        switch (keyword) {
        case YANG_NAMESPACE:
            /* module */
            if (want_prefix) {
                fprintf(stderr, "Unexpected keyword \"%s\", expected \"prefix\".\n", keyword2str(keyword));
                goto error;
            }
            if (*name_space) {
                fprintf(stderr, "Unexpected keyword \"%s\", already encountered.\n", keyword2str(keyword));
                goto error;
            }
            *name_space = strdup(word);
            if (!*name_space) {
                fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                goto error;
            }
            break;
        case YANG_IMPORT:
            if (want_prefix) {
                fprintf(stderr, "Unexpected keyword \"%s\", expected \"prefix\".\n", keyword2str(keyword));
                goto error;
            }
            if (import_modules && import_prefixes && import_count) {
                new_buf = realloc(*import_modules, (*import_count + 1) * sizeof **import_modules);
                if (!new_buf) {
                    fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                    goto error;
                }
                *import_modules = new_buf;

                new_buf = realloc(*import_prefixes, (*import_count + 1) * sizeof **import_prefixes);
                if (!new_buf) {
                    fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                    goto error;
                }
                *import_prefixes = new_buf;

                (*import_modules)[*import_count] = strdup(word);
                if (!(*import_modules)[*import_count]) {
                    fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                    goto error;
                }
                (*import_prefixes)[*import_count] = NULL;
                ++(*import_count);
            }
            want_prefix = 1;
            break;
        case YANG_BELONGS_TO:
            /* submodule */
            if (want_prefix) {
                fprintf(stderr, "Unexpected keyword \"%s\", expected \"prefix\".\n", keyword2str(keyword));
                goto error;
            }
            if (*name_space) {
                fprintf(stderr, "Unexpected keyword \"%s\", already encountered.\n", keyword2str(keyword));
                goto error;
            }
            *name_space = strdup(word);
            if (!*name_space) {
                fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                goto error;
            }
            break;
        case YANG_PREFIX:
            if (!want_prefix) {
                /* main (sub)module prefix */
                if (prefix) {
                    *prefix = strdup(word);
                    if (!*prefix) {
                        fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                        goto error;
                    }
                }
            } else {
                if (import_modules && import_prefixes && import_count) {
                    (*import_prefixes)[*import_count - 1] = strdup(word);
                    if (!(*import_prefixes)[*import_count - 1]) {
                        fprintf(stderr, "Memory allocation error (%s).\n", strerror(errno));
                        goto error;
                    }
                }
                want_prefix = 0;
            }
            break;
        case YANG_SEMICOLON:
        case YANG_LEFT_BRACE:
        case YANG_RIGHT_BRACE:
        case YANG_REVISION_DATE:
            /* the only ones that can appear before prefix */
            break;
        case YANG_CONTACT:
        case YANG_DESCRIPTION:
        case YANG_REFERENCE:
        case YANG_REVISION:
        case YANG_EXTENSION:
        case YANG_FEATURE:
        case YANG_IDENTITY:
        case YANG_TYPEDEF:
        case YANG_GROUPING:
        case YANG_CONTAINER:
        case YANG_LEAF:
        case YANG_LEAF_LIST:
        case YANG_LIST:
        case YANG_CHOICE:
        case YANG_ANYXML:
        case YANG_ANYDATA:
        case YANG_USES:
        case YANG_AUGMENT:
        case YANG_RPC:
        case YANG_NOTIFICATION:
        case YANG_DEVIATION:
            /* no import can follow */
            goto success;
        default:
            if (want_prefix) {
                fprintf(stderr, "Unexpected keyword \"%s\", expected \"prefix\".\n", keyword2str(keyword));
                goto error;
            }
            break;
        }
    }

success:
    if (want_prefix) {
        fprintf(stderr, "Unexpected EOF/keyword, expected \"prefix\".\n");
        goto error;
    }
    if (prefix && !(*prefix)) {
        fprintf(stderr, "Module prefix/submodule \"belongs-to\" prefix not found.\n");
        goto error;
    }
    if (!(*name_space)) {
        fprintf(stderr, "Module namespace/submodule \"belongs-to\" module not found.\n");
        goto error;
    }
    return 0;

error:
    if (import_modules && import_prefixes && import_count) {
        for (i = 0; i < *import_count; ++i) {
            free((*import_modules)[i]);
            free((*import_prefixes)[i]);
        }
        free(*import_modules);
        free(*import_prefixes);
    }
    if (prefix) {
        free(*prefix);
    }
    free(*name_space);
    return -1;
}

static int
convert_yang2yin(FILE *out, FILE *in, const char *search_dir)
{
    char *buf = NULL, *word, *name_space, *prefix, **import_modules, **import_prefixes;
    int buf_len = 0, ret = 0, import_count, i;
    enum yang_token keyword;
    enum yang_arg arg;
    FILE *mod_file = NULL;

    /*
     * 1st module parsing
     */

    /* learn whether it's a module or submodule */
    word = get_word(in, &buf, &buf_len);
    if (!word) {
        free(buf);
        return -1;
    }

    keyword = get_keyword(word, &arg);
    if (!keyword) {
        fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
        free(buf);
        return -1;
    }

    if ((keyword != YANG_MODULE) && (keyword != YANG_SUBMODULE)) {
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        free(buf);
        return -1;
    }

    word = get_word(in, &buf, &buf_len);
    if (!word) {
        free(buf);
        return -1;
    }

    /* learn namespace, imports of the (sub)module being converted */
    if (find_namespace_imports(in, &buf, &buf_len, &name_space, &prefix, &import_modules, &import_prefixes, &import_count)) {
        free(buf);
        return -1;
    }

    /* learn the main module namespace */
    if (keyword == YANG_SUBMODULE) {
        mod_file = open_module(name_space, search_dir);
        if (!mod_file) {
            fprintf(stderr, "Failed to open the module \"%s\".\n", name_space);
            ret = -1;
            goto cleanup;
        }
        free(name_space);
        name_space = NULL;

        if (find_namespace_imports(mod_file, &buf, &buf_len, &name_space, NULL, NULL, NULL, NULL)) {
            ret = -1;
            goto cleanup;
        }

        fclose(mod_file);
        mod_file = NULL;
    }

    /* learn the namespaces of import modules */
    for (i = 0; i < import_count; ++i) {
        mod_file = open_module(import_modules[i], search_dir);
        if (!mod_file) {
            fprintf(stderr, "Failed to open the module \"%s\".\n", import_modules[i]);
            ret = -1;
            goto cleanup;
        }
        free(import_modules[i]);
        import_modules[i] = NULL;

        if (find_namespace_imports(mod_file, &buf, &buf_len, &import_modules[i], NULL, NULL, NULL, NULL)) {
            ret = -1;
            goto cleanup;
        }

        fclose(mod_file);
        mod_file = NULL;
    }

    rewind(in);

    /*
     * 2nd module parsing
     */

    /* print xml header */
    fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

    word = get_word(in, &buf, &buf_len);
    if (!word) {
        ret = -1;
        goto cleanup;
    }

    keyword = get_keyword(word, &arg);
    if (!keyword) {
        fprintf(stderr, "Unexpected characters (\"%.20s\"...).\n", word);
        ret = -1;
        goto cleanup;
    }

    if ((keyword == YANG_MODULE) || (keyword == YANG_SUBMODULE)) {
        ret = print_sub_module(keyword, arg, out, in, &buf, &buf_len, name_space, prefix, import_modules,
                               import_prefixes, import_count);
    } else {
        fprintf(stderr, "Unexpected keyword \"%s\".\n", keyword2str(keyword));
        ret = -1;
        goto cleanup;
    }

cleanup:
    free(name_space);
    free(prefix);
    for (i = 0; i < import_count; ++i) {
        free(import_modules[i]);
        free(import_prefixes[i]);
    }
    free(import_modules);
    free(import_prefixes);
    if (mod_file) {
        fclose(mod_file);
    }
    free(buf);
    return ret;
}

int
main(int argc, char **argv)
{
    const char *in_file = NULL, *out_file = NULL, *search_dir = ".";
    char *ptr;
    FILE *input = NULL, *output = NULL;
    int ret = 1;

    switch (argc) {
    case 1:
        break;
    case 2:
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
            fprintf(stdout, "Usage:\n\t%s [input-file] [output-file]\n", argv[0]);
            fprintf(stdout, "\n\tinput-file:   intput yang file path. If empty, input from stdin. \n");
            fprintf(stdout, "\toutput-file:  output yin file path. If empty, output to stdout.  \n\n");
            return 0;
        }
        in_file = argv[1];
        break;
    case 3:
        in_file = argv[1];
        out_file = argv[2];
        break;
    default:
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    if (in_file) {
        input = fopen(in_file, "r");
        if (!input) {
            fprintf(stderr, "Failed to open \"%s\" for reading (%s).\n", in_file, strerror(errno));
            goto cleanup;
        }

        if ((ptr = strrchr(in_file, '/'))) {
            *ptr = '\0';
            search_dir = in_file;
        }
    }
    if (out_file) {
        output = fopen(out_file, "w");
        if (!input) {
            fprintf(stderr, "Failed to open \"%s\" for writing (%s).\n", out_file, strerror(errno));
            goto cleanup;
        }
    }

    ret = convert_yang2yin(output ? output : stdout, input ? input : stdin, search_dir);

cleanup:
    if (input) {
        fclose(input);
    }
    if (output) {
        fclose(output);
    }
    return ret;
}
