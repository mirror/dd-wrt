/**
 * @file tree_data_common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Parsing and validation common functions for data trees
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "lyb.h"
#include "parser_data.h"
#include "plugins_exts.h"
#include "printer_data.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"
#include "xpath.h"

/**
 * @brief Callback for checking first instance hash table values equivalence.
 *
 * @param[in] val1_p If not @p mod, pointer to the first instance.
 * @param[in] val2_p If not @p mod, pointer to the found dup inst item.
 */
static ly_bool
lyht_dup_inst_ht_equal_cb(void *val1_p, void *val2_p, ly_bool mod, void *UNUSED(cb_data))
{
    if (mod) {
        struct lyd_dup_inst **item1 = val1_p, **item2 = val2_p;

        /* equal on 2 dup inst items */
        return *item1 == *item2 ? 1 : 0;
    } else {
        struct lyd_node **first_inst = val1_p;
        struct lyd_dup_inst **item = val2_p;

        /* equal on dup inst item and a first instance */
        return (*item)->set->dnodes[0] == *first_inst ? 1 : 0;
    }
}

/**
 * @brief Find an entry in duplicate instance cache for an instance. Create it if it does not exist.
 *
 * @param[in] first_inst First instance of the cache entry.
 * @param[in] dup_inst_ht Duplicate instance cache hash table.
 * @return Instance cache entry.
 */
static struct lyd_dup_inst *
lyd_dup_inst_get(const struct lyd_node *first_inst, struct ly_ht **dup_inst_ht)
{
    struct lyd_dup_inst **item_p, *item;

    if (*dup_inst_ht) {
        /* find the item of the first instance */
        if (!lyht_find(*dup_inst_ht, &first_inst, first_inst->hash, (void **)&item_p)) {
            return *item_p;
        }
    } else {
        /* create the hash table */
        *dup_inst_ht = lyht_new(2, sizeof item, lyht_dup_inst_ht_equal_cb, NULL, 1);
        LY_CHECK_RET(!*dup_inst_ht, NULL);
    }

    /* first instance has no dup inst item, create it */
    item = calloc(1, sizeof *item);
    LY_CHECK_RET(!item, NULL);

    /* add into the hash table */
    if (lyht_insert(*dup_inst_ht, &item, first_inst->hash, NULL)) {
        return NULL;
    }

    return item;
}

LY_ERR
lyd_dup_inst_next(struct lyd_node **inst, const struct lyd_node *siblings, struct ly_ht **dup_inst_ht)
{
    struct lyd_dup_inst *dup_inst;

    if (!*inst) {
        /* no match, inst is unchanged */
        return LY_SUCCESS;
    }

    /* there can be more exact same instances (even if not allowed in invalid data) and we must make sure we do not
     * match a single node more times */
    dup_inst = lyd_dup_inst_get(*inst, dup_inst_ht);
    LY_CHECK_ERR_RET(!dup_inst, LOGMEM(LYD_CTX(siblings)), LY_EMEM);

    if (!dup_inst->used) {
        /* we did not cache these instances yet, do so */
        lyd_find_sibling_dup_inst_set(siblings, *inst, &dup_inst->set);
        assert(dup_inst->set->count && (dup_inst->set->dnodes[0] == *inst));
    }

    if (dup_inst->used == dup_inst->set->count) {
        if (lysc_is_dup_inst_list((*inst)->schema)) {
            /* we have used all the instances */
            *inst = NULL;
        } /* else just keep using the last (ideally only) instance */
    } else {
        assert(dup_inst->used < dup_inst->set->count);

        /* use another instance */
        *inst = dup_inst->set->dnodes[dup_inst->used];
        ++dup_inst->used;
    }

    return LY_SUCCESS;
}

/**
 * @brief Callback for freeing first instance hash table values.
 */
static void
lyht_dup_inst_ht_free_cb(void *val_p)
{
    struct lyd_dup_inst **item = val_p;

    ly_set_free((*item)->set, NULL);
    free(*item);
}

void
lyd_dup_inst_free(struct ly_ht *dup_inst_ht)
{
    lyht_free(dup_inst_ht, lyht_dup_inst_ht_free_cb);
}

struct lyd_node *
lys_getnext_data(const struct lyd_node *last, const struct lyd_node *sibling, const struct lysc_node **slast,
        const struct lysc_node *parent, const struct lysc_module *module)
{
    const struct lysc_node *siter = NULL;
    struct lyd_node *match = NULL;

    assert(parent || module);
    assert(!last || (slast && *slast));

    if (slast) {
        siter = *slast;
    }

    if (last && last->next && (last->next->schema == siter)) {
        /* return next data instance */
        return last->next;
    }

    /* find next schema node data instance */
    while ((siter = lys_getnext(siter, parent, module, 0))) {
        if (!lyd_find_sibling_val(sibling, siter, NULL, 0, &match)) {
            break;
        }
    }

    if (slast) {
        *slast = siter;
    }
    return match;
}

struct lyd_node **
lyd_node_child_p(struct lyd_node *node)
{
    assert(node);

    if (!node->schema) {
        return &((struct lyd_node_opaq *)node)->child;
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            return &((struct lyd_node_inner *)node)->child;
        default:
            return NULL;
        }
    }
}

LIBYANG_API_DEF LY_ERR
lyxp_vars_set(struct lyxp_var **vars, const char *name, const char *value)
{
    LY_ERR ret = LY_SUCCESS;
    char *var_name = NULL, *var_value = NULL;
    struct lyxp_var *item;

    if (!vars || !name || !value) {
        return LY_EINVAL;
    }

    /* if variable is already defined then change its value */
    if (*vars && !lyxp_vars_find(NULL, *vars, name, 0, &item)) {
        var_value = strdup(value);
        LY_CHECK_RET(!var_value, LY_EMEM);

        /* update value */
        free(item->value);
        item->value = var_value;
    } else {
        var_name = strdup(name);
        var_value = strdup(value);
        LY_CHECK_ERR_GOTO(!var_name || !var_value, ret = LY_EMEM, error);

        /* add new variable */
        LY_ARRAY_NEW_GOTO(NULL, *vars, item, ret, error);
        item->name = var_name;
        item->value = var_value;
    }

    return LY_SUCCESS;

error:
    free(var_name);
    free(var_value);
    return ret;
}

LIBYANG_API_DEF void
lyxp_vars_free(struct lyxp_var *vars)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!vars) {
        return;
    }

    LY_ARRAY_FOR(vars, u) {
        free(vars[u].name);
        free(vars[u].value);
    }

    LY_ARRAY_FREE(vars);
}

LIBYANG_API_DEF struct lyd_node *
lyd_child_no_keys(const struct lyd_node *node)
{
    struct lyd_node **children;

    if (!node) {
        return NULL;
    }

    if (!node->schema) {
        /* opaq node */
        return ((struct lyd_node_opaq *)node)->child;
    }

    children = lyd_node_child_p((struct lyd_node *)node);
    if (children) {
        struct lyd_node *child = *children;

        while (child && child->schema && (child->schema->flags & LYS_KEY)) {
            child = child->next;
        }
        return child;
    } else {
        return NULL;
    }
}

LIBYANG_API_DEF const struct lys_module *
lyd_owner_module(const struct lyd_node *node)
{
    const struct lyd_node_opaq *opaq;

    if (!node) {
        return NULL;
    }

    while (!node->schema && node->parent) {
        node = lyd_parent(node);
    }

    if (!node->schema) {
        /* top-level opaque node */
        opaq = (struct lyd_node_opaq *)node;
        switch (opaq->format) {
        case LY_VALUE_XML:
            if (opaq->name.module_ns) {
                return ly_ctx_get_module_implemented_ns(LYD_CTX(node), opaq->name.module_ns);
            }
            break;
        case LY_VALUE_JSON:
            if (opaq->name.module_name) {
                return ly_ctx_get_module_implemented(LYD_CTX(node), opaq->name.module_name);
            }
            break;
        default:
            return NULL;
        }

        return NULL;
    }

    return lysc_owner_module(node->schema);
}

LIBYANG_API_DEF const struct lys_module *
lyd_node_module(const struct lyd_node *node)
{
    const struct lyd_node_opaq *opaq;

    while (node) {
        /* data node */
        if (node->schema) {
            return node->schema->module;
        }

        /* opaque node */
        opaq = (struct lyd_node_opaq *)node;
        switch (opaq->format) {
        case LY_VALUE_XML:
            if (opaq->name.module_ns) {
                return ly_ctx_get_module_implemented_ns(LYD_CTX(node), opaq->name.module_ns);
            }
            break;
        case LY_VALUE_JSON:
            if (opaq->name.module_name) {
                return ly_ctx_get_module_implemented(LYD_CTX(node), opaq->name.module_name);
            }
            break;
        default:
            break;
        }

        node = lyd_parent(node);
    }

    return NULL;
}

void
lyd_first_module_sibling(struct lyd_node **node, const struct lys_module *mod)
{
    int cmp;
    struct lyd_node *first;
    const struct lys_module *own_mod;

    assert(node && mod);

    if (!*node) {
        return;
    }

    first = *node;
    own_mod = lyd_owner_module(first);
    cmp = own_mod ? strcmp(own_mod->name, mod->name) : 1;
    if (cmp > 0) {
        /* there may be some preceding data */
        while (first->prev->next) {
            first = first->prev;
            if (lyd_owner_module(first) == mod) {
                cmp = 0;
                break;
            }
        }
    }

    if (cmp == 0) {
        /* there may be some preceding data belonging to this module */
        while (first->prev->next) {
            if (lyd_owner_module(first->prev) != mod) {
                break;
            }
            first = first->prev;
        }
    }

    if (cmp < 0) {
        /* there may be some following data */
        LY_LIST_FOR(first, first) {
            if (lyd_owner_module(first) == mod) {
                cmp = 0;
                break;
            }
        }
    }

    if (cmp == 0) {
        /* we have found the first module data node */
        *node = first;
    }
}

const struct lys_module *
lyd_mod_next_module(struct lyd_node *tree, const struct lys_module *module, const struct ly_ctx *ctx, uint32_t *i,
        struct lyd_node **first)
{
    struct lyd_node *iter;
    const struct lys_module *mod;

    /* get the next module */
    if (module) {
        if (*i) {
            mod = NULL;
        } else {
            mod = module;
            ++(*i);
        }
    } else {
        do {
            mod = ly_ctx_get_module_iter(ctx, i);
        } while (mod && !mod->implemented);
    }

    /* find its data */
    *first = NULL;
    if (mod) {
        LY_LIST_FOR(tree, iter) {
            if (lyd_owner_module(iter) == mod) {
                *first = iter;
                break;
            }
        }
    }

    return mod;
}

const struct lys_module *
lyd_data_next_module(struct lyd_node **next, struct lyd_node **first)
{
    const struct lys_module *mod;

    if (!*next) {
        /* all data traversed */
        *first = NULL;
        return NULL;
    }

    *first = *next;

    /* prepare next */
    mod = lyd_owner_module(*next);
    LY_LIST_FOR(*next, *next) {
        if (lyd_owner_module(*next) != mod) {
            break;
        }
    }

    return mod;
}

LY_ERR
lyd_value_store(const struct ly_ctx *ctx, struct lyd_value *val, const struct lysc_type *type, const void *value,
        size_t value_len, ly_bool is_utf8, ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, ly_bool *incomplete)
{
    LY_ERR ret;
    struct ly_err_item *err = NULL;
    uint32_t options = 0;

    if (!value) {
        value = "";
    }
    if (incomplete) {
        *incomplete = 0;
    }

    if (dynamic && *dynamic) {
        options |= LYPLG_TYPE_STORE_DYNAMIC;
    }
    if (is_utf8) {
        options |= LYPLG_TYPE_STORE_IS_UTF8;
    }

    ret = type->plugin->store(ctx, type, value, value_len, options, format, prefix_data, hints, ctx_node, val, NULL, &err);
    if (dynamic) {
        *dynamic = 0;
    }

    if (ret == LY_EINCOMPLETE) {
        if (incomplete) {
            *incomplete = 1;
        }
    } else if (ret) {
        if (err) {
            LOGVAL_ERRITEM(ctx, err);
            ly_err_free(err);
        } else {
            LOGVAL(ctx, LYVE_OTHER, "Storing value failed.");
        }
        return ret;
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_value_validate_incomplete(const struct ly_ctx *ctx, const struct lysc_type *type, struct lyd_value *val,
        const struct lyd_node *ctx_node, const struct lyd_node *tree)
{
    LY_ERR ret;
    struct ly_err_item *err = NULL;

    assert(type->plugin->validate);

    ret = type->plugin->validate(ctx, type, ctx_node, tree, val, &err);
    if (ret) {
        if (err) {
            LOGVAL_ERRITEM(ctx, err);
            ly_err_free(err);
        } else {
            LOGVAL(ctx, LYVE_OTHER, "Resolving value \"%s\" failed.",
                    (char *)type->plugin->print(ctx, val, LY_VALUE_CANON, NULL, NULL, NULL));
        }
        return ret;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_err_item *err = NULL;
    struct lyd_value storage;
    struct lysc_type *type;

    LY_CHECK_ARG_RET(ctx, node, LY_EINVAL);

    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGARG(ctx, node);
        return LY_EINVAL;
    }

    type = ((struct lysc_node_leaf *)node)->type;
    rc = type->plugin->store(ctx ? ctx : node->module->ctx, type, value, value_len, 0, format, prefix_data, hints, node,
            &storage, NULL, &err);
    if (rc == LY_EINCOMPLETE) {
        /* actually success since we do not provide the context tree and call validation with
         * LY_TYPE_OPTS_INCOMPLETE_DATA */
        rc = LY_SUCCESS;
    } else if (rc && err) {
        if (ctx) {
            /* log only in case the ctx was provided as input parameter */
            if (err->path) {
                LOG_LOCSET(NULL, NULL, err->path, NULL);
            } else {
                LOG_LOCSET(node, NULL, NULL, NULL);
            }
            LOGVAL_ERRITEM(ctx, err);
            if (err->path) {
                LOG_LOCBACK(0, 0, 1, 0);
            } else {
                LOG_LOCBACK(1, 0, 1, 0);
            }
        }
        ly_err_free(err);
    }

    if (!rc) {
        type->plugin->free(ctx ? ctx : node->module->ctx, &storage);
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_value_validate(const struct ly_ctx *ctx, const struct lysc_node *schema, const char *value, size_t value_len,
        const struct lyd_node *ctx_node, const struct lysc_type **realtype, const char **canonical)
{
    LY_ERR rc;
    struct ly_err_item *err = NULL;
    struct lysc_type *type;
    struct lyd_value val = {0};
    ly_bool stored = 0, log = 1;

    LY_CHECK_ARG_RET(ctx, schema, !value_len || value, LY_EINVAL);

    if (!ctx) {
        ctx = schema->module->ctx;
        log = 0;
    }
    if (!value_len) {
        value = "";
    }
    type = ((struct lysc_node_leaf *)schema)->type;

    /* store */
    rc = type->plugin->store(ctx, type, value, value_len, 0, LY_VALUE_JSON, NULL,
            LYD_HINT_DATA, schema, &val, NULL, &err);
    if (!rc || (rc == LY_EINCOMPLETE)) {
        stored = 1;
    }

    if (ctx_node && (rc == LY_EINCOMPLETE)) {
        /* resolve */
        rc = type->plugin->validate(ctx, type, ctx_node, ctx_node, &val, &err);
    }

    if (rc && (rc != LY_EINCOMPLETE) && err) {
        if (log) {
            /* log error */
            if (err->path) {
                LOG_LOCSET(NULL, NULL, err->path, NULL);
            } else if (ctx_node) {
                LOG_LOCSET(NULL, ctx_node, NULL, NULL);
            } else {
                LOG_LOCSET(schema, NULL, NULL, NULL);
            }
            LOGVAL_ERRITEM(ctx, err);
            if (err->path) {
                LOG_LOCBACK(0, 0, 1, 0);
            } else if (ctx_node) {
                LOG_LOCBACK(0, 1, 0, 0);
            } else {
                LOG_LOCBACK(1, 0, 0, 0);
            }
        }
        ly_err_free(err);
    }

    if (!rc || (rc == LY_EINCOMPLETE)) {
        if (realtype) {
            /* return realtype */
            if (val.realtype->basetype == LY_TYPE_UNION) {
                *realtype = val.subvalue->value.realtype;
            } else {
                *realtype = val.realtype;
            }
        }

        if (canonical) {
            /* return canonical value */
            lydict_insert(ctx, val.realtype->plugin->print(ctx, &val, LY_VALUE_CANON, NULL, NULL, NULL), 0, canonical);
        }
    }

    if (stored) {
        /* free value */
        type->plugin->free(ctx ? ctx : schema->module->ctx, &val);
    }
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_value_compare(const struct lyd_node_term *node, const char *value, size_t value_len)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx;
    struct lysc_type *type;
    struct lyd_value val = {0};

    LY_CHECK_ARG_RET(node ? node->schema->module->ctx : NULL, node, value, LY_EINVAL);

    ctx = node->schema->module->ctx;
    type = ((struct lysc_node_leaf *)node->schema)->type;

    /* store the value */
    LOG_LOCSET(node->schema, &node->node, NULL, NULL);
    ret = lyd_value_store(ctx, &val, type, value, value_len, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, node->schema, NULL);
    LOG_LOCBACK(1, 1, 0, 0);
    LY_CHECK_RET(ret);

    /* compare values */
    ret = type->plugin->compare(&node->value, &val);

    type->plugin->free(ctx, &val);
    return ret;
}

LIBYANG_API_DEF ly_bool
lyd_is_default(const struct lyd_node *node)
{
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lyd_node_term *term;
    LY_ARRAY_COUNT_TYPE u;

    if (!(node->schema->nodetype & LYD_NODE_TERM)) {
        return 0;
    }

    term = (const struct lyd_node_term *)node;

    if (node->schema->nodetype == LYS_LEAF) {
        leaf = (const struct lysc_node_leaf *)node->schema;
        if (!leaf->dflt) {
            return 0;
        }

        /* compare with the default value */
        if (!leaf->type->plugin->compare(&term->value, leaf->dflt)) {
            return 1;
        }
    } else {
        llist = (const struct lysc_node_leaflist *)node->schema;
        if (!llist->dflts) {
            return 0;
        }

        LY_ARRAY_FOR(llist->dflts, u) {
            /* compare with each possible default value */
            if (!llist->type->plugin->compare(&term->value, llist->dflts[u])) {
                return 1;
            }
        }
    }

    return 0;
}

LIBYANG_API_DEF uint32_t
lyd_list_pos(const struct lyd_node *instance)
{
    const struct lyd_node *iter = NULL;
    uint32_t pos = 0;

    if (!instance || !(instance->schema->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
        return 0;
    }

    /* data instances are ordered, so we can stop when we found instance of other schema node */
    for (iter = instance; iter->schema == instance->schema; iter = iter->prev) {
        if (pos && (iter->next == NULL)) {
            /* overrun to the end of the siblings list */
            break;
        }
        ++pos;
    }

    return pos;
}

LIBYANG_API_DEF struct lyd_node *
lyd_first_sibling(const struct lyd_node *node)
{
    struct lyd_node *start;

    if (!node) {
        return NULL;
    }

    /* get the first sibling */
    if (node->parent) {
        start = node->parent->child;
    } else {
        for (start = (struct lyd_node *)node; start->prev->next; start = start->prev) {}
    }

    return start;
}

/**
 * @brief Check list node parsed into an opaque node for the reason.
 *
 * @param[in] node Opaque node.
 * @param[in] snode Schema node of @p opaq.
 * @return LY_SUCCESS if the node is valid;
 * @return LY_ERR on error.
 */
static LY_ERR
lyd_parse_opaq_list_error(const struct lyd_node *node, const struct lysc_node *snode)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_set key_set = {0};
    const struct lysc_node *key = NULL;
    const struct lyd_node *child;
    const struct lyd_node_opaq *opaq_k;
    uint32_t i;

    assert(!node->schema);

    /* get all keys into a set */
    while ((key = lys_getnext(key, snode, NULL, 0)) && (key->flags & LYS_KEY)) {
        LY_CHECK_GOTO(ret = ly_set_add(&key_set, (void *)key, 1, NULL), cleanup);
    }

    LY_LIST_FOR(lyd_child(node), child) {
        /* find the key schema node */
        for (i = 0; i < key_set.count; ++i) {
            key = key_set.snodes[i];
            if (!strcmp(key->name, LYD_NAME(child))) {
                break;
            }
        }
        if (i == key_set.count) {
            /* some other node, skip */
            continue;
        }

        /* key found */
        ly_set_rm_index(&key_set, i, NULL);

        if (child->schema) {
            /* valid key */
            continue;
        }

        /* check value */
        opaq_k = (struct lyd_node_opaq *)child;
        ret = ly_value_validate(LYD_CTX(node), key, opaq_k->value, strlen(opaq_k->value), opaq_k->format,
                opaq_k->val_prefix_data, opaq_k->hints);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (key_set.count) {
        /* missing keys */
        LOGVAL(LYD_CTX(node), LY_VCODE_NOKEY, key_set.snodes[0]->name);
        ret = LY_EVALID;
        goto cleanup;
    }

cleanup:
    ly_set_erase(&key_set, NULL);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_opaq_error(const struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    const struct ly_ctx *ctx;
    const struct lyd_node_opaq *opaq;
    const struct lyd_node *parent;
    const struct lys_module *mod;
    const struct lysc_node *sparent, *snode;
    uint32_t loc_node = 0, loc_path = 0;

    LY_CHECK_ARG_RET(LYD_CTX(node), node, !node->schema, LY_EINVAL);

    ctx = LYD_CTX(node);
    opaq = (struct lyd_node_opaq *)node;
    parent = lyd_parent(node);
    sparent = lyd_node_schema(parent);

    if (parent) {
        LOG_LOCSET(NULL, parent, NULL, NULL);
        ++loc_node;
    } else {
        LOG_LOCSET(NULL, NULL, "/", NULL);
        ++loc_path;
    }

    if (!opaq->name.module_ns) {
        LOGVAL(ctx, LYVE_REFERENCE, "Unknown module of node \"%s\".", opaq->name.name);
        rc = LY_EVALID;
        goto cleanup;
    }

    /* module */
    switch (opaq->format) {
    case LY_VALUE_XML:
        if (!sparent || strcmp(opaq->name.module_ns, sparent->module->ns)) {
            mod = ly_ctx_get_module_implemented_ns(ctx, opaq->name.module_ns);
            if (!mod) {
                LOGVAL(ctx, LYVE_REFERENCE, "No (implemented) module with namespace \"%s\" of node \"%s\" in the context.",
                        opaq->name.module_ns, opaq->name.name);
                rc = LY_EVALID;
                goto cleanup;
            }
        } else {
            /* inherit */
            mod = sparent->module;
        }
        break;
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        if (!sparent || strcmp(opaq->name.module_name, sparent->module->name)) {
            mod = ly_ctx_get_module_implemented(ctx, opaq->name.module_name);
            if (!mod) {
                LOGVAL(ctx, LYVE_REFERENCE, "No (implemented) module named \"%s\" of node \"%s\" in the context.",
                        opaq->name.module_name, opaq->name.name);
                rc = LY_EVALID;
                goto cleanup;
            }
        } else {
            /* inherit */
            mod = sparent->module;
        }
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Unsupported value format.");
        rc = LY_EINVAL;
        goto cleanup;
    }

    /* schema */
    snode = lys_find_child(sparent, mod, opaq->name.name, 0, 0, 0);
    if (!snode && sparent && (sparent->nodetype & (LYS_RPC | LYS_ACTION))) {
        /* maybe output node */
        snode = lys_find_child(sparent, mod, opaq->name.name, 0, 0, LYS_GETNEXT_OUTPUT);
    }
    if (!snode) {
        if (sparent) {
            LOGVAL(ctx, LYVE_REFERENCE, "Node \"%s\" not found as a child of \"%s\" node.", opaq->name.name,
                    sparent->name);
        } else {
            LOGVAL(ctx, LYVE_REFERENCE, "Node \"%s\" not found in the \"%s\" module.", opaq->name.name, mod->name);
        }
        rc = LY_EVALID;
        goto cleanup;
    }

    /* schema node exists */
    LOG_LOCBACK(0, loc_node, loc_path, 0);
    loc_node = 0;
    loc_path = 0;
    LOG_LOCSET(NULL, node, NULL, NULL);
    ++loc_node;

    if (snode->nodetype & LYD_NODE_TERM) {
        /* leaf / leaf-list */
        rc = ly_value_validate(ctx, snode, opaq->value, strlen(opaq->value), opaq->format, opaq->val_prefix_data, opaq->hints);
        LY_CHECK_GOTO(rc, cleanup);
    } else if (snode->nodetype == LYS_LIST) {
        /* list */
        rc = lyd_parse_opaq_list_error(node, snode);
        LY_CHECK_GOTO(rc, cleanup);
    } else if (snode->nodetype & LYD_NODE_INNER) {
        /* inner node */
        if (opaq->value) {
            LOGVAL(ctx, LYVE_DATA, "Invalid value \"%s\" for %s \"%s\".", opaq->value,
                    lys_nodetype2str(snode->nodetype), snode->name);
            rc = LY_EVALID;
            goto cleanup;
        }
    } else {
        LOGERR(ctx, LY_EINVAL, "Unexpected opaque schema node %s \"%s\".", lys_nodetype2str(snode->nodetype), snode->name);
        rc = LY_EINVAL;
        goto cleanup;
    }

    LOGERR(ctx, LY_EINVAL, "Unexpected valid opaque node %s \"%s\".", lys_nodetype2str(snode->nodetype), snode->name);
    rc = LY_EINVAL;

cleanup:
    LOG_LOCBACK(0, loc_node, loc_path, 0);
    return rc;
}

LIBYANG_API_DEF const char *
lyd_value_get_canonical(const struct ly_ctx *ctx, const struct lyd_value *value)
{
    LY_CHECK_ARG_RET(ctx, ctx, value, NULL);

    return value->_canonical ? value->_canonical :
           (const char *)value->realtype->plugin->print(ctx, value, LY_VALUE_CANON, NULL, NULL, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_any_value_str(const struct lyd_node *any, char **value_str)
{
    const struct lyd_node_any *a;
    struct lyd_node *tree = NULL;
    const char *str = NULL;
    ly_bool dynamic = 0;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, any, value_str, LY_EINVAL);
    LY_CHECK_ARG_RET(NULL, any->schema, any->schema->nodetype & LYS_ANYDATA, LY_EINVAL);

    a = (struct lyd_node_any *)any;
    *value_str = NULL;

    if (!a->value.str) {
        /* there is no value in the union */
        return LY_SUCCESS;
    }

    switch (a->value_type) {
    case LYD_ANYDATA_LYB:
        /* parse into a data tree */
        ret = lyd_parse_data_mem(LYD_CTX(any), a->value.mem, LYD_LYB, LYD_PARSE_ONLY, 0, &tree);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;
        break;
    case LYD_ANYDATA_DATATREE:
        tree = a->value.tree;
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        /* simply use the string */
        str = a->value.str;
        break;
    }

    if (tree) {
        /* print into a string */
        ret = lyd_print_mem(value_str, tree, LYD_XML, LYD_PRINT_WITHSIBLINGS);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        assert(str);
        *value_str = strdup(str);
        LY_CHECK_ERR_GOTO(!*value_str, LOGMEM(LYD_CTX(any)), cleanup);
    }

    /* success */

cleanup:
    if (dynamic) {
        lyd_free_all(tree);
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_any_copy_value(struct lyd_node *trg, const union lyd_any_value *value, LYD_ANYDATA_VALUETYPE value_type)
{
    struct lyd_node_any *t;

    LY_CHECK_ARG_RET(NULL, trg, LY_EINVAL);
    LY_CHECK_ARG_RET(NULL, trg->schema, trg->schema->nodetype & LYS_ANYDATA, LY_EINVAL);

    t = (struct lyd_node_any *)trg;

    /* free trg */
    switch (t->value_type) {
    case LYD_ANYDATA_DATATREE:
        lyd_free_all(t->value.tree);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        lydict_remove(LYD_CTX(trg), t->value.str);
        break;
    case LYD_ANYDATA_LYB:
        free(t->value.mem);
        break;
    }
    t->value.str = NULL;

    if (!value) {
        /* only free value in this case */
        return LY_SUCCESS;
    }

    /* copy src */
    t->value_type = value_type;
    switch (value_type) {
    case LYD_ANYDATA_DATATREE:
        if (value->tree) {
            LY_CHECK_RET(lyd_dup_siblings(value->tree, NULL, LYD_DUP_RECURSIVE, &t->value.tree));
        }
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        if (value->str) {
            LY_CHECK_RET(lydict_insert(LYD_CTX(trg), value->str, 0, &t->value.str));
        }
        break;
    case LYD_ANYDATA_LYB:
        if (value->mem) {
            int len = lyd_lyb_data_length(value->mem);

            LY_CHECK_RET(len == -1, LY_EINVAL);
            t->value.mem = malloc(len);
            LY_CHECK_ERR_RET(!t->value.mem, LOGMEM(LYD_CTX(trg)), LY_EMEM);
            memcpy(t->value.mem, value->mem, len);
        }
        break;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF const struct lysc_node *
lyd_node_schema(const struct lyd_node *node)
{
    const struct lysc_node *schema = NULL;
    const struct lyd_node *prev_iter = NULL, *iter;
    const struct lys_module *mod;

    if (!node) {
        return NULL;
    } else if (node->schema) {
        return node->schema;
    }

    /* find the first schema node in the parents */
    for (iter = lyd_parent(node); iter && !iter->schema; iter = lyd_parent(iter)) {}
    if (iter) {
        prev_iter = iter;
        schema = prev_iter->schema;
    }

    /* get schema node of an opaque node */
    do {
        /* get next data node */
        for (iter = node; lyd_parent(iter) != prev_iter; iter = lyd_parent(iter)) {}

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
        prev_iter = iter;
    } while (schema && (iter != node));

    return schema;
}

void
lyd_cont_set_dflt(struct lyd_node *node)
{
    const struct lyd_node *child;

    while (node) {
        if (!node->schema || (node->flags & LYD_DEFAULT) || !lysc_is_np_cont(node->schema)) {
            /* not a non-dflt NP container */
            break;
        }

        LY_LIST_FOR(lyd_child(node), child) {
            if (!(child->flags & LYD_DEFAULT)) {
                break;
            }
        }
        if (child) {
            /* explicit child, no dflt change */
            break;
        }

        /* set the dflt flag */
        node->flags |= LYD_DEFAULT;

        /* check all parent containers */
        node = lyd_parent(node);
    }
}

/**
 * @brief Comparison callback to match schema node with a schema of a data node.
 *
 * @param[in] val1_p Pointer to the schema node
 * @param[in] val2_p Pointer to the data node
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyd_hash_table_schema_val_equal(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lysc_node *val1;
    struct lyd_node *val2;

    val1 = *((struct lysc_node **)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    if (val1 == val2->schema) {
        /* schema match is enough */
        return 1;
    } else {
        return 0;
    }
}

LY_ERR
lyd_find_sibling_schema(const struct lyd_node *siblings, const struct lysc_node *schema, struct lyd_node **match)
{
    struct lyd_node **match_p;
    struct lyd_node_inner *parent;
    uint32_t hash;

    assert(schema);
    if (!siblings) {
        /* no data */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    parent = siblings->parent;
    if (parent && parent->schema && parent->children_ht) {
        /* calculate our hash */
        hash = lyht_hash_multi(0, schema->module->name, strlen(schema->module->name));
        hash = lyht_hash_multi(hash, schema->name, strlen(schema->name));
        hash = lyht_hash_multi(hash, NULL, 0);

        /* find by hash but use special hash table function (and stay thread-safe) */
        if (!lyht_find_with_val_cb(parent->children_ht, &schema, hash, lyd_hash_table_schema_val_equal, (void **)&match_p)) {
            siblings = *match_p;
        } else {
            /* not found */
            siblings = NULL;
        }
    } else {
        /* find first sibling */
        if (siblings->parent) {
            siblings = siblings->parent->child;
        } else {
            while (siblings->prev->next) {
                siblings = siblings->prev;
            }
        }

        /* search manually without hashes and ignore opaque nodes (cannot be found by hashes) */
        for ( ; siblings && siblings->schema; siblings = siblings->next) {
            /* schema match is enough */
            if (LYD_CTX(siblings) == schema->module->ctx) {
                if (siblings->schema == schema) {
                    break;
                }
            } else {
                if (!strcmp(LYD_NAME(siblings), schema->name) && !strcmp(siblings->schema->module->name, schema->module->name)) {
                    break;
                }
            }
        }
        if (siblings && !siblings->schema) {
            siblings = NULL;
        }
    }

    if (!siblings) {
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (match) {
        *match = (struct lyd_node *)siblings;
    }
    return LY_SUCCESS;
}

void
lyd_del_move_root(struct lyd_node **root, const struct lyd_node *to_del, const struct lys_module *mod)
{
    if (*root && (lyd_owner_module(*root) != mod)) {
        /* there are no data of mod so this is simply the first top-level sibling */
        mod = NULL;
    }

    if ((*root != to_del) || (*root)->parent) {
        return;
    }

    if (mod && (*root)->prev->next && (!(*root)->next || (lyd_owner_module(to_del) != lyd_owner_module((*root)->next)))) {
        /* there are no more nodes from mod, simply get the first top-level sibling */
        *root = lyd_first_sibling(*root);
    } else {
        *root = (*root)->next;
    }
}

LY_ERR
ly_nested_ext_schema(const struct lyd_node *parent, const struct lysc_node *sparent, const char *prefix,
        size_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data, const char *name, size_t name_len,
        const struct lysc_node **snode, struct lysc_ext_instance **ext)
{
    LY_ERR r;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ext_instance *nested_exts = NULL;
    lyplg_ext_data_snode_clb ext_snode_cb;

    /* check if there are any nested extension instances */
    if (parent && parent->schema) {
        nested_exts = parent->schema->exts;
    } else if (sparent) {
        nested_exts = sparent->exts;
    }
    LY_ARRAY_FOR(nested_exts, u) {
        if (!nested_exts[u].def->plugin) {
            /* no plugin */
            continue;
        }

        ext_snode_cb = nested_exts[u].def->plugin->snode;
        if (!ext_snode_cb) {
            /* not an extension with nested data */
            continue;
        }

        /* try to get the schema node */
        r = ext_snode_cb(&nested_exts[u], parent, sparent, prefix, prefix_len, format, prefix_data, name, name_len, snode);
        if (!r) {
            if (ext) {
                /* data successfully created, remember the ext instance */
                *ext = &nested_exts[u];
            }
            return LY_SUCCESS;
        } else if (r != LY_ENOT) {
            /* fatal error */
            return r;
        }
        /* data was not from this module, continue */
    }

    /* no extensions or none matched */
    return LY_ENOT;
}

void
ly_free_prefix_data(LY_VALUE_FORMAT format, void *prefix_data)
{
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;

    if (!prefix_data) {
        return;
    }

    switch (format) {
    case LY_VALUE_XML:
    case LY_VALUE_STR_NS:
        ns_list = prefix_data;
        for (i = 0; i < ns_list->count; ++i) {
            free(((struct lyxml_ns *)ns_list->objs[i])->prefix);
            free(((struct lyxml_ns *)ns_list->objs[i])->uri);
        }
        ly_set_free(ns_list, free);
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        prefixes = prefix_data;
        LY_ARRAY_FOR(prefixes, u) {
            free(prefixes[u].prefix);
        }
        LY_ARRAY_FREE(prefixes);
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_SCHEMA:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        break;
    }
}

LY_ERR
ly_dup_prefix_data(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *prefix_data,
        void **prefix_data_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ns *ns;
    struct lysc_prefix *prefixes = NULL, *orig_pref;
    struct ly_set *ns_list, *orig_ns;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;

    assert(!*prefix_data_p);

    switch (format) {
    case LY_VALUE_SCHEMA:
        *prefix_data_p = (void *)prefix_data;
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        /* copy all the value prefixes */
        orig_pref = (struct lysc_prefix *)prefix_data;
        LY_ARRAY_CREATE_GOTO(ctx, prefixes, LY_ARRAY_COUNT(orig_pref), ret, cleanup);
        *prefix_data_p = prefixes;

        LY_ARRAY_FOR(orig_pref, u) {
            if (orig_pref[u].prefix) {
                prefixes[u].prefix = strdup(orig_pref[u].prefix);
                LY_CHECK_ERR_GOTO(!prefixes[u].prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            }
            prefixes[u].mod = orig_pref[u].mod;
            LY_ARRAY_INCREMENT(prefixes);
        }
        break;
    case LY_VALUE_XML:
    case LY_VALUE_STR_NS:
        /* copy all the namespaces */
        LY_CHECK_GOTO(ret = ly_set_new(&ns_list), cleanup);
        *prefix_data_p = ns_list;

        orig_ns = (struct ly_set *)prefix_data;
        for (i = 0; i < orig_ns->count; ++i) {
            ns = calloc(1, sizeof *ns);
            LY_CHECK_ERR_GOTO(!ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(ns_list, ns, 1, NULL), cleanup);

            if (((struct lyxml_ns *)orig_ns->objs[i])->prefix) {
                ns->prefix = strdup(((struct lyxml_ns *)orig_ns->objs[i])->prefix);
                LY_CHECK_ERR_GOTO(!ns->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            }
            ns->uri = strdup(((struct lyxml_ns *)orig_ns->objs[i])->uri);
            LY_CHECK_ERR_GOTO(!ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        }
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        assert(!prefix_data);
        *prefix_data_p = NULL;
        break;
    }

cleanup:
    if (ret) {
        ly_free_prefix_data(format, *prefix_data_p);
        *prefix_data_p = NULL;
    }
    return ret;
}

LY_ERR
ly_store_prefix_data(const struct ly_ctx *ctx, const void *value, size_t value_len, LY_VALUE_FORMAT format,
        const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lys_module *mod;
    const struct lyxml_ns *ns;
    struct lyxml_ns *new_ns;
    struct ly_set *ns_list;
    struct lysc_prefix *prefixes = NULL, *val_pref;
    const char *value_iter, *value_next, *value_end;
    uint32_t substr_len;
    ly_bool is_prefix;

    switch (format) {
    case LY_VALUE_SCHEMA:
        /* copy all referenced modules as prefix - module pairs */
        if (!*prefix_data_p) {
            /* new prefix data */
            LY_ARRAY_CREATE_GOTO(ctx, prefixes, 0, ret, cleanup);
            *format_p = LY_VALUE_SCHEMA_RESOLVED;
            *prefix_data_p = prefixes;
        } else {
            /* reuse prefix data */
            assert(*format_p == LY_VALUE_SCHEMA_RESOLVED);
            prefixes = *prefix_data_p;
        }

        /* add current module for unprefixed values */
        LY_ARRAY_NEW_GOTO(ctx, prefixes, val_pref, ret, cleanup);
        *prefix_data_p = prefixes;

        val_pref->prefix = NULL;
        val_pref->mod = ((const struct lysp_module *)prefix_data)->mod;

        /* add all used prefixes */
        value_end = (char *)value + value_len;
        for (value_iter = value; value_iter; value_iter = value_next) {
            LY_CHECK_GOTO(ret = ly_value_prefix_next(value_iter, value_end, &substr_len, &is_prefix, &value_next), cleanup);
            if (is_prefix) {
                /* we have a possible prefix. Do we already have the prefix? */
                mod = ly_resolve_prefix(ctx, value_iter, substr_len, *format_p, *prefix_data_p);
                if (!mod) {
                    mod = ly_resolve_prefix(ctx, value_iter, substr_len, format, prefix_data);
                    if (mod) {
                        assert(*format_p == LY_VALUE_SCHEMA_RESOLVED);
                        /* store a new prefix - module pair */
                        LY_ARRAY_NEW_GOTO(ctx, prefixes, val_pref, ret, cleanup);
                        *prefix_data_p = prefixes;

                        val_pref->prefix = strndup(value_iter, substr_len);
                        LY_CHECK_ERR_GOTO(!val_pref->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        val_pref->mod = mod;
                    } /* else it is not even defined */
                } /* else the prefix is already present */
            }
        }
        break;
    case LY_VALUE_XML:
    case LY_VALUE_STR_NS:
        /* copy all referenced namespaces as prefix - namespace pairs */
        if (!*prefix_data_p) {
            /* new prefix data */
            LY_CHECK_GOTO(ret = ly_set_new(&ns_list), cleanup);
            *format_p = format;
            *prefix_data_p = ns_list;
        } else {
            /* reuse prefix data */
            assert(*format_p == format);
            ns_list = *prefix_data_p;
        }

        /* store default namespace */
        ns = lyxml_ns_get(prefix_data, NULL, 0);
        if (ns) {
            new_ns = calloc(1, sizeof *new_ns);
            LY_CHECK_ERR_GOTO(!new_ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(ns_list, new_ns, 1, NULL), cleanup);

            new_ns->prefix = NULL;
            new_ns->uri = strdup(ns->uri);
            LY_CHECK_ERR_GOTO(!new_ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        }

        /* add all used prefixes */
        value_end = (char *)value + value_len;
        for (value_iter = value; value_iter; value_iter = value_next) {
            LY_CHECK_GOTO(ret = ly_value_prefix_next(value_iter, value_end, &substr_len, &is_prefix, &value_next), cleanup);
            if (is_prefix) {
                /* we have a possible prefix. Do we already have the prefix? */
                ns = lyxml_ns_get(ns_list, value_iter, substr_len);
                if (!ns) {
                    ns = lyxml_ns_get(prefix_data, value_iter, substr_len);
                    if (ns) {
                        /* store a new prefix - namespace pair */
                        new_ns = calloc(1, sizeof *new_ns);
                        LY_CHECK_ERR_GOTO(!new_ns, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        LY_CHECK_GOTO(ret = ly_set_add(ns_list, new_ns, 1, NULL), cleanup);

                        new_ns->prefix = strndup(value_iter, substr_len);
                        LY_CHECK_ERR_GOTO(!new_ns->prefix, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                        new_ns->uri = strdup(ns->uri);
                        LY_CHECK_ERR_GOTO(!new_ns->uri, LOGMEM(ctx); ret = LY_EMEM, cleanup);
                    } /* else it is not even defined */
                } /* else the prefix is already present */
            }
        }
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_SCHEMA_RESOLVED:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        if (!*prefix_data_p) {
            /* new prefix data - simply copy all the prefix data */
            *format_p = format;
            LY_CHECK_GOTO(ret = ly_dup_prefix_data(ctx, format, prefix_data, prefix_data_p), cleanup);
        } /* else reuse prefix data - the prefix data are always the same, nothing to do */
        break;
    }

cleanup:
    if (ret) {
        ly_free_prefix_data(*format_p, *prefix_data_p);
        *prefix_data_p = NULL;
    }
    return ret;
}

const char *
ly_format2str(LY_VALUE_FORMAT format)
{
    switch (format) {
    case LY_VALUE_CANON:
        return "canonical";
    case LY_VALUE_SCHEMA:
        return "schema imports";
    case LY_VALUE_SCHEMA_RESOLVED:
        return "schema stored mapping";
    case LY_VALUE_XML:
        return "XML prefixes";
    case LY_VALUE_JSON:
        return "JSON module names";
    case LY_VALUE_LYB:
        return "LYB prefixes";
    default:
        break;
    }

    return NULL;
}

LIBYANG_API_DEF int
ly_time_tz_offset(void)
{
    return ly_time_tz_offset_at(time(NULL));
}

LIBYANG_API_DEF int
ly_time_tz_offset_at(time_t time)
{
    struct tm tm_local, tm_utc;
    int result = 0;

    /* init timezone */
    tzset();

    /* get local and UTC time */
    localtime_r(&time, &tm_local);
    gmtime_r(&time, &tm_utc);

    /* account for year/month/day change by adding/subtracting from the hours, the change cannot be more than 1 day */
    if (tm_local.tm_year < tm_utc.tm_year) {
        tm_utc.tm_hour += 24;
    } else if (tm_local.tm_year > tm_utc.tm_year) {
        tm_local.tm_hour += 24;
    } else if (tm_local.tm_mon < tm_utc.tm_mon) {
        tm_utc.tm_hour += 24;
    } else if (tm_local.tm_mon > tm_utc.tm_mon) {
        tm_local.tm_hour += 24;
    } else if (tm_local.tm_mday < tm_utc.tm_mday) {
        tm_utc.tm_hour += 24;
    } else if (tm_local.tm_mday > tm_utc.tm_mday) {
        tm_local.tm_hour += 24;
    }

    /* hours shift in seconds */
    result += (tm_local.tm_hour - tm_utc.tm_hour) * 3600;

    /* minutes shift in seconds */
    result += (tm_local.tm_min - tm_utc.tm_min) * 60;

    /* seconds shift */
    result += tm_local.tm_sec - tm_utc.tm_sec;

    return result;
}

LIBYANG_API_DEF LY_ERR
ly_time_str2time(const char *value, time_t *time, char **fractions_s)
{
    struct tm tm = {0};
    uint32_t i, frac_len;
    const char *frac;
    int64_t shift, shift_m;
    time_t t;

    LY_CHECK_ARG_RET(NULL, value, time, LY_EINVAL);

    tm.tm_year = atoi(&value[0]) - 1900;
    tm.tm_mon = atoi(&value[5]) - 1;
    tm.tm_mday = atoi(&value[8]);
    tm.tm_hour = atoi(&value[11]);
    tm.tm_min = atoi(&value[14]);
    tm.tm_sec = atoi(&value[17]);

    /* explicit checks for some gross errors */
    if (tm.tm_mon > 11) {
        LOGERR(NULL, LY_EINVAL, "Invalid date-and-time month \"%d\".", tm.tm_mon);
        return LY_EINVAL;
    }
    if ((tm.tm_mday < 1) || (tm.tm_mday > 31)) {
        LOGERR(NULL, LY_EINVAL, "Invalid date-and-time day of month \"%d\".", tm.tm_mday);
        return LY_EINVAL;
    }
    if (tm.tm_hour > 23) {
        LOGERR(NULL, LY_EINVAL, "Invalid date-and-time hours \"%d\".", tm.tm_hour);
        return LY_EINVAL;
    }
    if (tm.tm_min > 59) {
        LOGERR(NULL, LY_EINVAL, "Invalid date-and-time minutes \"%d\".", tm.tm_min);
        return LY_EINVAL;
    }
    if (tm.tm_sec > 60) {
        LOGERR(NULL, LY_EINVAL, "Invalid date-and-time seconds \"%d\".", tm.tm_sec);
        return LY_EINVAL;
    }

    t = timegm(&tm);
    i = 19;

    /* fractions of a second */
    if (value[i] == '.') {
        ++i;
        frac = &value[i];
        for (frac_len = 0; isdigit(frac[frac_len]); ++frac_len) {}

        i += frac_len;
    } else {
        frac = NULL;
    }

    /* apply offset */
    if ((value[i] == 'Z') || (value[i] == 'z')) {
        /* zero shift */
        shift = 0;
    } else {
        shift = strtol(&value[i], NULL, 10);
        if (shift > 23) {
            LOGERR(NULL, LY_EINVAL, "Invalid date-and-time timezone hour \"%" PRIi64 "\".", shift);
            return LY_EINVAL;
        }
        shift = shift * 60 * 60; /* convert from hours to seconds */

        shift_m = strtol(&value[i + 4], NULL, 10);
        if (shift_m > 59) {
            LOGERR(NULL, LY_EINVAL, "Invalid date-and-time timezone minutes \"%" PRIi64 "\".", shift_m);
            return LY_EINVAL;
        }
        shift_m *= 60; /* convert from minutes to seconds */

        /* correct sign */
        if (shift < 0) {
            shift_m *= -1;
        }

        /* connect hours and minutes of the shift */
        shift = shift + shift_m;
    }

    /* we have to shift to the opposite way to correct the time */
    t -= shift;

    *time = t;
    if (fractions_s) {
        if (frac) {
            *fractions_s = strndup(frac, frac_len);
            LY_CHECK_RET(!*fractions_s, LY_EMEM);
        } else {
            *fractions_s = NULL;
        }
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_time_time2str(time_t time, const char *fractions_s, char **str)
{
    struct tm tm;
    char zoneshift[12];
    int zonediff_s, zonediff_h, zonediff_m;

    LY_CHECK_ARG_RET(NULL, str, LY_EINVAL);

    /* init timezone */
    tzset();

    /* convert */
    if (!localtime_r(&time, &tm)) {
        return LY_ESYS;
    }

    /* get timezone offset (do not use tm_gmtoff to avoid portability problems) */
    zonediff_s = ly_time_tz_offset_at(time);
    zonediff_h = zonediff_s / 60 / 60;
    zonediff_m = zonediff_s / 60 % 60;
    sprintf(zoneshift, "%+03d:%02d", zonediff_h, zonediff_m);

    /* print */
    if (asprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d%s%s%s",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            fractions_s ? "." : "", fractions_s ? fractions_s : "", zoneshift) == -1) {
        return LY_EMEM;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_time_str2ts(const char *value, struct timespec *ts)
{
    LY_ERR rc;
    char *fractions_s, frac_buf[10];
    int frac_len;

    LY_CHECK_ARG_RET(NULL, value, ts, LY_EINVAL);

    rc = ly_time_str2time(value, &ts->tv_sec, &fractions_s);
    LY_CHECK_RET(rc);

    /* convert fractions of a second to nanoseconds */
    if (fractions_s) {
        /* init frac_buf with zeroes */
        memset(frac_buf, '0', 9);
        frac_buf[9] = '\0';

        frac_len = strlen(fractions_s);
        memcpy(frac_buf, fractions_s, frac_len > 9 ? 9 : frac_len);
        ts->tv_nsec = atol(frac_buf);
        free(fractions_s);
    } else {
        ts->tv_nsec = 0;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_time_ts2str(const struct timespec *ts, char **str)
{
    char frac_buf[10];

    LY_CHECK_ARG_RET(NULL, ts, str, ((ts->tv_nsec <= 999999999) && (ts->tv_nsec >= 0)), LY_EINVAL);

    /* convert nanoseconds to fractions of a second */
    if (ts->tv_nsec) {
        sprintf(frac_buf, "%09ld", ts->tv_nsec);
    }

    return ly_time_time2str(ts->tv_sec, ts->tv_nsec ? frac_buf : NULL, str);
}
