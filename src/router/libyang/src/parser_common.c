/**
 * @file parser_common.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang common parser functions.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup, strndup */

#ifdef __APPLE__
#define _DARWIN_C_SOURCE /* F_GETPATH */
#endif

#if defined (__NetBSD__) || defined (__OpenBSD__)
/* realpath */
#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1
#endif

#include "parser_internal.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat.h"
#include "dict.h"
#include "in_internal.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "path.h"
#include "plugins_exts/metadata.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

void
lyd_ctx_free(struct lyd_ctx *lydctx)
{
    ly_set_erase(&lydctx->node_types, NULL);
    ly_set_erase(&lydctx->meta_types, NULL);
    ly_set_erase(&lydctx->node_when, NULL);
    ly_set_erase(&lydctx->ext_node, free);
    ly_set_erase(&lydctx->ext_val, free);
}

LY_ERR
lyd_parser_notif_eventtime_validate(const struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_ctx *ctx = (struct ly_ctx *)LYD_CTX(node);
    struct lysc_ctx cctx;
    const struct lys_module *mod;
    LY_ARRAY_COUNT_TYPE u;
    struct ly_err_item *err = NULL;
    struct lysp_type *type_p = NULL;
    struct lysc_pattern **patterns = NULL;
    const char *value;

    LYSC_CTX_INIT_CTX(cctx, ctx);

    /* get date-and-time parsed type */
    mod = ly_ctx_get_module_latest(ctx, "ietf-yang-types");
    assert(mod);
    LY_ARRAY_FOR(mod->parsed->typedefs, u) {
        if (!strcmp(mod->parsed->typedefs[u].name, "date-and-time")) {
            type_p = &mod->parsed->typedefs[u].type;
            break;
        }
    }
    assert(type_p);

    /* compile patterns */
    assert(type_p->patterns);
    LY_CHECK_GOTO(rc = lys_compile_type_patterns(&cctx, type_p->patterns, NULL, &patterns), cleanup);

    /* validate */
    value = lyd_get_value(node);
    rc = lyplg_type_validate_patterns(patterns, value, strlen(value), &err);

cleanup:
    FREE_ARRAY(&cctx.free_ctx, patterns, lysc_pattern_free);
    if (rc && err) {
        ly_err_print(ctx, err);
        ly_err_free(err);
        LOGVAL(ctx, LYVE_DATA, "Invalid \"eventTime\" in the notification.");
    }
    return rc;
}

LY_ERR
lyd_parser_find_operation(const struct lyd_node *parent, uint32_t int_opts, struct lyd_node **op)
{
    const struct lyd_node *iter;

    *op = NULL;

    if (!parent) {
        /* no parent, nothing to look for */
        return LY_SUCCESS;
    }

    /* we need to find the operation node if it already exists */
    for (iter = parent; iter; iter = lyd_parent(iter)) {
        if (iter->schema && (iter->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
            break;
        }
    }

    if (!iter) {
        /* no operation found */
        return LY_SUCCESS;
    }

    if (!(int_opts & LYD_INTOPT_ANY)) {
        if (!(int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY))) {
            LOGERR(LYD_CTX(parent), LY_EINVAL, "Invalid parent %s \"%s\" node when not parsing any operation.",
                    lys_nodetype2str(iter->schema->nodetype), iter->schema->name);
            return LY_EINVAL;
        } else if ((iter->schema->nodetype == LYS_RPC) && !(int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_REPLY))) {
            LOGERR(LYD_CTX(parent), LY_EINVAL, "Invalid parent RPC \"%s\" node when not parsing RPC nor rpc-reply.",
                    iter->schema->name);
            return LY_EINVAL;
        } else if ((iter->schema->nodetype == LYS_ACTION) && !(int_opts & (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY))) {
            LOGERR(LYD_CTX(parent), LY_EINVAL, "Invalid parent action \"%s\" node when not parsing action nor rpc-reply.",
                    iter->schema->name);
            return LY_EINVAL;
        } else if ((iter->schema->nodetype == LYS_NOTIF) && !(int_opts & LYD_INTOPT_NOTIF)) {
            LOGERR(LYD_CTX(parent), LY_EINVAL, "Invalid parent notification \"%s\" node when not parsing a notification.",
                    iter->schema->name);
            return LY_EINVAL;
        }
    }

    *op = (struct lyd_node *)iter;
    return LY_SUCCESS;
}

const struct lysc_node *
lyd_parser_node_schema(const struct lyd_node *node)
{
    uint32_t i;
    const struct lyd_node *iter;
    const struct lysc_node *schema = NULL;
    const struct lys_module *mod;

    if (!node) {
        return NULL;
    } else if (node->schema) {
        /* simplest case */
        return node->schema;
    }

    /* find the first schema node in the parsed nodes */
    i = ly_log_location_dnode_count();
    if (i) {
        do {
            --i;
            if (ly_log_location_dnode(i)->schema) {
                /* this node is processed */
                schema = ly_log_location_dnode(i)->schema;
                ++i;
                break;
            }
        } while (i);
    }

    /* get schema node of an opaque node */
    do {
        /* get next data node */
        if (i == ly_log_location_dnode_count()) {
            iter = node;
        } else {
            iter = ly_log_location_dnode(i);
        }
        assert(!iter->schema);

        /* get module */
        mod = lyd_node_module(iter);
        if (!mod) {
            /* unknown module, no schema node */
            schema = NULL;
            break;
        }

        /* get schema node */
        schema = lys_find_child(schema, mod, LYD_NAME(iter), 0, 0, 0);

        /* move to the descendant */
        ++i;
    } while (schema && (iter != node));

    return schema;
}

LY_ERR
lyd_parser_check_schema(struct lyd_ctx *lydctx, const struct lysc_node *snode)
{
    LY_ERR rc = LY_SUCCESS;

    LOG_LOCSET(snode, NULL);

    if (lydctx->int_opts & LYD_INTOPT_ANY) {
        /* nothing to check, everything is allowed */
        goto cleanup;
    }

    if ((lydctx->parse_opts & LYD_PARSE_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
        LOGVAL(lydctx->data_ctx->ctx, LY_VCODE_UNEXPNODE, "state", snode->name);
        rc = LY_EVALID;
        goto cleanup;
    }

    if (snode->nodetype == LYS_RPC) {
        if (lydctx->int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_REPLY)) {
            if (lydctx->op_node) {
                goto error_node_dup;
            }
        } else {
            goto error_node_inval;
        }
    } else if (snode->nodetype == LYS_ACTION) {
        if (lydctx->int_opts & (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) {
            if (lydctx->op_node) {
                goto error_node_dup;
            }
        } else {
            goto error_node_inval;
        }
    } else if (snode->nodetype == LYS_NOTIF) {
        if (lydctx->int_opts & LYD_INTOPT_NOTIF) {
            if (lydctx->op_node) {
                goto error_node_dup;
            }
        } else {
            goto error_node_inval;
        }
    }

    /* success */
    goto cleanup;

error_node_dup:
    LOGVAL(lydctx->data_ctx->ctx, LYVE_DATA, "Unexpected %s element \"%s\", %s \"%s\" already parsed.",
            lys_nodetype2str(snode->nodetype), snode->name, lys_nodetype2str(lydctx->op_node->schema->nodetype),
            lydctx->op_node->schema->name);
    rc = LY_EVALID;
    goto cleanup;

error_node_inval:
    LOGVAL(lydctx->data_ctx->ctx, LYVE_DATA, "Unexpected %s element \"%s\".", lys_nodetype2str(snode->nodetype),
            snode->name);
    rc = LY_EVALID;

cleanup:
    LOG_LOCBACK(1, 0);
    return rc;
}

LY_ERR
lyd_parser_create_term(struct lyd_ctx *lydctx, const struct lysc_node *schema, const void *value, size_t value_len,
        ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, struct lyd_node **node)
{
    LY_ERR r;
    ly_bool incomplete;
    ly_bool store_only = (lydctx->parse_opts & LYD_PARSE_STORE_ONLY) == LYD_PARSE_STORE_ONLY ? 1 : 0;

    if ((r = lyd_create_term(schema, value, value_len, 1, store_only, dynamic, format, prefix_data,
            hints, &incomplete, node))) {
        if (lydctx->data_ctx->ctx != schema->module->ctx) {
            /* move errors to the main context */
            ly_err_move(schema->module->ctx, (struct ly_ctx *)lydctx->data_ctx->ctx);
        }
        return r;
    }

    if (incomplete && !(lydctx->parse_opts & LYD_PARSE_ONLY)) {
        LY_CHECK_RET(ly_set_add(&lydctx->node_types, *node, 1, NULL));
    }
    return LY_SUCCESS;
}

LY_ERR
lyd_parser_create_meta(struct lyd_ctx *lydctx, struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod,
        const char *name, size_t name_len, const void *value, size_t value_len, ly_bool *dynamic, LY_VALUE_FORMAT format,
        void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node)
{
    LY_ERR rc = LY_SUCCESS;
    char *dpath = NULL, *path = NULL;
    ly_bool incomplete;
    struct lyd_meta *first = NULL;
    ly_bool store_only = (lydctx->parse_opts & LYD_PARSE_STORE_ONLY) == LYD_PARSE_STORE_ONLY ? 1 : 0;

    if (meta && *meta) {
        /* remember the first metadata */
        first = *meta;
    }

    /* generate path to the metadata */
    LY_CHECK_RET(ly_vlog_build_data_path(lydctx->data_ctx->ctx, &dpath));
    if (asprintf(&path, "%s/@%s:%.*s", dpath, mod->name, (int)name_len, name) == -1) {
        LOGMEM(lydctx->data_ctx->ctx);
        rc = LY_EMEM;
        goto cleanup;
    }
    ly_log_location(NULL, NULL, path, NULL);

    LY_CHECK_GOTO(rc = lyd_create_meta(parent, meta, mod, name, name_len, value, value_len, 1, store_only, dynamic, format,
            prefix_data, hints, ctx_node, 0, &incomplete), cleanup);

    if (incomplete && !(lydctx->parse_opts & LYD_PARSE_ONLY)) {
        LY_CHECK_GOTO(rc = ly_set_add(&lydctx->meta_types, *meta, 1, NULL), cleanup);
    }

    if (first) {
        /* always return the first metadata */
        *meta = first;
    }

cleanup:
    ly_log_location_revert(0, 0, 1, 0);
    free(dpath);
    free(path);
    return rc;
}

LY_ERR
lyd_parse_check_keys(struct lyd_node *node)
{
    const struct lysc_node *skey = NULL;
    const struct lyd_node *key;

    assert(node->schema->nodetype == LYS_LIST);

    key = lyd_child(node);
    while ((skey = lys_getnext(skey, node->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            LOGVAL(LYD_CTX(node), LY_VCODE_NOKEY, skey->name);
            return LY_EVALID;
        }

        key = key->next;
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_parse_set_data_flags(struct lyd_node *node, struct lyd_meta **meta, struct lyd_ctx *lydctx,
        struct lysc_ext_instance *ext)
{
    struct lyd_meta *meta2, *prev_meta = NULL, *next_meta = NULL;
    struct lyd_ctx_ext_val *ext_val;

    if (lydctx->parse_opts & LYD_PARSE_NO_NEW) {
        node->flags &= ~LYD_NEW;
    }

    if (lysc_has_when(node->schema)) {
        if (lydctx->parse_opts & LYD_PARSE_WHEN_TRUE) {
            /* the condition was true before */
            node->flags |= LYD_WHEN_TRUE;
        }
        if (!(lydctx->parse_opts & LYD_PARSE_ONLY)) {
            /* remember we need to evaluate this node's when */
            LY_CHECK_RET(ly_set_add(&lydctx->node_when, node, 1, NULL));
        }
    }

    LY_LIST_FOR(*meta, meta2) {
        if (!strcmp(meta2->name, "default") && !strcmp(meta2->annotation->module->name, "ietf-netconf-with-defaults") &&
                meta2->value.boolean) {
            /* node is default according to the metadata */
            node->flags |= LYD_DEFAULT;

            next_meta = meta2->next;

            /* delete the metadata */
            if (meta != &node->meta) {
                *meta = (*meta)->next;
            }
            lyd_free_meta_single(meta2);
            if (prev_meta) {
                prev_meta->next = next_meta;
            }

            /* update dflt flag for all parent NP containers */
            lyd_cont_set_dflt(lyd_parent(node));
            break;
        }

        prev_meta = meta2;
    }

    if (ext) {
        /* parsed for an extension */
        node->flags |= LYD_EXT;

        if (!(lydctx->parse_opts & LYD_PARSE_ONLY)) {
            /* rememeber for validation */
            ext_val = malloc(sizeof *ext_val);
            LY_CHECK_ERR_RET(!ext_val, LOGMEM(LYD_CTX(node)), LY_EMEM);
            ext_val->ext = ext;
            ext_val->sibling = node;
            LY_CHECK_RET(ly_set_add(&lydctx->ext_val, ext_val, 1, NULL));
        }
    }

    return LY_SUCCESS;
}

void
lys_parser_fill_filepath(struct ly_ctx *ctx, struct ly_in *in, const char **filepath)
{
    char path[PATH_MAX];

#ifndef __APPLE__
    char proc_path[32];
    int len;
#endif

    LY_CHECK_ARG_RET(NULL, ctx, in, filepath, );
    if (*filepath) {
        /* filepath already set */
        return;
    }

    switch (in->type) {
    case LY_IN_FILEPATH:
        if (realpath(in->method.fpath.filepath, path) != NULL) {
            lydict_insert(ctx, path, 0, filepath);
        } else {
            lydict_insert(ctx, in->method.fpath.filepath, 0, filepath);
        }

        break;
    case LY_IN_FD:
#ifdef __APPLE__
        if (fcntl(in->method.fd, F_GETPATH, path) != -1) {
            lydict_insert(ctx, path, 0, filepath);
        }
#elif defined _WIN32
        HANDLE h = _get_osfhandle(in->method.fd);
        FILE_NAME_INFO info;

        if (GetFileInformationByHandleEx(h, FileNameInfo, &info, sizeof info)) {
            char *buf = calloc(info.FileNameLength + 1 /* trailing NULL */, MB_CUR_MAX);

            len = wcstombs(buf, info.FileName, info.FileNameLength * MB_CUR_MAX);
            lydict_insert(ctx, buf, len, filepath);
        }
#else
        /* get URI if there is /proc */
        sprintf(proc_path, "/proc/self/fd/%d", in->method.fd);
        if ((len = readlink(proc_path, path, PATH_MAX - 1)) > 0) {
            lydict_insert(ctx, path, len, filepath);
        }
#endif
        break;
    case LY_IN_MEMORY:
    case LY_IN_FILE:
        /* nothing to do */
        break;
    default:
        LOGINT(ctx);
        break;
    }
}

static LY_ERR lysp_stmt_container(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_choice(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_case(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_uses(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);
static LY_ERR lysp_stmt_grouping(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_grp **groupings);
static LY_ERR lysp_stmt_list(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings);

/**
 * @brief Validate stmt string value.
 *
 * @param[in] ctx Parser context.
 * @param[in] val_type String value type.
 * @param[in] val Value to validate.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_stmt_validate_value(struct lysp_ctx *ctx, enum yang_arg val_type, const char *val)
{
    uint8_t prefix = 0;
    ly_bool first = 1;
    uint32_t c;
    size_t utf8_char_len;

    if (!val) {
        if (val_type == Y_MAYBE_STR_ARG) {
            /* fine */
            return LY_SUCCESS;
        }

        LOGVAL_PARSER(ctx, LYVE_SYNTAX, "Missing an expected string.");
        return LY_EVALID;
    }

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
 * @brief Duplicate statement siblings, recursively.
 *
 * @param[in] ctx Parser context.
 * @param[in] stmt Statement to duplicate.
 * @param[out] first First duplicated statement, the rest follow.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_stmt_dup(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_stmt **first)
{
    struct lysp_stmt *child, *last = NULL;

    LY_LIST_FOR(stmt, stmt) {
        child = calloc(1, sizeof *child);
        LY_CHECK_ERR_RET(!child, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

        if (last) {
            last->next = child;
        } else {
            assert(!*first);
            *first = child;
        }
        last = child;

        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->stmt, 0, &child->stmt));
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &child->arg));
        child->format = stmt->format;
        LY_CHECK_RET(ly_dup_prefix_data(PARSER_CTX(ctx), stmt->format, stmt->prefix_data, &child->prefix_data));
        child->flags = stmt->flags;
        child->kw = stmt->kw;

        /* recursively */
        LY_CHECK_RET(lysp_stmt_dup(ctx, stmt->child, &child->child));
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_ext(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt insubstmt,
        LY_ARRAY_COUNT_TYPE insubstmt_index, struct lysp_ext_instance **exts)
{
    struct lysp_ext_instance *e;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *exts, e, LY_EMEM);

    /* store name and insubstmt info */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->stmt, 0, &e->name));
    e->parent_stmt = insubstmt;
    e->parent_stmt_index = insubstmt_index;
    e->parsed = NULL;
    LY_CHECK_RET(lysp_stmt_dup(ctx, stmt->child, &e->child));

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
lysp_stmt_text_field(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint32_t substmt_index, const char **value,
        enum yang_arg arg, struct lysp_ext_instance **exts)
{
    if (*value) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(stmt->kw));
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_qnames(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_qname **qnames, enum yang_arg arg,
        struct lysp_ext_instance **exts)
{
    struct lysp_qname *item;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *qnames, item, LY_EMEM);
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &item->str));
    item->mod = PARSER_CUR_PMOD(ctx);

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, LY_ARRAY_COUNT(*qnames) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_text_fields(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, const char ***texts, enum yang_arg arg,
        struct lysp_ext_instance **exts)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_status(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    int arg_len;

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "status");
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
lysp_stmt_when(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_when **when_p)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "when");
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
lysp_stmt_config(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    int arg_len;

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "config");
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
lysp_stmt_mandatory(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    int arg_len;

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "mandatory");
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
lysp_stmt_restr(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr *restr)
{
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &restr->arg.str));
    restr->arg.mod = PARSER_CUR_PMOD(ctx);

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_restrs(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr **restrs)
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
lysp_stmt_any(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw),
                    (any->nodetype & LYS_ANYDATA) == LYS_ANYDATA ? lyplg_ext_stmt2str(LY_STMT_ANYDATA) : lyplg_ext_stmt2str(LY_STMT_ANYXML));
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
lysp_stmt_type_enum_value_pos(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, int64_t *value, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    int arg_len;
    char *ptr = NULL;
    long long num = 0;
    unsigned long long unum = 0;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(stmt->kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    arg_len = strlen(stmt->arg);
    if (!arg_len || (stmt->arg[0] == '+') || ((stmt->arg[0] == '0') && (arg_len > 1)) ||
            ((stmt->kw == LY_STMT_POSITION) && !strncmp(stmt->arg, "-0", 2))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, lyplg_ext_stmt2str(stmt->kw));
        goto error;
    }

    errno = 0;
    if (stmt->kw == LY_STMT_VALUE) {
        num = strtoll(stmt->arg, &ptr, LY_BASE_DEC);
        if ((num < INT64_C(-2147483648)) || (num > INT64_C(2147483647))) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, lyplg_ext_stmt2str(stmt->kw));
            goto error;
        }
    } else {
        unum = strtoull(stmt->arg, &ptr, LY_BASE_DEC);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, lyplg_ext_stmt2str(stmt->kw));
            goto error;
        }
    }
    /* we have not parsed the whole argument */
    if (ptr - stmt->arg != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, lyplg_ext_stmt2str(stmt->kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, lyplg_ext_stmt2str(stmt->kw));
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_type_enum(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_type_enum **enums)
{
    struct lysp_type_enum *enm;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, stmt->kw == LY_STMT_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, stmt->arg));

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *enums, enm, LY_EMEM);

    if (stmt->kw == LY_STMT_ENUM) {
        LY_CHECK_RET(lysp_check_enum_name(ctx, stmt->arg, strlen(stmt->arg)));
    } /* else nothing specific for YANG_BIT */

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &enm->name));
    CHECK_UNIQUENESS(ctx, *enums, name, lyplg_ext_stmt2str(stmt->kw), enm->name);

    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &enm->dsc, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", lyplg_ext_stmt2str(stmt->kw));
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &enm->ref, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &enm->flags, &enm->exts));
            break;
        case LY_STMT_VALUE:
            LY_CHECK_ERR_RET(stmt->kw == LY_STMT_BIT, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw),
                    lyplg_ext_stmt2str(stmt->kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_POSITION:
            LY_CHECK_ERR_RET(stmt->kw == LY_STMT_ENUM, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw),
                    lyplg_ext_stmt2str(stmt->kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &enm->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_type_fracdigits(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *fracdig,
        struct lysp_ext_instance **exts)
{
    char *ptr;
    int arg_len;
    unsigned long long num;

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
    num = strtoull(stmt->arg, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if (ptr - stmt->arg != arg_len) {
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "fraction-digits");
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
lysp_stmt_type_reqinstance(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *reqinst, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    int arg_len;

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "require-instance");
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_pattern_modifier(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, const char **pat,
        struct lysp_ext_instance **exts)
{
    int arg_len;
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "modifier");
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
lysp_stmt_type_pattern(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr **patterns)
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
    restr->arg.mod = PARSER_CUR_PMOD(ctx);

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "pattern");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the deviate statement. Substatement of deviation statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] devs Array of deviates to add to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_deviate(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_deviate **devs, struct lysp_ext_instance **exts)
{
    (void)stmt;
    (void)devs;
    (void)exts;

    /* TODO */
    LOGERR(PARSER_CTX(ctx), LY_EINVAL, "Extension instance \"deviate\" substatement is not supported.");
    return LY_EINVAL;
}

/**
 * @brief Parse the deviation statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] deviations Array of deviations to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_deviation(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_deviation **deviations)
{
    struct lysp_deviation *dev;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *deviations, dev, LY_EMEM);

    /* store nodeid */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &dev->nodeid));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &dev->dsc, Y_STR_ARG, &dev->exts));
            break;
        case LY_STMT_DEVIATE:
            LY_CHECK_RET(lysp_stmt_deviate(ctx, child, &dev->deviates, &dev->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &dev->ref, Y_STR_ARG, &dev->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &dev->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_DEVIATION));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the yang-version statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[out] version Version to write to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_yangver(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *version, struct lysp_ext_instance **exts)
{
    if (*version) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* store flag */
    if (!strcmp(stmt->arg, "1")) {
        *version = LYS_VERSION_1_0;
    } else if (!strcmp(stmt->arg, "1.1")) {
        *version = LYS_VERSION_1_1;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)strlen(stmt->arg), stmt->arg, "yang-version");
        return LY_EVALID;
    }

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_YANG_VERSION));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the module statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] mod Module to fill.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_module(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_module *mod)
{
    (void)stmt;
    (void)mod;

    /* TODO */
    LOGERR(PARSER_CTX(ctx), LY_EINVAL, "Extension instance \"module\" substatement is not supported.");
    return LY_EINVAL;
}

/**
 * @brief Parse the submodule statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] submod Module to fill.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_submodule(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_submodule *submod)
{
    (void)stmt;
    (void)submod;

    /* TODO */
    LOGERR(PARSER_CTX(ctx), LY_EINVAL, "Extension instance \"submodule\" substatement is not supported.");
    return LY_EINVAL;
}

/**
 * @brief Parse the yin-element statement. Substatement of argument statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_yinelem(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    if (*flags & LYS_YINELEM_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* store flag */
    if (!strcmp(stmt->arg, "true")) {
        *flags |= LYS_YINELEM_TRUE;
    } else if (!strcmp(stmt->arg, "false")) {
        *flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)strlen(stmt->arg), stmt->arg, "yin-element");
        return LY_EVALID;
    }

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_YIN_ELEMENT));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the argument statement. Substatement of extension statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] ex Extension to fill.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_argument(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_ext *ex)
{
    if (ex->argname) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "argument");
        return LY_EVALID;
    }

    /* store argument name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_PREF_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &ex->argname));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_YIN_ELEMENT:
            LY_CHECK_RET(lysp_stmt_yinelem(ctx, child, &ex->flags, &ex->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &ex->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_ARGUMENT));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the extension statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] extensions Array of extensions to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_extension(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_ext **extensions)
{
    struct lysp_ext *ex;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *extensions, ex, LY_EMEM);

    /* store name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &ex->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &ex->dsc, Y_STR_ARG, &ex->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &ex->ref, Y_STR_ARG, &ex->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &ex->flags, &ex->exts));
            break;
        case LY_STMT_ARGUMENT:
            LY_CHECK_RET(lysp_stmt_argument(ctx, child, ex));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &ex->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_EXTENSION));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the feature statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] features Array of features to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_feature(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_feature **features)
{
    struct lysp_feature *feat;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *features, feat, LY_EMEM);

    /* store name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &feat->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &feat->dsc, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &feat->iffeatures, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &feat->ref, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &feat->flags, &feat->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &feat->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_FEATURE));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the identity statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] identities Array of identities to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_identity(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_ident **identities)
{
    struct lysp_ident *ident;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *identities, ident, LY_EMEM);

    /* store name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &ident->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &ident->dsc, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, &ident->iffeatures, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &ident->ref, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &ident->flags, &ident->exts));
            break;
        case LY_STMT_BASE:
            LY_CHECK_RET(lysp_stmt_text_fields(ctx, child, &ident->bases, Y_PREF_IDENTIF_ARG, &ident->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &ident->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_IDENTITY));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the import statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] imports Array of imports to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_import(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_import **imports)
{
    struct lysp_import *imp;
    const char *str = NULL;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *imports, imp, LY_EMEM);

    /* store name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &imp->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_PREFIX:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &imp->prefix, Y_IDENTIF_ARG, &imp->exts));
            LY_CHECK_RET(lysp_check_prefix(ctx, *imports, NULL, &imp->prefix), LY_EVALID);
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &imp->dsc, Y_STR_ARG, &imp->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &imp->ref, Y_STR_ARG, &imp->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, stmt, 0, &str, Y_STR_ARG, &imp->exts));
            strcpy(imp->rev, str);
            lydict_remove(PARSER_CTX(ctx), str);
            LY_CHECK_RET(lysp_check_date(ctx, imp->rev, LY_REV_SIZE - 1, "revision-date"));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &imp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_IMPORT));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the include statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] includes Array of identities to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_include(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_include **includes)
{
    struct lysp_include *inc;
    const char *str = NULL;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *includes, inc, LY_EMEM);

    /* store name */
    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_IDENTIF_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &inc->name));

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &inc->dsc, Y_STR_ARG, &inc->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &inc->ref, Y_STR_ARG, &inc->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, stmt, 0, &str, Y_STR_ARG, &inc->exts));
            strcpy(inc->rev, str);
            lydict_remove(PARSER_CTX(ctx), str);
            LY_CHECK_RET(lysp_check_date(ctx, inc->rev, LY_REV_SIZE - 1, "revision-date"));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &inc->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_INCLUDE));
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse the revision statement.
 *
 * @param[in] ctx parser context.
 * @param[in] stmt Source statement data from the parsed extension instance.
 * @param[in,out] includes Array of identities to add to.
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_revision(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_revision **revs)
{
    struct lysp_revision *rev;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *revs, rev, LY_EMEM);

    /* store date */
    LY_CHECK_RET(lysp_check_date(ctx, stmt->arg, strlen(stmt->arg), "revision"));
    strncpy(rev->date, stmt->arg, LY_REV_SIZE - 1);

    /* parse substatements */
    for (const struct lysp_stmt *child = stmt->child; child; child = child->next) {
        switch (child->kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &rev->dsc, Y_STR_ARG, &rev->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &rev->ref, Y_STR_ARG, &rev->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &rev->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(LY_STMT_REVISION));
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
lysp_stmt_type(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_type *type)
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
    type->pmod = PARSER_CUR_PMOD(ctx);

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
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(child->kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(lysp_stmt_restr(ctx, child, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case LY_STMT_PATH:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, 0, &str_path, Y_STR_ARG, &type->exts));
            ret = ly_path_parse(PARSER_CTX(ctx), NULL, str_path, 0, 1, LY_PATH_BEGIN_EITHER,
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
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(child->kw));
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "type");
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
lysp_stmt_leaf(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent, struct lysp_node **siblings)
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
            leaf->dflt.mod = PARSER_CUR_PMOD(ctx);
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "leaf");
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
lysp_stmt_maxelements(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint32_t *max, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    int arg_len;
    char *ptr;
    unsigned long long num;

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
        num = strtoull(stmt->arg, &ptr, LY_BASE_DEC);
        /* we have not parsed the whole argument */
        if (ptr - stmt->arg != arg_len) {
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "max-elements");
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_minelements(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint32_t *min, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    int arg_len;
    char *ptr;
    unsigned long long num;

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
    num = strtoull(stmt->arg, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if (ptr - stmt->arg != arg_len) {
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "min-elements");
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_orderedby(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    int arg_len;

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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "ordered-by");
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_leaflist(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "llist");
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
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_refine(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_refine **refines)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "refine");
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
lysp_stmt_typedef(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_tpdf **typedefs)
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
            tpdf->dflt.mod = PARSER_CUR_PMOD(ctx);
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "typedef");
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
lysp_stmt_inout(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_action_inout *inout_p)
{
    if (inout_p->nodetype) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(stmt->kw));
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
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", lyplg_ext_stmt2str(stmt->kw));
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
            PARSER_CHECK_STMTVER2_RET(ctx, "must", lyplg_ext_stmt2str(stmt->kw));
            LY_CHECK_RET(lysp_stmt_restrs(ctx, child, &inout_p->musts));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(lysp_stmt_grouping(ctx, child, &inout_p->node, &inout_p->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, stmt->kw, 0, &inout_p->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), lyplg_ext_stmt2str(stmt->kw));
            return LY_EVALID;
        }
    }

    if (!inout_p->child) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "data-def-stmt", lyplg_ext_stmt2str(stmt->kw));
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
lysp_stmt_action(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_action **actions)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), parent ? "action" : "rpc");
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
lysp_stmt_notif(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_notif **notifs)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "notification");
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
lysp_stmt_grouping(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_grp **groupings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "grouping");
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
lysp_stmt_augment(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node_augment **augments)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "augment");
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
lysp_stmt_uses(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "uses");
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
lysp_stmt_case(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "case");
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
lysp_stmt_choice(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            choice->dflt.mod = PARSER_CUR_PMOD(ctx);
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "choice");
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
lysp_stmt_container(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "container");
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
lysp_stmt_list(struct lysp_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_node *parent,
        struct lysp_node **siblings)
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
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(child->kw), "list");
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse generic statement structure into a specific parsed-schema structure.
 *
 * @param[in] pctx Parse context of the @p stmt being processed.
 * @param[in] stmt Generic statement structure to process.
 * @param[out] result Specific parsed-schema structure for the given statement. For the specific type for the particular statement, check the function code.
 * @param[in,out] exts [sized array](@ref sizedarrays) For extension instances in case of statements that do not store extension instances in their own list.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_stmt_parse(struct lysp_ctx *pctx, const struct lysp_stmt *stmt, void **result, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    uint16_t flags;

    switch (stmt->kw) {
    case LY_STMT_NOTIFICATION:
        ret = lysp_stmt_notif(pctx, stmt, NULL, (struct lysp_node_notif **)result);
        break;
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT: {
        struct lysp_node_action_inout *inout;

        *result = inout = calloc(1, sizeof *inout);
        LY_CHECK_ERR_RET(!inout, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        ret = lysp_stmt_inout(pctx, stmt, NULL, inout);
        break;
    }
    case LY_STMT_ACTION:
    case LY_STMT_RPC:
        ret = lysp_stmt_action(pctx, stmt, NULL, (struct lysp_node_action **)result);
        break;
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
        ret = lysp_stmt_any(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_AUGMENT:
        ret = lysp_stmt_augment(pctx, stmt, NULL, (struct lysp_node_augment **)result);
        break;
    case LY_STMT_CASE:
        ret = lysp_stmt_case(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_CHOICE:
        ret = lysp_stmt_choice(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_CONTAINER:
        ret = lysp_stmt_container(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_GROUPING:
        ret = lysp_stmt_grouping(pctx, stmt, NULL, (struct lysp_node_grp **)result);
        break;
    case LY_STMT_LEAF:
        ret = lysp_stmt_leaf(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_LEAF_LIST:
        ret = lysp_stmt_leaflist(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_LIST:
        ret = lysp_stmt_list(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_USES:
        ret = lysp_stmt_uses(pctx, stmt, NULL, (struct lysp_node **)result);
        break;
    case LY_STMT_BASE:
        ret = lysp_stmt_text_fields(pctx, stmt, (const char ***)result, Y_PREF_IDENTIF_ARG, exts);
        break;
    case LY_STMT_ARGUMENT:
    case LY_STMT_BELONGS_TO:
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_ERROR_APP_TAG:
    case LY_STMT_ERROR_MESSAGE:
    case LY_STMT_KEY:
    case LY_STMT_NAMESPACE:
    case LY_STMT_ORGANIZATION:
    case LY_STMT_PRESENCE:
    case LY_STMT_REFERENCE:
    case LY_STMT_REVISION_DATE:
    case LY_STMT_UNITS:
        ret = lysp_stmt_text_field(pctx, stmt, 0, (const char **)result, Y_STR_ARG, exts);
        break;
    case LY_STMT_BIT:
    case LY_STMT_ENUM:
        ret = lysp_stmt_type_enum(pctx, stmt, (struct lysp_type_enum **)result);
        break;
    case LY_STMT_CONFIG:
        assert(*result);
        ret = lysp_stmt_config(pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_DEFAULT:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_UNIQUE:
        ret = lysp_stmt_qnames(pctx, stmt, (struct lysp_qname **)result, Y_STR_ARG, exts);
        break;
    case LY_STMT_DEVIATE:
        ret = lysp_stmt_deviate(pctx, stmt, (struct lysp_deviate **)result, exts);
        break;
    case LY_STMT_DEVIATION:
        ret = lysp_stmt_deviation(pctx, stmt, (struct lysp_deviation **)result);
        break;
    case LY_STMT_EXTENSION:
        ret = lysp_stmt_extension(pctx, stmt, (struct lysp_ext **)result);
        break;
    case LY_STMT_EXTENSION_INSTANCE:
        ret = lysp_stmt_ext(pctx, stmt, LY_STMT_EXTENSION_INSTANCE, 0, (struct lysp_ext_instance **)result);
        break;
    case LY_STMT_FEATURE:
        ret = lysp_stmt_feature(pctx, stmt, (struct lysp_feature **)result);
        break;
    case LY_STMT_FRACTION_DIGITS:
        ret = lysp_stmt_type_fracdigits(pctx, stmt, *(uint8_t **)result, exts);
        break;
    case LY_STMT_LENGTH:
    case LY_STMT_RANGE: {
        struct lysp_restr *restr;

        *result = restr = calloc(1, sizeof *restr);
        LY_CHECK_ERR_RET(!restr, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);

        ret = lysp_stmt_restr(pctx, stmt, restr);
        break;
    }
    case LY_STMT_MUST:
        ret = lysp_stmt_restrs(pctx, stmt, (struct lysp_restr **)result);
        break;
    case LY_STMT_IDENTITY:
        ret = lysp_stmt_identity(pctx, stmt, (struct lysp_ident **)result);
        break;
    case LY_STMT_IMPORT:
        ret = lysp_stmt_import(pctx, stmt, (struct lysp_import **)result);
        break;
    case LY_STMT_INCLUDE:
        ret = lysp_stmt_include(pctx, stmt, (struct lysp_include **)result);
        break;
    case LY_STMT_MANDATORY:
        ret = lysp_stmt_mandatory(pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_MAX_ELEMENTS:
        flags = 0;
        ret = lysp_stmt_maxelements(pctx, stmt, *(uint32_t **)result, &flags, exts);
        break;
    case LY_STMT_MIN_ELEMENTS:
        flags = 0;
        ret = lysp_stmt_minelements(pctx, stmt, *(uint32_t **)result, &flags, exts);
        break;
    case LY_STMT_MODIFIER:
        ret = lysp_stmt_type_pattern_modifier(pctx, stmt, (const char **)result, exts);
        break;
    case LY_STMT_MODULE: {
        struct lysp_module *mod;

        *result = mod = calloc(1, sizeof *mod);
        LY_CHECK_ERR_RET(!mod, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        ret = lysp_stmt_module(pctx, stmt, mod);
        break;
    }
    case LY_STMT_ORDERED_BY:
        ret = lysp_stmt_orderedby(pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_PATH: {
        const char *str_path = NULL;

        LY_CHECK_RET(lysp_stmt_text_field(pctx, stmt, 0, &str_path, Y_STR_ARG, exts));
        ret = ly_path_parse(PARSER_CTX(pctx), NULL, str_path, 0, 1, LY_PATH_BEGIN_EITHER,
                LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, (struct lyxp_expr **)result);
        lydict_remove(PARSER_CTX(pctx), str_path);
        break;
    }
    case LY_STMT_PATTERN:
        ret = lysp_stmt_type_pattern(pctx, stmt, (struct lysp_restr **)result);
        break;
    case LY_STMT_POSITION:
    case LY_STMT_VALUE:
        flags = 0;
        ret = lysp_stmt_type_enum_value_pos(pctx, stmt, *(int64_t **)result, &flags, exts);
        break;
    case LY_STMT_PREFIX:
        ret = lysp_stmt_text_field(pctx, stmt, 0, (const char **)result, Y_IDENTIF_ARG, exts);
        break;
    case LY_STMT_REFINE:
        ret = lysp_stmt_refine(pctx, stmt, (struct lysp_refine **)result);
        break;
    case LY_STMT_REQUIRE_INSTANCE:
        flags = 0;
        ret = lysp_stmt_type_reqinstance(pctx, stmt, *(uint8_t **)result, &flags, exts);
        break;
    case LY_STMT_REVISION:
        ret = lysp_stmt_revision(pctx, stmt, (struct lysp_revision **)result);
        break;
    case LY_STMT_STATUS:
        ret = lysp_stmt_status(pctx, stmt, (uint16_t *)result, exts);
        break;
    case LY_STMT_SUBMODULE: {
        struct lysp_submodule *submod;

        *result = submod = calloc(1, sizeof *submod);
        LY_CHECK_ERR_RET(!submod, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        ret = lysp_stmt_submodule(pctx, stmt, submod);
        break;
    }
    case LY_STMT_TYPE: {
        struct lysp_type *type;

        *result = type = calloc(1, sizeof *type);
        LY_CHECK_ERR_RET(!type, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        ret = lysp_stmt_type(pctx, stmt, type);
        break;
    }
    case LY_STMT_TYPEDEF:
        ret = lysp_stmt_typedef(pctx, stmt, NULL, (struct lysp_tpdf **)result);
        break;
    case LY_STMT_WHEN:
        ret = lysp_stmt_when(pctx, stmt, (struct lysp_when **)result);
        break;
    case LY_STMT_YANG_VERSION:
        ret = lysp_stmt_yangver(pctx, stmt, *(uint8_t **)result, exts);
        break;
    case LY_STMT_YIN_ELEMENT:
        ret = lysp_stmt_yinelem(pctx, stmt, *(uint16_t **)result, exts);
        break;
    default:
        LOGINT(PARSER_CTX(pctx));
        return LY_EINT;
    }

    return ret;
}

LY_ERR
lys_parse_ext_instance_stmt(struct lysp_ctx *pctx, struct lysp_ext_substmt *substmt, struct lysp_stmt *stmt)
{
    LY_ERR rc = LY_SUCCESS;

    if (!substmt->storage) {
        /* nothing to parse, ignored */
        goto cleanup;
    }

    switch (stmt->kw) {
    case LY_STMT_NOTIFICATION:
    case LY_STMT_INPUT:
    case LY_STMT_OUTPUT:
    case LY_STMT_ACTION:
    case LY_STMT_RPC:
    case LY_STMT_ANYDATA:
    case LY_STMT_ANYXML:
    case LY_STMT_AUGMENT:
    case LY_STMT_CASE:
    case LY_STMT_CHOICE:
    case LY_STMT_CONTAINER:
    case LY_STMT_GROUPING:
    case LY_STMT_LEAF:
    case LY_STMT_LEAF_LIST:
    case LY_STMT_LIST:
    case LY_STMT_USES: {
        struct lysp_node **pnodes_p, *pnode = NULL;

        /* parse the node */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, (void **)&pnode, NULL), cleanup);

        /* usually is a linked-list of all the parsed schema nodes */
        pnodes_p = substmt->storage;
        while (*pnodes_p) {
            pnodes_p = &(*pnodes_p)->next;
        }
        *pnodes_p = pnode;

        break;
    }
    case LY_STMT_BASE:
    case LY_STMT_BIT:
    case LY_STMT_DEFAULT:
    case LY_STMT_DEVIATE:
    case LY_STMT_DEVIATION:
    case LY_STMT_ENUM:
    case LY_STMT_EXTENSION:
    case LY_STMT_EXTENSION_INSTANCE:
    case LY_STMT_FEATURE:
    case LY_STMT_IDENTITY:
    case LY_STMT_IF_FEATURE:
    case LY_STMT_IMPORT:
    case LY_STMT_INCLUDE:
    case LY_STMT_MUST:
    case LY_STMT_PATTERN:
    case LY_STMT_REFINE:
    case LY_STMT_REVISION:
    case LY_STMT_TYPEDEF:
    case LY_STMT_UNIQUE:
        /* parse, sized array */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, substmt->storage, NULL), cleanup);
        break;

    case LY_STMT_ARGUMENT:
    case LY_STMT_BELONGS_TO:
    case LY_STMT_CONTACT:
    case LY_STMT_DESCRIPTION:
    case LY_STMT_ERROR_APP_TAG:
    case LY_STMT_ERROR_MESSAGE:
    case LY_STMT_FRACTION_DIGITS:
    case LY_STMT_KEY:
    case LY_STMT_LENGTH:
    case LY_STMT_MANDATORY:
    case LY_STMT_MAX_ELEMENTS:
    case LY_STMT_MIN_ELEMENTS:
    case LY_STMT_MODIFIER:
    case LY_STMT_MODULE:
    case LY_STMT_NAMESPACE:
    case LY_STMT_ORGANIZATION:
    case LY_STMT_PATH:
    case LY_STMT_POSITION:
    case LY_STMT_PREFIX:
    case LY_STMT_PRESENCE:
    case LY_STMT_RANGE:
    case LY_STMT_REFERENCE:
    case LY_STMT_REQUIRE_INSTANCE:
    case LY_STMT_REVISION_DATE:
    case LY_STMT_SUBMODULE:
    case LY_STMT_TYPE:
    case LY_STMT_UNITS:
    case LY_STMT_VALUE:
    case LY_STMT_WHEN:
    case LY_STMT_YANG_VERSION:
    case LY_STMT_YIN_ELEMENT:
        /* single item */
        if (*(void **)substmt->storage) {
            LOGVAL(PARSER_CTX(pctx), LY_VCODE_DUPSTMT, stmt->stmt);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* parse */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, substmt->storage, NULL), cleanup);
        break;

    case LY_STMT_CONFIG:
        /* single item */
        if ((*(uint16_t *)substmt->storage) & LYS_CONFIG_MASK) {
            LOGVAL(PARSER_CTX(pctx), LY_VCODE_DUPSTMT, stmt->stmt);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* parse */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, substmt->storage, NULL), cleanup);
        break;

    case LY_STMT_ORDERED_BY:
        /* single item */
        if ((*(uint16_t *)substmt->storage) & LYS_ORDBY_MASK) {
            LOGVAL(PARSER_CTX(pctx), LY_VCODE_DUPSTMT, stmt->stmt);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* parse */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, substmt->storage, NULL), cleanup);
        break;

    case LY_STMT_STATUS:
        /* single item */
        if ((*(uint16_t *)substmt->storage) & LYS_STATUS_MASK) {
            LOGVAL(PARSER_CTX(pctx), LY_VCODE_DUPSTMT, stmt->stmt);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* parse */
        LY_CHECK_GOTO(rc = lysp_stmt_parse(pctx, stmt, substmt->storage, NULL), cleanup);
        break;

    default:
        LOGINT(PARSER_CTX(pctx));
        rc = LY_EINT;
        goto cleanup;
    }

cleanup:
    return rc;
}
