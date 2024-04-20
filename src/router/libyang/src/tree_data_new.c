/**
 * @file tree_data_new.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Data tree new functions
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

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "diff.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "path.h"
#include "plugins.h"
#include "plugins_exts/metadata.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"
#include "xpath.h"

LY_ERR
lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, ly_bool is_utf8, ly_bool *dynamic,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, ly_bool *incomplete, struct lyd_node **node)
{
    LY_ERR ret;
    struct lyd_node_term *term;

    assert(schema->nodetype & LYD_NODE_TERM);

    term = calloc(1, sizeof *term);
    LY_CHECK_ERR_RET(!term, LOGMEM(schema->module->ctx), LY_EMEM);

    term->schema = schema;
    term->prev = &term->node;
    term->flags = LYD_NEW;

    LOG_LOCSET(schema, NULL, NULL, NULL);
    ret = lyd_value_store(schema->module->ctx, &term->value, ((struct lysc_node_leaf *)term->schema)->type, value,
            value_len, is_utf8, dynamic, format, prefix_data, hints, schema, incomplete);
    LOG_LOCBACK(1, 0, 0, 0);
    LY_CHECK_ERR_RET(ret, free(term), ret);
    lyd_hash(&term->node);

    *node = &term->node;
    return ret;
}

LY_ERR
lyd_create_term2(const struct lysc_node *schema, const struct lyd_value *val, struct lyd_node **node)
{
    LY_ERR ret;
    struct lyd_node_term *term;
    struct lysc_type *type;

    assert(schema->nodetype & LYD_NODE_TERM);
    assert(val && val->realtype);

    term = calloc(1, sizeof *term);
    LY_CHECK_ERR_RET(!term, LOGMEM(schema->module->ctx), LY_EMEM);

    term->schema = schema;
    term->prev = &term->node;
    term->flags = LYD_NEW;

    type = ((struct lysc_node_leaf *)schema)->type;
    ret = type->plugin->duplicate(schema->module->ctx, val, &term->value);
    if (ret) {
        LOGERR(schema->module->ctx, ret, "Value duplication failed.");
        free(term);
        return ret;
    }
    lyd_hash(&term->node);

    *node = &term->node;
    return ret;
}

LY_ERR
lyd_create_inner(const struct lysc_node *schema, struct lyd_node **node)
{
    struct lyd_node_inner *in;

    assert(schema->nodetype & LYD_NODE_INNER);

    in = calloc(1, sizeof *in);
    LY_CHECK_ERR_RET(!in, LOGMEM(schema->module->ctx), LY_EMEM);

    in->schema = schema;
    in->prev = &in->node;
    in->flags = LYD_NEW;
    if ((schema->nodetype == LYS_CONTAINER) && !(schema->flags & LYS_PRESENCE)) {
        in->flags |= LYD_DEFAULT;
    }

    /* do not hash list with keys, we need them for the hash */
    if ((schema->nodetype != LYS_LIST) || (schema->flags & LYS_KEYLESS)) {
        lyd_hash(&in->node);
    }

    *node = &in->node;
    return LY_SUCCESS;
}

LY_ERR
lyd_create_list(const struct lysc_node *schema, const struct ly_path_predicate *predicates, const struct lyxp_var *vars,
        struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *list = NULL, *key;
    const struct lyd_value *value;
    struct lyd_value val = {0};
    struct lyxp_var *var;
    LY_ARRAY_COUNT_TYPE u;

    assert((schema->nodetype == LYS_LIST) && !(schema->flags & LYS_KEYLESS));

    /* create list */
    LY_CHECK_GOTO(ret = lyd_create_inner(schema, &list), cleanup);

    LOG_LOCSET(schema, NULL, NULL, NULL);

    /* create and insert all the keys */
    LY_ARRAY_FOR(predicates, u) {
        if (predicates[u].type == LY_PATH_PREDTYPE_LIST_VAR) {
            /* find the var */
            if ((ret = lyxp_vars_find(schema->module->ctx, vars, predicates[u].variable, 0, &var))) {
                goto cleanup;
            }

            /* store the value */
            LOG_LOCSET(predicates[u].key, NULL, NULL, NULL);
            ret = lyd_value_store(schema->module->ctx, &val, ((struct lysc_node_leaf *)predicates[u].key)->type,
                    var->value, strlen(var->value), 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, predicates[u].key, NULL);
            LOG_LOCBACK(1, 0, 0, 0);
            LY_CHECK_GOTO(ret, cleanup);

            value = &val;
        } else {
            assert(predicates[u].type == LY_PATH_PREDTYPE_LIST);
            value = &predicates[u].value;
        }

        ret = lyd_create_term2(predicates[u].key, value, &key);
        if (val.realtype) {
            val.realtype->plugin->free(schema->module->ctx, &val);
            memset(&val, 0, sizeof val);
        }
        LY_CHECK_GOTO(ret, cleanup);
        lyd_insert_node(list, NULL, key, 0);
    }

    /* hash having all the keys */
    lyd_hash(list);

    /* success */
    *node = list;
    list = NULL;

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    lyd_free_tree(list);
    return ret;
}

LY_ERR
lyd_create_list2(const struct lysc_node *schema, const char *keys, size_t keys_len, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *expr = NULL;
    uint32_t exp_idx = 0;
    struct ly_path_predicate *predicates = NULL;

    LOG_LOCSET(schema, NULL, NULL, NULL);

    /* parse keys */
    LY_CHECK_GOTO(ret = ly_path_parse_predicate(schema->module->ctx, NULL, keys, keys_len, LY_PATH_PREFIX_OPTIONAL,
            LY_PATH_PRED_KEYS, &expr), cleanup);

    /* compile them */
    LY_CHECK_GOTO(ret = ly_path_compile_predicate(schema->module->ctx, NULL, NULL, schema, expr, &exp_idx, LY_VALUE_JSON,
            NULL, &predicates), cleanup);

    /* create the list node */
    LY_CHECK_GOTO(ret = lyd_create_list(schema, predicates, NULL, node), cleanup);

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    lyxp_expr_free(schema->module->ctx, expr);
    ly_path_predicates_free(schema->module->ctx, predicates);
    return ret;
}

/**
 * @brief Learn actual any value type in case it is currently ::LYD_ANYDATA_STRING.
 *
 * @param[in] value Any value.
 * @param[out] value_type Detected value type.
 */
static void
lyd_create_any_string_valtype(const void *value, LYD_ANYDATA_VALUETYPE *value_type)
{
    /* detect format */
    if (!value) {
        /* interpret it as an empty data tree */
        *value_type = LYD_ANYDATA_DATATREE;
    } else if (((char *)value)[0] == '<') {
        *value_type = LYD_ANYDATA_XML;
    } else if (((char *)value)[0] == '{') {
        *value_type = LYD_ANYDATA_JSON;
    } else if (!strncmp(value, "lyb", 3)) {
        *value_type = LYD_ANYDATA_LYB;
    } else {
        /* really just some string */
        *value_type = LYD_ANYDATA_STRING;
    }
}

/**
 * @brief Convert an any value into a datatree.
 *
 * @param[in] ctx libyang context.
 * @param[in] value_in Any value as an input.
 * @param[in] value_type Any @p value type.
 * @param[in] log Whether parsing errors should be logged.
 * @param[out] tree Parsed data tree.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_create_any_datatree(const struct ly_ctx *ctx, struct ly_in *value_in, LYD_ANYDATA_VALUETYPE value_type, ly_bool log,
        struct lyd_node **tree)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_ctx *lydctx = NULL;
    uint32_t parse_opts, int_opts, log_opts = 0;

    *tree = NULL;

    /* set options */
    parse_opts = LYD_PARSE_ONLY | LYD_PARSE_OPAQ;
    int_opts = LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS;

    if (!log) {
        /* no logging */
        ly_temp_log_options(&log_opts);
    }

    switch (value_type) {
    case LYD_ANYDATA_DATATREE:
    case LYD_ANYDATA_STRING:
        /* unreachable */
        LOGINT_RET(ctx);
    case LYD_ANYDATA_XML:
        rc = lyd_parse_xml(ctx, NULL, NULL, tree, value_in, parse_opts, 0, int_opts, NULL, NULL, &lydctx);
        break;
    case LYD_ANYDATA_JSON:
        rc = lyd_parse_json(ctx, NULL, NULL, tree, value_in, parse_opts, 0, int_opts, NULL, NULL, &lydctx);
        break;
    case LYD_ANYDATA_LYB:
        rc = lyd_parse_lyb(ctx, NULL, NULL, tree, value_in, parse_opts | LYD_PARSE_STRICT, 0, int_opts, NULL, NULL, &lydctx);
        break;
    }
    if (lydctx) {
        lydctx->free(lydctx);
    }

    if (!log) {
        /* restore logging */
        ly_temp_log_options(NULL);
    }
    if (rc && *tree) {
        lyd_free_siblings(*tree);
        *tree = NULL;
    }
    return rc;
}

LY_ERR
lyd_create_any(const struct lysc_node *schema, const void *value, LYD_ANYDATA_VALUETYPE value_type, ly_bool use_value,
        struct lyd_node **node)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lyd_node *tree;
    struct lyd_node_any *any = NULL;
    union lyd_any_value any_val;
    struct ly_in *in = NULL;

    assert(schema->nodetype & LYD_NODE_ANY);

    any = calloc(1, sizeof *any);
    LY_CHECK_ERR_RET(!any, LOGMEM(schema->module->ctx), LY_EMEM);

    any->schema = schema;
    any->prev = &any->node;
    any->flags = LYD_NEW;

    if (schema->nodetype == LYS_ANYDATA) {
        /* anydata */
        if (value_type == LYD_ANYDATA_STRING) {
            /* detect value type */
            lyd_create_any_string_valtype(value, &value_type);
        }

        if (value_type != LYD_ANYDATA_DATATREE) {
            /* create input */
            assert(value);
            LY_CHECK_GOTO(rc = ly_in_new_memory(value, &in), cleanup);

            /* parse as a data tree */
            if ((r = lyd_create_any_datatree(schema->module->ctx, in, value_type, 1, &tree))) {
                LOGERR(schema->module->ctx, rc, "Failed to parse any content into a data tree.");
                rc = r;
                goto cleanup;
            }

            /* use the parsed data tree */
            if (use_value) {
                free((void *)value);
            }
            use_value = 1;
            value = tree;
            value_type = LYD_ANYDATA_DATATREE;
        }
    } else {
        /* anyxml */
        switch (value_type) {
        case LYD_ANYDATA_DATATREE:
            /* fine, just use the value */
            break;
        case LYD_ANYDATA_STRING:
            /* detect value type */
            lyd_create_any_string_valtype(value, &value_type);
            if ((value_type == LYD_ANYDATA_DATATREE) || (value_type == LYD_ANYDATA_STRING)) {
                break;
            }
        /* fallthrough */
        case LYD_ANYDATA_XML:
        case LYD_ANYDATA_JSON:
        case LYD_ANYDATA_LYB:
            if (!value) {
                /* nothing to parse */
                break;
            }

            /* create input */
            LY_CHECK_GOTO(rc = ly_in_new_memory(value, &in), cleanup);

            /* try to parse as a data tree */
            r = lyd_create_any_datatree(schema->module->ctx, in, value_type, 0, &tree);
            if (!r) {
                /* use the parsed data tree */
                if (use_value) {
                    free((void *)value);
                }
                use_value = 1;
                value = tree;
                value_type = LYD_ANYDATA_DATATREE;
            }
            break;
        }
    }

    if (use_value) {
        switch (value_type) {
        case LYD_ANYDATA_DATATREE:
            any->value.tree = (void *)value;
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_XML:
        case LYD_ANYDATA_JSON:
            LY_CHECK_GOTO(rc = lydict_insert_zc(schema->module->ctx, (void *)value, &any->value.str), cleanup);
            break;
        case LYD_ANYDATA_LYB:
            any->value.mem = (void *)value;
            break;
        }
        any->value_type = value_type;
    } else {
        any_val.str = value;
        LY_CHECK_GOTO(rc = lyd_any_copy_value(&any->node, &any_val, value_type), cleanup);
    }
    lyd_hash(&any->node);

cleanup:
    if (rc) {
        lyd_free_tree(&any->node);
    } else {
        *node = &any->node;
    }
    ly_in_free(in, 0);
    return rc;
}

LY_ERR
lyd_create_opaq(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *prefix, size_t pref_len,
        const char *module_key, size_t module_key_len, const char *value, size_t value_len, ly_bool *dynamic,
        LY_VALUE_FORMAT format, void *val_prefix_data, uint32_t hints, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node_opaq *opaq;

    assert(ctx && name && name_len && format);

    if (!value_len && (!dynamic || !*dynamic)) {
        value = "";
    }

    opaq = calloc(1, sizeof *opaq);
    LY_CHECK_ERR_GOTO(!opaq, LOGMEM(ctx); ret = LY_EMEM, finish);

    opaq->prev = &opaq->node;
    LY_CHECK_GOTO(ret = lydict_insert(ctx, name, name_len, &opaq->name.name), finish);

    if (pref_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, prefix, pref_len, &opaq->name.prefix), finish);
    }
    if (module_key_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, module_key, module_key_len, &opaq->name.module_ns), finish);
    }
    if (dynamic && *dynamic) {
        LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, (char *)value, &opaq->value), finish);
        *dynamic = 0;
    } else {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, value, value_len, &opaq->value), finish);
    }

    opaq->format = format;
    opaq->val_prefix_data = val_prefix_data;
    opaq->hints = hints;
    opaq->ctx = ctx;

finish:
    if (ret) {
        lyd_free_tree(&opaq->node);
        ly_free_prefix_data(format, val_prefix_data);
    } else {
        *node = &opaq->node;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_new_inner(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output,
        struct lyd_node **node)
{
    LY_ERR r;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct lysc_ext_instance *ext = NULL;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0,
            LYS_CONTAINER | LYS_NOTIF | LYS_RPC | LYS_ACTION, output ? LYS_GETNEXT_OUTPUT : 0);
    if (!schema && parent) {
        r = ly_nested_ext_schema(parent, NULL, module->name, strlen(module->name), LY_VALUE_JSON, NULL, name,
                strlen(name), &schema, &ext);
        LY_CHECK_RET(r && (r != LY_ENOT), r);
    }
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Inner node (container, notif, RPC, or action) \"%s\" not found.",
            name), LY_ENOTFOUND);

    LY_CHECK_RET(lyd_create_inner(schema, &ret));
    if (ext) {
        ret->flags |= LYD_EXT;
    }
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_ext_inner(const struct lysc_ext_instance *ext, const char *name, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, node, name, LY_EINVAL);

    schema = lysc_ext_find_node(ext, NULL, name, 0, LYS_CONTAINER | LYS_NOTIF | LYS_RPC | LYS_ACTION, 0);
    if (!schema) {
        if (ext->argument) {
            LOGERR(ctx, LY_EINVAL, "Inner node (not a list) \"%s\" not found in instance \"%s\" of extension %s.",
                    name, ext->argument, ext->def->name);
        } else {
            LOGERR(ctx, LY_EINVAL, "Inner node (not a list)  \"%s\" not found in instance of extension %s.",
                    name, ext->def->name);
        }
        return LY_ENOTFOUND;
    }
    LY_CHECK_RET(lyd_create_inner(schema, &ret));

    *node = ret;

    return LY_SUCCESS;
}

/**
 * @brief Create a new lits node instance without its keys.
 *
 * @param[in] ctx Context to use for logging.
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node must be #LYS_LIST.
 * @param[in] output Flag in case the @p parent is RPC/Action. If value is 0, the input's data nodes of the RPC/Action are
 * taken into consideration. Otherwise, the output's data node is going to be created.
 * @param[out] node Created node.
 * @return LY_ERR value.
 */
static LY_ERR
_lyd_new_list_node(const struct ly_ctx *ctx, const struct lyd_node *parent, const struct lys_module *module,
        const char *name, ly_bool output, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct lysc_ext_instance *ext = NULL;
    LY_ERR r;

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, output ? LYS_GETNEXT_OUTPUT : 0);
    if (!schema && parent) {
        r = ly_nested_ext_schema(parent, NULL, module->name, strlen(module->name), LY_VALUE_JSON, NULL, name,
                strlen(name), &schema, &ext);
        LY_CHECK_RET(r && (r != LY_ENOT), r);
    }
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), LY_ENOTFOUND);

    /* create list inner node */
    LY_CHECK_RET(lyd_create_inner(schema, &ret));

    if (ext) {
        ret->flags |= LYD_EXT;
    }

    *node = ret;
    return LY_SUCCESS;
}

/**
 * @brief Create a new list node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node must be #LYS_LIST.
 * @param[in] format Format of key values.
 * @param[in] output Flag in case the @p parent is RPC/Action. If value is 0, the input's data nodes of the RPC/Action are
 * taken into consideration. Otherwise, the output's data node is going to be created.
 * @param[out] node Optional created node.
 * @param[in] ap Ordered key values of the new list instance, all must be set. For ::LY_VALUE_LYB, every value must
 * be followed by the value length.
 * @return LY_ERR value.
 */
static LY_ERR
_lyd_new_list(struct lyd_node *parent, const struct lys_module *module, const char *name, LY_VALUE_FORMAT format,
        ly_bool output, struct lyd_node **node, va_list ap)
{
    struct lyd_node *ret = NULL, *key;
    const struct lysc_node *key_s;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);
    const void *key_val;
    uint32_t key_len;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    /* create the list node */
    LY_CHECK_RET(_lyd_new_list_node(ctx, parent, module, name, output, &ret));

    /* create and insert all the keys */
    for (key_s = lysc_node_child(ret->schema); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        if (format == LY_VALUE_LYB) {
            key_val = va_arg(ap, const void *);
            key_len = va_arg(ap, uint32_t);
        } else {
            key_val = va_arg(ap, const char *);
            key_len = key_val ? strlen((char *)key_val) : 0;
        }

        rc = lyd_create_term(key_s, key_val, key_len, 0, NULL, format, NULL, LYD_HINT_DATA, NULL, &key);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_insert_node(ret, NULL, key, 1);
    }

    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

cleanup:
    if (rc) {
        lyd_free_tree(ret);
        ret = NULL;
    } else if (node) {
        *node = ret;
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_list(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output,
        struct lyd_node **node, ...)
{
    LY_ERR rc;
    va_list ap;

    va_start(ap, node);

    rc = _lyd_new_list(parent, module, name, LY_VALUE_JSON, output, node, ap);

    va_end(ap);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_list_bin(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output,
        struct lyd_node **node, ...)
{
    LY_ERR rc;
    va_list ap;

    va_start(ap, node);

    rc = _lyd_new_list(parent, module, name, LY_VALUE_LYB, output, node, ap);

    va_end(ap);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_list_canon(struct lyd_node *parent, const struct lys_module *module, const char *name, ly_bool output,
        struct lyd_node **node, ...)
{
    LY_ERR rc;
    va_list ap;

    va_start(ap, node);

    rc = _lyd_new_list(parent, module, name, LY_VALUE_CANON, output, node, ap);

    va_end(ap);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_ext_list(const struct lysc_ext_instance *ext, const char *name, struct lyd_node **node, ...)
{
    struct lyd_node *ret = NULL, *key;
    const struct lysc_node *schema, *key_s;
    struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;
    va_list ap;
    const char *key_val;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, ext, node, name, LY_EINVAL);

    schema = lysc_ext_find_node(ext, NULL, name, 0, LYS_LIST, 0);
    if (!schema) {
        if (ext->argument) {
            LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found in instance \"%s\" of extension %s.",
                    name, ext->argument, ext->def->name);
        } else {
            LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found in instance of extension %s.", name, ext->def->name);
        }
        return LY_ENOTFOUND;
    }
    /* create list inner node */
    LY_CHECK_RET(lyd_create_inner(schema, &ret));

    va_start(ap, node);

    /* create and insert all the keys */
    for (key_s = lysc_node_child(schema); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        key_val = va_arg(ap, const char *);

        rc = lyd_create_term(key_s, key_val, key_val ? strlen(key_val) : 0, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA,
                NULL, &key);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_insert_node(ret, NULL, key, 1);
    }

cleanup:
    va_end(ap);
    if (rc) {
        lyd_free_tree(ret);
        ret = NULL;
    }
    *node = ret;
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_list2(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *keys,
        ly_bool output, struct lyd_node **node)
{
    LY_ERR r;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct lysc_ext_instance *ext = NULL;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }
    if (!keys) {
        keys = "";
    }

    /* find schema node */
    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYS_LIST, output ? LYS_GETNEXT_OUTPUT : 0);
    if (!schema && parent) {
        r = ly_nested_ext_schema(parent, NULL, module->name, strlen(module->name), LY_VALUE_JSON, NULL, name, strlen(name),
                &schema, &ext);
        LY_CHECK_RET(r && (r != LY_ENOT), r);
    }
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "List node \"%s\" not found.", name), LY_ENOTFOUND);

    if ((schema->flags & LYS_KEYLESS) && !keys[0]) {
        /* key-less list */
        LY_CHECK_RET(lyd_create_inner(schema, &ret));
    } else {
        /* create the list node */
        LY_CHECK_RET(lyd_create_list2(schema, keys, strlen(keys), &ret));
    }
    if (ext) {
        ret->flags |= LYD_EXT;
    }
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

/**
 * @brief Create a new list node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node must be #LYS_LIST.
 * @param[in] format Format of key values.
 * @param[in] key_values Ordered key values of the new list instance ended with NULL, all must be set.
 * @param[in] value_lengths Lengths of @p key_values, required for ::LY_VALUE_LYB, optional otherwise.
 * @param[in] output Flag in case the @p parent is RPC/Action. If value is 0, the input's data nodes of the RPC/Action are
 * taken into consideration. Otherwise, the output's data node is going to be created.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
static LY_ERR
_lyd_new_list3(struct lyd_node *parent, const struct lys_module *module, const char *name, LY_VALUE_FORMAT format,
        const void **key_values, uint32_t *value_lengths, ly_bool output, struct lyd_node **node)
{
    struct lyd_node *ret = NULL, *key;
    const struct lysc_node *key_s;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);
    const void *key_val;
    uint32_t key_len, i;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, key_values, (format != LY_VALUE_LYB) || value_lengths,
            LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    /* create the list node */
    LY_CHECK_RET(_lyd_new_list_node(ctx, parent, module, name, output, &ret));

    /* create and insert all the keys */
    i = 0;
    for (key_s = lysc_node_child(ret->schema); key_s && (key_s->flags & LYS_KEY); key_s = key_s->next) {
        key_val = key_values[i] ? key_values[i] : "";
        key_len = value_lengths ? value_lengths[i] : strlen(key_val);

        rc = lyd_create_term(key_s, key_val, key_len, 0, NULL, format, NULL, LYD_HINT_DATA, NULL, &key);
        LY_CHECK_GOTO(rc, cleanup);
        lyd_insert_node(ret, NULL, key, 1);
    }

    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

cleanup:
    if (rc) {
        lyd_free_tree(ret);
        ret = NULL;
    } else if (node) {
        *node = ret;
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_new_list3(struct lyd_node *parent, const struct lys_module *module, const char *name, const char **key_values,
        uint32_t *value_lengths, ly_bool output, struct lyd_node **node)
{
    return _lyd_new_list3(parent, module, name, LY_VALUE_JSON, (const void **)key_values, value_lengths, output, node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_list3_bin(struct lyd_node *parent, const struct lys_module *module, const char *name, const void **key_values,
        uint32_t *value_lengths, ly_bool output, struct lyd_node **node)
{
    return _lyd_new_list3(parent, module, name, LY_VALUE_LYB, key_values, value_lengths, output, node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_list3_canon(struct lyd_node *parent, const struct lys_module *module, const char *name, const char **key_values,
        uint32_t *value_lengths, ly_bool output, struct lyd_node **node)
{
    return _lyd_new_list3(parent, module, name, LY_VALUE_CANON, (const void **)key_values, value_lengths, output, node);
}

/**
 * @brief Create a new term node in the data tree.
 *
 * @param[in] parent Parent node for the node being created. NULL in case of creating a top level element.
 * @param[in] module Module of the node being created. If NULL, @p parent module will be used.
 * @param[in] name Schema node name of the new data node. The node can be ::LYS_LEAF or ::LYS_LEAFLIST.
 * @param[in] value Value of the node being created.
 * @param[in] value_len Length of @p value.
 * @param[in] format Format of @p value.
 * @param[in] output Flag in case the @p parent is RPC/Action. If value is 0, the input's data nodes of the RPC/Action are
 * taken into consideration. Otherwise, the output's data node is going to be created.
 * @param[out] node Optional created node.
 * @return LY_ERR value.
 */
static LY_ERR
_lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
        size_t value_len, LY_VALUE_FORMAT format, ly_bool output, struct lyd_node **node)
{
    LY_ERR r;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct lysc_ext_instance *ext = NULL;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_TERM, output ? LYS_GETNEXT_OUTPUT : 0);
    if (!schema && parent) {
        r = ly_nested_ext_schema(parent, NULL, module->name, strlen(module->name), LY_VALUE_JSON, NULL, name,
                strlen(name), &schema, &ext);
        LY_CHECK_RET(r && (r != LY_ENOT), r);
    }
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found.", name), LY_ENOTFOUND);

    LY_CHECK_RET(lyd_create_term(schema, value, value_len, 0, NULL, format, NULL, LYD_HINT_DATA, NULL, &ret));
    if (ext) {
        ret->flags |= LYD_EXT;
    }
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_term(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str,
        ly_bool output, struct lyd_node **node)
{
    return _lyd_new_term(parent, module, name, val_str, val_str ? strlen(val_str) : 0, LY_VALUE_JSON, output, node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_term_bin(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
        size_t value_len, ly_bool output, struct lyd_node **node)
{
    return _lyd_new_term(parent, module, name, value, value_len, LY_VALUE_LYB, output, node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_term_canon(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str,
        ly_bool output, struct lyd_node **node)
{
    return _lyd_new_term(parent, module, name, val_str, val_str ? strlen(val_str) : 0, LY_VALUE_CANON, output, node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_ext_term(const struct lysc_ext_instance *ext, const char *name, const char *val_str, struct lyd_node **node)
{
    LY_ERR rc;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, node, name, LY_EINVAL);

    schema = lysc_ext_find_node(ext, NULL, name, 0, LYD_NODE_TERM, 0);
    if (!schema) {
        if (ext->argument) {
            LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found in instance \"%s\" of extension %s.",
                    name, ext->argument, ext->def->name);
        } else {
            LOGERR(ctx, LY_EINVAL, "Term node \"%s\" not found in instance of extension %s.", name, ext->def->name);
        }
        return LY_ENOTFOUND;
    }
    rc = lyd_create_term(schema, val_str, val_str ? strlen(val_str) : 0, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA,
            NULL, &ret);
    LY_CHECK_RET(rc);

    *node = ret;

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_any(struct lyd_node *parent, const struct lys_module *module, const char *name, const void *value,
        ly_bool use_value, LYD_ANYDATA_VALUETYPE value_type, ly_bool output, struct lyd_node **node)
{
    LY_ERR r;
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct lysc_ext_instance *ext = NULL;
    const struct ly_ctx *ctx = parent ? LYD_CTX(parent) : (module ? module->ctx : NULL);

    LY_CHECK_ARG_RET(ctx, parent || module, parent || node, name,
            (value_type == LYD_ANYDATA_DATATREE) || (value_type == LYD_ANYDATA_STRING) || value, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);

    if (!module) {
        module = parent->schema->module;
    }

    schema = lys_find_child(parent ? parent->schema : NULL, module, name, 0, LYD_NODE_ANY, output ? LYS_GETNEXT_OUTPUT : 0);
    if (!schema && parent) {
        r = ly_nested_ext_schema(parent, NULL, module->name, strlen(module->name), LY_VALUE_JSON, NULL, name,
                strlen(name), &schema, &ext);
        LY_CHECK_RET(r && (r != LY_ENOT), r);
    }
    LY_CHECK_ERR_RET(!schema, LOGERR(ctx, LY_EINVAL, "Any node \"%s\" not found.", name), LY_ENOTFOUND);

    LY_CHECK_RET(lyd_create_any(schema, value, value_type, use_value, &ret));
    if (ext) {
        ret->flags |= LYD_EXT;
    }
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 0);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_ext_any(const struct lysc_ext_instance *ext, const char *name, const void *value, ly_bool use_value,
        LYD_ANYDATA_VALUETYPE value_type, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;
    const struct lysc_node *schema;
    struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, node, name, LY_EINVAL);

    schema = lysc_ext_find_node(ext, NULL, name, 0, LYD_NODE_ANY, 0);
    if (!schema) {
        if (ext->argument) {
            LOGERR(ctx, LY_EINVAL, "Any node \"%s\" not found in instance \"%s\" of extension %s.",
                    name, ext->argument, ext->def->name);
        } else {
            LOGERR(ctx, LY_EINVAL, "Any node \"%s\" not found in instance of extension %s.", name, ext->def->name);
        }
        return LY_ENOTFOUND;
    }
    LY_CHECK_RET(lyd_create_any(schema, value, value_type, use_value, &ret));

    *node = ret;

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_meta(const struct ly_ctx *ctx, struct lyd_node *parent, const struct lys_module *module, const char *name,
        const char *val_str, ly_bool clear_dflt, struct lyd_meta **meta)
{
    const char *prefix, *tmp;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(ctx, ctx || parent, name, module || strchr(name, ':'), parent || meta, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, parent ? LYD_CTX(parent) : NULL, module ? module->ctx : NULL, LY_EINVAL);
    if (!ctx) {
        ctx = module ? module->ctx : LYD_CTX(parent);
    }

    if (parent && !parent->schema) {
        LOGERR(ctx, LY_EINVAL, "Cannot add metadata \"%s\" to an opaque node \"%s\".", name, LYD_NAME(parent));
        return LY_EINVAL;
    }
    if (meta) {
        *meta = NULL;
    }

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return LY_EINVAL;
    }

    /* find the module */
    if (prefix) {
        module = ly_ctx_get_module_implemented2(ctx, prefix, pref_len);
        LY_CHECK_ERR_RET(!module, LOGERR(ctx, LY_EINVAL, "Module \"%.*s\" not found.", (int)pref_len, prefix), LY_ENOTFOUND);
    }

    /* set value if none */
    if (!val_str) {
        val_str = "";
    }

    return lyd_create_meta(parent, meta, module, name, name_len, val_str, strlen(val_str), 0, NULL, LY_VALUE_JSON,
            NULL, LYD_HINT_DATA, parent ? parent->schema : NULL, clear_dflt, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_new_meta2(const struct ly_ctx *ctx, struct lyd_node *parent, ly_bool clear_dflt, const struct lyd_attr *attr,
        struct lyd_meta **meta)
{
    const struct lys_module *mod;

    LY_CHECK_ARG_RET(NULL, ctx, attr, parent || meta, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, parent ? LYD_CTX(parent) : NULL, LY_EINVAL);

    if (parent && !parent->schema) {
        LOGERR(ctx, LY_EINVAL, "Cannot add metadata to an opaque node \"%s\".",
                ((struct lyd_node_opaq *)parent)->name.name);
        return LY_EINVAL;
    }
    if (meta) {
        *meta = NULL;
    }

    switch (attr->format) {
    case LY_VALUE_XML:
        mod = ly_ctx_get_module_implemented_ns(ctx, attr->name.module_ns);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Module with namespace \"%s\" not found.", attr->name.module_ns);
            return LY_ENOTFOUND;
        }
        break;
    case LY_VALUE_JSON:
        mod = ly_ctx_get_module_implemented(ctx, attr->name.module_name);
        if (!mod) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" not found.", attr->name.module_name);
            return LY_ENOTFOUND;
        }
        break;
    default:
        LOGINT_RET(ctx);
    }

    return lyd_create_meta(parent, meta, mod, attr->name.name, strlen(attr->name.name), attr->value, strlen(attr->value),
            0, NULL, attr->format, attr->val_prefix_data, attr->hints, parent ? parent->schema : NULL, clear_dflt, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_new_opaq(struct lyd_node *parent, const struct ly_ctx *ctx, const char *name, const char *value,
        const char *prefix, const char *module_name, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;

    LY_CHECK_ARG_RET(ctx, parent || ctx, parent || node, name, module_name, !prefix || !strcmp(prefix, module_name), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, parent ? LYD_CTX(parent) : NULL, LY_EINVAL);

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }
    if (!value) {
        value = "";
    }

    LY_CHECK_RET(lyd_create_opaq(ctx, name, strlen(name), prefix, prefix ? strlen(prefix) : 0, module_name,
            strlen(module_name), value, strlen(value), NULL, LY_VALUE_JSON, NULL, 0, &ret));
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 1);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_opaq2(struct lyd_node *parent, const struct ly_ctx *ctx, const char *name, const char *value,
        const char *prefix, const char *module_ns, struct lyd_node **node)
{
    struct lyd_node *ret = NULL;

    LY_CHECK_ARG_RET(ctx, parent || ctx, parent || node, name, module_ns, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, parent ? LYD_CTX(parent) : NULL, LY_EINVAL);

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }
    if (!value) {
        value = "";
    }

    LY_CHECK_RET(lyd_create_opaq(ctx, name, strlen(name), prefix, prefix ? strlen(prefix) : 0, module_ns,
            strlen(module_ns), value, strlen(value), NULL, LY_VALUE_XML, NULL, 0, &ret));
    if (parent) {
        lyd_insert_node(parent, NULL, ret, 1);
    }

    if (node) {
        *node = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_attr(struct lyd_node *parent, const char *module_name, const char *name, const char *value,
        struct lyd_attr **attr)
{
    struct lyd_attr *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    size_t pref_len, name_len, mod_len;

    LY_CHECK_ARG_RET(NULL, parent, !parent->schema, name, LY_EINVAL);

    ctx = LYD_CTX(parent);

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Attribute name \"%s\" is not valid.", name);
        return LY_EVALID;
    }

    if ((pref_len == 3) && !strncmp(prefix, "xml", 3)) {
        /* not a prefix but special name */
        name = prefix;
        name_len += 1 + pref_len;
        prefix = NULL;
        pref_len = 0;
    }

    /* get the module */
    if (module_name) {
        mod_len = strlen(module_name);
    } else {
        module_name = prefix;
        mod_len = pref_len;
    }

    /* set value if none */
    if (!value) {
        value = "";
    }

    LY_CHECK_RET(lyd_create_attr(parent, &ret, ctx, name, name_len, prefix, pref_len, module_name, mod_len, value,
            strlen(value), NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA));

    if (attr) {
        *attr = ret;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_attr2(struct lyd_node *parent, const char *module_ns, const char *name, const char *value,
        struct lyd_attr **attr)
{
    struct lyd_attr *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, parent, !parent->schema, name, LY_EINVAL);

    ctx = LYD_CTX(parent);

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Attribute name \"%s\" is not valid.", name);
        return LY_EVALID;
    }

    if ((pref_len == 3) && !strncmp(prefix, "xml", 3)) {
        /* not a prefix but special name */
        name = prefix;
        name_len += 1 + pref_len;
        prefix = NULL;
        pref_len = 0;
    }

    /* set value if none */
    if (!value) {
        value = "";
    }
    if (strchr(value, ':')) {
        LOGWRN(ctx, "Value \"%s\" prefix will never be interpreted as an XML prefix.", value);
    }

    LY_CHECK_RET(lyd_create_attr(parent, &ret, ctx, name, name_len, prefix, pref_len, module_ns,
            module_ns ? strlen(module_ns) : 0, value, strlen(value), NULL, LY_VALUE_XML, NULL, LYD_HINT_DATA));

    if (attr) {
        *attr = ret;
    }
    return LY_SUCCESS;
}

/**
 * @brief Change the value of a term (leaf or leaf-list) node.
 *
 * Node changed this way is always considered explicitly set, meaning its default flag
 * is always cleared.
 *
 * @param[in] term Term node to change.
 * @param[in] value New value to set.
 * @param[in] value_len Length of @p value.
 * @param[in] format Format of @p value.
 * @return LY_SUCCESS if value was changed,
 * @return LY_EEXIST if value was the same and only the default flag was cleared,
 * @return LY_ENOT if the values were equal and no change occured,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
_lyd_change_term(struct lyd_node *term, const void *value, size_t value_len, LY_VALUE_FORMAT format)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type *type;
    struct lyd_node_term *t;
    struct lyd_node *parent;
    struct lyd_value val;
    ly_bool dflt_change, val_change;

    assert(term && term->schema && (term->schema->nodetype & LYD_NODE_TERM));

    t = (struct lyd_node_term *)term;
    type = ((struct lysc_node_leaf *)term->schema)->type;

    /* parse the new value */
    LOG_LOCSET(term->schema, term, NULL, NULL);
    ret = lyd_value_store(LYD_CTX(term), &val, type, value, value_len, 0, NULL, format, NULL, LYD_HINT_DATA,
            term->schema, NULL);
    LOG_LOCBACK(term->schema ? 1 : 0, 1, 0, 0);
    LY_CHECK_GOTO(ret, cleanup);

    /* compare original and new value */
    if (type->plugin->compare(&t->value, &val)) {
        /* values differ, switch them */
        type->plugin->free(LYD_CTX(term), &t->value);
        t->value = val;
        val_change = 1;
    } else {
        /* same values, free the new stored one */
        type->plugin->free(LYD_CTX(term), &val);
        val_change = 0;
    }

    /* always clear the default flag */
    if (term->flags & LYD_DEFAULT) {
        for (parent = term; parent; parent = lyd_parent(parent)) {
            parent->flags &= ~LYD_DEFAULT;
        }
        dflt_change = 1;
    } else {
        dflt_change = 0;
    }

    if (val_change || dflt_change) {
        /* make the node non-validated */
        term->flags &= LYD_NEW;
    }

    if (val_change) {
        if (term->schema->nodetype == LYS_LEAFLIST) {
            /* leaf-list needs to be hashed again and re-inserted into parent */
            lyd_unlink_hash(term);
            lyd_hash(term);
            LY_CHECK_GOTO(ret = lyd_insert_hash(term), cleanup);
        } else if ((term->schema->flags & LYS_KEY) && term->parent) {
            /* list needs to be updated if its key was changed */
            assert(term->parent->schema->nodetype == LYS_LIST);
            lyd_unlink_hash(lyd_parent(term));
            lyd_hash(lyd_parent(term));
            LY_CHECK_GOTO(ret = lyd_insert_hash(lyd_parent(term)), cleanup);
        } /* else leaf that is not a key, its value is not used for its hash so it does not change */
    }

    /* retrun value */
    if (!val_change) {
        if (dflt_change) {
            /* only default flag change */
            ret = LY_EEXIST;
        } else {
            /* no change */
            ret = LY_ENOT;
        }
    } /* else value changed, LY_SUCCESS */

cleanup:
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_change_term(struct lyd_node *term, const char *val_str)
{
    LY_CHECK_ARG_RET(NULL, term, term->schema, term->schema->nodetype & LYD_NODE_TERM, LY_EINVAL);

    return _lyd_change_term(term, val_str, val_str ? strlen(val_str) : 0, LY_VALUE_JSON);
}

LIBYANG_API_DEF LY_ERR
lyd_change_term_bin(struct lyd_node *term, const void *value, size_t value_len)
{
    LY_CHECK_ARG_RET(NULL, term, term->schema, term->schema->nodetype & LYD_NODE_TERM, LY_EINVAL);

    return _lyd_change_term(term, value, value_len, LY_VALUE_LYB);
}

LIBYANG_API_DEF LY_ERR
lyd_change_term_canon(struct lyd_node *term, const char *val_str)
{
    LY_CHECK_ARG_RET(NULL, term, term->schema, term->schema->nodetype & LYD_NODE_TERM, LY_EINVAL);

    return _lyd_change_term(term, val_str, val_str ? strlen(val_str) : 0, LY_VALUE_CANON);
}

LIBYANG_API_DEF LY_ERR
lyd_change_meta(struct lyd_meta *meta, const char *val_str)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *m2 = NULL;
    struct lyd_value val;
    ly_bool val_change;

    LY_CHECK_ARG_RET(NULL, meta, LY_EINVAL);

    if (!val_str) {
        val_str = "";
    }

    /* parse the new value into a new meta structure */
    ret = lyd_create_meta(NULL, &m2, meta->annotation->module, meta->name, strlen(meta->name), val_str, strlen(val_str),
            0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, meta->parent ? meta->parent->schema : NULL, 0, NULL);
    LY_CHECK_GOTO(ret, cleanup);

    /* compare original and new value */
    if (lyd_compare_meta(meta, m2)) {
        /* values differ, switch them */
        val = meta->value;
        meta->value = m2->value;
        m2->value = val;
        val_change = 1;
    } else {
        val_change = 0;
    }

    /* retrun value */
    if (!val_change) {
        /* no change */
        ret = LY_ENOT;
    } /* else value changed, LY_SUCCESS */

cleanup:
    lyd_free_meta_single(m2);
    return ret;
}

/**
 * @brief Update node value.
 *
 * @param[in] node Node to update.
 * @param[in] value New value to set.
 * @param[in] value_len Length of @p value.
 * @param[in] value_type Type of @p value for anydata/anyxml node.
 * @param[in] format Format of @p value.
 * @param[out] new_parent Set to @p node if the value was updated, otherwise set to NULL.
 * @param[out] new_node Set to @p node if the value was updated, otherwise set to NULL.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_new_path_update(struct lyd_node *node, const void *value, size_t value_len, LYD_ANYDATA_VALUETYPE value_type,
        LY_VALUE_FORMAT format, struct lyd_node **new_parent, struct lyd_node **new_node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *new_any;

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_LIST:
        /* if it exists, there is nothing to update */
        *new_parent = NULL;
        *new_node = NULL;
        break;
    case LYS_LEAFLIST:
        if (!lysc_is_dup_inst_list(node->schema)) {
            /* if it exists, there is nothing to update */
            *new_parent = NULL;
            *new_node = NULL;
            break;
        }
    /* fallthrough */
    case LYS_LEAF:
        ret = _lyd_change_term(node, value, value_len, format);
        if ((ret == LY_SUCCESS) || (ret == LY_EEXIST)) {
            /* there was an actual change (at least of the default flag) */
            *new_parent = node;
            *new_node = node;
            ret = LY_SUCCESS;
        } else if (ret == LY_ENOT) {
            /* no change */
            *new_parent = NULL;
            *new_node = NULL;
            ret = LY_SUCCESS;
        } /* else error */
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* create a new any node */
        LY_CHECK_RET(lyd_create_any(node->schema, value, value_type, 0, &new_any));

        /* compare with the existing one */
        if (lyd_compare_single(node, new_any, 0)) {
            /* not equal, switch values (so that we can use generic node free) */
            ((struct lyd_node_any *)new_any)->value = ((struct lyd_node_any *)node)->value;
            ((struct lyd_node_any *)new_any)->value_type = ((struct lyd_node_any *)node)->value_type;
            ((struct lyd_node_any *)node)->value.str = value;
            ((struct lyd_node_any *)node)->value_type = value_type;

            *new_parent = node;
            *new_node = node;
        } else {
            /* they are equal */
            *new_parent = NULL;
            *new_node = NULL;
        }
        lyd_free_tree(new_any);
        break;
    default:
        LOGINT(LYD_CTX(node));
        ret = LY_EINT;
        break;
    }

    return ret;
}

static LY_ERR
lyd_new_path_check_find_lypath(struct ly_path *path, const char *str_path, const char *value, size_t value_len,
        LY_VALUE_FORMAT format, uint32_t options)
{
    LY_ERR r;
    struct ly_path_predicate *pred;
    struct lyd_value val;
    const struct lysc_node *schema = NULL;
    LY_ARRAY_COUNT_TYPE u, new_count;
    int create = 0;

    assert(path);

    /* go through all the compiled nodes */
    LY_ARRAY_FOR(path, u) {
        schema = path[u].node;

        if (lysc_is_dup_inst_list(schema)) {
            if (!path[u].predicates ||
                    ((schema->nodetype == LYS_LEAFLIST) && (path[u].predicates[0].type == LY_PATH_PREDTYPE_LEAFLIST))) {
                /* creating a new key-less list or state leaf-list instance */
                create = 1;
                new_count = u;
            } else if (path[u].predicates[0].type != LY_PATH_PREDTYPE_POSITION) {
                LOG_LOCSET(schema, NULL, NULL, NULL);
                LOGVAL(schema->module->ctx, LYVE_XPATH, "Invalid predicate for state %s \"%s\" in path \"%s\".",
                        lys_nodetype2str(schema->nodetype), schema->name, str_path);
                LOG_LOCBACK(1, 0, 0, 0);
                return LY_EINVAL;
            }
        } else if ((schema->nodetype == LYS_LIST) &&
                (!path[u].predicates || (path[u].predicates[0].type != LY_PATH_PREDTYPE_LIST))) {
            if ((u < LY_ARRAY_COUNT(path) - 1) || !(options & LYD_NEW_PATH_OPAQ)) {
                LOG_LOCSET(schema, NULL, NULL, NULL);
                LOGVAL(schema->module->ctx, LYVE_XPATH, "Predicate missing for %s \"%s\" in path \"%s\".",
                        lys_nodetype2str(schema->nodetype), schema->name, str_path);
                LOG_LOCBACK(1, 0, 0, 0);
                return LY_EINVAL;
            } /* else creating an opaque list */
        } else if ((schema->nodetype == LYS_LEAFLIST) &&
                (!path[u].predicates || (path[u].predicates[0].type != LY_PATH_PREDTYPE_LEAFLIST))) {
            r = LY_SUCCESS;
            if (options & LYD_NEW_PATH_OPAQ) {
                r = lyd_value_validate(NULL, schema, value, value_len, NULL, NULL, NULL);
            }
            if (!r) {
                /* try to store the value */
                LY_CHECK_RET(lyd_value_store(schema->module->ctx, &val, ((struct lysc_node_leaflist *)schema)->type,
                        value, value_len, 0, NULL, format, NULL, LYD_HINT_DATA, schema, NULL));
                ++((struct lysc_type *)val.realtype)->refcount;

                /* store the new predicate so that it is used when searching for this instance */
                LY_ARRAY_NEW_RET(schema->module->ctx, path[u].predicates, pred, LY_EMEM);
                pred->type = LY_PATH_PREDTYPE_LEAFLIST;
                pred->value = val;
            } /* else we have opaq flag and the value is not valid, leave no predicate and then create an opaque node */
        }
    }

    if (create) {
        /* hide the nodes that should always be created so they are not found */
        while (new_count < LY_ARRAY_COUNT(path)) {
            LY_ARRAY_DECREMENT(path);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Create a new node in the data tree based on a path. All node types can be created.
 *
 * If @p path points to a list key, the key value from the predicate is used and @p value is ignored.
 * Also, if a leaf-list is being created and both a predicate is defined in @p path
 * and @p value is set, the predicate is preferred.
 *
 * For key-less lists and state leaf-lists, positional predicates can be used. If no preciate is used for these
 * nodes, they are always created.
 *
 * @param[in] parent Data parent to add to/modify, can be NULL. Note that in case a first top-level sibling is used,
 * it may no longer be first if @p path is absolute and starts with a non-existing top-level node inserted
 * before @p parent. Use ::lyd_first_sibling() to adjust @p parent in these cases.
 * @param[in] ctx libyang context, must be set if @p parent is NULL.
 * @param[in] ext Extension instance where the node being created is defined. This argument takes effect only for absolute
 * path or when the relative paths touches document root (top-level). In such cases the present extension instance replaces
 * searching for the appropriate module.
 * @param[in] path [Path](@ref howtoXPath) to create.
 * @param[in] value Value of the new leaf/leaf-list (const char *) in ::LY_VALUE_JSON format. If creating an
 * anyxml/anydata node, the expected type depends on @p value_type. For other node types, it should be NULL.
 * @param[in] value_len Length of @p value in bytes. May be 0 if @p value is a zero-terminated string. Ignored when
 * creating anyxml/anydata nodes.
 * @param[in] value_type Anyxml/anydata node @p value type.
 * @param[in] options Bitmask of options, see @ref pathoptions.
 * @param[out] new_parent Optional first parent node created. If only one node was created, equals to @p new_node.
 * @param[out] new_node Optional last node created.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_new_path_(struct lyd_node *parent, const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, const char *path,
        const void *value, size_t value_len, LYD_ANYDATA_VALUETYPE value_type, uint32_t options,
        struct lyd_node **new_parent, struct lyd_node **new_node)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lyxp_expr *exp = NULL;
    struct ly_path *p = NULL;
    struct lyd_node *nparent = NULL, *nnode = NULL, *node = NULL, *cur_parent;
    const struct lysc_node *schema;
    const struct lyd_value *val = NULL;
    LY_ARRAY_COUNT_TYPE path_idx = 0, orig_count = 0;
    LY_VALUE_FORMAT format;

    assert(parent || ctx);
    assert(path && ((path[0] == '/') || parent));
    assert(!(options & LYD_NEW_PATH_BIN_VALUE) || !(options & LYD_NEW_PATH_CANON_VALUE));

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }
    if (value && !value_len) {
        value_len = strlen(value);
    }
    if (options & LYD_NEW_PATH_BIN_VALUE) {
        format = LY_VALUE_LYB;
    } else if (options & LYD_NEW_PATH_CANON_VALUE) {
        format = LY_VALUE_CANON;
    } else {
        format = LY_VALUE_JSON;
    }

    /* parse path */
    LY_CHECK_GOTO(ret = ly_path_parse(ctx, NULL, path, strlen(path), 0, LY_PATH_BEGIN_EITHER, LY_PATH_PREFIX_FIRST,
            LY_PATH_PRED_SIMPLE, &exp), cleanup);

    /* compile path */
    LY_CHECK_GOTO(ret = ly_path_compile(ctx, NULL, lyd_node_schema(parent), ext, exp, options & LYD_NEW_PATH_OUTPUT ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, 0, LY_VALUE_JSON, NULL, &p), cleanup);

    /* check the compiled path before searching existing nodes, it may be shortened */
    orig_count = LY_ARRAY_COUNT(p);
    LY_CHECK_GOTO(ret = lyd_new_path_check_find_lypath(p, path, value, value_len, format, options), cleanup);

    /* try to find any existing nodes in the path */
    if (parent) {
        r = ly_path_eval_partial(p, parent, NULL, options & LYD_NEW_PATH_WITH_OPAQ, &path_idx, &node);
        if (r == LY_SUCCESS) {
            if (orig_count == LY_ARRAY_COUNT(p)) {
                /* the node exists, are we supposed to update it or is it just a default? */
                if (!(options & LYD_NEW_PATH_UPDATE) && !(node->flags & LYD_DEFAULT)) {
                    LOG_LOCSET(NULL, node, NULL, NULL);
                    LOGVAL(ctx, LYVE_REFERENCE, "Path \"%s\" already exists.", path);
                    LOG_LOCBACK(0, 1, 0, 0);
                    ret = LY_EEXIST;
                    goto cleanup;
                } else if ((options & LYD_NEW_PATH_UPDATE) && lysc_is_key(node->schema)) {
                    /* fine, the key value must not be changed and has to be in the predicate to be found */
                    goto cleanup;
                }

                /* update the existing node */
                ret = lyd_new_path_update(node, value, value_len, value_type, format, &nparent, &nnode);
                goto cleanup;
            } /* else we were not searching for the whole path */
        } else if (r == LY_EINCOMPLETE) {
            /* some nodes were found, adjust the iterator to the next segment */
            ++path_idx;
        } else if (r == LY_ENOTFOUND) {
            /* we will create the nodes from top-level, default behavior (absolute path), or from the parent (relative path) */
            if (lysc_data_parent(p[0].node)) {
                node = parent;
            }
        } else {
            /* error */
            ret = r;
            goto cleanup;
        }
    }

    /* restore the full path for creating new nodes */
    while (orig_count > LY_ARRAY_COUNT(p)) {
        LY_ARRAY_INCREMENT(p);
    }

    /* create all the non-existing nodes in a loop */
    for ( ; path_idx < LY_ARRAY_COUNT(p); ++path_idx) {
        cur_parent = node;
        schema = p[path_idx].node;

        switch (schema->nodetype) {
        case LYS_LIST:
            if (lysc_is_dup_inst_list(schema)) {
                /* create key-less list instance */
                LY_CHECK_GOTO(ret = lyd_create_inner(schema, &node), cleanup);
            } else if ((options & LYD_NEW_PATH_OPAQ) && !p[path_idx].predicates) {
                /* creating opaque list without keys */
                LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), NULL, 0,
                        schema->module->name, strlen(schema->module->name), NULL, 0, NULL, LY_VALUE_JSON, NULL,
                        LYD_NODEHINT_LIST, &node), cleanup);
            } else {
                /* create standard list instance */
                LY_CHECK_GOTO(ret = lyd_create_list(schema, p[path_idx].predicates, NULL, &node), cleanup);
            }
            break;
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            LY_CHECK_GOTO(ret = lyd_create_inner(schema, &node), cleanup);
            break;
        case LYS_LEAFLIST:
            if ((options & LYD_NEW_PATH_OPAQ) &&
                    (!p[path_idx].predicates || (p[path_idx].predicates[0].type != LY_PATH_PREDTYPE_LEAFLIST))) {
                /* we have not checked this only for dup-inst lists, otherwise it must be opaque */
                r = LY_EVALID;
                if (lysc_is_dup_inst_list(schema)) {
                    /* validate value */
                    r = lyd_value_validate(NULL, schema, value ? value : "", value_len, NULL, NULL, NULL);
                }
                if (r && (r != LY_EINCOMPLETE)) {
                    /* creating opaque leaf-list */
                    LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), value, value_len,
                            schema->module->name, strlen(schema->module->name), NULL, 0, NULL, format, NULL,
                            LYD_NODEHINT_LEAFLIST, &node), cleanup);
                    break;
                }
            }

            /* get value to set */
            if (p[path_idx].predicates && (p[path_idx].predicates[0].type == LY_PATH_PREDTYPE_LEAFLIST)) {
                val = &p[path_idx].predicates[0].value;
            }

            /* create a leaf-list instance */
            if (val) {
                LY_CHECK_GOTO(ret = lyd_create_term2(schema, val, &node), cleanup);
            } else {
                LY_CHECK_GOTO(ret = lyd_create_term(schema, value, value_len, 0, NULL, format, NULL, LYD_HINT_DATA,
                        NULL, &node), cleanup);
            }
            break;
        case LYS_LEAF:
            if (lysc_is_key(schema) && cur_parent->schema) {
                /* it must have been already created or some error will occur later */
                lyd_find_sibling_schema(lyd_child(cur_parent), schema, &node);
                assert(node);
                goto next_iter;
            }

            if (options & LYD_NEW_PATH_OPAQ) {
                if (cur_parent && !cur_parent->schema) {
                    /* always create opaque nodes for opaque parents */
                    r = LY_ENOT;
                } else {
                    /* validate value */
                    r = lyd_value_validate(NULL, schema, value ? value : "", value_len, NULL, NULL, NULL);
                }
                if (r && (r != LY_EINCOMPLETE)) {
                    /* creating opaque leaf */
                    LY_CHECK_GOTO(ret = lyd_create_opaq(ctx, schema->name, strlen(schema->name), value, value_len,
                            schema->module->name, strlen(schema->module->name), NULL, 0, NULL, format, NULL, 0, &node),
                            cleanup);
                    break;
                }
            }

            /* create a leaf instance */
            LY_CHECK_GOTO(ret = lyd_create_term(schema, value, value_len, 0, NULL, format, NULL, LYD_HINT_DATA, NULL,
                    &node), cleanup);
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            LY_CHECK_GOTO(ret = lyd_create_any(schema, value, value_type, 0, &node), cleanup);
            break;
        default:
            LOGINT(ctx);
            ret = LY_EINT;
            goto cleanup;
        }

        if (p[path_idx].ext) {
            node->flags |= LYD_EXT;
        }
        if (cur_parent) {
            /* connect to the parent */
            lyd_insert_node(cur_parent, NULL, node, 0);
        } else if (parent) {
            /* connect to top-level siblings */
            lyd_insert_node(NULL, &parent, node, 0);
        }

next_iter:
        /* update remembered nodes */
        if (!nparent) {
            nparent = node;
        }
        nnode = node;
    }

cleanup:
    lyxp_expr_free(ctx, exp);
    if (p) {
        while (orig_count > LY_ARRAY_COUNT(p)) {
            LY_ARRAY_INCREMENT(p);
        }
    }
    ly_path_free(ctx, p);
    if (!ret) {
        /* set out params only on success */
        if (new_parent) {
            *new_parent = nparent;
        }
        if (new_node) {
            *new_node = nnode;
        }
    } else {
        lyd_free_tree(nparent);
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_new_path(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const char *value, uint32_t options,
        struct lyd_node **node)
{
    LY_CHECK_ARG_RET(ctx, parent || ctx, path, (path[0] == '/') || parent,
            !(options & LYD_NEW_PATH_BIN_VALUE) || !(options & LYD_NEW_PATH_CANON_VALUE), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, ctx, LY_EINVAL);

    return lyd_new_path_(parent, ctx, NULL, path, value, 0, LYD_ANYDATA_STRING, options, node, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_new_path2(struct lyd_node *parent, const struct ly_ctx *ctx, const char *path, const void *value,
        size_t value_len, LYD_ANYDATA_VALUETYPE value_type, uint32_t options, struct lyd_node **new_parent,
        struct lyd_node **new_node)
{
    LY_CHECK_ARG_RET(ctx, parent || ctx, path, (path[0] == '/') || parent,
            !(options & LYD_NEW_PATH_BIN_VALUE) || !(options & LYD_NEW_PATH_CANON_VALUE), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, ctx, LY_EINVAL);

    return lyd_new_path_(parent, ctx, NULL, path, value, value_len, value_type, options, new_parent, new_node);
}

LIBYANG_API_DEF LY_ERR
lyd_new_ext_path(struct lyd_node *parent, const struct lysc_ext_instance *ext, const char *path, const void *value,
        uint32_t options, struct lyd_node **node)
{
    const struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, path, (path[0] == '/') || parent,
            !(options & LYD_NEW_PATH_BIN_VALUE) || !(options & LYD_NEW_PATH_CANON_VALUE), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(parent ? LYD_CTX(parent) : NULL, ctx, LY_EINVAL);

    return lyd_new_path_(parent, ctx, ext, path, value, 0, LYD_ANYDATA_STRING, options, node, NULL);
}

LY_ERR
lyd_new_implicit_r(struct lyd_node *parent, struct lyd_node **first, const struct lysc_node *sparent,
        const struct lys_module *mod, struct ly_set *node_when, struct ly_set *node_types, struct ly_set *ext_node,
        uint32_t impl_opts, struct lyd_node **diff)
{
    LY_ERR ret;
    const struct lysc_node *iter = NULL;
    struct lyd_node *node = NULL;
    struct lyd_value **dflts;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t getnext_opts;

    assert(first && (parent || sparent || mod));

    if (!sparent && parent) {
        sparent = parent->schema;
    }

    getnext_opts = LYS_GETNEXT_WITHCHOICE;
    if (impl_opts & LYD_IMPLICIT_OUTPUT) {
        getnext_opts |= LYS_GETNEXT_OUTPUT;
    }

    while ((iter = lys_getnext(iter, sparent, mod ? mod->compiled : NULL, getnext_opts))) {
        if ((impl_opts & LYD_IMPLICIT_NO_STATE) && (iter->flags & LYS_CONFIG_R)) {
            continue;
        } else if ((impl_opts & LYD_IMPLICIT_NO_CONFIG) && (iter->flags & LYS_CONFIG_W)) {
            continue;
        }

        switch (iter->nodetype) {
        case LYS_CHOICE:
            node = lys_getnext_data(NULL, *first, NULL, iter, NULL);
            if (!node && ((struct lysc_node_choice *)iter)->dflt) {
                /* create default case data */
                LY_CHECK_RET(lyd_new_implicit_r(parent, first, &((struct lysc_node_choice *)iter)->dflt->node,
                        NULL, node_when, node_types, ext_node, impl_opts, diff));
            } else if (node) {
                /* create any default data in the existing case */
                assert(node->schema->parent->nodetype == LYS_CASE);
                LY_CHECK_RET(lyd_new_implicit_r(parent, first, node->schema->parent, NULL, node_when, node_types,
                        ext_node, impl_opts, diff));
            }
            break;
        case LYS_CONTAINER:
            if (!(iter->flags & LYS_PRESENCE) && lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default NP container */
                LY_CHECK_RET(lyd_create_inner(iter, &node));
                node->flags = LYD_DEFAULT | (lysc_has_when(iter) ? LYD_WHEN_TRUE : 0);
                lyd_insert_node(parent, first, node, 0);

                if (lysc_has_when(iter) && node_when) {
                    /* remember to resolve when */
                    LY_CHECK_RET(ly_set_add(node_when, node, 1, NULL));
                }
                if (ext_node) {
                    /* store for ext instance node validation, if needed */
                    LY_CHECK_RET(lyd_validate_node_ext(node, ext_node));
                }
                if (diff) {
                    /* add into diff */
                    LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
                }

                /* create any default children */
                LY_CHECK_RET(lyd_new_implicit_r(node, lyd_node_child_p(node), NULL, NULL, node_when, node_types,
                        ext_node, impl_opts, diff));
            }
            break;
        case LYS_LEAF:
            if (!(impl_opts & LYD_IMPLICIT_NO_DEFAULTS) && ((struct lysc_node_leaf *)iter)->dflt &&
                    lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create default leaf */
                ret = lyd_create_term2(iter, ((struct lysc_node_leaf *)iter)->dflt, &node);
                if (ret == LY_EINCOMPLETE) {
                    if (node_types) {
                        /* remember to resolve type */
                        LY_CHECK_RET(ly_set_add(node_types, node, 1, NULL));
                    }
                } else if (ret) {
                    return ret;
                }
                node->flags = LYD_DEFAULT | (lysc_has_when(iter) ? LYD_WHEN_TRUE : 0);
                lyd_insert_node(parent, first, node, 0);

                if (lysc_has_when(iter) && node_when) {
                    /* remember to resolve when */
                    LY_CHECK_RET(ly_set_add(node_when, node, 1, NULL));
                }
                if (ext_node) {
                    /* store for ext instance node validation, if needed */
                    LY_CHECK_RET(lyd_validate_node_ext(node, ext_node));
                }
                if (diff) {
                    /* add into diff */
                    LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
                }
            }
            break;
        case LYS_LEAFLIST:
            if (!(impl_opts & LYD_IMPLICIT_NO_DEFAULTS) && ((struct lysc_node_leaflist *)iter)->dflts &&
                    lyd_find_sibling_val(*first, iter, NULL, 0, NULL)) {
                /* create all default leaf-lists */
                dflts = ((struct lysc_node_leaflist *)iter)->dflts;
                LY_ARRAY_FOR(dflts, u) {
                    ret = lyd_create_term2(iter, dflts[u], &node);
                    if (ret == LY_EINCOMPLETE) {
                        if (node_types) {
                            /* remember to resolve type */
                            LY_CHECK_RET(ly_set_add(node_types, node, 1, NULL));
                        }
                    } else if (ret) {
                        return ret;
                    }
                    node->flags = LYD_DEFAULT | (lysc_has_when(iter) ? LYD_WHEN_TRUE : 0);
                    lyd_insert_node(parent, first, node, 0);

                    if (lysc_has_when(iter) && node_when) {
                        /* remember to resolve when */
                        LY_CHECK_RET(ly_set_add(node_when, node, 1, NULL));
                    }
                    if (ext_node) {
                        /* store for ext instance node validation, if needed */
                        LY_CHECK_RET(lyd_validate_node_ext(node, ext_node));
                    }
                    if (diff) {
                        /* add into diff */
                        LY_CHECK_RET(lyd_val_diff_add(node, LYD_DIFF_OP_CREATE, diff));
                    }
                }
            }
            break;
        default:
            /* without defaults */
            break;
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_new_implicit_tree(struct lyd_node *tree, uint32_t implicit_options, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node;
    struct ly_set node_when = {0};

    LY_CHECK_ARG_RET(NULL, tree, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    LYD_TREE_DFS_BEGIN(tree, node) {
        if (node->schema->nodetype & LYD_NODE_INNER) {
            LY_CHECK_GOTO(ret = lyd_new_implicit_r(node, lyd_node_child_p(node), NULL, NULL, &node_when, NULL,
                    NULL, implicit_options, diff), cleanup);
        }

        LYD_TREE_DFS_END(tree, node);
    }

    /* resolve when and remove any invalid defaults */
    ret = lyd_validate_unres(&tree, NULL, 0, &node_when, LYXP_IGNORE_WHEN, NULL, NULL, NULL, NULL, 0, diff);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    ly_set_erase(&node_when, NULL);
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_new_implicit_all(struct lyd_node **tree, const struct ly_ctx *ctx, uint32_t implicit_options, struct lyd_node **diff)
{
    const struct lys_module *mod;
    struct lyd_node *d = NULL;
    uint32_t i = 0;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(ctx, tree, *tree || ctx, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(*tree ? LYD_CTX(*tree) : NULL, ctx, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }
    if (!ctx) {
        ctx = LYD_CTX(*tree);
    }

    /* add nodes for each module one-by-one */
    while ((mod = ly_ctx_get_module_iter(ctx, &i))) {
        if (!mod->implemented) {
            continue;
        }

        LY_CHECK_GOTO(ret = lyd_new_implicit_module(tree, mod, implicit_options, diff ? &d : NULL), cleanup);
        if (d) {
            /* merge into one diff */
            lyd_insert_sibling(*diff, d, diff);

            d = NULL;
        }
    }

cleanup:
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_new_implicit_module(struct lyd_node **tree, const struct lys_module *module, uint32_t implicit_options,
        struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *root, *d = NULL;
    struct ly_set node_when = {0};

    LY_CHECK_ARG_RET(NULL, tree, module, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(*tree ? LYD_CTX(*tree) : NULL, module ? module->ctx : NULL, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    /* add all top-level defaults for this module */
    LY_CHECK_GOTO(ret = lyd_new_implicit_r(NULL, tree, NULL, module, &node_when, NULL, NULL, implicit_options, diff),
            cleanup);

    /* resolve when and remove any invalid defaults */
    LY_CHECK_GOTO(ret = lyd_validate_unres(tree, module, 0, &node_when, LYXP_IGNORE_WHEN, NULL, NULL, NULL, NULL,
            0, diff), cleanup);

    /* process nested nodes */
    LY_LIST_FOR(*tree, root) {
        LY_CHECK_GOTO(ret = lyd_new_implicit_tree(root, implicit_options, diff ? &d : NULL), cleanup);

        if (d) {
            /* merge into one diff */
            lyd_insert_sibling(*diff, d, diff);
            d = NULL;
        }
    }

cleanup:
    ly_set_erase(&node_when, NULL);
    if (ret && diff) {
        lyd_free_all(*diff);
        *diff = NULL;
    }
    return ret;
}
