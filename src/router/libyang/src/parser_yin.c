/**
 * @file parser_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief YIN parser.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_internal.h"
#include "parser_schema.h"
#include "path.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"

struct lys_glob_unres;

/**
 * @brief check if given string is URI of yin namespace.
 *
 * @param ns Namespace URI to check.
 *
 * @return true if ns equals YIN_NS_URI false otherwise.
 */
#define IS_YIN_NS(ns) (strcmp(ns, YIN_NS_URI) == 0)

enum yin_argument {
    YIN_ARG_UNKNOWN = 0,   /**< parsed argument can not be matched with any supported yin argument keyword */
    YIN_ARG_NAME,          /**< argument name */
    YIN_ARG_TARGET_NODE,   /**< argument target-node */
    YIN_ARG_MODULE,        /**< argument module */
    YIN_ARG_VALUE,         /**< argument value */
    YIN_ARG_TEXT,          /**< argument text */
    YIN_ARG_CONDITION,     /**< argument condition */
    YIN_ARG_URI,           /**< argument uri */
    YIN_ARG_DATE,          /**< argument data */
    YIN_ARG_TAG,           /**< argument tag */
    YIN_ARG_NONE           /**< empty (special value) */
};

const char * const yin_attr_list[] = {
    [YIN_ARG_NAME] = "name",
    [YIN_ARG_TARGET_NODE] = "target-node",
    [YIN_ARG_MODULE] = "module",
    [YIN_ARG_VALUE] = "value",
    [YIN_ARG_TEXT] = "text",
    [YIN_ARG_CONDITION] = "condition",
    [YIN_ARG_URI] = "uri",
    [YIN_ARG_DATE] = "date",
    [YIN_ARG_TAG] = "tag",
    [YIN_ARG_NONE] = "none",
};

#define yin_attr2str(STMT) yin_attr_list[STMT]

#define VALID_VALS1 " Only valid value is \"%s\"."
#define VALID_VALS2 " Valid values are \"%s\" and \"%s\"."
#define VALID_VALS3 " Valid values are \"%s\", \"%s\" and \"%s\"."
#define VALID_VALS4 " Valid values are \"%s\", \"%s\", \"%s\" and \"%s\"."

/* shortcut to determin if keyword can in general be subelement of deviation regardles of it's type */
#define isdevsub(kw) (kw == LY_STMT_CONFIG || kw == LY_STMT_DEFAULT || kw == LY_STMT_MANDATORY || \
                      kw == LY_STMT_MAX_ELEMENTS || kw == LY_STMT_MIN_ELEMENTS ||              \
                      kw == LY_STMT_MUST || kw == LY_STMT_TYPE || kw == LY_STMT_UNIQUE ||         \
                      kw == LY_STMT_UNITS || kw == LY_STMT_EXTENSION_INSTANCE)

/* flags to set constraints of subelements */
#define YIN_SUBELEM_MANDATORY   0x01    /**< is set when subelement is mandatory */
#define YIN_SUBELEM_UNIQUE      0x02    /**< is set when subelement is unique */
#define YIN_SUBELEM_FIRST       0x04    /**< is set when subelement is actually yang argument mapped to yin element */
#define YIN_SUBELEM_VER2        0x08    /**< subelemnt is allowed only in modules with version at least 2 (YANG 1.1) */

#define YIN_SUBELEM_PARSED      0x80    /**< is set during parsing when given subelement is encountered for the first
                                             time to simply check validity of given constraints */

struct yin_subelement {
    enum ly_stmt type;      /**< type of keyword */
    void *dest;             /**< meta infromation passed to responsible function (mostly information about where parsed subelement should be stored) */
    uint16_t flags;         /**< describes constraints of subelement can be set to YIN_SUBELEM_MANDATORY, YIN_SUBELEM_UNIQUE, YIN_SUBELEM_FIRST, YIN_SUBELEM_VER2, and YIN_SUBELEM_DEFAULT_TEXT */
};

/* Meta information passed to yin_parse_argument function,
   holds information about where content of argument element will be stored. */
struct yin_argument_meta {
    uint16_t *flags;        /**< Argument flags */
    const char **argument;  /**< Argument value */
};

/**
 * @brief Meta information passed to functions working with tree_schema,
 *        that require additional information about parent node.
 */
struct tree_node_meta {
    struct lysp_node *parent;       /**< parent node */
    struct lysp_node **nodes;    /**< linked list of siblings */
};

/**
 * @brief Meta information passed to yin_parse_import function.
 */
struct import_meta {
    const char *prefix;             /**< module prefix. */
    struct lysp_import **imports;   /**< imports to add to. */
};

/**
 * @brief Meta information passed to yin_parse_include function.
 */
struct include_meta {
    const char *name;               /**< Module/submodule name. */
    struct lysp_include **includes; /**< [Sized array](@ref sizedarrays) of parsed includes to add to. */
};

/**
 * @brief Meta information passed to yin_parse_inout function.
 */
struct inout_meta {
    struct lysp_node *parent;          /**< Parent node. */
    struct lysp_node_action_inout *inout_p; /**< inout_p Input/output pointer to write to. */
};

/**
 * @brief Meta information passed to yin_parse_minmax function.
 */
struct minmax_dev_meta {
    uint32_t *lim;                      /**< min/max value to write to. */
    uint16_t *flags;                    /**< min/max flags to write to. */
    struct lysp_ext_instance **exts;    /**< extension instances to add to. */
};

LY_ERR yin_parse_content(struct lys_yin_parser_ctx *ctx, struct yin_subelement *subelem_info, size_t subelem_info_size,
        enum ly_stmt current_element, const char **text_content, struct lysp_ext_instance **exts);

/**
 * @brief Match yang keyword from yin data.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] name Start of keyword name
 * @param[in] name_len Lenght of keyword name.
 * @param[in] prefix Start of keyword prefix.
 * @param[in] prefix_len Lenght of prefix.
 * @param[in] parrent Identification of parrent element, use LY_STMT_NONE for elements without parrent.
 *
 * @return yang_keyword values.
 */
enum ly_stmt
yin_match_keyword(struct lys_yin_parser_ctx *ctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, enum ly_stmt parent)
{
    const char *start = NULL;
    enum ly_stmt kw = LY_STMT_NONE;
    const struct lyxml_ns *ns = NULL;
    struct ly_in *in;

    if (!name || (name_len == 0)) {
        return LY_STMT_NONE;
    }

    ns = lyxml_ns_get(&ctx->xmlctx->ns, prefix, prefix_len);
    if (ns) {
        if (!IS_YIN_NS(ns->uri)) {
            return LY_STMT_EXTENSION_INSTANCE;
        }
    } else {
        /* elements without namespace are automatically unknown */
        return LY_STMT_NONE;
    }

    LY_CHECK_RET(ly_in_new_memory(name, &in), LY_STMT_NONE);
    start = in->current;
    kw = lysp_match_kw(in, NULL);
    name = in->current;
    ly_in_free(in, 0);

    if (name - start == (long int)name_len) {
        /* this is done because of collision in yang statement value and yang argument mapped to yin element value */
        if ((kw == LY_STMT_VALUE) && (parent == LY_STMT_ERROR_MESSAGE)) {
            return LY_STMT_ARG_VALUE;
        }
        return kw;
    } else {
        if (strncmp(start, "text", name_len) == 0) {
            return LY_STMT_ARG_TEXT;
        } else {
            return LY_STMT_NONE;
        }
    }
}

/**
 * @brief Match argument name.
 *
 * @param[in] name String representing name.
 * @param[in] len Lenght of the name.
 *
 * @return yin_argument values.
 */
enum yin_argument
yin_match_argument_name(const char *name, size_t len)
{
    enum yin_argument arg = YIN_ARG_UNKNOWN;
    size_t already_read = 0;

    LY_CHECK_RET(len == 0, YIN_ARG_NONE);

#define READ_INC(LEN) already_read += LEN
#define ARG_SET(STMT) arg=STMT
#define ARG_CHECK(STR, LEN) (!strncmp((name) + already_read, STR, LEN) && (READ_INC(LEN)))

    switch (*name) {
    case 'c':
        READ_INC(1);
        if (ARG_CHECK("ondition", 8)) {
            ARG_SET(YIN_ARG_CONDITION);
        }
        break;

    case 'd':
        READ_INC(1);
        if (ARG_CHECK("ate", 3)) {
            ARG_SET(YIN_ARG_DATE);
        }
        break;

    case 'm':
        READ_INC(1);
        if (ARG_CHECK("odule", 5)) {
            ARG_SET(YIN_ARG_MODULE);
        }
        break;

    case 'n':
        READ_INC(1);
        if (ARG_CHECK("ame", 3)) {
            ARG_SET(YIN_ARG_NAME);
        }
        break;

    case 't':
        READ_INC(1);
        if (ARG_CHECK("a", 1)) {
            if (ARG_CHECK("g", 1)) {
                ARG_SET(YIN_ARG_TAG);
            } else if (ARG_CHECK("rget-node", 9)) {
                ARG_SET(YIN_ARG_TARGET_NODE);
            }
        } else if (ARG_CHECK("ext", 3)) {
            ARG_SET(YIN_ARG_TEXT);
        }
        break;

    case 'u':
        READ_INC(1);
        if (ARG_CHECK("ri", 2)) {
            ARG_SET(YIN_ARG_URI);
        }
        break;

    case 'v':
        READ_INC(1);
        if (ARG_CHECK("alue", 4)) {
            ARG_SET(YIN_ARG_VALUE);
        }
        break;
    }

    /* whole argument must be matched */
    if (already_read != len) {
        arg = YIN_ARG_UNKNOWN;
    }

#undef ARG_CHECK
#undef ARG_SET
#undef READ_INC

    return arg;
}

#define IS_NODE_ELEM(kw) (kw == LY_STMT_ANYXML || kw == LY_STMT_ANYDATA || kw == LY_STMT_LEAF || kw == LY_STMT_LEAF_LIST || \
                          kw == LY_STMT_TYPEDEF || kw == LY_STMT_USES || kw == LY_STMT_LIST || kw == LY_STMT_NOTIFICATION || \
                          kw == LY_STMT_GROUPING || kw == LY_STMT_CONTAINER || kw == LY_STMT_CASE || kw == LY_STMT_CHOICE || \
                          kw == LY_STMT_ACTION || kw == LY_STMT_RPC || kw == LY_STMT_AUGMENT)

#define HAS_META(kw) (IS_NODE_ELEM(kw) || kw == LY_STMT_IMPORT || kw == LY_STMT_INCLUDE || kw == LY_STMT_INPUT || kw == LY_STMT_OUTPUT)

/**
 * @brief Free subelems information allocated on heap.
 *
 * @param[in] count Size of subelems array.
 * @param[in] subelems Subelems array to free.
 */
static void
subelems_deallocator(size_t count, struct yin_subelement *subelems)
{
    for (size_t i = 0; i < count; ++i) {
        if (HAS_META(subelems[i].type)) {
            free(subelems[i].dest);
        }
    }

    free(subelems);
}

/**
 * @brief Allocate subelems information on heap.
 *
 * @param[in] ctx YIN parser context, used for logging.
 * @param[in] count Number of subelements.
 * @param[in] parent Parent node if any.
 * @param[out] result Allocated subelems array.
 *
 * @return LY_SUCCESS on success LY_EMEM on memmory allocation failure.
 */
static LY_ERR
subelems_allocator(struct lys_yin_parser_ctx *ctx, size_t count, struct lysp_node *parent,
        struct yin_subelement **result, ...)
{
    va_list ap;

    *result = calloc(count, sizeof **result);
    LY_CHECK_GOTO(!(*result), mem_err);

    va_start(ap, result);
    for (size_t i = 0; i < count; ++i) {
        /* TYPE */
        (*result)[i].type = va_arg(ap, enum ly_stmt);
        /* DEST */
        if (IS_NODE_ELEM((*result)[i].type)) {
            struct tree_node_meta *node_meta = NULL;
            node_meta = calloc(1, sizeof *node_meta);
            LY_CHECK_GOTO(!node_meta, mem_err);
            node_meta->parent = parent;
            node_meta->nodes = va_arg(ap, void *);
            (*result)[i].dest = node_meta;
        } else if ((*result)[i].type == LY_STMT_IMPORT) {
            struct import_meta *imp_meta = NULL;
            imp_meta = calloc(1, sizeof *imp_meta);
            LY_CHECK_GOTO(!imp_meta, mem_err);
            imp_meta->prefix = va_arg(ap, const char *);
            imp_meta->imports = va_arg(ap, struct lysp_import **);
            (*result)[i].dest = imp_meta;
        } else if ((*result)[i].type == LY_STMT_INCLUDE) {
            struct include_meta *inc_meta = NULL;
            inc_meta = calloc(1, sizeof *inc_meta);
            LY_CHECK_GOTO(!inc_meta, mem_err);
            inc_meta->name = va_arg(ap, const char *);
            inc_meta->includes = va_arg(ap, struct lysp_include **);
            (*result)[i].dest = inc_meta;
        } else if (((*result)[i].type == LY_STMT_INPUT) || ((*result)[i].type == LY_STMT_OUTPUT)) {
            struct inout_meta *inout_meta = NULL;
            inout_meta = calloc(1, sizeof *inout_meta);
            LY_CHECK_GOTO(!inout_meta, mem_err);
            inout_meta->parent = parent;
            inout_meta->inout_p = va_arg(ap, struct lysp_node_action_inout *);
            (*result)[i].dest = inout_meta;
        } else {
            (*result)[i].dest = va_arg(ap, void *);
        }
        /* FLAGS */
        (*result)[i].flags = va_arg(ap, int);
    }
    va_end(ap);

    return LY_SUCCESS;

mem_err:
    subelems_deallocator(count, *result);
    LOGMEM(ctx->xmlctx->ctx);
    return LY_EMEM;
}

/**
 * @brief Check that val is valid UTF8 character sequence of val_type.
 *        Doesn't check empty string, only character validity.
 *
 * @param[in] ctx Yin parser context for logging.
 * @param[in] val_type Type of the input string to select method of checking character validity.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_validate_value(struct lys_yin_parser_ctx *ctx, enum yang_arg val_type)
{
    uint8_t prefix = 0;
    uint32_t c;
    size_t utf8_char_len, already_read = 0;
    const char *val;

    assert((ctx->xmlctx->status == LYXML_ELEM_CONTENT) || (ctx->xmlctx->status == LYXML_ATTR_CONTENT));

    val = ctx->xmlctx->value;
    while (already_read < ctx->xmlctx->value_len) {
        LY_CHECK_ERR_RET(ly_getutf8((const char **)&val, &c, &utf8_char_len),
                LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INCHAR, (val)[-utf8_char_len]), LY_EVALID);

        switch (val_type) {
        case Y_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar((struct lys_parser_ctx *)ctx, c, already_read ? 0 : 1, NULL));
            break;
        case Y_PREF_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar((struct lys_parser_ctx *)ctx, c, already_read ? 0 : 1, &prefix));
            break;
        case Y_STR_ARG:
        case Y_MAYBE_STR_ARG:
            LY_CHECK_RET(lysp_check_stringchar((struct lys_parser_ctx *)ctx, c));
            break;
        }

        already_read += utf8_char_len;
        LY_CHECK_ERR_RET(already_read > ctx->xmlctx->value_len, LOGINT(ctx->xmlctx->ctx), LY_EINT);
    }

    if ((already_read == 0) &&
            ((val_type == Y_PREF_IDENTIF_ARG) || (val_type == Y_IDENTIF_ARG))) {
        /* Empty identifier or prefix*/
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YIN, "Empty identifier is not allowed.");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse yin attributes.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] arg_type Type of argument that is expected in parsed element (use YIN_ARG_NONE for elements without
 *            special argument).
 * @param[out] arg_val Where value of argument should be stored. Can be NULL iff arg_type is specified as YIN_ARG_NONE.
 * @param[in] val_type Type of expected value of attribute.
 * @param[in] current_element Identification of current element, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_attribute(struct lys_yin_parser_ctx *ctx, enum yin_argument arg_type, const char **arg_val, enum yang_arg val_type,
        enum ly_stmt current_element)
{
    enum yin_argument arg = YIN_ARG_UNKNOWN;
    bool found = false;

    /* validation of attributes */
    while (ctx->xmlctx->status == LYXML_ATTRIBUTE) {
        /* yin arguments represented as attributes have no namespace, which in this case means no prefix */
        if (!ctx->xmlctx->prefix) {
            arg = yin_match_argument_name(ctx->xmlctx->name, ctx->xmlctx->name_len);
            if (arg == YIN_ARG_NONE) {
                /* skip it */
                LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
            } else if (arg == arg_type) {
                LY_CHECK_ERR_RET(found, LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_DUP_ATTR,
                        yin_attr2str(arg), ly_stmt2str(current_element)), LY_EVALID);
                found = true;

                /* go to value */
                LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
                LY_CHECK_RET(yin_validate_value(ctx, val_type));
                INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, *arg_val);
                LY_CHECK_RET(!(*arg_val), LY_EMEM);
            } else {
                LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_ATTR, ctx->xmlctx->name_len,
                        ctx->xmlctx->name, ly_stmt2str(current_element));
                return LY_EVALID;
            }
        } else {
            /* skip it */
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        }

        /* next attribute */
        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    }

    /* anything else than Y_MAYBE_STR_ARG is mandatory */
    if ((val_type != Y_MAYBE_STR_ARG) && !found) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LYVE_SYNTAX_YIN, "Missing mandatory attribute %s of %s element.",
                yin_attr2str(arg_type), ly_stmt2str(current_element));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get record with given type.
 *
 * @param[in] type Type of wanted record.
 * @param[in] array_size Size of array.
 * @param[in] array Searched array.
 *
 * @return Pointer to desired record on success, NULL if element is not in the array.
 */
static struct yin_subelement *
get_record(enum ly_stmt type, LY_ARRAY_COUNT_TYPE array_size, struct yin_subelement *array)
{
    for (LY_ARRAY_COUNT_TYPE u = 0; u < array_size; ++u) {
        if (array[u].type == type) {
            return &array[u];
        }
    }
    return NULL;
}

/**
 * @brief Helper function to check mandatory constraint of subelement.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] subelem_info Array of information about subelements.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in] current_element Identification of element that is currently being parsed, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_check_subelem_mandatory_constraint(struct lys_yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
        signed char subelem_info_size, enum ly_stmt current_element)
{
    for (signed char i = 0; i < subelem_info_size; ++i) {
        /* if there is element that is mandatory and isn't parsed log error and return LY_EVALID */
        if ((subelem_info[i].flags & YIN_SUBELEM_MANDATORY) && !(subelem_info[i].flags & YIN_SUBELEM_PARSED)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_MAND_SUBELEM,
                    ly_stmt2str(subelem_info[i].type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Helper function to check "first" constraint of subelement.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] subelem_info Array of information about subelements.
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in] current_element Identification of element that is currently being parsed, used for logging.
 * @param[in] exp_first Record in subelem_info array that is expected to be defined as first subelement, used for logging.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_check_subelem_first_constraint(struct lys_yin_parser_ctx *ctx, struct yin_subelement *subelem_info,
        signed char subelem_info_size, enum ly_stmt current_element,
        struct yin_subelement *exp_first)
{
    for (signed char i = 0; i < subelem_info_size; ++i) {
        if (subelem_info[i].flags & YIN_SUBELEM_PARSED) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_FIRT_SUBELEM,
                    ly_stmt2str(exp_first->type), ly_stmt2str(current_element));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute,
 * for example prefix or namespace element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element.
 * @param[out] value Where value of attribute should be stored.
 * @param[in] arg_type Expected type of attribute.
 * @param[in] arg_val_type Type of expected value of attribute.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_simple_element(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, const char **value,
        enum yin_argument arg_type, enum yang_arg arg_val_type, struct lysp_ext_instance **exts)
{
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, arg_type, value, arg_val_type, kw));
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), kw, NULL, exts);
}

/**
 * @brief Parse path element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element.
 * @param[out] type Type structure to store parsed value, flags and extension instances.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_path(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, struct lysp_type *type)
{
    LY_ERR ret;
    const char *str_path;

    LY_CHECK_RET(yin_parse_simple_element(ctx, kw, &str_path, YIN_ARG_VALUE, Y_STR_ARG, &type->exts));
    ret = ly_path_parse(ctx->xmlctx->ctx, NULL, str_path, 0, LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &type->path);
    lydict_remove(ctx->xmlctx->ctx, str_path);
    LY_CHECK_RET(ret);
    type->flags |= LYS_SET_PATH;

    return LY_SUCCESS;
}

/**
 * @brief Parse pattern element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] type Type structure to store parsed value, flags and extension instances.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_pattern(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    const char *real_value = NULL;
    char *saved_value = NULL;
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, type->patterns, restr, LY_EMEM);
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &real_value, Y_STR_ARG, LY_STMT_PATTERN));
    size_t len = strlen(real_value);

    saved_value = malloc(len + 2);
    LY_CHECK_ERR_RET(!saved_value, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
    memmove(saved_value + 1, real_value, len);
    lydict_remove(ctx->xmlctx->ctx, real_value);
    saved_value[0] = LYSP_RESTR_PATTERN_ACK;
    saved_value[len + 1] = '\0';
    LY_CHECK_RET(lydict_insert_zc(ctx->xmlctx->ctx, saved_value, &restr->arg.str));
    restr->arg.mod = ctx->parsed_mod;
    type->flags |= LYS_SET_PATTERN;

    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &restr->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_APP_TAG, &restr->eapptag, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_MESSAGE, &restr->emsg, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MODIFIER, &restr->arg, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &restr->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_PATTERN, NULL, &restr->exts);
}

/**
 * @brief Parse fraction-digits element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] type Type structure to store value, flags and extension instances.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_fracdigits(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_FRACTION_DIGITS));

    if ((temp_val[0] == '\0') || (temp_val[0] == '0') || !isdigit(temp_val[0])) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "fraction-digits");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(temp_val, &ptr, LY_BASE_DEC);
    if (*ptr != '\0') {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "fraction-digits");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > LY_TYPE_DEC64_FD_MAX)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "fraction-digits");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);
    type->fraction_digits = num;
    type->flags |= LYS_SET_FRDIGITS;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_FRACTION_DIGITS, NULL, &type->exts);
}

/**
 * @brief Parse enum element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] type Type structure to store parsed value, flags and extension instances.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_enum(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    struct lysp_type_enum *en;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, type->enums, en, LY_EMEM);
    type->flags |= LYS_SET_ENUM;
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &en->name, Y_STR_ARG, LY_STMT_ENUM));
    LY_CHECK_RET(lysp_check_enum_name((struct lys_parser_ctx *)ctx, en->name, strlen(en->name)));
    CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(en->name), "enum");
    CHECK_UNIQUENESS((struct lys_parser_ctx *)ctx, type->enums, name, "enum", en->name);

    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &en->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &en->iffeatures, 0},
        {LY_STMT_REFERENCE, &en->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &en->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_VALUE, en, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_ENUM, NULL, &en->exts);
}

/**
 * @brief Parse bit element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] type Type structure to store parsed value, flags and extension instances.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_bit(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    struct lysp_type_enum *en;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, type->bits, en, LY_EMEM);
    type->flags |= LYS_SET_BIT;
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &en->name, Y_IDENTIF_ARG, LY_STMT_BIT));
    CHECK_UNIQUENESS((struct lys_parser_ctx *)ctx, type->bits, name, "bit", en->name);

    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &en->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &en->iffeatures, 0},
        {LY_STMT_POSITION, en, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &en->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &en->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_BIT, NULL, &en->exts);
}

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute, that can have
 * more instances, such as base or if-feature.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element.
 * @param[out] values Parsed values to add to.
 * @param[in] arg_type Expected type of attribute.
 * @param[in] arg_val_type Type of expected value of attribute.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_simple_elements(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, const char ***values, enum yin_argument arg_type,
        enum yang_arg arg_val_type, struct lysp_ext_instance **exts)
{
    const char **value;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *values, value, LY_EMEM);
    LY_ARRAY_COUNT_TYPE index = LY_ARRAY_COUNT(*values) - 1;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, &index, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, arg_type, value, arg_val_type, kw));

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), kw, NULL, exts);
}

/**
 * @brief Parse simple element without any special constraints and argument mapped to yin attribute.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element.
 * @param[in] subinfo Information about subelement, is used to determin which function should be called and where to store parsed value.
 * @param[in] arg_type Expected type of attribute.
 * @param[in] arg_val_type Type of expected value of attribute.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_simple_elem(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, struct yin_subelement *subinfo,
        enum yin_argument arg_type, enum yang_arg arg_val_type, struct lysp_ext_instance **exts)
{
    if (subinfo->flags & YIN_SUBELEM_UNIQUE) {
        LY_CHECK_RET(yin_parse_simple_element(ctx, kw, (const char **)subinfo->dest,
                arg_type, arg_val_type, exts));
    } else {
        LY_CHECK_RET(yin_parse_simple_elements(ctx, kw, (const char ***)subinfo->dest,
                arg_type, arg_val_type, exts));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse base element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] parent Identification of parent element.
 * @param[out] dest Where parsed values should be stored.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_base(struct lys_yin_parser_ctx *ctx, enum ly_stmt parent, void *dest, struct lysp_ext_instance **exts)
{
    struct lysp_type *type = NULL;

    if (parent == LY_STMT_TYPE) {
        type = (struct lysp_type *)dest;
        LY_CHECK_RET(yin_parse_simple_elements(ctx, LY_STMT_BASE, &type->bases, YIN_ARG_NAME,
                Y_PREF_IDENTIF_ARG, exts));
        type->flags |= LYS_SET_BASE;
    } else if (parent == LY_STMT_IDENTITY) {
        LY_CHECK_RET(yin_parse_simple_elements(ctx, LY_STMT_BASE, (const char ***)dest,
                YIN_ARG_NAME, Y_PREF_IDENTIF_ARG, exts));
    } else {
        LOGINT(ctx->xmlctx->ctx);
        return LY_EINT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse require-instance element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] type Type structure to store value, flag and extensions.
 * @return LY_ERR values.
 */
static LY_ERR
yin_pasrse_reqinstance(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    type->flags |= LYS_SET_REQINST;
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_REQUIRE_INSTANCE));
    if (strcmp(temp_val, "true") == 0) {
        type->require_instance = 1;
    } else if (strcmp(temp_val, "false") != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_val, "value",
                "require-instance", "true", "false");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_REQUIRE_INSTANCE, NULL, &type->exts);
}

/**
 * @brief Parse modifier element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] pat Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_modifier(struct lys_yin_parser_ctx *ctx, const char **pat, struct lysp_ext_instance **exts)
{
    assert(**pat == 0x06);
    const char *temp_val;
    char *modified_val;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_MODIFIER));
    if (strcmp(temp_val, "invert-match") != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS1, temp_val, "value",
                "modifier", "invert-match");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    /* allocate new value */
    modified_val = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!modified_val, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
    strcpy(modified_val, *pat);
    lydict_remove(ctx->xmlctx->ctx, *pat);

    /* modify the new value */
    modified_val[0] = LYSP_RESTR_PATTERN_NACK;
    LY_CHECK_RET(lydict_insert_zc(ctx->xmlctx->ctx, modified_val, pat));

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_MODIFIER, NULL, exts);
}

/**
 * @brief Parse a restriction element (length, range or one instance of must).
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] restr_kw Identificaton of element that is being parsed, can be set to LY_STMT_MUST, LY_STMT_LENGTH or LY_STMT_RANGE.
 * @param[in] restr Value to write to.
 */
static LY_ERR
yin_parse_restriction(struct lys_yin_parser_ctx *ctx, enum ly_stmt restr_kw, struct lysp_restr *restr)
{
    assert(restr_kw == LY_STMT_MUST || restr_kw == LY_STMT_LENGTH || restr_kw == LY_STMT_RANGE);
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &restr->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_APP_TAG, &restr->eapptag, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_MESSAGE, &restr->emsg, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &restr->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };
    /* argument of must is called condition, but argument of length and range is called value */
    enum yin_argument arg_type = (restr_kw == LY_STMT_MUST) ? YIN_ARG_CONDITION : YIN_ARG_VALUE;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, arg_type, &restr->arg.str, Y_STR_ARG, restr_kw));
    restr->arg.mod = ctx->parsed_mod;

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), restr_kw, NULL, &restr->exts);
}

/**
 * @brief Parse range element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] type Type structure to store parsed value and flags.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_range(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    type->range = calloc(1, sizeof *type->range);
    LY_CHECK_ERR_RET(!type->range, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
    LY_CHECK_RET(yin_parse_restriction(ctx, LY_STMT_RANGE, type->range));
    type->flags |= LYS_SET_RANGE;

    return LY_SUCCESS;
}

/**
 * @brief Parse length element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] type Type structure to store parsed value and flags.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_length(struct lys_yin_parser_ctx *ctx, struct lysp_type *type)
{
    type->length = calloc(1, sizeof *type->length);
    LY_CHECK_ERR_RET(!type->length, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
    LY_CHECK_RET(yin_parse_restriction(ctx, LY_STMT_LENGTH, type->length));
    type->flags |= LYS_SET_LENGTH;

    return LY_SUCCESS;
}

/**
 * @brief Parse must element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] restrs Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_must(struct lys_yin_parser_ctx *ctx, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *restrs, restr, LY_EMEM);
    return yin_parse_restriction(ctx, LY_STMT_MUST, restr);
}

/**
 * @brief Parse a node id into an array.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element.
 * @param[in] subinfo Information about subelement, is used to determin which function should be called and where to store parsed value.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_qname(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, struct yin_subelement *subinfo,
        struct lysp_ext_instance **exts)
{
    struct lysp_qname *qname, **qnames;

    switch (kw) {
    case LY_STMT_DEFAULT:
        if (subinfo->flags & YIN_SUBELEM_UNIQUE) {
            qname = (struct lysp_qname *)subinfo->dest;
        } else {
            qnames = (struct lysp_qname **)subinfo->dest;
            LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *qnames, qname, LY_EMEM);
        }
        qname->mod = ctx->parsed_mod;
        return yin_parse_simple_element(ctx, kw, &qname->str, YIN_ARG_VALUE, Y_STR_ARG, exts);
    case LY_STMT_UNIQUE:
        assert(!(subinfo->flags & YIN_SUBELEM_UNIQUE));
        qnames = (struct lysp_qname **)subinfo->dest;
        LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *qnames, qname, LY_EMEM);
        qname->mod = ctx->parsed_mod;
        return yin_parse_simple_element(ctx, kw, &qname->str, YIN_ARG_TAG, Y_STR_ARG, exts);
    case LY_STMT_IF_FEATURE:
        assert(!(subinfo->flags & YIN_SUBELEM_UNIQUE));
        qnames = (struct lysp_qname **)subinfo->dest;
        LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *qnames, qname, LY_EMEM);
        qname->mod = ctx->parsed_mod;
        return yin_parse_simple_element(ctx, kw, &qname->str, YIN_ARG_NAME, Y_STR_ARG, exts);
    default:
        break;
    }

    LOGINT(ctx->xmlctx->ctx);
    return LY_EINT;
}

/**
 * @brief Parse position or value element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] kw Type of current element, can be set to LY_STMT_POSITION or LY_STMT_VALUE.
 * @param[out] enm Enum structure to save value, flags and extension instances.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_value_pos(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, struct lysp_type_enum *enm)
{
    assert(kw == LY_STMT_POSITION || kw == LY_STMT_VALUE);
    const char *temp_val = NULL;
    char *ptr;
    long int num = 0;
    unsigned long int unum = 0;

    /* set value flag */
    enm->flags |= LYS_SET_VALUE;

    /* get attribute value */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, kw));
    if (!temp_val || (temp_val[0] == '\0') || (temp_val[0] == '+') ||
            ((temp_val[0] == '0') && (temp_val[1] != '\0')) || ((kw == LY_STMT_POSITION) && !strcmp(temp_val, "-0"))) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", ly_stmt2str(kw));
        goto error;
    }

    /* convert value */
    errno = 0;
    if (kw == LY_STMT_VALUE) {
        num = strtol(temp_val, &ptr, LY_BASE_DEC);
        if ((num < INT64_C(-2147483648)) || (num > INT64_C(2147483647))) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", ly_stmt2str(kw));
            goto error;
        }
    } else {
        unum = strtoul(temp_val, &ptr, LY_BASE_DEC);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", ly_stmt2str(kw));
            goto error;
        }
    }
    /* check if whole argument value was converted */
    if (*ptr != '\0') {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", ly_stmt2str(kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB_YIN, temp_val, "value", ly_stmt2str(kw));
        goto error;
    }
    /* save correctly ternary operator can't be used because num and unum have different signes */
    if (kw == LY_STMT_VALUE) {
        enm->value = num;
    } else {
        enm->value = unum;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    /* parse subelements */
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), kw, NULL, &enm->exts);

error:
    lydict_remove(ctx->xmlctx->ctx, temp_val);
    return LY_EVALID;
}

/**
 * @brief Parse belongs-to element.
 *
 * @param[in] ctx YIN parser context for logging and to store current state.
 * @param[out] submod Structure of submodule that is being parsed.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values
 */
static LY_ERR
yin_parse_belongs_to(struct lys_yin_parser_ctx *ctx, struct lysp_submodule *submod, struct lysp_ext_instance **exts)
{
    const char *belongsto;
    struct yin_subelement subelems[] = {
        {LY_STMT_PREFIX, &submod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_MODULE, &belongsto, Y_IDENTIF_ARG, LY_STMT_BELONGS_TO));
    if (ctx->parsed_mod->mod->name != belongsto) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YIN, "Submodule \"belongs-to\" value \"%s\" does not match its module name \"%s\".",
                belongsto, ctx->parsed_mod->mod->name);
        lydict_remove(ctx->xmlctx->ctx, belongsto);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, belongsto);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_BELONGS_TO, NULL, exts);
}

/**
 * @brief Function to parse meta tags (description, contact, ...) eg. elements with
 * text element as child.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] elem_type Type of element can be set to LY_STMT_ORGANIZATION or LY_STMT_CONTACT or LY_STMT_DESCRIPTION or LY_STMT_REFERENCE.
 * @param[out] value Where the content of meta element should be stored.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_meta(struct lys_yin_parser_ctx *ctx, enum ly_stmt elem_type, const char **value, struct lysp_ext_instance **exts)
{
    assert(elem_type == LY_STMT_ORGANIZATION || elem_type == LY_STMT_CONTACT || elem_type == LY_STMT_DESCRIPTION || elem_type == LY_STMT_REFERENCE);

    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        {LY_STMT_ARG_TEXT, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST}
    };

    /* check attributes */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NONE, NULL, Y_MAYBE_STR_ARG, elem_type));

    /* parse content */
    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), elem_type, NULL, exts);
}

/**
 * @brief Parse error-message element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] value Where the content of error-message element should be stored.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_err_msg(struct lys_yin_parser_ctx *ctx, const char **value, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        {LY_STMT_ARG_VALUE, value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE | YIN_SUBELEM_FIRST}
    };

    /* check attributes */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NONE, NULL, Y_MAYBE_STR_ARG, LY_STMT_ERROR_MESSAGE));

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_ERROR_MESSAGE, NULL, exts);
}

/**
 * @brief Parse type element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] parent Identification of parent element.
 * @param[in,out] type Type to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_type(struct lys_yin_parser_ctx *ctx, enum ly_stmt parent, struct yin_subelement *subinfo)
{
    struct lysp_type *type = NULL;

    if (parent == LY_STMT_DEVIATE) {
        *(struct lysp_type **)subinfo->dest = calloc(1, sizeof **(struct lysp_type **)subinfo->dest);
        LY_CHECK_ERR_RET(!(*(struct lysp_type **)subinfo->dest), LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
        type = *((struct lysp_type **)subinfo->dest);
    } else {
        type = (struct lysp_type *)subinfo->dest;
    }

    /* type as child of another type */
    if (parent == LY_STMT_TYPE) {
        struct lysp_type *nested_type = NULL;
        LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, type->types, nested_type, LY_EMEM);
        type->flags |= LYS_SET_TYPE;
        type = nested_type;
    }

    type->pmod = ctx->parsed_mod;

    struct yin_subelement subelems[] = {
        {LY_STMT_BASE, type, 0},
        {LY_STMT_BIT, type, 0},
        {LY_STMT_ENUM, type, 0},
        {LY_STMT_FRACTION_DIGITS, type, YIN_SUBELEM_UNIQUE},
        {LY_STMT_LENGTH, type, YIN_SUBELEM_UNIQUE},
        {LY_STMT_PATH, type, YIN_SUBELEM_UNIQUE},
        {LY_STMT_PATTERN, type, 0},
        {LY_STMT_RANGE, type, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REQUIRE_INSTANCE, type, YIN_SUBELEM_UNIQUE},
        {LY_STMT_TYPE, type},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &type->name, Y_PREF_IDENTIF_ARG, LY_STMT_TYPE));
    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_TYPE, NULL, &type->exts);
}

/**
 * @brief Parse max-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] max Value to write to.
 * @param[in] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_maxelements(struct lys_yin_parser_ctx *ctx, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    *flags |= LYS_SET_MAX;
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_MAX_ELEMENTS));
    if (!temp_val || (temp_val[0] == '\0') || (temp_val[0] == '0') || ((temp_val[0] != 'u') && !isdigit(temp_val[0]))) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "max-elements");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }

    if (strcmp(temp_val, "unbounded")) {
        errno = 0;
        num = strtoul(temp_val, &ptr, LY_BASE_DEC);
        if (*ptr != '\0') {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "max-elements");
            lydict_remove(ctx->xmlctx->ctx, temp_val);
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB_YIN, temp_val, "value", "max-elements");
            lydict_remove(ctx->xmlctx->ctx, temp_val);
            return LY_EVALID;
        }
        *max = num;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);
    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_MAX_ELEMENTS, NULL, exts);
}

/**
 * @brief Parse min-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] min Value to write to.
 * @param[in] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_minelements(struct lys_yin_parser_ctx *ctx, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    char *ptr;
    unsigned long int num;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    *flags |= LYS_SET_MIN;
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_MIN_ELEMENTS));

    if (!temp_val || (temp_val[0] == '\0') || ((temp_val[0] == '0') && (temp_val[1] != '\0'))) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "min-elements");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(temp_val, &ptr, LY_BASE_DEC);
    if (ptr[0] != 0) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN, temp_val, "value", "min-elements");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_OOB_YIN, temp_val, "value", "min-elements");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    *min = num;
    lydict_remove(ctx->xmlctx->ctx, temp_val);
    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_MIN_ELEMENTS, NULL, exts);
}

/**
 * @brief Parse min-elements or max-elements element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] parent Identification of parent element.
 * @param[in] current Identification of current element.
 * @param[in] dest Where the parsed value and flags should be stored.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_minmax(struct lys_yin_parser_ctx *ctx, enum ly_stmt parent, enum ly_stmt current, void *dest)
{
    assert(current == LY_STMT_MAX_ELEMENTS || current == LY_STMT_MIN_ELEMENTS);
    assert(parent == LY_STMT_LEAF_LIST || parent == LY_STMT_REFINE || parent == LY_STMT_LIST || parent == LY_STMT_DEVIATE);
    uint32_t *lim;
    uint16_t *flags;
    struct lysp_ext_instance **exts;

    if (parent == LY_STMT_LEAF_LIST) {
        lim = (current == LY_STMT_MAX_ELEMENTS) ? &((struct lysp_node_leaflist *)dest)->max : &((struct lysp_node_leaflist *)dest)->min;
        flags = &((struct lysp_node_leaflist *)dest)->flags;
        exts = &((struct lysp_node_leaflist *)dest)->exts;
    } else if (parent == LY_STMT_REFINE) {
        lim = (current == LY_STMT_MAX_ELEMENTS) ? &((struct lysp_refine *)dest)->max : &((struct lysp_refine *)dest)->min;
        flags = &((struct lysp_refine *)dest)->flags;
        exts = &((struct lysp_refine *)dest)->exts;
    } else if (parent == LY_STMT_LIST) {
        lim = (current == LY_STMT_MAX_ELEMENTS) ? &((struct lysp_node_list *)dest)->max : &((struct lysp_node_list *)dest)->min;
        flags = &((struct lysp_node_list *)dest)->flags;
        exts = &((struct lysp_node_list *)dest)->exts;
    } else {
        lim = ((struct minmax_dev_meta *)dest)->lim;
        flags = ((struct minmax_dev_meta *)dest)->flags;
        exts = ((struct minmax_dev_meta *)dest)->exts;
    }

    if (current == LY_STMT_MAX_ELEMENTS) {
        LY_CHECK_RET(yin_parse_maxelements(ctx, lim, flags, exts));
    } else {
        LY_CHECK_RET(yin_parse_minelements(ctx, lim, flags, exts));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse ordered-by element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_orderedby(struct lys_yin_parser_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_ORDERED_BY));
    if (strcmp(temp_val, "system") == 0) {
        *flags |= LYS_ORDBY_SYSTEM;
    } else if (strcmp(temp_val, "user") == 0) {
        *flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_val, "value",
                "ordered-by", "system", "user");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_ORDERED_BY, NULL, exts);
}

/**
 * @brief Parse any-data or any-xml element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] any_kw Identification of current element, can be set to LY_STMT_ANYDATA or LY_STMT_ANYXML
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_any(struct lys_yin_parser_ctx *ctx, enum ly_stmt any_kw, struct tree_node_meta *node_meta)
{
    struct lysp_node_anydata *any;

    /* create new sibling */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, any, next, LY_EMEM);
    any->nodetype = (any_kw == LY_STMT_ANYDATA) ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &any->name, Y_IDENTIF_ARG, any_kw));

    struct yin_subelement subelems[] = {
        {LY_STMT_CONFIG, &any->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DESCRIPTION, &any->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &any->iffeatures, 0},
        {LY_STMT_MANDATORY, &any->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MUST, &any->musts, 0},
        {LY_STMT_REFERENCE, &any->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &any->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_WHEN, &any->when, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), any_kw, NULL, &any->exts);
}

/**
 * @brief parse leaf element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_leaf(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_leaf *leaf;

    /* create structure new leaf */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, leaf, next, LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = node_meta->parent;

    /* parser argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &leaf->name, Y_IDENTIF_ARG, LY_STMT_LEAF));

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_CONFIG, &leaf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DEFAULT, &leaf->dflt, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DESCRIPTION, &leaf->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &leaf->iffeatures, 0},
        {LY_STMT_MANDATORY, &leaf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MUST, &leaf->musts, 0},
        {LY_STMT_REFERENCE, &leaf->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &leaf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_TYPE, &leaf->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
        {LY_STMT_UNITS, &leaf->units, YIN_SUBELEM_UNIQUE},
        {LY_STMT_WHEN, &leaf->when, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_LEAF, NULL, &leaf->exts);
}

/**
 * @brief Parse leaf-list element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_leaflist(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_leaflist *llist;

    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, llist, next, LY_EMEM);

    llist->nodetype = LYS_LEAFLIST;
    llist->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &llist->name, Y_IDENTIF_ARG, LY_STMT_LEAF_LIST));

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_CONFIG, &llist->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DEFAULT, &llist->dflts, 0},
        {LY_STMT_DESCRIPTION, &llist->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &llist->iffeatures, 0},
        {LY_STMT_MAX_ELEMENTS, llist, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MIN_ELEMENTS, llist, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MUST, &llist->musts, 0},
        {LY_STMT_ORDERED_BY, &llist->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &llist->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &llist->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_TYPE, &llist->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
        {LY_STMT_UNITS, &llist->units, YIN_SUBELEM_UNIQUE},
        {LY_STMT_WHEN, &llist->when, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    LY_CHECK_RET(yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_LEAF_LIST, NULL, &llist->exts));

    /* check invalid combination of subelements */
    if ((llist->min) && (llist->dflts)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INCHILDSTMSCOMB_YIN, "min-elements", "default", "leaf-list");
        return LY_EVALID;
    }
    if (llist->max && (llist->min > llist->max)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_MINMAX, llist->min, llist->max);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse typedef element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] typedef_meta Meta information about parent node and typedefs to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_typedef(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *typedef_meta)
{
    struct lysp_tpdf *tpdf;
    struct lysp_tpdf **tpdfs = (struct lysp_tpdf **)typedef_meta->nodes;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *tpdfs, tpdf, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &tpdf->name, Y_IDENTIF_ARG, LY_STMT_TYPEDEF));

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_DEFAULT, &tpdf->dflt, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DESCRIPTION, &tpdf->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &tpdf->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &tpdf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_TYPE, &tpdf->type, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_MANDATORY},
        {LY_STMT_UNITS, &tpdf->units, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    LY_CHECK_RET(yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_TYPEDEF, NULL, &tpdf->exts));

    /* store data for collision check */
    if (typedef_meta->parent && !(typedef_meta->parent->nodetype & (LYS_GROUPING | LYS_RPC | LYS_ACTION | LYS_INPUT |
            LYS_OUTPUT | LYS_NOTIF))) {
        LY_CHECK_RET(ly_set_add(&ctx->tpdfs_nodes, typedef_meta->parent, 0, NULL));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse refine element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] refines Refines to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_refine(struct lys_yin_parser_ctx *ctx, struct lysp_refine **refines)
{
    struct lysp_refine *rf;

    /* allocate new refine */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *refines, rf, LY_EMEM);

    /* parse attribute */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_TARGET_NODE, &rf->nodeid, Y_STR_ARG, LY_STMT_REFINE));
    CHECK_NONEMPTY(ctx, strlen(rf->nodeid), "refine");

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_CONFIG, &rf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DEFAULT, &rf->dflts, 0},
        {LY_STMT_DESCRIPTION, &rf->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &rf->iffeatures, 0},
        {LY_STMT_MANDATORY, &rf->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MAX_ELEMENTS, rf, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MIN_ELEMENTS, rf, YIN_SUBELEM_UNIQUE},
        {LY_STMT_MUST, &rf->musts, 0},
        {LY_STMT_PRESENCE, &rf->presence, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &rf->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_REFINE, NULL, &rf->exts);
}

/**
 * @brief Parse uses element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_uses(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_uses *uses;

    /* create new uses */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, uses, next, LY_EMEM);
    uses->nodetype = LYS_USES;
    uses->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &uses->name, Y_PREF_IDENTIF_ARG, LY_STMT_USES));

    /* parse content */
    struct tree_node_meta augments = {(struct lysp_node *)uses, (struct lysp_node **)&uses->augments};
    struct yin_subelement subelems[] = {
        {LY_STMT_AUGMENT, &augments, 0},
        {LY_STMT_DESCRIPTION, &uses->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &uses->iffeatures, 0},
        {LY_STMT_REFERENCE, &uses->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFINE, &uses->refines, 0},
        {LY_STMT_STATUS, &uses->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_WHEN, &uses->when, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    LY_CHECK_RET(yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_USES, NULL, &uses->exts));

    return LY_SUCCESS;
}

/**
 * @brief Parse revision element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] revs Parsed revisions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_revision(struct lys_yin_parser_ctx *ctx, struct lysp_revision **revs)
{
    struct lysp_revision *rev;
    const char *temp_date = NULL;

    /* allocate new reivison */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *revs, rev, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_DATE, &temp_date, Y_STR_ARG, LY_STMT_REVISION));
    /* check value */
    if (lysp_check_date((struct lys_parser_ctx *)ctx, temp_date, strlen(temp_date), "revision")) {
        lydict_remove(ctx->xmlctx->ctx, temp_date);
        return LY_EVALID;
    }
    memcpy(rev->date, temp_date, LY_REV_SIZE);
    lydict_remove(ctx->xmlctx->ctx, temp_date);

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &rev->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &rev->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_REVISION, NULL, &rev->exts);
}

/**
 * @brief Parse include element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] inc_meta Meta informatinou about module/submodule name and includes to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_include(struct lys_yin_parser_ctx *ctx, struct include_meta *inc_meta)
{
    struct lysp_include *inc;

    /* allocate new include */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *inc_meta->includes, inc, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_MODULE, &inc->name, Y_IDENTIF_ARG, LY_STMT_INCLUDE));

    /* submodules share the namespace with the module names, so there must not be
     * a module of the same name in the context, no need for revision matching */
    if (!strcmp(inc_meta->name, inc->name) || ly_ctx_get_module_latest(ctx->xmlctx->ctx, inc->name)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_NAME2_COL, "module", "submodule", inc->name);
        return LY_EVALID;
    }

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &inc->dsc, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_VER2},
        {LY_STMT_REFERENCE, &inc->ref, YIN_SUBELEM_UNIQUE | YIN_SUBELEM_VER2},
        {LY_STMT_REVISION_DATE, &inc->rev, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_INCLUDE, NULL, &inc->exts);
}

/**
 * @brief Parse revision-date element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] rev Array to store the parsed value in.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_revision_date(struct lys_yin_parser_ctx *ctx, char *rev, struct lysp_ext_instance **exts)
{
    const char *temp_rev;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_DATE, &temp_rev, Y_STR_ARG, LY_STMT_REVISION_DATE));
    LY_CHECK_ERR_RET(lysp_check_date((struct lys_parser_ctx *)ctx, temp_rev, strlen(temp_rev), "revision-date") != LY_SUCCESS,
            lydict_remove(ctx->xmlctx->ctx, temp_rev), LY_EVALID);

    strcpy(rev, temp_rev);
    lydict_remove(ctx->xmlctx->ctx, temp_rev);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_REVISION_DATE, NULL, exts);
}

/**
 * @brief Parse config element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_config(struct lys_yin_parser_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_CONFIG));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_CONFIG_W;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_val, "value", "config",
                "true", "false");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_CONFIG, NULL, exts);
}

/**
 * @brief Parse yang-version element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] version Storage for the parsed information.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_yangversion(struct lys_yin_parser_ctx *ctx, uint8_t *version, struct lysp_ext_instance **exts)
{
    const char *temp_version = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_version, Y_STR_ARG, LY_STMT_YANG_VERSION));
    if (strcmp(temp_version, "1") == 0) {
        *version = LYS_VERSION_1_0;
    } else if (strcmp(temp_version, "1.1") == 0) {
        *version = LYS_VERSION_1_1;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_version, "value",
                "yang-version", "1", "1.1");
        lydict_remove(ctx->xmlctx->ctx, temp_version);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_version);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_YANG_VERSION, NULL, exts);
}

/**
 * @brief Parse import element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] imp_meta Meta information about prefix and imports to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_import(struct lys_yin_parser_ctx *ctx, struct import_meta *imp_meta)
{
    struct lysp_import *imp;

    /* allocate new element in sized array for import */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *imp_meta->imports, imp, LY_EMEM);

    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &imp->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_PREFIX, &imp->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &imp->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REVISION_DATE, imp->rev, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    /* parse import attributes */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_MODULE, &imp->name, Y_IDENTIF_ARG, LY_STMT_IMPORT));
    LY_CHECK_RET(yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_IMPORT, NULL, &imp->exts));
    /* check prefix validity */
    LY_CHECK_RET(lysp_check_prefix((struct lys_parser_ctx *)ctx, *imp_meta->imports, imp_meta->prefix, &imp->prefix), LY_EVALID);

    return LY_SUCCESS;
}

/**
 * @brief Parse mandatory element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_mandatory(struct lys_yin_parser_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_MANDATORY));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_MAND_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_val, "value",
                "mandatory", "true", "false");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_MANDATORY, NULL, exts);
}

/**
 * @brief Parse status element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_status(struct lys_yin_parser_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *value = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &value, Y_STR_ARG, LY_STMT_STATUS));
    if (strcmp(value, "current") == 0) {
        *flags |= LYS_STATUS_CURR;
    } else if (strcmp(value, "deprecated") == 0) {
        *flags |= LYS_STATUS_DEPRC;
    } else if (strcmp(value, "obsolete") == 0) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS3, value, "value",
                "status", "current", "deprecated", "obsolete");
        lydict_remove(ctx->xmlctx->ctx, value);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, value);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_STATUS, NULL, exts);
}

/**
 * @brief Parse when element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[out] when_p When pointer to parse to.
 */
static LY_ERR
yin_parse_when(struct lys_yin_parser_ctx *ctx, struct lysp_when **when_p)
{
    struct lysp_when *when;
    LY_ERR ret = LY_SUCCESS;

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);

    ret = lyxml_ctx_next(ctx->xmlctx);
    LY_CHECK_ERR_RET(ret, free(when), ret);

    ret = yin_parse_attribute(ctx, YIN_ARG_CONDITION, &when->cond, Y_STR_ARG, LY_STMT_WHEN);
    LY_CHECK_ERR_RET(ret, free(when), ret);

    *when_p = when;
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &when->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &when->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_WHEN, NULL, &when->exts);
}

/**
 * @brief Parse yin-elemenet element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] data Data to read from, always moved to currently handled position.
 * @param[in,out] flags Flags to add to.
 * @prama[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_yin_element(struct lys_yin_parser_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    const char *temp_val = NULL;
    struct yin_subelement subelems[] = {
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_YIN_ELEMENT));
    if (strcmp(temp_val, "true") == 0) {
        *flags |= LYS_YINELEM_TRUE;
    } else if (strcmp(temp_val, "false") == 0) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS2, temp_val, "value",
                "yin-element", "true", "false");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_YIN_ELEMENT, NULL, exts);
}

/**
 * @brief Parse argument element.
 *
 * @param[in,out] xmlctx Xml context.
 * @param[in,out] arg_meta Meta information about destionation of parsed data.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_argument(struct lys_yin_parser_ctx *ctx, struct yin_argument_meta *arg_meta, struct lysp_ext_instance **exts)
{
    struct yin_subelement subelems[] = {
        {LY_STMT_YIN_ELEMENT, arg_meta->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, arg_meta->argument, Y_IDENTIF_ARG, LY_STMT_ARGUMENT));

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_ARGUMENT, NULL, exts);
}

/**
 * @brief Parse the extension statement.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] extensions Extensions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_extension(struct lys_yin_parser_ctx *ctx, struct lysp_ext **extensions)
{
    struct lysp_ext *ex;

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *extensions, ex, LY_EMEM);
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &ex->name, Y_IDENTIF_ARG, LY_STMT_EXTENSION));

    struct yin_argument_meta arg_info = {&ex->flags, &ex->argname};
    struct yin_subelement subelems[] = {
        {LY_STMT_ARGUMENT, &arg_info, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DESCRIPTION, &ex->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_REFERENCE, &ex->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &ex->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_EXTENSION, NULL, &ex->exts);
}

/**
 * @brief Parse feature element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] features Features to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_feature(struct lys_yin_parser_ctx *ctx, struct lysp_feature **features)
{
    struct lysp_feature *feat;

    /* allocate new feature */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *features, feat, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &feat->name, Y_IDENTIF_ARG, LY_STMT_FEATURE));

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &feat->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &feat->iffeatures, 0},
        {LY_STMT_REFERENCE, &feat->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &feat->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_FEATURE, NULL, &feat->exts);
}

/**
 * @brief Parse identity element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] identities Identities to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_identity(struct lys_yin_parser_ctx *ctx, struct lysp_ident **identities)
{
    struct lysp_ident *ident;

    /* allocate new identity */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *identities, ident, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &ident->name, Y_IDENTIF_ARG, LY_STMT_IDENTITY));

    /* parse content */
    struct yin_subelement subelems[] = {
        {LY_STMT_BASE, &ident->bases, 0},
        {LY_STMT_DESCRIPTION, &ident->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_IF_FEATURE, &ident->iffeatures, YIN_SUBELEM_VER2},
        {LY_STMT_REFERENCE, &ident->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_STATUS, &ident->flags, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_IDENTITY, NULL, &ident->exts);
}

/**
 * @brief Parse list element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_list(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_list *list;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, list, next, LY_EMEM);
    list->nodetype = LYS_LIST;
    list->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &list->name, Y_IDENTIF_ARG, LY_STMT_LIST));

    /* parse list content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 25, (struct lysp_node *)list, &subelems,
            LY_STMT_ACTION, &list->actions, 0,
            LY_STMT_ANYDATA, &list->child, 0,
            LY_STMT_ANYXML, &list->child, 0,
            LY_STMT_CHOICE, &list->child, 0,
            LY_STMT_CONFIG, &list->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_CONTAINER, &list->child, 0,
            LY_STMT_DESCRIPTION, &list->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_GROUPING, &list->groupings, 0,
            LY_STMT_IF_FEATURE, &list->iffeatures, 0,
            LY_STMT_KEY, &list->key, YIN_SUBELEM_UNIQUE,
            LY_STMT_LEAF, &list->child, 0,
            LY_STMT_LEAF_LIST, &list->child, 0,
            LY_STMT_LIST, &list->child, 0,
            LY_STMT_MAX_ELEMENTS, list, YIN_SUBELEM_UNIQUE,
            LY_STMT_MIN_ELEMENTS, list, YIN_SUBELEM_UNIQUE,
            LY_STMT_MUST, &list->musts, 0,
            LY_STMT_NOTIFICATION, &list->notifs, 0,
            LY_STMT_ORDERED_BY, &list->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &list->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &list->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_TYPEDEF, &list->typedefs, 0,
            LY_STMT_UNIQUE, &list->uniques, 0,
            LY_STMT_USES, &list->child, 0,
            LY_STMT_WHEN, &list->when, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_LIST, NULL, &list->exts);
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    if (list->max && (list->min > list->max)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_MINMAX, list->min, list->max);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse notification element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] notif_meta Meta information about parent node and notifications to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_notification(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *notif_meta)
{
    struct lysp_node_notif *notif;
    struct lysp_node_notif **notifs = (struct lysp_node_notif **)notif_meta->nodes;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* allocate new notification */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, notifs, notif, next, LY_EMEM);
    notif->nodetype = LYS_NOTIF;
    notif->parent = notif_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &notif->name, Y_IDENTIF_ARG, LY_STMT_NOTIFICATION));

    /* parse notification content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 16, (struct lysp_node *)notif, &subelems,
            LY_STMT_ANYDATA, &notif->child, 0,
            LY_STMT_ANYXML, &notif->child, 0,
            LY_STMT_CHOICE, &notif->child, 0,
            LY_STMT_CONTAINER, &notif->child, 0,
            LY_STMT_DESCRIPTION, &notif->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_GROUPING, &notif->groupings, 0,
            LY_STMT_IF_FEATURE, &notif->iffeatures, 0,
            LY_STMT_LEAF, &notif->child, 0,
            LY_STMT_LEAF_LIST, &notif->child, 0,
            LY_STMT_LIST, &notif->child, 0,
            LY_STMT_MUST, &notif->musts, YIN_SUBELEM_VER2,
            LY_STMT_REFERENCE, &notif->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &notif->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_TYPEDEF, &notif->typedefs, 0,
            LY_STMT_USES, &notif->child, 0,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));

    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_NOTIFICATION, NULL, &notif->exts);
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    return LY_SUCCESS;
}

/**
 * @brief Parse grouping element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in,out] gr_meta Meta information about parent node and groupings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_grouping(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *gr_meta)
{
    struct lysp_node_grp *grp;
    struct lysp_node_grp **grps = (struct lysp_node_grp **)gr_meta->nodes;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* create new grouping */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, grps, grp, next, LY_EMEM);
    grp->nodetype = LYS_GROUPING;
    grp->parent = gr_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &grp->name, Y_IDENTIF_ARG, LY_STMT_GROUPING));

    /* parse grouping content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 16, &grp->node, &subelems,
            LY_STMT_ACTION, &grp->actions, 0,
            LY_STMT_ANYDATA, &grp->child, 0,
            LY_STMT_ANYXML, &grp->child, 0,
            LY_STMT_CHOICE, &grp->child, 0,
            LY_STMT_CONTAINER, &grp->child, 0,
            LY_STMT_DESCRIPTION, &grp->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_GROUPING, &grp->groupings, 0,
            LY_STMT_LEAF, &grp->child, 0,
            LY_STMT_LEAF_LIST, &grp->child, 0,
            LY_STMT_LIST, &grp->child, 0,
            LY_STMT_NOTIFICATION, &grp->notifs, 0,
            LY_STMT_REFERENCE, &grp->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &grp->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_TYPEDEF, &grp->typedefs, 0,
            LY_STMT_USES, &grp->child, 0,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_GROUPING, NULL, &grp->exts);
    subelems_deallocator(subelems_size, subelems);

    return ret;
}

/**
 * @brief Parse container element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_container(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_container *cont;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* create new container */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, cont, next, LY_EMEM);
    cont->nodetype = LYS_CONTAINER;
    cont->parent = node_meta->parent;

    /* parse aegument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME,  &cont->name, Y_IDENTIF_ARG, LY_STMT_CONTAINER));

    /* parse container content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 21, (struct lysp_node *)cont, &subelems,
            LY_STMT_ACTION, &cont->actions, YIN_SUBELEM_VER2,
            LY_STMT_ANYDATA, &cont->child, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &cont->child, 0,
            LY_STMT_CHOICE, &cont->child, 0,
            LY_STMT_CONFIG, &cont->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_CONTAINER, &cont->child, 0,
            LY_STMT_DESCRIPTION, &cont->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_GROUPING, &cont->groupings, 0,
            LY_STMT_IF_FEATURE, &cont->iffeatures, 0,
            LY_STMT_LEAF, &cont->child, 0,
            LY_STMT_LEAF_LIST, &cont->child, 0,
            LY_STMT_LIST, &cont->child, 0,
            LY_STMT_MUST, &cont->musts, 0,
            LY_STMT_NOTIFICATION, &cont->notifs, YIN_SUBELEM_VER2,
            LY_STMT_PRESENCE, &cont->presence, YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &cont->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &cont->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_TYPEDEF, &cont->typedefs, 0,
            LY_STMT_USES, &cont->child, 0,
            LY_STMT_WHEN, &cont->when, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_CONTAINER, NULL, &cont->exts);
    subelems_deallocator(subelems_size, subelems);

    return ret;
}

/**
 * @brief Parse case element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_case(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    struct lysp_node_case *cas;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* create new case */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, cas, next, LY_EMEM);
    cas->nodetype = LYS_CASE;
    cas->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &cas->name, Y_IDENTIF_ARG, LY_STMT_CASE));

    /* parse case content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 14, (struct lysp_node *)cas, &subelems,
            LY_STMT_ANYDATA, &cas->child, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &cas->child, 0,
            LY_STMT_CHOICE, &cas->child, 0,
            LY_STMT_CONTAINER, &cas->child, 0,
            LY_STMT_DESCRIPTION, &cas->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_IF_FEATURE, &cas->iffeatures, 0,
            LY_STMT_LEAF, &cas->child, 0,
            LY_STMT_LEAF_LIST, &cas->child, 0,
            LY_STMT_LIST, &cas->child, 0,
            LY_STMT_REFERENCE, &cas->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &cas->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_USES, &cas->child, 0,
            LY_STMT_WHEN, &cas->when, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_CASE, NULL, &cas->exts);
    subelems_deallocator(subelems_size, subelems);

    return ret;
}

/**
 * @brief Parse choice element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] node_meta Meta information about parent node and siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_choice(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *node_meta)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    struct lysp_node_choice *choice;
    size_t subelems_size;

    /* create new choice */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, node_meta->nodes, choice, next, LY_EMEM);

    choice->nodetype = LYS_CHOICE;
    choice->parent = node_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &choice->name, Y_IDENTIF_ARG, LY_STMT_CHOICE));

    /* parse choice content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 17, (struct lysp_node *)choice, &subelems,
            LY_STMT_ANYDATA, &choice->child, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &choice->child, 0,
            LY_STMT_CASE, &choice->child, 0,
            LY_STMT_CHOICE, &choice->child, YIN_SUBELEM_VER2,
            LY_STMT_CONFIG, &choice->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_CONTAINER, &choice->child, 0,
            LY_STMT_DEFAULT, &choice->dflt, YIN_SUBELEM_UNIQUE,
            LY_STMT_DESCRIPTION, &choice->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_IF_FEATURE, &choice->iffeatures, 0,
            LY_STMT_LEAF, &choice->child, 0,
            LY_STMT_LEAF_LIST, &choice->child, 0,
            LY_STMT_LIST, &choice->child, 0,
            LY_STMT_MANDATORY, &choice->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &choice->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &choice->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_WHEN, &choice->when, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_CHOICE, NULL, &choice->exts);
    subelems_deallocator(subelems_size, subelems);
    return ret;
}

/**
 * @brief Parse input or output element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] inout_kw Identification of input/output element.
 * @param[in] inout_meta Meta information about parent node and siblings and input/output pointer to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_inout(struct lys_yin_parser_ctx *ctx, enum ly_stmt inout_kw, struct inout_meta *inout_meta)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* initiate structure */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), (inout_kw == LY_STMT_INPUT) ? "input" : "output", 0, &inout_meta->inout_p->name));
    inout_meta->inout_p->nodetype = (inout_kw == LY_STMT_INPUT) ? LYS_INPUT : LYS_OUTPUT;
    inout_meta->inout_p->parent = inout_meta->parent;

    /* check attributes */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NONE, NULL, Y_MAYBE_STR_ARG, inout_kw));

    /* parser input/output content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 12, (struct lysp_node *)inout_meta->inout_p, &subelems,
            LY_STMT_ANYDATA, &inout_meta->inout_p->child, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &inout_meta->inout_p->child, 0,
            LY_STMT_CHOICE, &inout_meta->inout_p->child, 0,
            LY_STMT_CONTAINER, &inout_meta->inout_p->child, 0,
            LY_STMT_GROUPING, &inout_meta->inout_p->groupings, 0,
            LY_STMT_LEAF, &inout_meta->inout_p->child, 0,
            LY_STMT_LEAF_LIST, &inout_meta->inout_p->child, 0,
            LY_STMT_LIST, &inout_meta->inout_p->child, 0,
            LY_STMT_MUST, &inout_meta->inout_p->musts, YIN_SUBELEM_VER2,
            LY_STMT_TYPEDEF, &inout_meta->inout_p->typedefs, 0,
            LY_STMT_USES, &inout_meta->inout_p->child, 0,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, inout_kw, NULL, &inout_meta->inout_p->exts);
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    if (!inout_meta->inout_p->child) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_MISSTMT, "data-def-stmt", ly_stmt2str(inout_kw));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse action element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] act_meta Meta information about parent node and actions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_action(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *act_meta)
{
    struct lysp_node_action *act, **acts = (struct lysp_node_action **)act_meta->nodes;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;
    enum ly_stmt kw = act_meta->parent ? LY_STMT_ACTION : LY_STMT_RPC;

    /* create new action */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, acts, act, next, LY_EMEM);
    act->nodetype = act_meta->parent ? LYS_ACTION : LYS_RPC;
    act->parent = act_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &act->name, Y_IDENTIF_ARG, kw));

    /* parse content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 9, (struct lysp_node *)act, &subelems,
            LY_STMT_DESCRIPTION, &act->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_GROUPING, &act->groupings, 0,
            LY_STMT_IF_FEATURE, &act->iffeatures, 0,
            LY_STMT_INPUT, &act->input, YIN_SUBELEM_UNIQUE,
            LY_STMT_OUTPUT, &act->output, YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &act->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &act->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_TYPEDEF, &act->typedefs, 0,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = (yin_parse_content(ctx, subelems, subelems_size, kw, NULL, &act->exts));
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    /* always initialize inout, they are technically present (needed for later deviations/refines) */
    if (!act->input.nodetype) {
        act->input.nodetype = LYS_INPUT;
        act->input.parent = (struct lysp_node *)act;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "input", 0, &act->input.name));
    }
    if (!act->output.nodetype) {
        act->output.nodetype = LYS_OUTPUT;
        act->output.parent = (struct lysp_node *)act;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "output", 0, &act->output.name));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse augment element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] aug_meta Meta information about parent node and augments to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_augment(struct lys_yin_parser_ctx *ctx, struct tree_node_meta *aug_meta)
{
    struct lysp_node_augment *aug;
    struct lysp_node_augment **augs = (struct lysp_node_augment **)aug_meta->nodes;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    size_t subelems_size;

    /* create new augment */
    LY_LIST_NEW_RET(ctx->xmlctx->ctx, augs, aug, next, LY_EMEM);
    aug->nodetype = LYS_AUGMENT;
    aug->parent = aug_meta->parent;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_TARGET_NODE, &aug->nodeid, Y_STR_ARG, LY_STMT_AUGMENT));
    CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(aug->nodeid), "augment");

    /* parser augment content */
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 17, (struct lysp_node *)aug, &subelems,
            LY_STMT_ACTION, &aug->actions, YIN_SUBELEM_VER2,
            LY_STMT_ANYDATA, &aug->child, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &aug->child, 0,
            LY_STMT_CASE, &aug->child, 0,
            LY_STMT_CHOICE, &aug->child, 0,
            LY_STMT_CONTAINER, &aug->child, 0,
            LY_STMT_DESCRIPTION, &aug->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_IF_FEATURE, &aug->iffeatures, 0,
            LY_STMT_LEAF, &aug->child, 0,
            LY_STMT_LEAF_LIST, &aug->child, 0,
            LY_STMT_LIST, &aug->child, 0,
            LY_STMT_NOTIFICATION, &aug->notifs, YIN_SUBELEM_VER2,
            LY_STMT_REFERENCE, &aug->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_STATUS, &aug->flags, YIN_SUBELEM_UNIQUE,
            LY_STMT_USES, &aug->child, 0,
            LY_STMT_WHEN, &aug->when, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));
    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_AUGMENT, NULL, &aug->exts);
    subelems_deallocator(subelems_size, subelems);

    return ret;
}

/**
 * @brief Parse deviate element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] deviates Deviates to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_deviate(struct lys_yin_parser_ctx *ctx, struct lysp_deviate **deviates)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t dev_mod;
    const char *temp_val;
    struct lysp_deviate *d;
    struct lysp_deviate_add *d_add = NULL;
    struct lysp_deviate_rpl *d_rpl = NULL;
    struct lysp_deviate_del *d_del = NULL;

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, &temp_val, Y_STR_ARG, LY_STMT_DEVIATE));

    if (strcmp(temp_val, "not-supported") == 0) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if (strcmp(temp_val, "add") == 0) {
        dev_mod = LYS_DEV_ADD;
    } else if (strcmp(temp_val, "replace") == 0) {
        dev_mod = LYS_DEV_REPLACE;
    } else if (strcmp(temp_val, "delete") == 0) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INVAL_YIN VALID_VALS4, temp_val, "value", "deviate",
                "not-supported", "add", "replace", "delete");
        lydict_remove(ctx->xmlctx->ctx, temp_val);
        return LY_EVALID;
    }
    lydict_remove(ctx->xmlctx->ctx, temp_val);

    if (dev_mod == LYS_DEV_NOT_SUPPORTED) {
        d = calloc(1, sizeof *d);
        LY_CHECK_ERR_RET(!d, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
        struct yin_subelement subelems[] = {
            {LY_STMT_EXTENSION_INSTANCE, NULL, 0}
        };
        ret = yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_DEVIATE, NULL, &d->exts);

    } else if (dev_mod == LYS_DEV_ADD) {
        d_add = calloc(1, sizeof *d_add);
        LY_CHECK_ERR_RET(!d_add, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_add;
        struct minmax_dev_meta min = {&d_add->min, &d_add->flags, &d_add->exts};
        struct minmax_dev_meta max = {&d_add->max, &d_add->flags, &d_add->exts};
        struct yin_subelement subelems[] = {
            {LY_STMT_CONFIG, &d_add->flags, YIN_SUBELEM_UNIQUE},
            {LY_STMT_DEFAULT, &d_add->dflts, 0},
            {LY_STMT_MANDATORY, &d_add->flags, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MAX_ELEMENTS, &max, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MIN_ELEMENTS, &min, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MUST, &d_add->musts, 0},
            {LY_STMT_UNIQUE, &d_add->uniques, 0},
            {LY_STMT_UNITS, &d_add->units, YIN_SUBELEM_UNIQUE},
            {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        };
        ret = yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_DEVIATE, NULL, &d_add->exts);

    } else if (dev_mod == LYS_DEV_REPLACE) {
        d_rpl = calloc(1, sizeof *d_rpl);
        LY_CHECK_ERR_RET(!d_rpl, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_rpl;
        struct minmax_dev_meta min = {&d_rpl->min, &d_rpl->flags, &d_rpl->exts};
        struct minmax_dev_meta max = {&d_rpl->max, &d_rpl->flags, &d_rpl->exts};
        struct yin_subelement subelems[] = {
            {LY_STMT_CONFIG, &d_rpl->flags, YIN_SUBELEM_UNIQUE},
            {LY_STMT_DEFAULT, &d_rpl->dflt, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MANDATORY, &d_rpl->flags, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MAX_ELEMENTS, &max, YIN_SUBELEM_UNIQUE},
            {LY_STMT_MIN_ELEMENTS, &min, YIN_SUBELEM_UNIQUE},
            {LY_STMT_TYPE, &d_rpl->type, YIN_SUBELEM_UNIQUE},
            {LY_STMT_UNITS, &d_rpl->units, YIN_SUBELEM_UNIQUE},
            {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        };
        ret = yin_parse_content(ctx, subelems, ly_sizeofarray(subelems),  LY_STMT_DEVIATE, NULL, &d_rpl->exts);

    } else {
        d_del = calloc(1, sizeof *d_del);
        LY_CHECK_ERR_RET(!d_del, LOGMEM(ctx->xmlctx->ctx), LY_EMEM);
        d = (struct lysp_deviate *)d_del;
        struct yin_subelement subelems[] = {
            {LY_STMT_DEFAULT, &d_del->dflts, 0},
            {LY_STMT_MUST, &d_del->musts, 0},
            {LY_STMT_UNIQUE, &d_del->uniques, 0},
            {LY_STMT_UNITS, &d_del->units, YIN_SUBELEM_UNIQUE},
            {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        };
        ret = yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_DEVIATE, NULL, &d_del->exts);
    }
    LY_CHECK_GOTO(ret, cleanup);

    d->mod = dev_mod;
    /* insert into siblings */
    LY_LIST_INSERT(deviates, d, next);

    return ret;

cleanup:
    free(d);
    return ret;
}

/**
 * @brief Parse deviation element.
 *
 * @param[in,out] ctx YIN parser context for logging and to store current state.
 * @param[in] deviations Deviations to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_deviation(struct lys_yin_parser_ctx *ctx, struct lysp_deviation **deviations)
{
    struct lysp_deviation *dev;

    /* create new deviation */
    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *deviations, dev, LY_EMEM);

    /* parse argument */
    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_TARGET_NODE, &dev->nodeid, Y_STR_ARG, LY_STMT_DEVIATION));
    CHECK_NONEMPTY((struct lys_parser_ctx *)ctx, strlen(dev->nodeid), "deviation");
    struct yin_subelement subelems[] = {
        {LY_STMT_DESCRIPTION, &dev->dsc, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DEVIATE, &dev->deviates, YIN_SUBELEM_MANDATORY},
        {LY_STMT_REFERENCE, &dev->ref, YIN_SUBELEM_UNIQUE},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
    };

    return yin_parse_content(ctx, subelems, ly_sizeofarray(subelems), LY_STMT_DEVIATION, NULL, &dev->exts);
}

/**
 * @brief map keyword to keyword-group.
 *
 * @param[in] ctx YIN parser context used for logging.
 * @param[in] kw Keyword that is child of module or submodule.
 * @param[out] group Group of keyword.
 *
 * @return LY_SUCCESS on success LY_EINT if kw can't be mapped to kw_group, should not happen if called correctly.
 */
static LY_ERR
kw2kw_group(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, enum yang_module_stmt *group)
{
    switch (kw) {
    /* module header */
    case LY_STMT_NONE:
    case LY_STMT_NAMESPACE:
    case LY_STMT_PREFIX:
    case LY_STMT_BELONGS_TO:
    case LY_STMT_YANG_VERSION:
        *group = Y_MOD_MODULE_HEADER;
        break;
    /* linkage */
    case LY_STMT_INCLUDE:
    case LY_STMT_IMPORT:
        *group = Y_MOD_LINKAGE;
        break;
    /* meta */
    case LY_STMT_ORGANIZATION:
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_REFERENCE:
        *group = Y_MOD_META;
        break;
    /* revision */
    case LY_STMT_REVISION:
        *group = Y_MOD_REVISION;
        break;
    /* body */
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
    case LY_STMT_AUGMENT:
    case LY_STMT_CHOICE:
    case LY_STMT_CONTAINER:
    case LY_STMT_DEVIATION:
    case LY_STMT_EXTENSION:
    case LY_STMT_FEATURE:
    case LY_STMT_GROUPING:
    case LY_STMT_IDENTITY:
    case LY_STMT_LEAF:
    case LY_STMT_LEAF_LIST:
    case LY_STMT_LIST:
    case LY_STMT_NOTIFICATION:
    case LY_STMT_RPC:
    case LY_STMT_TYPEDEF:
    case LY_STMT_USES:
    case LY_STMT_EXTENSION_INSTANCE:
        *group = Y_MOD_BODY;
        break;
    default:
        LOGINT(ctx->xmlctx->ctx);
        return LY_EINT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check if relative order of two keywords is valid.
 *
 * @param[in] ctx YIN parser context used for logging.
 * @param[in] kw Current keyword.
 * @param[in] next_kw Next keyword.
 * @param[in] parrent Identification of parrent element, can be se to to LY_STMT_MODULE of LY_STMT_SUBMODULE,
 *            because relative order is required only in module and submodule sub-elements, used for logging.
 *
 * @return LY_SUCCESS on success and LY_EVALID if relative order is invalid.
 */
static LY_ERR
yin_check_relative_order(struct lys_yin_parser_ctx *ctx, enum ly_stmt kw, enum ly_stmt next_kw, enum ly_stmt parrent)
{
    assert(parrent == LY_STMT_MODULE || parrent == LY_STMT_SUBMODULE);
    enum yang_module_stmt gr, next_gr;

    LY_CHECK_RET(kw2kw_group(ctx, kw, &gr));
    LY_CHECK_RET(kw2kw_group(ctx, next_kw, &next_gr));

    if (gr > next_gr) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INORDER_YIN, ly_stmt2str(parrent), ly_stmt2str(next_kw), ly_stmt2str(kw));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse argument of extension subelement that is classic yang keyword and not another instance of extension.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] elem_type Type of element that is currently being parsed.
 * @param[out] arg Value to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
yin_parse_extension_instance_arg(struct lys_yin_parser_ctx *ctx, enum ly_stmt elem_type, const char **arg)
{
    enum ly_stmt child;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));

    switch (elem_type) {
    case LY_STMT_ACTION:
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
    case LY_STMT_ARGUMENT:
    case LY_STMT_BASE:
    case LY_STMT_BIT:
    case LY_STMT_CASE:
    case LY_STMT_CHOICE:
    case LY_STMT_CONTAINER:
    case LY_STMT_ENUM:
    case LY_STMT_EXTENSION:
    case LY_STMT_FEATURE:
    case LY_STMT_GROUPING:
    case LY_STMT_IDENTITY:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_LEAF:
    case LY_STMT_LEAF_LIST:
    case LY_STMT_LIST:
    case LY_STMT_MODULE:
    case LY_STMT_NOTIFICATION:
    case LY_STMT_RPC:
    case LY_STMT_SUBMODULE:
    case LY_STMT_TYPE:
    case LY_STMT_TYPEDEF:
    case LY_STMT_UNITS:
    case LY_STMT_USES:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_AUGMENT:
    case LY_STMT_DEVIATION:
    case LY_STMT_REFINE:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_TARGET_NODE, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_CONFIG:
    case LY_STMT_DEFAULT:
    case LY_STMT_DEVIATE:
    case LY_STMT_ERROR_APP_TAG:
    case LY_STMT_FRACTION_DIGITS:
    case LY_STMT_KEY:
    case LY_STMT_LENGTH:
    case LY_STMT_MANDATORY:
    case LY_STMT_MAX_ELEMENTS:
    case LY_STMT_MIN_ELEMENTS:
    case LY_STMT_MODIFIER:
    case LY_STMT_ORDERED_BY:
    case LY_STMT_PATH:
    case LY_STMT_PATTERN:
    case LY_STMT_POSITION:
    case LY_STMT_PREFIX:
    case LY_STMT_PRESENCE:
    case LY_STMT_RANGE:
    case LY_STMT_REQUIRE_INSTANCE:
    case LY_STMT_STATUS:
    case LY_STMT_VALUE:
    case LY_STMT_YANG_VERSION:
    case LY_STMT_YIN_ELEMENT:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_VALUE, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_IMPORT:
    case LY_STMT_INCLUDE:
    case LY_STMT_BELONGS_TO:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_MODULE, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NONE, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_MUST:
    case LY_STMT_WHEN:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_CONDITION, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_NAMESPACE:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_URI, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_REVISION:
    case LY_STMT_REVISION_DATE:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_DATE, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    case LY_STMT_UNIQUE:
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_TAG, arg, Y_MAYBE_STR_ARG, elem_type));
        break;
    /* argument is mapped to yin element */
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_ORGANIZATION:
    case LY_STMT_REFERENCE:
    case LY_STMT_ERROR_MESSAGE:
        /* there shouldn't be any attribute, argument is supposed to be first subelement */
        LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NONE, arg, Y_MAYBE_STR_ARG, elem_type));

        /* no content */
        assert(ctx->xmlctx->status == LYXML_ELEM_CONTENT);
        if (ctx->xmlctx->ws_only) {
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        }
        if (((ctx->xmlctx->status == LYXML_ELEM_CONTENT) && !ctx->xmlctx->ws_only) || (ctx->xmlctx->status != LYXML_ELEMENT)) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_FIRT_SUBELEM,
                    elem_type == LY_STMT_ERROR_MESSAGE ? "value" : "text", ly_stmt2str(elem_type));
            return LY_EVALID;
        }

        /* parse child element */
        child = yin_match_keyword(ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, ctx->xmlctx->prefix, ctx->xmlctx->prefix_len, elem_type);
        if (((elem_type == LY_STMT_ERROR_MESSAGE) && (child != LY_STMT_ARG_VALUE)) ||
                ((elem_type != LY_STMT_ERROR_MESSAGE) && (child != LY_STMT_ARG_TEXT))) {
            LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_SUBELEM, ctx->xmlctx->name_len, ctx->xmlctx->name,
                    ly_stmt2str(elem_type));
            return LY_EVALID;
        }
        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));

        /* no attributes expected? TODO */
        while (ctx->xmlctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        }

        /* load and save content */
        INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, *arg);
        LY_CHECK_RET(!*arg, LY_EMEM);

        /* load closing tag of subelement */
        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));

        /* if only subelement was parsed as argument, load also closing tag TODO what? */
        /*if (ctx->xmlctx->status == LYXML_ELEMENT) {
            LY_CHECK_RET(lyxml_get_element(&ctx->xmlctx, data, &prefix, &prefix_len, &name, &name_len));
        }*/
        break;
    default:
        LOGINT(ctx->xmlctx->ctx);
        return LY_EINT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse yin element into generic structure.
 *
 * @param[in,out] ctx Yin parser context for XML context, logging, and to store current state.
 * @param[in] parent Identification of parent element.
 * @param[out] element Where the element structure should be stored.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_element_generic(struct lys_yin_parser_ctx *ctx, enum ly_stmt parent, struct lysp_stmt **element)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_stmt *last = NULL, *new = NULL;
    char *id;

    assert(ctx->xmlctx->status == LYXML_ELEMENT);

    /* allocate new structure for element */
    *element = calloc(1, sizeof(**element));
    LY_CHECK_ERR_GOTO(!(*element), LOGMEM(ctx->xmlctx->ctx); ret = LY_EMEM, cleanup);

    /* store identifier */
    if (ctx->xmlctx->prefix_len) {
        if (asprintf(&id, "%.*s:%.*s", (int)ctx->xmlctx->prefix_len, ctx->xmlctx->prefix, (int)ctx->xmlctx->name_len,
                ctx->xmlctx->name) == -1) {
            LOGMEM(ctx->xmlctx->ctx);
            ret = LY_EMEM;
            goto cleanup;
        }
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx->xmlctx->ctx, id, &(*element)->stmt), cleanup);

        /* store prefix data for the statement */
        LY_CHECK_GOTO(ret = ly_store_prefix_data(ctx->xmlctx->ctx, (*element)->stmt, strlen((*element)->stmt), LY_VALUE_XML,
                &ctx->xmlctx->ns, &(*element)->format, &(*element)->prefix_data), cleanup);
    } else {
        LY_CHECK_GOTO(ret = lydict_insert(ctx->xmlctx->ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, &(*element)->stmt), cleanup);
    }

    (*element)->kw = yin_match_keyword(ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, ctx->xmlctx->prefix,
            ctx->xmlctx->prefix_len, parent);

    last = (*element)->child;
    if ((*element)->kw == LY_STMT_NONE) {
        /* unrecognized element */
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_SUBELEM, ctx->xmlctx->name_len, ctx->xmlctx->name,
                ly_stmt2str(parent));
        ret = LY_EVALID;
        goto cleanup;
    } else if ((*element)->kw != LY_STMT_EXTENSION_INSTANCE) {
        /* element is known yang keyword, which means argument can be parsed correctly. */
        ret = yin_parse_extension_instance_arg(ctx, (*element)->kw, &(*element)->arg);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);

        /* load attributes in generic way, save all attributes in linked list */
        while (ctx->xmlctx->status == LYXML_ATTRIBUTE) {
            new = calloc(1, sizeof(*last));
            LY_CHECK_ERR_GOTO(!new, LOGMEM(ctx->xmlctx->ctx); ret = LY_EMEM, cleanup);
            if (!(*element)->child) {
                /* save first */
                (*element)->child = new;
            } else {
                last->next = new;
            }
            last = new;

            last->flags |= LYS_YIN_ATTR;
            LY_CHECK_GOTO(ret = lydict_insert(ctx->xmlctx->ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, &last->stmt), cleanup);
            last->kw = LY_STMT_NONE;
            /* attributes with prefix are ignored */
            if (!ctx->xmlctx->prefix) {
                LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);

                INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, last->arg);
                LY_CHECK_ERR_GOTO(!last->arg, ret = LY_EMEM, cleanup);
            } else {
                LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
            }
            LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
        }
    }

    if ((ctx->xmlctx->status != LYXML_ELEM_CONTENT) || ctx->xmlctx->ws_only) {
        LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
        while (ctx->xmlctx->status == LYXML_ELEMENT) {
            /* parse subelements */
            ret = yin_parse_element_generic(ctx, (*element)->kw, &new);
            LY_CHECK_GOTO(ret, cleanup);
            if (!(*element)->child) {
                /* save first */
                (*element)->child = new;
            } else {
                last->next = new;
            }
            last = new;

            assert(ctx->xmlctx->status == LYXML_ELEM_CLOSE);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
        }
    } else {
        /* save element content */
        if (ctx->xmlctx->value_len) {
            INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, (*element)->arg);
            LY_CHECK_ERR_GOTO(!(*element)->arg, ret = LY_EMEM, cleanup);

            /* store prefix data for the argument as well */
            LY_CHECK_GOTO(ret = ly_store_prefix_data(ctx->xmlctx->ctx, (*element)->arg, strlen((*element)->arg), LY_VALUE_XML,
                    &ctx->xmlctx->ns, &(*element)->format, &(*element)->prefix_data), cleanup);
        }

        /* read closing tag */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse instance of extension.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] subelem The statement this extension instance is a subelement of.
 * @param[in] subelem_index Index of the keyword instance this extension instance is a subelement of
 * @param[in,out] exts Extension instance to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_extension_instance(struct lys_yin_parser_ctx *ctx, enum ly_stmt subelem, LY_ARRAY_COUNT_TYPE subelem_index,
        struct lysp_ext_instance **exts)
{
    struct lysp_ext_instance *e;
    struct lysp_stmt *last_subelem = NULL, *new_subelem = NULL;
    char *ext_name;

    assert(ctx->xmlctx->status == LYXML_ELEMENT);
    assert(exts);

    LY_ARRAY_NEW_RET(ctx->xmlctx->ctx, *exts, e, LY_EMEM);

    if (!ctx->xmlctx->prefix_len) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX, "Extension instance \"%*.s\" without the mandatory prefix.",
                (int)ctx->xmlctx->name_len, ctx->xmlctx->name);
        return LY_EVALID;
    }

    /* store prefixed name */
    if (asprintf(&ext_name, "%.*s:%.*s", (int)ctx->xmlctx->prefix_len, ctx->xmlctx->prefix, (int)ctx->xmlctx->name_len,
            ctx->xmlctx->name) == -1) {
        LOGMEM(ctx->xmlctx->ctx);
        return LY_EMEM;
    }
    LY_CHECK_RET(lydict_insert_zc(ctx->xmlctx->ctx, ext_name, &e->name));

    /* store prefix data for the name */
    LY_CHECK_RET(ly_store_prefix_data(ctx->xmlctx->ctx, e->name, strlen(e->name), LY_VALUE_XML, &ctx->xmlctx->ns,
            &e->format, &e->prefix_data));

    e->parent_stmt = subelem;
    e->parent_stmt_index = subelem_index;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));

    /* store attributes as subelements */
    while (ctx->xmlctx->status == LYXML_ATTRIBUTE) {
        if (!ctx->xmlctx->prefix) {
            new_subelem = calloc(1, sizeof(*new_subelem));
            if (!e->child) {
                e->child = new_subelem;
            } else {
                last_subelem->next = new_subelem;
            }
            last_subelem = new_subelem;

            last_subelem->flags |= LYS_YIN_ATTR;
            LY_CHECK_RET(lydict_insert(ctx->xmlctx->ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, &last_subelem->stmt));
            LY_CHECK_RET(!last_subelem->stmt, LY_EMEM);
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));

            INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic,
                    last_subelem->arg);
            LY_CHECK_RET(!last_subelem->arg, LY_EMEM);
        } else {
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        }

        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    }

    /* parse subelements */
    assert(ctx->xmlctx->status == LYXML_ELEM_CONTENT);
    if (ctx->xmlctx->ws_only) {
        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        while (ctx->xmlctx->status == LYXML_ELEMENT) {
            LY_CHECK_RET(yin_parse_element_generic(ctx, LY_STMT_EXTENSION_INSTANCE, &new_subelem));
            if (!e->child) {
                e->child = new_subelem;
            } else {
                last_subelem->next = new_subelem;
            }
            last_subelem = new_subelem;

            assert(ctx->xmlctx->status == LYXML_ELEM_CLOSE);
            LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
        }
    } else if (ctx->xmlctx->value_len) {
        /* save text content */
        INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, e->argument);
        LY_CHECK_RET(!e->argument, LY_EMEM);

        /* store prefix data for the argument as well */
        LY_CHECK_RET(ly_store_prefix_data(ctx->xmlctx->ctx, e->argument, strlen(e->argument), LY_VALUE_XML,
                &ctx->xmlctx->ns, &e->format, &e->prefix_data));

        /* parser next */
        LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    }

    e->parsed = NULL;

    return LY_SUCCESS;
}

/**
 * @brief Generic function for content parsing
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] subelem_info array of valid subelement types and meta information
 * @param[in] subelem_info_size Size of subelem_info array.
 * @param[in] current_element Type of current element.
 * @param[out] text_content Where the text content of element should be stored if any. Text content is ignored if set to NULL.
 * @param[in,out] exts Extension instance to add to. Can be set to null if element cannot have extension as subelements.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_content(struct lys_yin_parser_ctx *ctx, struct yin_subelement *subelem_info, size_t subelem_info_size,
        enum ly_stmt current_element, const char **text_content, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYXML_PARSER_STATUS next_status;
    enum ly_stmt kw = LY_STMT_NONE, last_kw = LY_STMT_NONE;
    struct yin_subelement *subelem = NULL;

    assert(ctx->xmlctx->status == LYXML_ELEM_CONTENT);

    if (ctx->xmlctx->ws_only) {
        /* check whether there are any children */
        LY_CHECK_GOTO(ret = lyxml_ctx_peek(ctx->xmlctx, &next_status), cleanup);
    } else {
        /* we want to parse the value */
        next_status = LYXML_ELEM_CLOSE;
    }

    if (next_status == LYXML_ELEMENT) {
        LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);

        /* current element has subelements as content */
        while (ctx->xmlctx->status == LYXML_ELEMENT) {
            /* match keyword */
            last_kw = kw;
            kw = yin_match_keyword(ctx, ctx->xmlctx->name, ctx->xmlctx->name_len, ctx->xmlctx->prefix,
                    ctx->xmlctx->prefix_len, current_element);

            /* check if this element can be child of current element */
            subelem = get_record(kw, subelem_info_size, subelem_info);
            if (!subelem) {
                if ((current_element == LY_STMT_DEVIATE) && isdevsub(kw)) {
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INDEV_YIN, ly_stmt2str(kw));
                } else {
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_UNEXP_SUBELEM, ctx->xmlctx->name_len,
                            ctx->xmlctx->name, ly_stmt2str(current_element));
                }
                ret = LY_EVALID;
                goto cleanup;
            }

            /* relative order is required only in module and submodule sub-elements */
            if ((current_element == LY_STMT_MODULE) || (current_element == LY_STMT_SUBMODULE)) {
                ret = yin_check_relative_order(ctx, last_kw, kw, current_element);
                LY_CHECK_GOTO(ret, cleanup);
            }

            /* flag check */
            if ((subelem->flags & YIN_SUBELEM_UNIQUE) && (subelem->flags & YIN_SUBELEM_PARSED)) {
                /* subelement uniquenes */
                LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_SUBELEM_REDEF, ly_stmt2str(kw), ly_stmt2str(current_element));
                return LY_EVALID;
            }
            if (subelem->flags & YIN_SUBELEM_FIRST) {
                /* subelement is supposed to be defined as first subelement */
                ret = yin_check_subelem_first_constraint(ctx, subelem_info, subelem_info_size, current_element, subelem);
                LY_CHECK_GOTO(ret, cleanup);
            }
            if (subelem->flags & YIN_SUBELEM_VER2) {
                /* subelement is supported only in version 1.1 or higher */
                if (ctx->parsed_mod->version < LYS_VERSION_1_1) {
                    LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_INSUBELEM2, ly_stmt2str(kw), ly_stmt2str(current_element));
                    ret = LY_EVALID;
                    goto cleanup;
                }
            }
            /* note that element was parsed for easy uniqueness check in next iterations */
            subelem->flags |= YIN_SUBELEM_PARSED;

            switch (kw) {
            /* call responsible function */
            case LY_STMT_EXTENSION_INSTANCE:
                ret = yin_parse_extension_instance(ctx, current_element,
                        (subelem->dest) ? *((LY_ARRAY_COUNT_TYPE *)subelem->dest) : 0, exts);
                break;
            case LY_STMT_ACTION:
            case LY_STMT_RPC:
                ret = yin_parse_action(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_ANYDATA:
            case LY_STMT_ANYXML:
                ret = yin_parse_any(ctx, kw, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_ARGUMENT:
                ret = yin_parse_argument(ctx, (struct yin_argument_meta *)subelem->dest, exts);
                break;
            case LY_STMT_AUGMENT:
                ret = yin_parse_augment(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_BASE:
                ret = yin_parse_base(ctx, current_element, subelem->dest, exts);
                break;
            case LY_STMT_BELONGS_TO:
                ret = yin_parse_belongs_to(ctx, (struct lysp_submodule *)subelem->dest, exts);
                break;
            case LY_STMT_BIT:
                ret = yin_parse_bit(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_CASE:
                ret = yin_parse_case(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_CHOICE:
                ret = yin_parse_choice(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_CONFIG:
                ret = yin_parse_config(ctx, (uint16_t *)subelem->dest, exts);
                break;
            case LY_STMT_CONTACT:
            case LY_STMT_DESCRIPTION:
            case LY_STMT_ORGANIZATION:
            case LY_STMT_REFERENCE:
                ret = yin_parse_meta(ctx, kw, (const char **)subelem->dest, exts);
                break;
            case LY_STMT_CONTAINER:
                ret = yin_parse_container(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_DEFAULT:
                ret = yin_parse_qname(ctx, kw, subelem, exts);
                break;
            case LY_STMT_ERROR_APP_TAG:
            case LY_STMT_KEY:
            case LY_STMT_PRESENCE:
                ret = yin_parse_simple_elem(ctx, kw, subelem, YIN_ARG_VALUE, Y_STR_ARG, exts);
                break;
            case LY_STMT_DEVIATE:
                ret = yin_parse_deviate(ctx, (struct lysp_deviate **)subelem->dest);
                break;
            case LY_STMT_DEVIATION:
                ret = yin_parse_deviation(ctx, (struct lysp_deviation **)subelem->dest);
                break;
            case LY_STMT_ENUM:
                ret = yin_parse_enum(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_ERROR_MESSAGE:
                ret = yin_parse_err_msg(ctx, (const char **)subelem->dest, exts);
                break;
            case LY_STMT_EXTENSION:
                ret = yin_parse_extension(ctx, (struct lysp_ext **)subelem->dest);
                break;
            case LY_STMT_FEATURE:
                ret = yin_parse_feature(ctx, (struct lysp_feature **)subelem->dest);
                break;
            case LY_STMT_FRACTION_DIGITS:
                ret = yin_parse_fracdigits(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_GROUPING:
                ret = yin_parse_grouping(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_IDENTITY:
                ret = yin_parse_identity(ctx, (struct lysp_ident **)subelem->dest);
                break;
            case LY_STMT_UNIQUE:
            case LY_STMT_IF_FEATURE:
                ret = yin_parse_qname(ctx, kw, subelem, exts);
                break;
            case LY_STMT_UNITS:
                ret = yin_parse_simple_elem(ctx, kw, subelem, YIN_ARG_NAME, Y_STR_ARG, exts);
                break;
            case LY_STMT_IMPORT:
                ret = yin_parse_import(ctx, (struct import_meta *)subelem->dest);
                break;
            case LY_STMT_INCLUDE:
                if ((current_element == LY_STMT_SUBMODULE) && (ctx->parsed_mod->version == LYS_VERSION_1_1)) {
                    LOGWRN(PARSER_CTX(ctx), "YANG version 1.1 expects all includes in main module, includes in submodules (%s) are not necessary.",
                            ((struct lysp_submodule *)(ctx->parsed_mod))->name);
                }
                ret = yin_parse_include(ctx, (struct include_meta *)subelem->dest);
                break;
            case LY_STMT_INPUT:
            case LY_STMT_OUTPUT:
                ret = yin_parse_inout(ctx, kw, (struct inout_meta *)subelem->dest);
                break;
            case LY_STMT_LEAF:
                ret = yin_parse_leaf(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_LEAF_LIST:
                ret = yin_parse_leaflist(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_LENGTH:
                ret = yin_parse_length(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_LIST:
                ret = yin_parse_list(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_MANDATORY:
                ret = yin_parse_mandatory(ctx, (uint16_t *)subelem->dest, exts);
                break;
            case LY_STMT_MAX_ELEMENTS:
            case LY_STMT_MIN_ELEMENTS:
                ret = yin_parse_minmax(ctx, current_element, kw, subelem->dest);
                break;
            case LY_STMT_MODIFIER:
                ret = yin_parse_modifier(ctx, (const char **)subelem->dest, exts);
                break;
            case LY_STMT_MUST:
                ret = yin_parse_must(ctx, (struct lysp_restr **)subelem->dest);
                break;
            case LY_STMT_NAMESPACE:
                ret = yin_parse_simple_elem(ctx, kw, subelem, YIN_ARG_URI, Y_STR_ARG, exts);
                break;
            case LY_STMT_NOTIFICATION:
                ret = yin_parse_notification(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_ORDERED_BY:
                ret = yin_parse_orderedby(ctx, (uint16_t *)subelem->dest, exts);
                break;
            case LY_STMT_PATH:
                ret = yin_parse_path(ctx, kw, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_PATTERN:
                ret = yin_parse_pattern(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_VALUE:
            case LY_STMT_POSITION:
                ret = yin_parse_value_pos(ctx, kw, (struct lysp_type_enum *)subelem->dest);
                break;
            case LY_STMT_PREFIX:
                ret = yin_parse_simple_elem(ctx, kw, subelem, YIN_ARG_VALUE, Y_IDENTIF_ARG, exts);
                break;
            case LY_STMT_RANGE:
                ret = yin_parse_range(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_REFINE:
                ret = yin_parse_refine(ctx, (struct lysp_refine **)subelem->dest);
                break;
            case LY_STMT_REQUIRE_INSTANCE:
                ret = yin_pasrse_reqinstance(ctx, (struct lysp_type *)subelem->dest);
                break;
            case LY_STMT_REVISION:
                ret = yin_parse_revision(ctx, (struct lysp_revision **)subelem->dest);
                break;
            case LY_STMT_REVISION_DATE:
                ret = yin_parse_revision_date(ctx, (char *)subelem->dest, exts);
                break;
            case LY_STMT_STATUS:
                ret = yin_parse_status(ctx, (uint16_t *)subelem->dest, exts);
                break;
            case LY_STMT_TYPE:
                ret = yin_parse_type(ctx, current_element, subelem);
                break;
            case LY_STMT_TYPEDEF:
                ret = yin_parse_typedef(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_USES:
                ret = yin_parse_uses(ctx, (struct tree_node_meta *)subelem->dest);
                break;
            case LY_STMT_WHEN:
                ret = yin_parse_when(ctx, (struct lysp_when **)subelem->dest);
                break;
            case LY_STMT_YANG_VERSION:
                ret = yin_parse_yangversion(ctx, (uint8_t *)subelem->dest, exts);
                break;
            case LY_STMT_YIN_ELEMENT:
                ret = yin_parse_yin_element(ctx, (uint16_t *)subelem->dest, exts);
                break;
            case LY_STMT_ARG_TEXT:
            case LY_STMT_ARG_VALUE:
                /* TODO what to do with content/attributes? */
                LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
                while (ctx->xmlctx->status == LYXML_ATTRIBUTE) {
                    LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
                    LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
                }
                ret = yin_parse_content(ctx, NULL, 0, kw, (const char **)subelem->dest, NULL);
                break;
            default:
                LOGINT(ctx->xmlctx->ctx);
                ret = LY_EINT;
            }
            LY_CHECK_GOTO(ret, cleanup);
            subelem = NULL;

            LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
        }
    } else {
        LY_CHECK_RET(ret);
        /* elements with text or none content */
        /* save text content, if text_content isn't set, it's just ignored */
        /* no resources are allocated in this branch, no need to use cleanup label */
        LY_CHECK_RET(yin_validate_value(ctx, Y_STR_ARG));
        if (text_content) {
            INSERT_STRING_RET(ctx->xmlctx->ctx, ctx->xmlctx->value, ctx->xmlctx->value_len, ctx->xmlctx->dynamic, *text_content);
            LY_CHECK_RET(!*text_content, LY_EMEM);
        }

        LY_CHECK_GOTO(ret = lyxml_ctx_next(ctx->xmlctx), cleanup);
    }

    /* mandatory subelemnts are checked only after whole element was succesfully parsed */
    LY_CHECK_RET(yin_check_subelem_mandatory_constraint(ctx, subelem_info, subelem_info_size, current_element));

cleanup:
    return ret;
}

/**
 * @brief Parse module element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[out] mod Parsed module structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_mod(struct lys_yin_parser_ctx *ctx, struct lysp_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    const struct lysp_submodule *dup;
    size_t subelems_size;

    mod->is_submod = 0;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &mod->mod->name, Y_IDENTIF_ARG, LY_STMT_MODULE));
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 28, NULL, &subelems,
            LY_STMT_ANYDATA, &mod->data, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &mod->data, 0,
            LY_STMT_AUGMENT, &mod->augments, 0,
            LY_STMT_CHOICE, &mod->data, 0,
            LY_STMT_CONTACT, &mod->mod->contact, YIN_SUBELEM_UNIQUE,
            LY_STMT_CONTAINER, &mod->data, 0,
            LY_STMT_DESCRIPTION, &mod->mod->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_DEVIATION, &mod->deviations, 0,
            LY_STMT_EXTENSION, &mod->extensions, 0,
            LY_STMT_FEATURE, &mod->features, 0,
            LY_STMT_GROUPING, &mod->groupings, 0,
            LY_STMT_IDENTITY, &mod->identities, 0,
            LY_STMT_IMPORT, mod->mod->prefix, &mod->imports, 0,
            LY_STMT_INCLUDE, mod->mod->name, &mod->includes, 0,
            LY_STMT_LEAF, &mod->data, 0,
            LY_STMT_LEAF_LIST, &mod->data, 0,
            LY_STMT_LIST, &mod->data, 0,
            LY_STMT_NAMESPACE, &mod->mod->ns, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE,
            LY_STMT_NOTIFICATION, &mod->notifs, 0,
            LY_STMT_ORGANIZATION, &mod->mod->org, YIN_SUBELEM_UNIQUE,
            LY_STMT_PREFIX, &mod->mod->prefix, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &mod->mod->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_REVISION, &mod->revs, 0,
            LY_STMT_RPC, &mod->rpcs, 0,
            LY_STMT_TYPEDEF, &mod->typedefs, 0,
            LY_STMT_USES, &mod->data, 0,
            LY_STMT_YANG_VERSION, &mod->version, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));

    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_MODULE, NULL, &mod->exts);
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule_latest(ctx->xmlctx->ctx, mod->mod->name);
    if (dup) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_NAME2_COL, "module", "submodule", mod->mod->name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse submodule element.
 *
 * @param[in,out] ctx Yin parser context for logging and to store current state.
 * @param[in] mod_attrs Attributes of submodule element.
 * @param[out] submod Parsed submodule structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
yin_parse_submod(struct lys_yin_parser_ctx *ctx, struct lysp_submodule *submod)
{
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement *subelems = NULL;
    const struct lysp_submodule *dup;
    size_t subelems_size;

    submod->is_submod = 1;

    LY_CHECK_RET(lyxml_ctx_next(ctx->xmlctx));
    LY_CHECK_RET(yin_parse_attribute(ctx, YIN_ARG_NAME, &submod->name, Y_IDENTIF_ARG, LY_STMT_SUBMODULE));
    LY_CHECK_RET(subelems_allocator(ctx, subelems_size = 27, NULL, &subelems,
            LY_STMT_ANYDATA, &submod->data, YIN_SUBELEM_VER2,
            LY_STMT_ANYXML, &submod->data, 0,
            LY_STMT_AUGMENT, &submod->augments, 0,
            LY_STMT_BELONGS_TO, submod, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE,
            LY_STMT_CHOICE, &submod->data, 0,
            LY_STMT_CONTACT, &submod->contact, YIN_SUBELEM_UNIQUE,
            LY_STMT_CONTAINER, &submod->data, 0,
            LY_STMT_DESCRIPTION, &submod->dsc, YIN_SUBELEM_UNIQUE,
            LY_STMT_DEVIATION, &submod->deviations, 0,
            LY_STMT_EXTENSION, &submod->extensions, 0,
            LY_STMT_FEATURE, &submod->features, 0,
            LY_STMT_GROUPING, &submod->groupings, 0,
            LY_STMT_IDENTITY, &submod->identities, 0,
            LY_STMT_IMPORT, submod->prefix, &submod->imports, 0,
            LY_STMT_INCLUDE, submod->name, &submod->includes, 0,
            LY_STMT_LEAF, &submod->data, 0,
            LY_STMT_LEAF_LIST, &submod->data, 0,
            LY_STMT_LIST, &submod->data, 0,
            LY_STMT_NOTIFICATION, &submod->notifs, 0,
            LY_STMT_ORGANIZATION, &submod->org, YIN_SUBELEM_UNIQUE,
            LY_STMT_REFERENCE, &submod->ref, YIN_SUBELEM_UNIQUE,
            LY_STMT_REVISION, &submod->revs, 0,
            LY_STMT_RPC, &submod->rpcs, 0,
            LY_STMT_TYPEDEF, &submod->typedefs, 0,
            LY_STMT_USES, &submod->data, 0,
            LY_STMT_YANG_VERSION, &submod->version, YIN_SUBELEM_UNIQUE,
            LY_STMT_EXTENSION_INSTANCE, NULL, 0));

    ret = yin_parse_content(ctx, subelems, subelems_size, LY_STMT_SUBMODULE, NULL, &submod->exts);
    subelems_deallocator(subelems_size, subelems);
    LY_CHECK_RET(ret);

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule_latest(ctx->xmlctx->ctx, submod->name);
    if (dup && strcmp(dup->mod->name, submod->mod->name)) {
        LOGVAL_PARSER((struct lys_parser_ctx *)ctx, LY_VCODE_NAME_COL, "submodules", dup->name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

LY_ERR
yin_parse_submodule(struct lys_yin_parser_ctx **yin_ctx, struct ly_ctx *ctx, struct lys_parser_ctx *main_ctx,
        struct ly_in *in, struct lysp_submodule **submod)
{
    enum ly_stmt kw = LY_STMT_NONE;
    LY_ERR ret = LY_SUCCESS;
    struct lysp_submodule *mod_p = NULL;

    /* create context */
    *yin_ctx = calloc(1, sizeof **yin_ctx);
    LY_CHECK_ERR_RET(!(*yin_ctx), LOGMEM(ctx), LY_EMEM);
    (*yin_ctx)->format = LYS_IN_YIN;
    (*yin_ctx)->unres = main_ctx->unres;
    LY_CHECK_RET(lyxml_ctx_new(ctx, in, &(*yin_ctx)->xmlctx));

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    mod_p->mod = main_ctx->parsed_mod->mod;
    mod_p->parsing = 1;
    (*yin_ctx)->parsed_mod = (struct lysp_module *)mod_p;

    /* map the typedefs and groupings list from main context to the submodule's context */
    memcpy(&(*yin_ctx)->tpdfs_nodes, &main_ctx->tpdfs_nodes, sizeof main_ctx->tpdfs_nodes);
    memcpy(&(*yin_ctx)->grps_nodes, &main_ctx->grps_nodes, sizeof main_ctx->grps_nodes);

    /* check submodule */
    kw = yin_match_keyword(*yin_ctx, (*yin_ctx)->xmlctx->name, (*yin_ctx)->xmlctx->name_len, (*yin_ctx)->xmlctx->prefix,
            (*yin_ctx)->xmlctx->prefix_len, LY_STMT_NONE);
    if (kw == LY_STMT_MODULE) {
        LOGERR(ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_SUBMODULE) {
        LOGVAL_PARSER((struct lys_parser_ctx *)*yin_ctx, LY_VCODE_MOD_SUBOMD, ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    ret = yin_parse_submod(*yin_ctx, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* skip possible trailing whitespaces at end of the input */
    while (isspace(in->current[0])) {
        if (in->current[0] == '\n') {
            LY_IN_NEW_LINE(in);
        }
        ly_in_skip(in, 1);
    }
    if (in->current[0]) {
        LOGVAL_PARSER((struct lys_parser_ctx *)*yin_ctx, LY_VCODE_TRAILING_SUBMOD, 15, in->current,
                strlen(in->current) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p->parsing = 0;
    *submod = mod_p;

cleanup:
    if (ret) {
        lysp_module_free((struct lysp_module *)mod_p);
        yin_parser_ctx_free(*yin_ctx);
        *yin_ctx = NULL;
    }
    return ret;
}

LY_ERR
yin_parse_module(struct lys_yin_parser_ctx **yin_ctx, struct ly_in *in, struct lys_module *mod, struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS;
    enum ly_stmt kw = LY_STMT_NONE;
    struct lysp_module *mod_p = NULL;

    /* create context */
    *yin_ctx = calloc(1, sizeof **yin_ctx);
    LY_CHECK_ERR_RET(!(*yin_ctx), LOGMEM(mod->ctx), LY_EMEM);
    (*yin_ctx)->format = LYS_IN_YIN;
    (*yin_ctx)->unres = unres;
    LY_CHECK_RET(lyxml_ctx_new(mod->ctx, in, &(*yin_ctx)->xmlctx));

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(mod->ctx), cleanup);
    mod_p->mod = mod;
    mod_p->parsing = 1;
    (*yin_ctx)->parsed_mod = mod_p;

    /* check module */
    kw = yin_match_keyword(*yin_ctx, (*yin_ctx)->xmlctx->name, (*yin_ctx)->xmlctx->name_len, (*yin_ctx)->xmlctx->prefix,
            (*yin_ctx)->xmlctx->prefix_len, LY_STMT_NONE);
    if (kw == LY_STMT_SUBMODULE) {
        LOGERR(mod->ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_MODULE) {
        LOGVAL_PARSER((struct lys_parser_ctx *)*yin_ctx, LY_VCODE_MOD_SUBOMD, ly_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* parse module substatements */
    ret = yin_parse_mod(*yin_ctx, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* skip possible trailing whitespaces at end of the input */
    while (isspace(in->current[0])) {
        if (in->current[0] == '\n') {
            LY_IN_NEW_LINE(in);
        }
        ly_in_skip(in, 1);
    }
    if (in->current[0]) {
        LOGVAL_PARSER((struct lys_parser_ctx *)*yin_ctx, LY_VCODE_TRAILING_MOD, 15, in->current,
                strlen(in->current) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p->parsing = 0;
    mod->parsed = mod_p;

cleanup:
    if (ret != LY_SUCCESS) {
        lysp_module_free(mod_p);
        yin_parser_ctx_free(*yin_ctx);
        *yin_ctx = NULL;
    }
    return ret;
}
