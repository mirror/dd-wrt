/**
 * @file parser_stmt.c
 * @author Radek Krejčí <rkrejci@cesnet.cz>
 * @brief Parser of the extension substatements.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dict.h"
#include "log.h"
#include "parser_schema.h"
#include "path.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

static LY_ERR lysp_stmt_container(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_choice(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_case(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_uses(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_grouping(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_grp **groupings);
static LY_ERR lysp_stmt_list(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);

static LY_ERR
lysp_stmt_validate_value(struct lys_parser_ctx *ctx, enum yang_arg val_type, const char *val)
{
    uint8_t prefix = 0;
    ly_bool first = 1;
    uint32_t c;
    size_t utf8_char_len;

    while (*val) {
        LY_CHECK_ERR_RET(ly_getutf8(&val, &c, &utf8_char_len),
                LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, (val)[-utf8_char_len]), LY_EVALID);

        switch (val_type) {
        case Y_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar(ctx, c, first, NULL));
            break;
        case Y_PREF_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar(ctx, c, first, &prefix));
            break;
        case Y_STR_ARG:
        case Y_MAYBE_STR_ARG:
            LY_CHECK_RET(lysp_check_stringchar(ctx, c));
            break;
        }
        first = 0;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse extension instance.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] insubstmt The statement this extension instance is a substatement of.
 * @param[in] insubstmt_index Index of the keyword instance this extension instance is a substatement of.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_ext(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt insubstmt,
        LY_ARRAY_COUNT_TYPE insubstmt_index, struct lysp_ext_instance **exts)
{
    struct lysp_ext_instance *e;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *exts, e, LY_EMEM);

    /* store name and insubstmt info */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->stmt, 0, &e->name));
    e->parent_stmt = insubstmt;
    e->parent_stmt_index = insubstmt_index;
    e->parsed = NULL;
    /* TODO (duplicate) e->child = stmt->child; */

    /* get optional argument */
    if (stmt->arg) {
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &e->argument));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a generic text field without specific constraints. Those are contact, organization,
 * description, etc...
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] substmt_index Index of this substatement.
 * @param[in,out] value Place to store the parsed value.
 * @param[in] arg Type of the YANG keyword argument (of the value).
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_text_field(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint32_t substmt_index,
        const char **value, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    if (*value) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(stmt->kw));
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, value));

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, substmt_index, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a qname that can have more instances such as if-feature.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] qnames Parsed qnames to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_qnames(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt,
        struct lysp_qname **qnames, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    struct lysp_qname *item;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *qnames, item, LY_EMEM);
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &item->str));
    item->mod = ctx->parsed_mod;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, LY_ARRAY_COUNT(*qnames) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a generic text field that can have more instances such as base.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] texts Parsed values to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_text_fields(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt,
        const char ***texts, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    const char **item;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *texts, item, LY_EMEM);
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, item));

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, LY_ARRAY_COUNT(*texts) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the status statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_status(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("current")) && !strncmp(stmt->arg, "current", arg_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((arg_len == ly_strlen_const("deprecated")) && !strncmp(stmt->arg, "deprecated", arg_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((arg_len == ly_strlen_const("obsolete")) && !strncmp(stmt->arg, "obsolete", arg_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "status");
        return LY_EVALID;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_STATUS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "status");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the when statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] when_p When pointer to parse to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_when(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_when **when_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_when *when;

    if (*when_p) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "when");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_RET(!when, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    *when_p = when;

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &when->cond));

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &when->dsc, Y_STR_ARG, &when->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &when->ref, Y_STR_ARG, &when->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_WHEN, 0, &when->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "when");
            return LY_EVALID;
        }
    }
    return ret;
}

/**
 * @brief Parse the config statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_config(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;

    if (*flags & LYS_CONFIG_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "config");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("true")) && !strncmp(stmt->arg, "true", arg_len)) {
        *flags |= LYS_CONFIG_W;
    } else if ((arg_len == ly_strlen_const("false")) && !strncmp(stmt->arg, "false", arg_len)) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "config");
        return LY_EVALID;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_CONFIG, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "config");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the mandatory statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_mandatory(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;

    if (*flags & LYS_MAND_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "mandatory");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("true")) && !strncmp(stmt->arg, "true", arg_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((arg_len == ly_strlen_const("false")) && !strncmp(stmt->arg, "false", arg_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "mandatory");
        return LY_EVALID;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_MANDATORY, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "mandatory");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a restriction such as range or length.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_restr(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr *restr)
{
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &restr->arg.str));
    restr->arg.mod = ctx->parsed_mod;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a restriction that can have more instances such as must.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] restrs Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_restrs(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *restrs, restr, LY_EMEM);
    return lysp_stmt_restr(ctx, stmt, restr);
}

/**
 * @brief Parse the anydata or anyxml statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_any(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_anydata *any;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new structure and insert into siblings */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, any, next, LY_EMEM);

    any->nodetype = stmt->kw == LY_STMT_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = parent;

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &any->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &any->flags, &any->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &any->dsc, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &any->iffeatures, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(lysp_stmt_mandatory(ctx, child, &any->flags, &any->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &any->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &any->ref, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &any->flags, &any->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &any->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &any->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw),
                    (any->nodetype & LYS_ANYDATA) == LYS_ANYDATA ? ly_stmt2str(LY_STMT_ANYDATA) : ly_stmt2str(LY_STMT_ANYXML));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the value or position statement. Substatement of type enum statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] value Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_enum_value_pos(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, int64_t *value, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *ptr = NULL;
    long int num = 0;
    unsigned long int unum = 0;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(stmt->kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    arg_len = strlen(stmt->arg);
    if (!arg_len || (stmt->arg[0] == '+') || ((stmt->arg[0] == '0') && (arg_len > 1)) ||
            ((stmt->kw == LY_STMT_POSITION) && !strncmp(stmt->arg, "-0", 2))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(stmt->kw));
        goto error;
    }

    errno = 0;
    if (stmt->kw == LY_STMT_VALUE) {
        num = strtol(stmt->arg, &ptr, LY_BASE_DEC);
        if ((num < INT64_C(-2147483648)) || (num > INT64_C(2147483647))) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(stmt->kw));
            goto error;
        }
    } else {
        unum = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(stmt->kw));
            goto error;
        }
    }
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - stmt->arg) != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(stmt->kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, ly_stmt2str(stmt->kw));
        goto error;
    }
    if (stmt->kw == LY_STMT_VALUE) {
        *value = num;
    } else {
        *value = unum;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw == LY_STMT_VALUE ? LY_STMT_VALUE : LY_STMT_POSITION, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;

error:
    return LY_EVALID;
}

/**
 * @brief Parse the enum or bit statement. Substatement of type statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] enums Enums or bits to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_enum(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_type_enum **enums)
{
    struct lysp_type_enum *enm;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, stmt->kw == LY_STMT_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, stmt->arg));

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *enums, enm, LY_EMEM);

    if (stmt->kw == LY_STMT_ENUM) {
        LY_CHECK_RET(lysp_check_enum_name(ctx, stmt->arg, strlen(stmt->arg)));
    } /* else nothing specific for YANG_BIT */

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &enm->name));
    CHECK_UNIQUENESS(ctx, *enums, name, ly_stmt2str(stmt->kw), enm->name);

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &enm->dsc, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", ly_stmt2str(stmt->kw));
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &enm->ref, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &enm->flags, &enm->exts));
            break;
        case LY_STMT_VALUE:
            LY_CHECK_ERR_RET(stmt->kw == LY_STMT_BIT, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw),
                    ly_stmt2str(stmt->kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_POSITION:
            LY_CHECK_ERR_RET(stmt->kw == LY_STMT_ENUM, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw),
                    ly_stmt2str(stmt->kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &enm->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the fraction-digits statement. Substatement of type statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] fracdig Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_fracdigits(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *fracdig, struct lysp_ext_instance **exts)
{
    char *ptr;
    size_t arg_len;
    unsigned long int num;

    if (*fracdig) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if (!arg_len || (stmt->arg[0] == '0') || !isdigit(stmt->arg[0])) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - stmt->arg) != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > LY_TYPE_DEC64_FD_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }
    *fracdig = num;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_FRACTION_DIGITS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "fraction-digits");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the require-instance statement. Substatement of type statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] reqinst Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_reqinstance(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *reqinst, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    size_t arg_len;

    if (*flags & LYS_SET_REQINST) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    *flags |= LYS_SET_REQINST;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("true")) && !strncmp(stmt->arg, "true", arg_len)) {
        *reqinst = 1;
    } else if ((arg_len != ly_strlen_const("false")) || strncmp(stmt->arg, "false", arg_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "require-instance");
        return LY_EVALID;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_REQUIRE_INSTANCE, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "require-instance");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the modifier statement. Substatement of type pattern statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] pat Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_pattern_modifier(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, const char **pat, struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *buf;

    if ((*pat)[0] == LYSP_RESTR_PATTERN_NACK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len != ly_strlen_const("invert-match")) || strncmp(stmt->arg, "invert-match", arg_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "modifier");
        return LY_EVALID;
    }

    /* replace the value in the dictionary */
    buf = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    strcpy(buf, *pat);
    lydict_remove(PARSER_CTX(ctx), *pat);

    assert(buf[0] == LYSP_RESTR_PATTERN_ACK);
    buf[0] = LYSP_RESTR_PATTERN_NACK;
    LY_CHECK_RET(lydict_insert_zc(PARSER_CTX(ctx), buf, pat));

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_MODIFIER, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "modifier");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the pattern statement. Substatement of type statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] patterns Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_pattern(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr **patterns)
{
    char *buf;
    size_t arg_len;
    struct lysp_restr *restr;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *patterns, restr, LY_EMEM);
    arg_len = strlen(stmt->arg);

    /* add special meaning first byte */
    buf = malloc(arg_len + 2);
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    memmove(buf + 1, stmt->arg, arg_len);
    buf[0] = LYSP_RESTR_PATTERN_ACK; /* pattern's default regular-match flag */
    buf[arg_len + 1] = '\0'; /* terminating NULL byte */
    LY_CHECK_RET(lydict_insert_zc(PARSER_CTX(ctx), buf, &restr->arg.str));
    restr->arg.mod = ctx->parsed_mod;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_MODIFIER:
            PARSER_CHECK_STMTVER2_RET(ctx, "modifier", "pattern");
            LY_CHECK_RET(lysp_stmt_type_pattern_modifier(ctx, child, &restr->arg.str, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_PATTERN, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "pattern");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the type statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] type Type to wrote to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_type *type)
{
    struct lysp_type *nest_type;
    const char *str_path = NULL;
    LY_ERR ret;

    if (type->name) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_PREF_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &type->name));
    type->pmod = ctx->parsed_mod;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_BASE:
            LY_CHECK_RET(lysp_stmt_text_fields(ctx, child, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts));
            type->flags |= LYS_SET_BASE;
            break;
        case LY_STMT_BIT:
            LY_CHECK_RET(lysp_stmt_type_enum(ctx, child, &type->bits));
            type->flags |= LYS_SET_BIT;
            break;
        case LY_STMT_ENUM:
            LY_CHECK_RET(lysp_stmt_type_enum(ctx, child, &type->enums));
            type->flags |= LYS_SET_ENUM;
            break;
        case LY_STMT_FRACTION_DIGITS:
            LY_CHECK_RET(lysp_stmt_type_fracdigits(ctx, child, &type->fraction_digits, &type->exts));
            type->flags |= LYS_SET_FRDIGITS;
            break;
        case LY_STMT_LENGTH:
            if (type->length) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(child->kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(lysp_stmt_restr(ctx, child, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case LY_STMT_PATH:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &str_path, Y_STR_ARG, &type->exts));
            ret = ly_path_parse(PARSER_CTX(ctx), NULL, str_path, 0, LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
                    LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &type->path);
            lydict_remove(PARSER_CTX(ctx), str_path);
            LY_CHECK_RET(ret);
            type->flags |= LYS_SET_PATH;
            break;
        case LY_STMT_PATTERN:
            LY_CHECK_RET(lysp_stmt_type_pattern(ctx, child, &type->patterns));
            type->flags |= LYS_SET_PATTERN;
            break;
        case LY_STMT_RANGE:
            if (type->range) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(child->kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(lysp_stmt_restr(ctx, child, type->range));
            type->flags |= LYS_SET_RANGE;
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            LY_CHECK_RET(lysp_stmt_type_reqinstance(ctx, child, &type->require_instance, &type->flags, &type->exts));
            /* LYS_SET_REQINST checked and set inside lysp_stmt_type_reqinstance() */
            break;
        case LY_STMT_TYPE:
            LY_ARRAY_NEW_RET(PARSER_CTX(ctx), type->types, nest_type, LY_EMEM);
            LY_CHECK_RET(lysp_stmt_type(ctx, child, nest_type));
            type->flags |= LYS_SET_TYPE;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_TYPE, 0, &type->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "type");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the leaf statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_leaf(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_leaf *leaf;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new leaf structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, leaf, next, LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = parent;

    /* get name */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &leaf->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &leaf->dflt.str, Y_STR_ARG, &leaf->exts));
            leaf->dflt.mod = ctx->parsed_mod;
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &leaf->dsc, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &leaf->iffeatures, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(lysp_stmt_mandatory(ctx, child, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &leaf->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &leaf->ref, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(lysp_stmt_type(ctx, child, &leaf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &leaf->units, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &leaf->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_LEAF, 0, &leaf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "leaf");
            return LY_EVALID;
        }
    }

    /* mandatory substatements */
    if (!leaf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the max-elements statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] max Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_maxelements(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt,
        uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *ptr;
    unsigned long int num;

    if (*flags & LYS_SET_MAX) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "max-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MAX;

    /* get value */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);

    if (!arg_len || (stmt->arg[0] == '0') || ((stmt->arg[0] != 'u') && !isdigit(stmt->arg[0]))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "max-elements");
        return LY_EVALID;
    }

    if ((arg_len != ly_strlen_const("unbounded")) || strncmp(stmt->arg, "unbounded", arg_len)) {
        errno = 0;
        num = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
        /* we have not parsed the whole argument */
        if ((size_t)(ptr - stmt->arg) != arg_len) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "max-elements");
            return LY_EVALID;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, "max-elements");
            return LY_EVALID;
        }

        *max = num;
    } else {
        /* unbounded */
        *max = 0;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_MAX_ELEMENTS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "max-elements");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the min-elements statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] min Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_minelements(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt,
        uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *ptr;
    unsigned long int num;

    if (*flags & LYS_SET_MIN) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "min-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MIN;

    /* get value */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);

    if (!arg_len || !isdigit(stmt->arg[0]) || ((stmt->arg[0] == '0') && (arg_len > 1))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "min-elements");
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - stmt->arg) != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "min-elements");
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, "min-elements");
        return LY_EVALID;
    }
    *min = num;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_MIN_ELEMENTS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "min-elements");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the ordered-by statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_orderedby(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;

    if (*flags & LYS_ORDBY_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "ordered-by");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("system")) && !strncmp(stmt->arg, "system", arg_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((arg_len == ly_strlen_const("user")) && !strncmp(stmt->arg, "user", arg_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "ordered-by");
        return LY_EVALID;
    }

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_ORDERED_BY, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "ordered-by");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the leaf-list statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_leaflist(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_leaflist *llist;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new leaf-list structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, llist, next, LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;
    llist->parent = parent;

    /* get name */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &llist->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &llist->flags, &llist->exts));
            break;
        case LY_STMT_DEFAULT:
            PARSER_CHECK_STMTVER2_RET(ctx, "default", "leaf-list");
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &llist->dflts, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &llist->dsc, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &llist->iffeatures, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_maxelements(ctx, child, &llist->max, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_minelements(ctx, child, &llist->min, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &llist->musts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(lysp_stmt_orderedby(ctx, child, &llist->flags, &llist->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &llist->ref, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &llist->flags, &llist->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(lysp_stmt_type(ctx, child, &llist->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &llist->units, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &llist->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_LEAF_LIST, 0, &llist->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "llist");
            return LY_EVALID;
        }
    }

    /* mandatory substatements */
    if (!llist->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf-list");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the refine statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] refines Refines to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_refine(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_refine **refines)
{
    struct lysp_refine *rf;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *refines, rf, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &rf->nodeid));

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &rf->flags, &rf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &rf->dflts, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &rf->dsc, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", "refine");
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &rf->iffeatures, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_maxelements(ctx, child, &rf->max, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_minelements(ctx, child, &rf->min, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &rf->musts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(lysp_stmt_mandatory(ctx, child, &rf->flags, &rf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &rf->ref, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &rf->presence, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_REFINE, 0, &rf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "refine");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the typedef statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] typedefs Typedefs to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_typedef(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_tpdf **typedefs)
{
    struct lysp_tpdf *tpdf;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *typedefs, tpdf, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &tpdf->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &tpdf->dflt.str, Y_STR_ARG, &tpdf->exts));
            tpdf->dflt.mod = ctx->parsed_mod;
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &tpdf->dsc, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &tpdf->ref, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &tpdf->flags, &tpdf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(lysp_stmt_type(ctx, child, &tpdf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &tpdf->units, Y_STR_ARG, &tpdf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_TYPEDEF, 0, &tpdf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "typedef");
            return LY_EVALID;
        }
    }

    /* mandatory substatements */
    if (!tpdf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "typedef");
        return LY_EVALID;
    }

    /* store data for collision check */
    if (parent && !(parent->nodetype & (LYS_GROUPING | LYS_RPC | LYS_ACTION | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF))) {
        LY_CHECK_RET(ly_set_add(&ctx->tpdfs_nodes, parent, 0, NULL));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the input or output statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] inout_p Input/output pointer to write to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_inout(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_action_inout *inout_p)
{
    if (inout_p->nodetype) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(stmt->kw));
        return LY_EVALID;
    }

    /* initiate structure */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->kw == LY_STMT_INPUT ? "input" : "output", 0, &inout_p->name));
    inout_p->nodetype = stmt->kw == LY_STMT_INPUT ? LYS_INPUT : LYS_OUTPUT;
    inout_p->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", ly_stmt2str(stmt->kw));
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &inout_p->node, &inout_p->child));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &inout_p->node, &inout_p->typedefs));
            break;
        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", ly_stmt2str(stmt->kw));
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &inout_p->musts));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &inout_p->node, &inout_p->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &inout_p->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), ly_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }

    if (!inout_p->child) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "data-def-stmt", ly_stmt2str(stmt->kw));
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the action statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] actions Actions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_action(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node_action **actions)
{
    struct lysp_node_action *act;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    LY_LIST_NEW_RET(PARSER_CTX(ctx), actions, act, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &act->name));
    act->nodetype = parent ? LYS_ACTION : LYS_RPC;
    act->parent = parent;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &act->dsc, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &act->iffeatures, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &act->ref, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &act->flags, &act->exts));
            break;

        case LY_STMT_INPUT:
            LY_CHECK_RET(lysp_stmt_inout(ctx, child, &act->node, &act->input));
            break;
        case LY_STMT_OUTPUT:
            LY_CHECK_RET(lysp_stmt_inout(ctx, child, &act->node, &act->output));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &act->node, &act->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &act->node, &act->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, parent ? LY_STMT_ACTION : LY_STMT_RPC, 0, &act->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), parent ? "action" : "rpc");
            return LY_EVALID;
        }
    }

    /* always initialize inout, they are technically present (needed for later deviations/refines) */
    if (!act->input.nodetype) {
        act->input.nodetype = LYS_INPUT;
        act->input.parent = &act->node;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "input", 0, &act->input.name));
    }
    if (!act->output.nodetype) {
        act->output.nodetype = LYS_OUTPUT;
        act->output.parent = &act->node;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "output", 0, &act->output.name));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the notification statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] notifs Notifications to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_notif(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node_notif **notifs)
{
    struct lysp_node_notif *notif;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    LY_LIST_NEW_RET(PARSER_CTX(ctx), notifs, notif, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &notif->name));
    notif->nodetype = LYS_NOTIF;
    notif->parent = parent;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &notif->dsc, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &notif->iffeatures, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &notif->ref, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &notif->flags, &notif->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "notification");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_case(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &notif->node, &notif->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &notif->node, &notif->child));
            break;

        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", "notification");
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &notif->musts));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &notif->node, &notif->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &notif->node, &notif->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_NOTIFICATION, 0, &notif->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "notification");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the grouping statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] groupings Groupings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_grouping(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node_grp **groupings)
{
    struct lysp_node_grp *grp;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    LY_LIST_NEW_RET(PARSER_CTX(ctx), groupings, grp, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &grp->name));
    grp->nodetype = LYS_GROUPING;
    grp->parent = parent;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &grp->dsc, Y_STR_ARG, &grp->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &grp->ref, Y_STR_ARG, &grp->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &grp->flags, &grp->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "grouping");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &grp->node, &grp->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &grp->node, &grp->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &grp->node, &grp->typedefs));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "grouping");
            LY_CHECK_RET(lysp_stmt_action(ctx, child, &grp->node, &grp->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &grp->node, &grp->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "grouping");
            LY_CHECK_RET(lysp_stmt_notif(ctx, child, &grp->node, &grp->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_GROUPING, 0, &grp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "grouping");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the augment statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] augments Augments to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_augment(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node_augment **augments)
{
    struct lysp_node_augment *aug;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    LY_LIST_NEW_RET(PARSER_CTX(ctx), augments, aug, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &aug->nodeid));
    aug->nodetype = LYS_AUGMENT;
    aug->parent = parent;

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &aug->dsc, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &aug->iffeatures, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &aug->ref, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &aug->flags, &aug->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &aug->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "augment");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(lysp_stmt_case(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &aug->node, &aug->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &aug->node, &aug->child));
            break;

        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "augment");
            LY_CHECK_RET(lysp_stmt_action(ctx, child, &aug->node, &aug->actions));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "augment");
            LY_CHECK_RET(lysp_stmt_notif(ctx, child, &aug->node, &aug->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_AUGMENT, 0, &aug->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "augment");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the uses statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_uses(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_uses *uses;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_PREF_IDENTIF_ARG, stmt->arg));

    /* create uses structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, uses, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &uses->name));
    uses->nodetype = LYS_USES;
    uses->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &uses->dsc, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &uses->iffeatures, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &uses->ref, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &uses->flags, &uses->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &uses->when));
            break;

        case LY_STMT_REFINE:
            LY_CHECK_RET(lysp_stmt_refine(ctx, child, &uses->refines));
            break;
        case LY_STMT_AUGMENT:
            LY_CHECK_RET(lysp_stmt_augment(ctx, child, &uses->node, &uses->augments));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_USES, 0, &uses->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "uses");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the case statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_case(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_case *cas;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new case structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, cas, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &cas->name));
    cas->nodetype = LYS_CASE;
    cas->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &cas->dsc, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &cas->iffeatures, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &cas->ref, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &cas->flags, &cas->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &cas->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "case");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &cas->node, &cas->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_CASE, 0, &cas->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "case");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the choice statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_choice(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_choice *choice;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new choice structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, choice, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &choice->name));
    choice->nodetype = LYS_CHOICE;
    choice->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &choice->flags, &choice->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &choice->dsc, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &choice->iffeatures, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(lysp_stmt_mandatory(ctx, child, &choice->flags, &choice->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &choice->ref, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &choice->flags, &choice->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &choice->when));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &choice->dflt.str, Y_PREF_IDENTIF_ARG, &choice->exts));
            choice->dflt.mod = ctx->parsed_mod;
            break;
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "choice");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(lysp_stmt_case(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_CHOICE:
            PARSER_CHECK_STMTVER2_RET(ctx, "choice", "choice");
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &choice->node, &choice->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_CHOICE, 0, &choice->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "choice");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the container statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_container(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_container *cont;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new container structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, cont, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &cont->name));
    cont->nodetype = LYS_CONTAINER;
    cont->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &cont->flags, &cont->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &cont->dsc, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &cont->iffeatures, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &cont->ref, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &cont->flags, &cont->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &cont->when));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &cont->presence, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "container");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &cont->node, &cont->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &cont->node, &cont->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &cont->node, &cont->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &cont->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "container");
            LY_CHECK_RET(lysp_stmt_action(ctx, child, &cont->node, &cont->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &cont->node, &cont->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "container");
            LY_CHECK_RET(lysp_stmt_notif(ctx, child, &cont->node, &cont->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_CONTAINER, 0, &cont->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "container");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the list statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in] parent Parent node to connect to (not into).
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_list(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
{
    struct lysp_node_list *list;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));

    /* create new list structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, list, next, LY_EMEM);

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &list->name));
    list->nodetype = LYS_LIST;
    list->parent = parent;

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(lysp_stmt_config(ctx, child, &list->flags, &list->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &list->dsc, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &list->iffeatures, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &list->ref, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &list->flags, &list->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(lysp_stmt_when(ctx, child, &list->when));
            break;
        case LY_STMT_KEY:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &list->key, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_maxelements(ctx, child, &list->max, &list->flags, &list->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(lysp_stmt_minelements(ctx, child, &list->min, &list->flags, &list->exts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(lysp_stmt_orderedby(ctx, child, &list->flags, &list->exts));
            break;
        case LY_STMT_UNIQUE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &list->uniques, Y_STR_ARG, &list->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "list");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(lysp_stmt_any(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(lysp_stmt_choice(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(lysp_stmt_container(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(lysp_stmt_leaf(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(lysp_stmt_leaflist(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(lysp_stmt_list(ctx, child, &list->node, &list->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(lysp_stmt_uses(ctx, child, &list->node, &list->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(lysp_stmt_typedef(ctx, child, &list->node, &list->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &list->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "list");
            LY_CHECK_RET(lysp_stmt_action(ctx, child, &list->node, &list->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &list->node, &list->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "list");
            LY_CHECK_RET(lysp_stmt_notif(ctx, child, &list->node, &list->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LY_STMT_LIST, 0, &list->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(child->kw), "list");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lysp_stmt_parse(struct lysc_ctx *ctx, const struct lysp_stmt *stmt, void **result, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    uint16_t flags;
    struct lys_parser_ctx pctx = {0};

    pctx.format = LYS_IN_YANG;
    pctx.parsed_mod = ctx->pmod;

    LOG_LOCSET(NULL, NULL, ctx->path, NULL);

    switch (stmt->kw) {
    case LY_STMT_ACTION:
    case LY_STMT_RPC:
        ret = lysp_stmt_action(&pctx, stmt, NULL, (struct lysp_node_action **)result);
        break;
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
        ret = lysp_stmt_any(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_AUGMENT:
        ret = lysp_stmt_augment(&pctx, stmt, NULL, (struct lysp_node_augment **)result);
        break;
    case LY_STMT_BASE:
        ret = lysp_stmt_text_fields(&pctx, stmt, (const char ***)result, Y_PREF_IDENTIF_ARG, exts);
        break;
    case LY_STMT_BIT:
    case LY_STMT_ENUM:
        ret = lysp_stmt_type_enum(&pctx, stmt, (struct lysp_type_enum **)result);
        break;
    case LY_STMT_CASE:
        ret = lysp_stmt_case(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_CHOICE:
        ret = lysp_stmt_choice(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_CONFIG:
        ret = lysp_stmt_config(&pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_ERROR_APP_TAG:
    case LY_STMT_ERROR_MESSAGE:
    case LY_STMT_KEY:
    case LY_STMT_NAMESPACE:
    case LY_STMT_ORGANIZATION:
    case LY_STMT_PRESENCE:
    case LY_STMT_REFERENCE:
    case LY_STMT_UNITS:
        ret = lysp_stmt_text_field(&pctx, stmt, 0, (const char **)result, Y_STR_ARG, exts);
        break;
    case LY_STMT_CONTAINER:
        ret = lysp_stmt_container(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_DEFAULT:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_UNIQUE:
        ret = lysp_stmt_qnames(&pctx, stmt, (struct lysp_qname **)result, Y_STR_ARG, exts);
        break;
    case LY_STMT_EXTENSION_INSTANCE:
        ret = lysp_stmt_ext(&pctx, stmt, LY_STMT_EXTENSION_INSTANCE, 0, exts);
        break;
    case LY_STMT_FRACTION_DIGITS:
        ret = lysp_stmt_type_fracdigits(&pctx, stmt, *(uint8_t **)result, exts);
        break;
    case LY_STMT_GROUPING:
        ret = lysp_stmt_grouping(&pctx, stmt, NULL, (struct lysp_node_grp **)result);
        break;
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT: {
        struct lysp_node_action_inout *inout;

        *result = inout = calloc(1, sizeof *inout);
        LY_CHECK_ERR_RET(!inout, LOGMEM(ctx->ctx), LY_EMEM);
        ret = lysp_stmt_inout(&pctx, stmt, NULL, inout);
        break;
    }
    case LY_STMT_LEAF:
        ret = lysp_stmt_leaf(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_LEAF_LIST:
        ret = lysp_stmt_leaflist(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_LENGTH:
    case LY_STMT_MUST:
    case LY_STMT_RANGE:
        ret = lysp_stmt_restrs(&pctx, stmt, (struct lysp_restr **)result);
        break;
    case LY_STMT_LIST:
        ret = lysp_stmt_list(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_MANDATORY:
        ret = lysp_stmt_mandatory(&pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_MAX_ELEMENTS:
        flags = 0;
        ret = lysp_stmt_maxelements(&pctx, stmt, *(uint32_t **)result, &flags, exts);
        break;
    case LY_STMT_MIN_ELEMENTS:
        flags = 0;
        ret = lysp_stmt_minelements(&pctx, stmt, *(uint32_t **)result, &flags, exts);
        break;
    case LY_STMT_MODIFIER:
        ret = lysp_stmt_type_pattern_modifier(&pctx, stmt, (const char **)result, exts);
        break;
    case LY_STMT_NOTIFICATION:
        ret = lysp_stmt_notif(&pctx, stmt, NULL, (struct lysp_node_notif **)result);
        break;
    case LY_STMT_ORDERED_BY:
        ret = lysp_stmt_orderedby(&pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_PATH: {
        const char *str_path = NULL;

        LY_CHECK_RET(lysp_stmt_text_field(&pctx, stmt, 0, &str_path, Y_STR_ARG, exts));
        ret = ly_path_parse(ctx->ctx, NULL, str_path, 0, LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
                LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, (struct lyxp_expr **)result);
        lydict_remove(ctx->ctx, str_path);
        break;
    }
    case LY_STMT_PATTERN:
        ret = lysp_stmt_type_pattern(&pctx, stmt, (struct lysp_restr **)result);
        break;
    case LY_STMT_POSITION:
    case LY_STMT_VALUE:
        flags = 0;
        ret = lysp_stmt_type_enum_value_pos(&pctx, stmt, *(int64_t **)result, &flags, exts);
        break;
    case LY_STMT_PREFIX:
        ret = lysp_stmt_text_field(&pctx, stmt, 0, (const char **)result, Y_IDENTIF_ARG, exts);
        break;
    case LY_STMT_REFINE:
        ret = lysp_stmt_refine(&pctx, stmt, (struct lysp_refine **)result);
        break;
    case LY_STMT_REQUIRE_INSTANCE:
        flags = 0;
        ret = lysp_stmt_type_reqinstance(&pctx, stmt, *(uint8_t **)result, &flags, exts);
        break;
    case LY_STMT_STATUS:
        ret = lysp_stmt_status(&pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_TYPE: {
        struct lysp_type *type;

        *result = type = calloc(1, sizeof *type);
        LY_CHECK_ERR_RET(!type, LOGMEM(ctx->ctx), LY_EMEM);
        ret = lysp_stmt_type(&pctx, stmt, type);
        break;
    }
    case LY_STMT_TYPEDEF:
        ret = lysp_stmt_typedef(&pctx, stmt, NULL, (struct lysp_tpdf **)result);
        break;
    case LY_STMT_USES:
        ret = lysp_stmt_uses(&pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_WHEN:
        ret = lysp_stmt_when(&pctx, stmt, (struct lysp_when **)result);
        break;
    default:
        LOGINT(ctx->ctx);
        return LY_EINT;
    }

    LOG_LOCBACK(0, 0, 1, 0);
    return ret;
}
