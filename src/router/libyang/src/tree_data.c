/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Data tree functions
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

#include "tree_data.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "context.h"
#include "dict.h"
#include "diff.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "ly_common.h"
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
#include "tree_data_sorted.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"
#include "xpath.h"

static LY_ERR lyd_compare_siblings_(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options,
        ly_bool parental_schemas_checked);

static LYD_FORMAT
lyd_parse_get_format(const struct ly_in *in, LYD_FORMAT format)
{
    if (!format && (in->type == LY_IN_FILEPATH)) {
        /* unknown format - try to detect it from filename's suffix */
        const char *path = in->method.fpath.filepath;
        size_t len = strlen(path);

        /* ignore trailing whitespaces */
        for ( ; len > 0 && isspace(path[len - 1]); len--) {}

        if ((len >= LY_XML_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_XML_SUFFIX_LEN], LY_XML_SUFFIX, LY_XML_SUFFIX_LEN)) {
            format = LYD_XML;
        } else if ((len >= LY_JSON_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_JSON_SUFFIX_LEN], LY_JSON_SUFFIX, LY_JSON_SUFFIX_LEN)) {
            format = LYD_JSON;
        } else if ((len >= LY_LYB_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_LYB_SUFFIX_LEN], LY_LYB_SUFFIX, LY_LYB_SUFFIX_LEN)) {
            format = LYD_LYB;
        } /* else still unknown */
    }

    return format;
}

/**
 * @brief Parse YANG data into a data tree.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extenion instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first parsed node.
 * @param[in] in Input handle to read the input from.
 * @param[in] format Expected format of the data in @p in.
 * @param[in] parse_opts Options for parser.
 * @param[in] val_opts Options for validation.
 * @param[out] op Optional pointer to the parsed operation, if any.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_parse(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_in *in, LYD_FORMAT format, uint32_t parse_opts, uint32_t val_opts, struct lyd_node **op)
{
    LY_ERR r = LY_SUCCESS, rc = LY_SUCCESS;
    struct lyd_ctx *lydctx = NULL;
    struct ly_set parsed = {0};
    uint32_t i, int_opts = 0;
    const struct ly_err_item *eitem;
    ly_bool subtree_sibling = 0;

    assert(ctx && (parent || first_p));

    format = lyd_parse_get_format(in, format);
    if (first_p) {
        *first_p = NULL;
    }

    /* remember input position */
    in->func_start = in->current;

    /* set internal options */
    if (!(parse_opts & LYD_PARSE_SUBTREE)) {
        int_opts = LYD_INTOPT_WITH_SIBLINGS;
    }

    /* parse the data */
    switch (format) {
    case LYD_XML:
        r = lyd_parse_xml(ctx, ext, parent, first_p, in, parse_opts, val_opts, int_opts, &parsed,
                &subtree_sibling, &lydctx);
        break;
    case LYD_JSON:
        r = lyd_parse_json(ctx, ext, parent, first_p, in, parse_opts, val_opts, int_opts, &parsed,
                &subtree_sibling, &lydctx);
        break;
    case LYD_LYB:
        r = lyd_parse_lyb(ctx, ext, parent, first_p, in, parse_opts, val_opts, int_opts, &parsed,
                &subtree_sibling, &lydctx);
        break;
    case LYD_UNKNOWN:
        LOGARG(ctx, format);
        r = LY_EINVAL;
        break;
    }
    if (r) {
        rc = r;
        if ((r != LY_EVALID) || !lydctx || !(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR)) {
            goto cleanup;
        }

        eitem = ly_err_last(ctx);
        assert(eitem);
        if (eitem->vecode == LYVE_SYNTAX) {
            /* cannot get more errors on a syntax error */
            goto cleanup;
        }
    }

    if (parent && parsed.count) {
        /* use the first parsed node */
        if (first_p) {
            *first_p = parsed.dnodes[0];
        } else {
            first_p = &parsed.dnodes[0];
        }
    }

    if (!(parse_opts & LYD_PARSE_ONLY)) {
        /* validate data */
        r = lyd_validate(first_p, NULL, ctx, val_opts, 0, &lydctx->node_when, &lydctx->node_types, &lydctx->meta_types,
                &lydctx->ext_node, &lydctx->ext_val, NULL);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

    /* set the operation node */
    if (op) {
        *op = lydctx->op_node;
    }

cleanup:
    if (lydctx) {
        lydctx->free(lydctx);
    }
    if (rc) {
        if (parent) {
            /* free all the parsed subtrees */
            for (i = 0; i < parsed.count; ++i) {
                lyd_free_tree(parsed.dnodes[i]);
            }
        } else {
            /* free everything */
            lyd_free_all(*first_p);
            *first_p = NULL;
        }
    } else if (subtree_sibling) {
        rc = LY_ENOT;
    }
    ly_set_erase(&parsed, NULL);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_ext_data(const struct lysc_ext_instance *ext, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        uint32_t parse_options, uint32_t validate_options, struct lyd_node **tree)
{
    const struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, in, parent || tree, LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(parse_options & ~LYD_PARSE_OPTS_MASK), LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(validate_options & ~LYD_VALIDATE_OPTS_MASK), LY_EINVAL);

    return lyd_parse(ctx, ext, parent, tree, in, format, parse_options, validate_options, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_parse_data(const struct ly_ctx *ctx, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        uint32_t parse_options, uint32_t validate_options, struct lyd_node **tree)
{
    LY_CHECK_ARG_RET(ctx, ctx, in, parent || tree, LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(parse_options & ~LYD_PARSE_OPTS_MASK), LY_EINVAL);
    LY_CHECK_ARG_RET(ctx, !(validate_options & ~LYD_VALIDATE_OPTS_MASK), LY_EINVAL);

    return lyd_parse(ctx, NULL, parent, tree, in, format, parse_options, validate_options, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_parse_data_mem(const struct ly_ctx *ctx, const char *data, LYD_FORMAT format, uint32_t parse_options,
        uint32_t validate_options, struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_memory(data, &in));
    ret = lyd_parse_data(ctx, NULL, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_data_fd(const struct ly_ctx *ctx, int fd, LYD_FORMAT format, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_fd(fd, &in));
    ret = lyd_parse_data(ctx, NULL, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_data_path(const struct ly_ctx *ctx, const char *path, LYD_FORMAT format, uint32_t parse_options,
        uint32_t validate_options, struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;

    LY_CHECK_RET(ly_in_new_filepath(path, 0, &in));
    ret = lyd_parse_data(ctx, NULL, in, format, parse_options, validate_options, tree);

    ly_in_free(in, 0);
    return ret;
}

/**
 * @brief Parse YANG data into an operation data tree, in case the extension instance is specified, keep the searching
 * for schema nodes locked inside the extension instance.
 *
 * At least one of @p parent, @p tree, or @p op must always be set.
 *
 * Specific @p data_type values have different parameter meaning as mentioned for ::lyd_parse_op().
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Extension instance providing the specific schema tree to match with the data being parsed.
 * @param[in] parent Optional parent to connect the parsed nodes to.
 * @param[in] in Input handle to read the input from.
 * @param[in] format Expected format of the data in @p in.
 * @param[in] data_type Expected operation to parse (@ref datatype).
 * @param[out] tree Optional full parsed data tree. If @p parent is set, set to NULL.
 * @param[out] op Optional parsed operation node.
 * @return LY_ERR value.
 * @return LY_ENOT if @p data_type is a NETCONF message and the root XML element is not the expected one.
 */
static LY_ERR
lyd_parse_op_(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct ly_in *in, LYD_FORMAT format, enum lyd_type data_type, struct lyd_node **tree, struct lyd_node **op)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_ctx *lydctx = NULL;
    struct ly_set parsed = {0};
    struct lyd_node *first = NULL, *envp = NULL;
    uint32_t i, parse_opts, val_opts, int_opts = 0;
    ly_bool proto_msg = 0;

    if (!ctx) {
        ctx = LYD_CTX(parent);
    }
    if (tree) {
        *tree = NULL;
    }
    if (op) {
        *op = NULL;
    }

    format = lyd_parse_get_format(in, format);

    /* remember input position */
    in->func_start = in->current;

    /* set parse and validation opts */
    parse_opts = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    val_opts = 0;

    switch (data_type) {
    case LYD_TYPE_RPC_NETCONF:
    case LYD_TYPE_NOTIF_NETCONF:
        LY_CHECK_ARG_RET(ctx, format == LYD_XML, !parent, tree, op, LY_EINVAL);
        proto_msg = 1;
        break;
    case LYD_TYPE_REPLY_NETCONF:
        LY_CHECK_ARG_RET(ctx, format == LYD_XML, parent, parent->schema, parent->schema->nodetype & (LYS_RPC | LYS_ACTION),
                tree, !op, LY_EINVAL);
        proto_msg = 1;
        break;
    case LYD_TYPE_RPC_RESTCONF:
    case LYD_TYPE_REPLY_RESTCONF:
        LY_CHECK_ARG_RET(ctx, parent, parent->schema, parent->schema->nodetype & (LYS_RPC | LYS_ACTION), tree, !op, LY_EINVAL);
        proto_msg = 1;
        break;
    case LYD_TYPE_NOTIF_RESTCONF:
        LY_CHECK_ARG_RET(ctx, format == LYD_JSON, !parent, tree, op, LY_EINVAL);
        proto_msg = 1;
        break;

    /* set internal opts */
    case LYD_TYPE_RPC_YANG:
        int_opts = LYD_INTOPT_RPC | LYD_INTOPT_ACTION | (parent ? LYD_INTOPT_WITH_SIBLINGS : LYD_INTOPT_NO_SIBLINGS);
        break;
    case LYD_TYPE_NOTIF_YANG:
        int_opts = LYD_INTOPT_NOTIF | (parent ? LYD_INTOPT_WITH_SIBLINGS : LYD_INTOPT_NO_SIBLINGS);
        break;
    case LYD_TYPE_REPLY_YANG:
        int_opts = LYD_INTOPT_REPLY | (parent ? LYD_INTOPT_WITH_SIBLINGS : LYD_INTOPT_NO_SIBLINGS);
        break;
    case LYD_TYPE_DATA_YANG:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* parse a full protocol message */
    if (proto_msg) {
        if (format == LYD_XML) {
            /* parse the NETCONF (or RESTCONF XML) message */
            rc = lyd_parse_xml_netconf(ctx, ext, parent, &first, in, parse_opts, val_opts, data_type, &envp, &parsed, &lydctx);
        } else {
            /* parse the RESTCONF message */
            rc = lyd_parse_json_restconf(ctx, ext, parent, &first, in, parse_opts, val_opts, data_type, &envp, &parsed, &lydctx);
        }
        if (rc) {
            if (envp) {
                /* special situation when the envelopes were parsed successfully */
                *tree = envp;
            }
            goto cleanup;
        }

        /* set out params correctly */
        if (envp) {
            /* special out param meaning */
            *tree = envp;
        } else {
            *tree = parent ? NULL : first;
        }
        if (op) {
            *op = lydctx->op_node;
        }
        goto cleanup;
    }

    /* parse the data */
    switch (format) {
    case LYD_XML:
        rc = lyd_parse_xml(ctx, ext, parent, &first, in, parse_opts, val_opts, int_opts, &parsed, NULL, &lydctx);
        break;
    case LYD_JSON:
        rc = lyd_parse_json(ctx, ext, parent, &first, in, parse_opts, val_opts, int_opts, &parsed, NULL, &lydctx);
        break;
    case LYD_LYB:
        rc = lyd_parse_lyb(ctx, ext, parent, &first, in, parse_opts, val_opts, int_opts, &parsed, NULL, &lydctx);
        break;
    case LYD_UNKNOWN:
        LOGARG(ctx, format);
        rc = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(rc, cleanup);

    /* set out params correctly */
    if (tree) {
        *tree = parent ? NULL : first;
    }
    if (op) {
        *op = lydctx->op_node;
    }

cleanup:
    if (lydctx) {
        lydctx->free(lydctx);
    }
    if (rc) {
        /* free all the parsed nodes */
        if (parsed.count) {
            i = parsed.count;
            do {
                --i;
                lyd_free_tree(parsed.dnodes[i]);
            } while (i);
        }
        if (tree && !envp) {
            *tree = NULL;
        }
        if (op) {
            *op = NULL;
        }
    }
    ly_set_erase(&parsed, NULL);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_op(const struct ly_ctx *ctx, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        enum lyd_type data_type, struct lyd_node **tree, struct lyd_node **op)
{
    LY_CHECK_ARG_RET(ctx, ctx || parent, in, data_type, parent || tree || op, LY_EINVAL);

    return lyd_parse_op_(ctx, NULL, parent, in, format, data_type, tree, op);
}

LIBYANG_API_DEF LY_ERR
lyd_parse_ext_op(const struct lysc_ext_instance *ext, struct lyd_node *parent, struct ly_in *in, LYD_FORMAT format,
        enum lyd_type data_type, struct lyd_node **tree, struct lyd_node **op)
{
    const struct ly_ctx *ctx = ext ? ext->module->ctx : NULL;

    LY_CHECK_ARG_RET(ctx, ext, in, data_type, parent || tree || op, LY_EINVAL);

    return lyd_parse_op_(ctx, ext, parent, in, format, data_type, tree, op);
}

struct lyd_node *
lyd_insert_get_next_anchor(const struct lyd_node *first_sibling, const struct lyd_node *new_node)
{
    const struct lysc_node *schema, *sparent;
    struct lyd_node *match = NULL;
    ly_bool found;
    uint32_t getnext_opts;

    assert(new_node);

    if (!first_sibling || !new_node->schema || (LYD_CTX(first_sibling) != LYD_CTX(new_node))) {
        /* insert at the end, no next anchor */
        return NULL;
    }

    getnext_opts = 0;
    if (new_node->schema->flags & LYS_IS_OUTPUT) {
        getnext_opts = LYS_GETNEXT_OUTPUT;
    }

    if (first_sibling->parent && first_sibling->parent->schema && first_sibling->parent->children_ht) {
        /* find the anchor using hashes */
        sparent = first_sibling->parent->schema;
        schema = lys_getnext(new_node->schema, sparent, NULL, getnext_opts);
        while (schema) {
            /* keep trying to find the first existing instance of the closest following schema sibling,
             * otherwise return NULL - inserting at the end */
            if (!lyd_find_sibling_schema(first_sibling, schema, &match)) {
                break;
            }

            schema = lys_getnext(schema, sparent, NULL, getnext_opts);
        }
    } else {
        /* find the anchor without hashes */
        match = (struct lyd_node *)first_sibling;
        sparent = lysc_data_parent(new_node->schema);
        if (!sparent) {
            /* we are in top-level, skip all the data from preceding modules */
            LY_LIST_FOR(match, match) {
                if (!match->schema || (strcmp(lyd_owner_module(match)->name, lyd_owner_module(new_node)->name) >= 0)) {
                    break;
                }
            }
        }

        /* get the first schema sibling */
        schema = lys_getnext(NULL, sparent, new_node->schema->module->compiled, getnext_opts);
        if (!schema) {
            /* must be a top-level extension instance data, no anchor */
            return NULL;
        }

        found = 0;
        LY_LIST_FOR(match, match) {
            if (!match->schema || (lyd_owner_module(match) != lyd_owner_module(new_node))) {
                /* we have found an opaque node, which must be at the end, so use it OR
                 * modules do not match, so we must have traversed all the data from new_node module (if any),
                 * we have found the first node of the next module, that is what we want */
                break;
            }

            /* skip schema nodes until we find the instantiated one */
            while (!found) {
                if (new_node->schema == schema) {
                    /* we have found the schema of the new node, continue search to find the first
                     * data node with a different schema (after our schema) */
                    found = 1;
                    break;
                }
                if (match->schema == schema) {
                    /* current node (match) is a data node still before the new node, continue search in data */
                    break;
                }

                schema = lys_getnext(schema, sparent, new_node->schema->module->compiled, getnext_opts);
                if (!schema) {
                    /* must be a top-level extension instance data, no anchor */
                    return NULL;
                }
            }

            if (found && (match->schema != new_node->schema)) {
                /* find the next node after we have found our node schema data instance */
                break;
            }
        }
    }

    return match;
}

void
lyd_insert_after_node(struct lyd_node **first_sibling_p, struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node_inner *par;
    struct lyd_node *first_sibling;

    assert(!node->next && (node->prev == node));

    if (sibling->next) {
        /* sibling had a succeeding node */
        sibling->next->prev = node;
        node->next = sibling->next;
    } else {
        /* sibling was last, find first sibling and change its prev */
        if (first_sibling_p && *first_sibling_p) {
            assert(!(*first_sibling_p)->prev->next);
            (*first_sibling_p)->prev = node;
        } else {
            first_sibling = lyd_first_sibling(sibling);
            first_sibling->prev = node;
            if (first_sibling_p) {
                *first_sibling_p = first_sibling;
            }
        }
    }
    node->prev = sibling;
    sibling->next = node;
    node->parent = sibling->parent;

    for (par = node->parent; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
    }
}

void
lyd_insert_before_node(struct lyd_node *sibling, struct lyd_node *node)
{
    struct lyd_node_inner *par;

    assert(!node->next && (node->prev == node));

    node->next = sibling;
    /* covers situation of sibling being first */
    node->prev = sibling->prev;
    sibling->prev = node;
    if (node->prev->next) {
        /* sibling had a preceding node */
        node->prev->next = node;
    } else if (sibling->parent) {
        /* sibling was first and we must also change parent child pointer */
        sibling->parent->child = node;
    }
    node->parent = sibling->parent;

    for (par = node->parent; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
    }
}

/**
 * @brief Insert node as the first and only child of a parent.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] parent Parent to insert into.
 * @param[in] node Node to insert.
 */
static void
lyd_insert_only_child(struct lyd_node *parent, struct lyd_node *node)
{
    struct lyd_node_inner *par;

    assert(parent && !lyd_child(parent) && !node->next && (node->prev == node));
    assert(!parent->schema || (parent->schema->nodetype & LYD_NODE_INNER));

    par = (struct lyd_node_inner *)parent;

    par->child = node;
    node->parent = par;

    for ( ; par; par = par->parent) {
        if ((par->flags & LYD_DEFAULT) && !(node->flags & LYD_DEFAULT)) {
            /* remove default flags from NP containers */
            par->flags &= ~LYD_DEFAULT;
        }
    }
}

/**
 * @brief Learn whether a list instance has all the keys.
 *
 * @param[in] list List instance to check.
 * @return non-zero if all the keys were found,
 * @return 0 otherwise.
 */
static int
lyd_insert_has_keys(const struct lyd_node *list)
{
    const struct lyd_node *key;
    const struct lysc_node *skey = NULL;

    assert(list->schema->nodetype == LYS_LIST);
    key = lyd_child(list);
    while ((skey = lys_getnext(skey, list->schema, NULL, 0)) && (skey->flags & LYS_KEY)) {
        if (!key || (key->schema != skey)) {
            /* key missing */
            return 0;
        }

        key = key->next;
    }

    /* all keys found */
    return 1;
}

/**
 * @brief Get the first subsequent data node that contains a different schema definition.
 *
 * @param[in] first_sibling First sibling, NULL if no top-level sibling exist yet.
 * @param[in] node Node to be inserted.
 * @return Subsequent data node with a different schema.
 */
static struct lyd_node *
lyd_insert_node_find_anchor(struct lyd_node *first_sibling, struct lyd_node *node)
{
    struct lyd_node *anchor;

    if (first_sibling && (first_sibling->flags & LYD_EXT)) {
        return NULL;
    }

    /* find the anchor, so we can insert somewhere before it */
    anchor = lyd_insert_get_next_anchor(first_sibling, node);
    /* cannot insert data node after opaque nodes */
    if (!anchor && node->schema && first_sibling && !first_sibling->prev->schema) {
        anchor = first_sibling->prev;
        while ((anchor != first_sibling) && !anchor->prev->schema) {
            anchor = anchor->prev;
        }
    }

    return anchor;
}

/**
 * @brief Insert @p node as the last node.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_sibling First sibling, NULL if no top-level sibling exist yet.
 * Can be also NULL if @p parent is set.
 * @param[in] node Individual node (without siblings) to insert.
 */
static void
lyd_insert_node_last(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node)
{
    assert(first_sibling && node);

    if (*first_sibling) {
        lyd_insert_after_node(first_sibling, (*first_sibling)->prev, node);
    } else if (parent) {
        lyd_insert_only_child(parent, node);
        *first_sibling = node;
    } else {
        *first_sibling = node;
    }
}

void
lyd_insert_node_ordby_schema(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node)
{
    struct lyd_node *anchor;

    assert(first_sibling && node);

    if ((anchor = lyd_insert_node_find_anchor(*first_sibling, node))) {
        lyd_insert_before_node(anchor, node);
        *first_sibling = *first_sibling != anchor ? *first_sibling : node;
    } else if (*first_sibling && node->schema && !(*first_sibling)->prev->schema) {
        /* cannot insert data node after opaque nodes */
        anchor = (*first_sibling)->prev;
        while ((anchor != *first_sibling) && !anchor->prev->schema) {
            anchor = anchor->prev;
        }
        lyd_insert_before_node(anchor, node);
        *first_sibling = *first_sibling != anchor ? *first_sibling : node;
    } else {
        lyd_insert_node_last(parent, first_sibling, node);
    }
}

void
lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling_p, struct lyd_node *node, uint32_t order)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *first_sibling, *leader;

    /* inserting list without its keys is not supported */
    assert((parent || first_sibling_p) && node && (node->hash || !node->schema));
    assert(!parent || !parent->schema ||
            (parent->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF)));

    if (!parent && first_sibling_p && (*first_sibling_p)) {
        parent = lyd_parent(*first_sibling_p);
    }
    first_sibling = parent ? lyd_child(parent) : *first_sibling_p;

    if ((order == LYD_INSERT_NODE_LAST) || !node->schema || (first_sibling && (first_sibling->flags & LYD_EXT))) {
        lyd_insert_node_last(parent, &first_sibling, node);
    } else if (order == LYD_INSERT_NODE_LAST_BY_SCHEMA) {
        lyd_insert_node_ordby_schema(parent, &first_sibling, node);
    } else if (lyds_is_supported(node) &&
            (lyd_find_sibling_schema(first_sibling, node->schema, &leader) == LY_SUCCESS)) {
        ret = lyds_insert(&first_sibling, &leader, node);
        if (ret) {
            /* The operation on the sorting tree unexpectedly failed due to some internal issue,
             * but insert the node anyway although the nodes will not be sorted.
             */
            LOGWRN(LYD_CTX(node), "Data in \"%s\" are not sorted.", node->schema->name);
            lyd_insert_node_ordby_schema(parent, &first_sibling, node);
        }
    } else {
        lyd_insert_node_ordby_schema(parent, &first_sibling, node);
    }

    /* insert into parent HT */
    lyd_insert_hash(node);

    /* finish hashes for our parent, if needed and possible */
    if (node->schema && (node->schema->flags & LYS_KEY) && parent && parent->schema && lyd_insert_has_keys(parent)) {
        lyd_hash(parent);

        /* now we can insert even the list into its parent HT */
        lyd_insert_hash(parent);
    }

    if (first_sibling_p) {
        *first_sibling_p = first_sibling;
    }

#ifndef NDEBUG
    if ((order == LYD_INSERT_NODE_LAST) && lyds_is_supported(node) &&
            (node->prev->schema == node->schema) && (lyds_compare_single(node->prev, node) > 0)) {
        LOGWRN(LYD_CTX(node), "Data in \"%s\" are not sorted, inserted node should not be added to the end.",
                node->schema->name);
    }
#endif
}

/**
 * @brief Check that @p node can be unlinked.
 *
 * @param[in] node Node to check
 * @return LY_ERR value.
 */
static LY_ERR
lyd_unlink_check(struct lyd_node *node)
{
    if (!node) {
        return LY_SUCCESS;
    }

    if (lysc_is_key(node->schema) && node->parent) {
        LOGERR(LYD_CTX(node), LY_EINVAL, "Cannot unlink a list key \"%s\", unlink the list instance instead.",
                LYD_NAME(node));
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Move schema instances before anchor or as the last.
 *
 * The nodes will remain sorted according to the schema.
 *
 * @param[in] first_dst First sibling, destination.
 * @param[in] node Starting node, all following nodes with the same schema will be moved.
 * @param[out] next_p Next node that has a different schema or NULL.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_move_nodes_ordby_schema(struct lyd_node **first_dst, struct lyd_node *node, struct lyd_node **next_p)
{
    struct lyd_node *second, *anchor, *iter, *next, *dst, *src, *first_src = NULL;

    assert(first_dst && *first_dst && !(*first_dst)->prev->next && node && next_p);

    if ((anchor = lyd_insert_node_find_anchor(*first_dst, node))) {
        /* move the first node to the correct place according to the schema */
        LY_CHECK_RET(lyd_unlink_check(node));
        second = node->next;
        lyd_unlink_ignore_lyds(&first_src, node);
        lyd_insert_before_node(anchor, node);
        lyd_insert_hash(node);
        *first_dst = *first_dst != anchor ? *first_dst : node;
        if (!second || (node->schema != second->schema)) {
            /* no more nodes to move */
            *next_p = second;
            return LY_SUCCESS;
        }
        dst = node;
        src = second;
    } else {
        /* just move all instances to the end */
        dst = (*first_dst)->prev;
        src = node;
    }

    /* move the rest of source instances after @p node */
    LY_LIST_FOR_SAFE(src, next, iter) {
        LY_CHECK_RET(lyd_unlink_check(iter));
        if (iter->schema != src->schema) {
            break;
        }
        lyd_unlink_ignore_lyds(&first_src, iter);
        lyd_insert_after_node(first_dst, dst, iter);
        lyd_insert_hash(iter);
        dst = iter;
    }
    *next_p = iter;

    return LY_SUCCESS;
}

/**
 * @brief Move nodes regardless of schema.
 *
 * The destination for the move is NULL, or a childless parent.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in] first_src First sibling, all following nodes will be moved.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_move_nodes_at_once(struct lyd_node *parent, struct lyd_node *first_src)
{
    struct lyd_node *start, *next, *iter, *first_dst;

    assert(!lyd_child(parent) && first_src && !first_src->prev->next && !first_src->parent);

    LY_CHECK_RET(lyd_unlink_check(first_src));

    /* move the first node */
    start = first_src->next;
    first_dst = first_src;
    if (parent) {
        lyd_unlink_ignore_lyds(&first_src, first_dst);
        lyd_insert_only_child(parent, first_dst);
        lyd_insert_hash(first_dst);
    } else {
        lyd_unlink_ignore_lyds(&first_src, first_dst);
    }

    /* move the rest of the nodes */
    LY_LIST_FOR_SAFE(start, next, iter) {
        LY_CHECK_RET(lyd_unlink_check(iter));
        lyd_unlink_ignore_lyds(&first_src, iter);
        lyd_insert_after_node(&first_dst, first_dst->prev, iter);
        lyd_insert_hash(iter);
    }

    return LY_SUCCESS;
}

/**
 * @brief Move the nodes in parts according to the schema.
 *
 * @param[in,out] first_dst First sibling, destination.
 * @param[in] first_src First sibling, all following nodes will be moved.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_move_nodes_by_schema(struct lyd_node **first_dst, struct lyd_node *first_src)
{
    LY_ERR ret;
    struct lyd_node *next, *iter, *leader;

    assert(first_dst && *first_dst && !(*first_dst)->prev->next && first_src &&
            !first_src->prev->next && !first_src->parent);

    for (iter = first_src; iter; iter = next) {
        if (lyds_is_supported(iter) &&
                (lyd_find_sibling_schema(*first_dst, iter->schema, &leader) == LY_SUCCESS)) {
            ret = lyds_merge(first_dst, &leader, &first_src, iter, &next);
            if (ret) {
                /* The operation on the sorting tree unexpectedly failed due to some internal issue,
                 * but insert the node anyway although the nodes will not be sorted.
                 */
                LOGWRN(LYD_CTX(first_src), "Data in \"%s\" are not sorted.", leader->schema->name);
                LY_CHECK_RET(lyd_move_nodes_ordby_schema(first_dst, next, &next));
            }
        } else {
            LY_CHECK_RET(lyd_move_nodes_ordby_schema(first_dst, iter, &next));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move a nodes into parent/siblings.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_dst_p First sibling, NULL if no top-level sibling exist yet.
 * Can be also NULL if @p parent is set.
 * @param[in] first_src First sibling, all following nodes will be moved.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_move_nodes(struct lyd_node *parent, struct lyd_node **first_dst_p, struct lyd_node *first_src)
{
    LY_ERR ret;
    struct lyd_node *first_dst;

    assert((parent || first_dst_p) && first_src && !first_src->prev->next);

    if (!first_dst_p || !*first_dst_p) {
        first_dst = lyd_child(parent);
    } else {
        first_dst = *first_dst_p;
    }

    if (first_dst) {
        ret = lyd_move_nodes_by_schema(&first_dst, first_src);
    } else {
        ret = lyd_move_nodes_at_once(parent, first_src);
        first_dst = first_src;
    }

    if (first_dst_p) {
        *first_dst_p = first_dst;
    }

    return ret;
}

/**
 * @brief Check schema place of a node to be inserted.
 *
 * @param[in] parent Schema node of the parent data node.
 * @param[in] sibling Schema node of a sibling data node.
 * @param[in] schema Schema node if the data node to be inserted.
 * @return LY_SUCCESS on success.
 * @return LY_EINVAL if the place is invalid.
 */
static LY_ERR
lyd_insert_check_schema(const struct lysc_node *parent, const struct lysc_node *sibling, const struct lysc_node *schema)
{
    const struct lysc_node *par2;

    assert(!parent || !(parent->nodetype & (LYS_CASE | LYS_CHOICE)));
    assert(!sibling || !(sibling->nodetype & (LYS_CASE | LYS_CHOICE)));
    assert(!schema || !(schema->nodetype & (LYS_CASE | LYS_CHOICE)));

    if (!schema || (!parent && !sibling)) {
        /* opaque nodes can be inserted wherever */
        return LY_SUCCESS;
    }

    if (!parent) {
        parent = lysc_data_parent(sibling);
    }

    /* find schema parent */
    par2 = lysc_data_parent(schema);

    if (parent) {
        /* inner node */
        if (par2 != parent) {
            LOGERR(schema->module->ctx, LY_EINVAL, "Cannot insert, parent of \"%s\" is not \"%s\".", schema->name,
                    parent->name);
            return LY_EINVAL;
        }
    } else {
        /* top-level node */
        if (par2) {
            LOGERR(schema->module->ctx, LY_EINVAL, "Cannot insert, node \"%s\" is not top-level.", schema->name);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_insert_child(struct lyd_node *parent, struct lyd_node *node)
{
    LY_CHECK_ARG_RET(NULL, parent, node, !parent->schema || (parent->schema->nodetype & LYD_NODE_INNER), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(LYD_CTX(parent), LYD_CTX(node), LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(parent->schema, NULL, node->schema));

    if (node->parent || node->prev->next || !node->next) {
        LY_CHECK_RET(lyd_unlink_tree(node));
        lyd_insert_node(parent, NULL, node, LYD_INSERT_NODE_DEFAULT);
    } else {
        LY_CHECK_RET(lyd_move_nodes(parent, NULL, node));
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_ext_insert(struct lyd_node *parent, struct lyd_node *first)
{
    struct lyd_node *iter;

    LY_CHECK_ARG_RET(NULL, parent, first, !first->parent, !first->prev->next,
            !parent->schema || (parent->schema->nodetype & LYD_NODE_INNER), LY_EINVAL);

    if (first->schema && (first->schema->flags & LYS_KEY)) {
        LOGERR(LYD_CTX(parent), LY_EINVAL, "Cannot insert key \"%s\".", first->schema->name);
        return LY_EINVAL;
    }

    while (first) {
        iter = first->next;
        lyd_unlink(first);
        lyd_insert_node(parent, NULL, first, LYD_INSERT_NODE_LAST);
        first = iter;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, struct lyd_node **first)
{
    struct lyd_node *first_sibling;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (sibling) {
        LY_CHECK_RET(lyd_insert_check_schema(NULL, sibling->schema, node->schema));
    }

    first_sibling = lyd_first_sibling(sibling);
    if (node->parent || node->prev->next || !node->next) {
        LY_CHECK_RET(lyd_unlink_tree(node));
        lyd_insert_node(NULL, &first_sibling, node, LYD_INSERT_NODE_DEFAULT);
    } else {
        LY_CHECK_RET(lyd_move_nodes(NULL, &first_sibling, node));
    }

    if (first) {
        *first = first_sibling;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    LY_CHECK_ARG_RET(NULL, sibling, node, sibling != node, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(LYD_CTX(sibling), LYD_CTX(node), LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(NULL, sibling->schema, node->schema));

    if (node->schema && (!(node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(node->schema->flags & LYS_ORDBY_USER))) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Can be used only for user-ordered nodes.");
        return LY_EINVAL;
    }
    if (node->schema && sibling->schema && (node->schema != sibling->schema)) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Cannot insert before a different schema node instance.");
        return LY_EINVAL;
    }

    lyd_unlink(node);
    lyd_insert_before_node(sibling, node);
    lyd_insert_hash(node);

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    LY_CHECK_ARG_RET(NULL, sibling, node, sibling != node, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(LYD_CTX(sibling), LYD_CTX(node), LY_EINVAL);

    LY_CHECK_RET(lyd_insert_check_schema(NULL, sibling->schema, node->schema));

    if (node->schema && (!(node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(node->schema->flags & LYS_ORDBY_USER))) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Can be used only for user-ordered nodes.");
        return LY_EINVAL;
    }
    if (node->schema && sibling->schema && (node->schema != sibling->schema)) {
        LOGERR(LYD_CTX(sibling), LY_EINVAL, "Cannot insert after a different schema node instance.");
        return LY_EINVAL;
    }

    lyd_unlink(node);
    lyd_insert_after_node(NULL, sibling, node);
    lyd_insert_hash(node);

    return LY_SUCCESS;
}

void
lyd_unlink_ignore_lyds(struct lyd_node **first_sibling_p, struct lyd_node *node)
{
    struct lyd_node *first_sibling;

    /* update hashes while still linked into the tree */
    lyd_unlink_hash(node);

    /* unlink leafref nodes */
    if (node->schema && (node->schema->nodetype & LYD_NODE_TERM)) {
        lyd_free_leafref_nodes((struct lyd_node_term *)node);
    }

    /* unlink from siblings */
    if (node->next) {
        node->next->prev = node->prev;
        if (node->prev->next) {
            node->prev->next = node->next;
        } else if (first_sibling_p) {
            /* unlinking the first node */
            *first_sibling_p = node->next;
        }
    } else {
        /* unlinking the last node */
        /* update the "last" pointer from the first node */
        if (first_sibling_p && *first_sibling_p) {
            (*first_sibling_p)->prev = node->prev;
        } else {
            first_sibling = lyd_first_sibling(node);
            first_sibling->prev = node->prev;
            if (first_sibling_p) {
                *first_sibling_p = first_sibling;
            }
        }
        node->prev->next = NULL;
    }

    /* unlink from parent */
    if (node->parent) {
        if (node->parent->child == node) {
            /* the node is the first child */
            node->parent->child = node->next;
        }

        /* check for NP container whether its last non-default node is not being unlinked */
        lyd_cont_set_dflt(lyd_parent(node));

        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;
}

void
lyd_unlink(struct lyd_node *node)
{
    struct lyd_node *leader;

    if (!node) {
        return;
    }

    /* unlink from the lyds tree */
    if (lyds_is_supported(node)) {
        if (!node->prev->next || (node->prev->schema != node->schema)) {
            leader = node;
        } else {
            lyd_find_sibling_val(node, node->schema, NULL, 0, &leader);
            assert(leader);
        }
        lyds_unlink(&leader, node);
    }

    /* unlink data tree */
    lyd_unlink_ignore_lyds(NULL, node);
}

LIBYANG_API_DEF LY_ERR
lyd_unlink_siblings(struct lyd_node *node)
{
    struct lyd_node *next, *iter, *leader, *start, *first_sibling = NULL;

    if (lyds_is_supported(node) && node->prev->next && (node->prev->schema == node->schema)) {
        /* unlink starts at the non-first item in the (leaf-)list */
        lyd_find_sibling_val(node, node->schema, NULL, 0, &leader);
        lyds_split(&first_sibling, leader, node, &start);
    } else {
        /* unlink @p node */
        LY_CHECK_RET(lyd_unlink_check(node));
        start = node->next;
        lyd_unlink_ignore_lyds(&first_sibling, node);
    }

    /* continue unlinking the rest */
    LY_LIST_FOR_SAFE(start, next, iter) {
        LY_CHECK_RET(lyd_unlink_check(iter));
        lyd_unlink_ignore_lyds(&first_sibling, iter);
        lyd_insert_after_node(&node, node->prev, iter);
        lyd_insert_hash(iter);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_unlink_tree(struct lyd_node *node)
{
    LY_CHECK_RET(lyd_unlink_check(node));
    lyd_unlink(node);

    return LY_SUCCESS;
}

void
lyd_insert_meta(struct lyd_node *parent, struct lyd_meta *meta, ly_bool clear_dflt)
{
    struct lyd_meta *last, *iter;

    assert(parent);

    if (!meta) {
        return;
    }

    for (iter = meta; iter; iter = iter->next) {
        iter->parent = parent;
    }

    /* insert as the last attribute */
    if (parent->meta) {
        for (last = parent->meta; last->next; last = last->next) {}
        last->next = meta;
    } else {
        parent->meta = meta;
    }

    /* remove default flags from NP containers */
    while (clear_dflt && parent && (parent->schema->nodetype == LYS_CONTAINER) && (parent->flags & LYD_DEFAULT)) {
        parent->flags &= ~LYD_DEFAULT;
        parent = lyd_parent(parent);
    }
}

void
lyd_unlink_meta_single(struct lyd_meta *meta)
{
    struct lyd_meta *iter;

    if (!meta) {
        return;
    }

    if (meta->parent && (meta->parent->meta == meta)) {
        meta->parent->meta = meta->next;
    } else if (meta->parent) {
        for (iter = meta->parent->meta; iter->next && (iter->next != meta); iter = iter->next) {}
        if (iter->next) {
            iter->next = meta->next;
        }
    }

    meta->next = NULL;
    meta->parent = NULL;
}

/**
 * @brief Get the annotation definition in the module.
 *
 * @param[in] mod Metadata module (with the annotation definition).
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @return compiled YANG extension instance on success.
 */
static struct lysc_ext_instance *
lyd_get_meta_annotation(const struct lys_module *mod, const char *name, size_t name_len)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyplg_ext *plugin;

    if (!mod) {
        return NULL;
    }

    LY_ARRAY_FOR(mod->compiled->exts, u) {
        plugin = mod->compiled->exts[u].def->plugin;
        if (plugin && !strncmp(plugin->id, "ly2 metadata", 12) &&
                !ly_strncmp(mod->compiled->exts[u].argument, name, name_len)) {
            return &mod->compiled->exts[u];
        }
    }

    return NULL;
}

LY_ERR
lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
        size_t name_len, const char *value, size_t value_len, ly_bool is_utf8, ly_bool store_only, ly_bool *dynamic,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node, ly_bool clear_dflt,
        ly_bool *incomplete)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_ext_instance *ant = NULL;
    const struct lysc_type *ant_type;
    struct lyd_meta *mt, *last;

    assert((parent || meta) && mod);

    ant = lyd_get_meta_annotation(mod, name, name_len);
    if (!ant) {
        /* attribute is not defined as a metadata annotation (RFC 7952) */
        LOGVAL(mod->ctx, LYVE_REFERENCE, "Annotation definition for attribute \"%s:%.*s\" not found.",
                mod->name, (int)name_len, name);
        ret = LY_EINVAL;
        goto cleanup;
    }

    mt = calloc(1, sizeof *mt);
    LY_CHECK_ERR_GOTO(!mt, LOGMEM(mod->ctx); ret = LY_EMEM, cleanup);
    mt->parent = parent;
    mt->annotation = ant;
    lyplg_ext_get_storage(ant, LY_STMT_TYPE, sizeof ant_type, (const void **)&ant_type);
    ret = lyd_value_store(mod->ctx, &mt->value, ant_type, value, value_len, is_utf8, store_only, dynamic, format, prefix_data, hints,
            ctx_node, incomplete);
    LY_CHECK_ERR_GOTO(ret, free(mt), cleanup);
    ret = lydict_insert(mod->ctx, name, name_len, &mt->name);
    LY_CHECK_ERR_GOTO(ret, free(mt), cleanup);

    /* insert as the last attribute */
    if (parent) {
        lyd_insert_meta(parent, mt, clear_dflt);
    } else if (*meta) {
        for (last = *meta; last->next; last = last->next) {}
        last->next = mt;
    }

    if (meta) {
        *meta = mt;
    }

cleanup:
    return ret;
}

void
lyd_insert_attr(struct lyd_node *parent, struct lyd_attr *attr)
{
    struct lyd_attr *last, *iter;
    struct lyd_node_opaq *opaq;

    assert(parent && !parent->schema);

    if (!attr) {
        return;
    }

    opaq = (struct lyd_node_opaq *)parent;
    for (iter = attr; iter; iter = iter->next) {
        iter->parent = opaq;
    }

    /* insert as the last attribute */
    if (opaq->attr) {
        for (last = opaq->attr; last->next; last = last->next) {}
        last->next = attr;
    } else {
        opaq->attr = attr;
    }
}

LY_ERR
lyd_create_attr(struct lyd_node *parent, struct lyd_attr **attr, const struct ly_ctx *ctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, const char *module_key, size_t module_key_len, const char *value,
        size_t value_len, ly_bool *dynamic, LY_VALUE_FORMAT format, void *val_prefix_data, uint32_t hints)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_attr *at, *last;

    assert(ctx && (parent || attr) && (!parent || !parent->schema));
    assert(name && name_len && format);

    if (!value_len && (!dynamic || !*dynamic)) {
        value = "";
    }

    at = calloc(1, sizeof *at);
    LY_CHECK_ERR_RET(!at, LOGMEM(ctx); ly_free_prefix_data(format, val_prefix_data), LY_EMEM);

    LY_CHECK_GOTO(ret = lydict_insert(ctx, name, name_len, &at->name.name), finish);
    if (prefix_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, prefix, prefix_len, &at->name.prefix), finish);
    }
    if (module_key_len) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, module_key, module_key_len, &at->name.module_ns), finish);
    }

    if (dynamic && *dynamic) {
        ret = lydict_insert_zc(ctx, (char *)value, &at->value);
        LY_CHECK_GOTO(ret, finish);
        *dynamic = 0;
    } else {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, value, value_len, &at->value), finish);
    }
    at->format = format;
    at->val_prefix_data = val_prefix_data;
    at->hints = hints;

    /* insert as the last attribute */
    if (parent) {
        lyd_insert_attr(parent, at);
    } else if (*attr) {
        for (last = *attr; last->next; last = last->next) {}
        last->next = at;
    }

finish:
    if (ret) {
        lyd_free_attr_single(ctx, at);
    } else if (attr) {
        *attr = at;
    }
    return LY_SUCCESS;
}

/**
 * @brief Check the equality of the two schemas from different contexts.
 *
 * @param schema1 of first node.
 * @param schema2 of second node.
 * @return 1 if the schemas are equal otherwise 0.
 */
static ly_bool
lyd_compare_schema_equal(const struct lysc_node *schema1, const struct lysc_node *schema2)
{
    if (!schema1 && !schema2) {
        return 1;
    } else if (!schema1 || !schema2) {
        return 0;
    }

    assert(schema1->module->ctx != schema2->module->ctx);

    if (schema1->nodetype != schema2->nodetype) {
        return 0;
    }

    if (strcmp(schema1->name, schema2->name)) {
        return 0;
    }

    if (strcmp(schema1->module->name, schema2->module->name)) {
        return 0;
    }

    return 1;
}

/**
 * @brief Check the equality of the schemas for all parent nodes.
 *
 * Both nodes must be from different contexts.
 *
 * @param node1 Data of first node.
 * @param node2 Data of second node.
 * @return 1 if the all related parental schemas are equal otherwise 0.
 */
static ly_bool
lyd_compare_schema_parents_equal(const struct lyd_node *node1, const struct lyd_node *node2)
{
    const struct lysc_node *parent1, *parent2;

    assert(node1 && node2);

    for (parent1 = node1->schema->parent, parent2 = node2->schema->parent;
            parent1 && parent2;
            parent1 = parent1->parent, parent2 = parent2->parent) {
        if (!lyd_compare_schema_equal(parent1, parent2)) {
            return 0;
        }
    }

    if (parent1 || parent2) {
        return 0;
    }

    return 1;
}

/**
 * @brief Compare 2 nodes values including opaque node values.
 *
 * @param[in] node1 First node to compare.
 * @param[in] node2 Second node to compare.
 * @return LY_SUCCESS if equal.
 * @return LY_ENOT if not equal.
 * @return LY_ERR on error.
 */
static LY_ERR
lyd_compare_single_value(const struct lyd_node *node1, const struct lyd_node *node2)
{
    const struct lyd_node_opaq *opaq1 = NULL, *opaq2 = NULL;
    const char *val1, *val2, *col;
    const struct lys_module *mod;
    char *val_dyn = NULL;
    LY_ERR rc = LY_SUCCESS;

    if (!node1->schema) {
        opaq1 = (struct lyd_node_opaq *)node1;
    }
    if (!node2->schema) {
        opaq2 = (struct lyd_node_opaq *)node2;
    }

    if (opaq1 && opaq2 && (opaq1->format == LY_VALUE_XML) && (opaq2->format == LY_VALUE_XML)) {
        /* opaque XML and opaque XML node */
        if (lyxml_value_compare(LYD_CTX(node1), opaq1->value, opaq1->val_prefix_data, LYD_CTX(node2), opaq2->value,
                opaq2->val_prefix_data)) {
            return LY_ENOT;
        }
        return LY_SUCCESS;
    }

    /* get their values */
    if (opaq1 && ((opaq1->format == LY_VALUE_XML) || (opaq1->format == LY_VALUE_STR_NS)) && (col = strchr(opaq1->value, ':'))) {
        /* XML value with a prefix, try to transform it into a JSON (canonical) value */
        mod = ly_resolve_prefix(LYD_CTX(node1), opaq1->value, col - opaq1->value, opaq1->format, opaq1->val_prefix_data);
        if (!mod) {
            /* unable to compare */
            return LY_ENOT;
        }

        if (asprintf(&val_dyn, "%s%s", mod->name, col) == -1) {
            LOGMEM(LYD_CTX(node1));
            return LY_EMEM;
        }
        val1 = val_dyn;
    } else {
        val1 = lyd_get_value(node1);
    }
    if (opaq2 && ((opaq2->format == LY_VALUE_XML) || (opaq2->format == LY_VALUE_STR_NS)) && (col = strchr(opaq2->value, ':'))) {
        mod = ly_resolve_prefix(LYD_CTX(node2), opaq2->value, col - opaq2->value, opaq2->format, opaq2->val_prefix_data);
        if (!mod) {
            return LY_ENOT;
        }

        assert(!val_dyn);
        if (asprintf(&val_dyn, "%s%s", mod->name, col) == -1) {
            LOGMEM(LYD_CTX(node2));
            return LY_EMEM;
        }
        val2 = val_dyn;
    } else {
        val2 = lyd_get_value(node2);
    }

    /* compare values */
    if (strcmp(val1, val2)) {
        rc = LY_ENOT;
    }

    free(val_dyn);
    return rc;
}

/**
 * @brief Compare 2 data nodes if they are equivalent regarding the schema tree.
 *
 * Works correctly even if @p node1 and @p node2 have different contexts.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @param[in] options Various @ref datacompareoptions.
 * @param[in] parental_schemas_checked Flag set if parent schemas were checked for match.
 * @return LY_SUCCESS if the nodes are equivalent.
 * @return LY_ENOT if the nodes are not equivalent.
 */
static LY_ERR
lyd_compare_single_schema(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options,
        ly_bool parental_schemas_checked)
{
    if (LYD_CTX(node1) == LYD_CTX(node2)) {
        /* same contexts */
        if (options & LYD_COMPARE_OPAQ) {
            if (lyd_node_schema(node1) != lyd_node_schema(node2)) {
                return LY_ENOT;
            }
        } else {
            if (node1->schema != node2->schema) {
                return LY_ENOT;
            }
        }
    } else {
        /* different contexts */
        if (!lyd_compare_schema_equal(node1->schema, node2->schema)) {
            return LY_ENOT;
        }
        if (!parental_schemas_checked) {
            if (!lyd_compare_schema_parents_equal(node1, node2)) {
                return LY_ENOT;
            }
            parental_schemas_checked = 1;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Compare 2 data nodes if they are equivalent regarding the data they contain.
 *
 * Works correctly even if @p node1 and @p node2 have different contexts.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @param[in] options Various @ref datacompareoptions.
 * @return LY_SUCCESS if the nodes are equivalent.
 * @return LY_ENOT if the nodes are not equivalent.
 */
static LY_ERR
lyd_compare_single_data(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options)
{
    const struct lyd_node *iter1, *iter2;
    struct lyd_node_any *any1, *any2;
    int len1, len2;
    LY_ERR r;

    if (!(options & LYD_COMPARE_OPAQ) && (node1->hash != node2->hash)) {
        return LY_ENOT;
    }
    /* equal hashes do not mean equal nodes, they can be just in collision so the nodes must be checked explicitly */

    if (!node1->schema || !node2->schema) {
        if (!(options & LYD_COMPARE_OPAQ) && ((node1->schema && !node2->schema) || (!node1->schema && node2->schema))) {
            return LY_ENOT;
        }
        if ((!node1->schema && !node2->schema) || (node1->schema && (node1->schema->nodetype & LYD_NODE_TERM)) ||
                (node2->schema && (node2->schema->nodetype & LYD_NODE_TERM))) {
            /* compare values only if there are any to compare */
            if ((r = lyd_compare_single_value(node1, node2))) {
                return r;
            }
        }

        if (options & LYD_COMPARE_FULL_RECURSION) {
            return lyd_compare_siblings_(lyd_child(node1), lyd_child(node2), options, 1);
        }
        return LY_SUCCESS;
    } else {
        switch (node1->schema->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            if (options & LYD_COMPARE_DEFAULTS) {
                if ((node1->flags & LYD_DEFAULT) != (node2->flags & LYD_DEFAULT)) {
                    return LY_ENOT;
                }
            }
            if ((r = lyd_compare_single_value(node1, node2))) {
                return r;
            }

            return LY_SUCCESS;
        case LYS_CONTAINER:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            /* implicit container is always equal to a container with non-default descendants */
            if (options & LYD_COMPARE_FULL_RECURSION) {
                return lyd_compare_siblings_(lyd_child(node1), lyd_child(node2), options, 1);
            }
            return LY_SUCCESS;
        case LYS_LIST:
            iter1 = lyd_child(node1);
            iter2 = lyd_child(node2);

            if (options & LYD_COMPARE_FULL_RECURSION) {
                return lyd_compare_siblings_(iter1, iter2, options, 1);
            } else if (node1->schema->flags & LYS_KEYLESS) {
                /* always equal */
                return LY_SUCCESS;
            }

            /* lists with keys, their equivalence is based on their keys */
            for (const struct lysc_node *key = lysc_node_child(node1->schema);
                    key && (key->flags & LYS_KEY);
                    key = key->next) {
                if (!iter1 || !iter2) {
                    return (iter1 == iter2) ? LY_SUCCESS : LY_ENOT;
                }
                r = lyd_compare_single_schema(iter1, iter2, options, 1);
                LY_CHECK_RET(r);
                r = lyd_compare_single_data(iter1, iter2, options);
                LY_CHECK_RET(r);

                iter1 = iter1->next;
                iter2 = iter2->next;
            }

            return LY_SUCCESS;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            any1 = (struct lyd_node_any *)node1;
            any2 = (struct lyd_node_any *)node2;

            if (any1->value_type != any2->value_type) {
                return LY_ENOT;
            }
            switch (any1->value_type) {
            case LYD_ANYDATA_DATATREE:
                return lyd_compare_siblings_(any1->value.tree, any2->value.tree, options, 1);
            case LYD_ANYDATA_STRING:
            case LYD_ANYDATA_XML:
            case LYD_ANYDATA_JSON:
                if ((!any1->value.str && any2->value.str) || (any1->value.str && !any2->value.str)) {
                    return LY_ENOT;
                } else if (!any1->value.str && !any2->value.str) {
                    return LY_SUCCESS;
                }
                len1 = strlen(any1->value.str);
                len2 = strlen(any2->value.str);
                if ((len1 != len2) || strcmp(any1->value.str, any2->value.str)) {
                    return LY_ENOT;
                }
                return LY_SUCCESS;
            case LYD_ANYDATA_LYB:
                len1 = lyd_lyb_data_length(any1->value.mem);
                len2 = lyd_lyb_data_length(any2->value.mem);
                if ((len1 == -1) || (len2 == -1) || (len1 != len2) || memcmp(any1->value.mem, any2->value.mem, len1)) {
                    return LY_ENOT;
                }
                return LY_SUCCESS;
            }
        }
    }

    LOGINT(LYD_CTX(node1));
    return LY_EINT;
}

/**
 * @brief Compare all siblings at a node level.
 *
 * @param[in] node1 First sibling list.
 * @param[in] node2 Second sibling list.
 * @param[in] options Various @ref datacompareoptions.
 * @param[in] parental_schemas_checked Flag set if parent schemas were checked for match.
 * @return LY_SUCCESS if equal.
 * @return LY_ENOT if not equal.
 * @return LY_ERR on error.
 */
static LY_ERR
lyd_compare_siblings_(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options,
        ly_bool parental_schemas_checked)
{
    LY_ERR r;
    const struct lyd_node *iter2;

    while (node1 && node2) {
        /* schema match */
        r = lyd_compare_single_schema(node1, node2, options, parental_schemas_checked);
        LY_CHECK_RET(r);

        if (node1->schema && (((node1->schema->nodetype == LYS_LIST) && !(node1->schema->flags & LYS_KEYLESS)) ||
                ((node1->schema->nodetype == LYS_LEAFLIST) && (node1->schema->flags & LYS_CONFIG_W))) &&
                (node1->schema->flags & LYS_ORDBY_SYSTEM)) {
            /* find a matching instance in case they are ordered differently */
            r = lyd_find_sibling_first(node2, node1, (struct lyd_node **)&iter2);
            if (r == LY_ENOTFOUND) {
                /* no matching instance, data not equal */
                r = LY_ENOT;
            }
            LY_CHECK_RET(r);
        } else {
            /* compare with the current node */
            iter2 = node2;
        }

        /* data match */
        r = lyd_compare_single_data(node1, iter2, options | LYD_COMPARE_FULL_RECURSION);
        LY_CHECK_RET(r);

        node1 = node1->next;
        node2 = node2->next;
    }

    return (node1 || node2) ? LY_ENOT : LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_compare_single(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options)
{
    LY_ERR r;

    if (!node1 || !node2) {
        return (node1 == node2) ? LY_SUCCESS : LY_ENOT;
    }

    /* schema match */
    if ((r = lyd_compare_single_schema(node1, node2, options, 0))) {
        return r;
    }

    /* data match */
    return lyd_compare_single_data(node1, node2, options);
}

LIBYANG_API_DEF LY_ERR
lyd_compare_siblings(const struct lyd_node *node1, const struct lyd_node *node2, uint32_t options)
{
    return lyd_compare_siblings_(node1, node2, options, 0);
}

LIBYANG_API_DEF LY_ERR
lyd_compare_meta(const struct lyd_meta *meta1, const struct lyd_meta *meta2)
{
    const struct ly_ctx *ctx;

    if (!meta1 || !meta2) {
        if (meta1 == meta2) {
            return LY_SUCCESS;
        } else {
            return LY_ENOT;
        }
    }

    ctx = meta1->annotation->module->ctx;
    if ((ctx != meta2->annotation->module->ctx) || (meta1->annotation != meta2->annotation)) {
        return LY_ENOT;
    }

    return meta1->value.realtype->plugin->compare(ctx, &meta1->value, &meta2->value);
}

/**
 * @brief Create a copy of the attribute.
 *
 * @param[in] attr Attribute to copy.
 * @param[in] node Opaque where to append the new attribute.
 * @param[out] dup Optional created attribute copy.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_dup_attr_single(const struct lyd_attr *attr, struct lyd_node *node, struct lyd_attr **dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_attr *a, *last;
    struct lyd_node_opaq *opaq = (struct lyd_node_opaq *)node;

    LY_CHECK_ARG_RET(NULL, attr, node, !node->schema, LY_EINVAL);

    /* create a copy */
    a = calloc(1, sizeof *attr);
    LY_CHECK_ERR_RET(!a, LOGMEM(LYD_CTX(node)), LY_EMEM);

    LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), attr->name.name, 0, &a->name.name), finish);
    LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), attr->name.prefix, 0, &a->name.prefix), finish);
    LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), attr->name.module_ns, 0, &a->name.module_ns), finish);
    LY_CHECK_GOTO(ret = lydict_insert(LYD_CTX(node), attr->value, 0, &a->value), finish);
    a->hints = attr->hints;
    a->format = attr->format;
    if (attr->val_prefix_data) {
        ret = ly_dup_prefix_data(LYD_CTX(node), attr->format, attr->val_prefix_data, &a->val_prefix_data);
        LY_CHECK_GOTO(ret, finish);
    }

    /* insert as the last attribute */
    a->parent = opaq;
    if (opaq->attr) {
        for (last = opaq->attr; last->next; last = last->next) {}
        last->next = a;
    } else {
        opaq->attr = a;
    }

finish:
    if (ret) {
        lyd_free_attr_single(LYD_CTX(node), a);
    } else if (dup) {
        *dup = a;
    }
    return LY_SUCCESS;
}

/**
 * @brief Find @p schema equivalent in @p trg_ctx.
 *
 * @param[in] schema Schema node to find.
 * @param[in] trg_ctx Target context to search in.
 * @param[in] parent Data parent of @p schema, if any.
 * @param[in] log Whether to log directly.
 * @param[out] trg_schema Found schema from @p trg_ctx to use.
 * @return LY_RRR value.
 */
static LY_ERR
lyd_find_schema_ctx(const struct lysc_node *schema, const struct ly_ctx *trg_ctx, const struct lyd_node *parent,
        ly_bool log, const struct lysc_node **trg_schema)
{
    const struct lysc_node *src_parent = NULL, *trg_parent = NULL, *sp, *tp;
    const struct lys_module *trg_mod = NULL;
    char *path;

    if (!schema) {
        /* opaque node */
        *trg_schema = NULL;
        return LY_SUCCESS;
    }

    if (lysc_data_parent(schema) && parent && parent->schema) {
        /* start from schema parent */
        trg_parent = parent->schema;
        src_parent = lysc_data_parent(schema);
    }

    do {
        /* find the next parent */
        sp = schema;
        while (lysc_data_parent(sp) != src_parent) {
            sp = lysc_data_parent(sp);
        }
        src_parent = sp;

        if (!src_parent->parent) {
            /* find the module first */
            trg_mod = ly_ctx_get_module_implemented(trg_ctx, src_parent->module->name);
            if (!trg_mod) {
                if (log) {
                    LOGERR(trg_ctx, LY_ENOTFOUND, "Module \"%s\" not present/implemented in the target context.",
                            src_parent->module->name);
                }
                return LY_ENOTFOUND;
            }
        }

        /* find the next parent */
        assert(trg_parent || trg_mod);
        tp = NULL;
        while ((tp = lys_getnext(tp, trg_parent, trg_mod ? trg_mod->compiled : NULL, 0))) {
            if (!strcmp(tp->name, src_parent->name) && !strcmp(tp->module->name, src_parent->module->name)) {
                break;
            }
        }
        if (!tp) {
            /* schema node not found */
            if (log) {
                path = lysc_path(src_parent, LYSC_PATH_LOG, NULL, 0);
                LOGERR(trg_ctx, LY_ENOTFOUND, "Schema node \"%s\" not found in the target context.", path);
                free(path);
            }
            return LY_ENOTFOUND;
        }

        trg_parent = tp;
    } while (schema != src_parent);

    /* success */
    *trg_schema = trg_parent;
    return LY_SUCCESS;
}

/**
 * @brief Duplicate a single node and connect it into @p parent (if present) or last of @p first siblings.
 *
 * Ignores ::LYD_DUP_WITH_PARENTS which is supposed to be handled by lyd_dup().
 *
 * @param[in] node Node to duplicate.
 * @param[in] trg_ctx Target context for duplicated nodes.
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in] insert_order Options for inserting (sorting) duplicated node, @ref insertorder.
 * @param[in,out] first First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup_p Pointer where the created duplicated node is placed (besides connecting it to @p parent / @p first).
 * @return LY_ERR value.
 */
static LY_ERR
lyd_dup_r(const struct lyd_node *node, const struct ly_ctx *trg_ctx, struct lyd_node *parent, uint32_t insert_order,
        struct lyd_node **first, uint32_t options, struct lyd_node **dup_p)
{
    LY_ERR ret;
    struct lyd_node *dup = NULL;
    struct lyd_meta *meta;
    struct lyd_attr *attr;
    struct lyd_node_any *any;
    const struct lysc_type *type;
    const char *val_can;

    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);

    if (node->flags & LYD_EXT) {
        if (options & LYD_DUP_NO_EXT) {
            /* no not duplicate this subtree */
            return LY_SUCCESS;
        }

        /* we need to use the same context */
        trg_ctx = LYD_CTX(node);
    }

    if (!node->schema) {
        dup = calloc(1, sizeof(struct lyd_node_opaq));
        ((struct lyd_node_opaq *)dup)->ctx = trg_ctx;
    } else {
        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_CONTAINER:
        case LYS_LIST:
            dup = calloc(1, sizeof(struct lyd_node_inner));
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            dup = calloc(1, sizeof(struct lyd_node_term));
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            dup = calloc(1, sizeof(struct lyd_node_any));
            break;
        default:
            LOGINT(trg_ctx);
            ret = LY_EINT;
            goto error;
        }
    }
    LY_CHECK_ERR_GOTO(!dup, LOGMEM(trg_ctx); ret = LY_EMEM, error);

    if (options & LYD_DUP_WITH_FLAGS) {
        dup->flags = node->flags;
    } else {
        dup->flags = (node->flags & (LYD_DEFAULT | LYD_EXT)) | LYD_NEW;
    }
    if (options & LYD_DUP_WITH_PRIV) {
        dup->priv = node->priv;
    }
    if (trg_ctx == LYD_CTX(node)) {
        dup->schema = node->schema;
    } else {
        ret = lyd_find_schema_ctx(node->schema, trg_ctx, parent, 1, &dup->schema);
        if (ret) {
            /* has no schema but is not an opaque node */
            free(dup);
            dup = NULL;
            goto error;
        }
    }
    dup->prev = dup;

    /* duplicate metadata/attributes */
    if (!(options & LYD_DUP_NO_META)) {
        if (!node->schema) {
            LY_LIST_FOR(((struct lyd_node_opaq *)node)->attr, attr) {
                LY_CHECK_GOTO(ret = lyd_dup_attr_single(attr, dup, NULL), error);
            }
        } else {
            LY_LIST_FOR(node->meta, meta) {
                LY_CHECK_GOTO(ret = lyd_dup_meta_single_to_ctx(trg_ctx, meta, dup, NULL), error);
            }
        }
    }

    /* nodetype-specific work */
    if (!dup->schema) {
        struct lyd_node_opaq *opaq = (struct lyd_node_opaq *)dup;
        struct lyd_node_opaq *orig = (struct lyd_node_opaq *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_r(child, trg_ctx, dup, LYD_INSERT_NODE_LAST, NULL, options, NULL), error);
            }
        }
        LY_CHECK_GOTO(ret = lydict_insert(trg_ctx, orig->name.name, 0, &opaq->name.name), error);
        LY_CHECK_GOTO(ret = lydict_insert(trg_ctx, orig->name.prefix, 0, &opaq->name.prefix), error);
        LY_CHECK_GOTO(ret = lydict_insert(trg_ctx, orig->name.module_ns, 0, &opaq->name.module_ns), error);
        LY_CHECK_GOTO(ret = lydict_insert(trg_ctx, orig->value, 0, &opaq->value), error);
        opaq->hints = orig->hints;
        opaq->format = orig->format;
        if (orig->val_prefix_data) {
            ret = ly_dup_prefix_data(trg_ctx, opaq->format, orig->val_prefix_data, &opaq->val_prefix_data);
            LY_CHECK_GOTO(ret, error);
        }
    } else if (dup->schema->nodetype & LYD_NODE_TERM) {
        struct lyd_node_term *term = (struct lyd_node_term *)dup;
        struct lyd_node_term *orig = (struct lyd_node_term *)node;

        term->hash = orig->hash;
        if (trg_ctx == LYD_CTX(node)) {
            ret = orig->value.realtype->plugin->duplicate(trg_ctx, &orig->value, &term->value);
            LY_CHECK_ERR_GOTO(ret, LOGERR(trg_ctx, ret, "Value duplication failed."), error);
        } else {
            /* store canonical value in the target context */
            val_can = lyd_get_value(node);
            type = ((struct lysc_node_leaf *)term->schema)->type;
            ret = lyd_value_store(trg_ctx, &term->value, type, val_can, strlen(val_can), 1, 1, NULL, LY_VALUE_CANON, NULL,
                    LYD_HINT_DATA, term->schema, NULL);
            LY_CHECK_GOTO(ret, error);
        }
    } else if (dup->schema->nodetype & LYD_NODE_INNER) {
        struct lyd_node_inner *orig = (struct lyd_node_inner *)node;
        struct lyd_node *child;

        if (options & LYD_DUP_RECURSIVE) {
            /* duplicate all the children */
            LY_LIST_FOR(orig->child, child) {
                LY_CHECK_GOTO(ret = lyd_dup_r(child, trg_ctx, dup, LYD_INSERT_NODE_LAST, NULL, options, NULL), error);
            }
        } else if ((dup->schema->nodetype == LYS_LIST) && !(dup->schema->flags & LYS_KEYLESS)) {
            /* always duplicate keys of a list */
            for (child = orig->child; child && lysc_is_key(child->schema); child = child->next) {
                LY_CHECK_GOTO(ret = lyd_dup_r(child, trg_ctx, dup, LYD_INSERT_NODE_LAST, NULL, options, NULL), error);
            }
        }
        lyd_hash(dup);
    } else if (dup->schema->nodetype & LYD_NODE_ANY) {
        dup->hash = node->hash;
        any = (struct lyd_node_any *)node;
        LY_CHECK_GOTO(ret = lyd_any_copy_value(dup, &any->value, any->value_type), error);
    }

    /* insert */
    lyd_insert_node(parent, first, dup, insert_order);

    if (dup_p) {
        *dup_p = dup;
    }
    return LY_SUCCESS;

error:
    lyd_free_tree(dup);
    return ret;
}

/**
 * @brief Duplicate a (leaf-)list and connect it into @p parent (if present) or last of @p first siblings.
 *
 * @param[in] orig Node to duplicate.
 * @param[in] trg_ctx Target context for duplicated nodes.
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup_p Pointer where the created duplicated node is placed (besides connecting it to @p parent / @p first).
 * @return LY_ERR value.
 */
static LY_ERR
lyd_dup_list(const struct lyd_node **orig, const struct ly_ctx *trg_ctx, struct lyd_node *parent,
        struct lyd_node **first, uint32_t options, struct lyd_node **dup_p)
{
    LY_ERR rc;
    struct lyd_node *start, *leader, *dup;
    const struct lysc_node *schema;
    uint32_t insert_order;

    /* duplicate leader */
    start = (*orig)->next;
    schema = (*orig)->schema;
    rc = lyd_dup_r(*orig, trg_ctx, parent, LYD_INSERT_NODE_DEFAULT, first, options, &leader);
    LY_CHECK_RET(rc);

    if (!start || !start->schema || !LYD_NODE_IS_ALONE(leader)) {
        /* no other instances */
        if (dup_p) {
            *dup_p = leader;
        }
        return LY_SUCCESS;
    }

    /* duplicate the rest of the nodes in the (leaf-)list */
    insert_order = leader->next ? LYD_INSERT_NODE_LAST_BY_SCHEMA : LYD_INSERT_NODE_LAST;
    LY_LIST_FOR(start, *orig) {
        if (schema != (*orig)->schema) {
            break;
        }
        rc = lyd_dup_r(*orig, trg_ctx, parent, insert_order, first, options, &dup);
        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    if (dup_p) {
        *dup_p = leader;
    }

    return rc;
}

/**
 * @brief Get a parent node to connect duplicated subtree to.
 *
 * @param[in] node Node (subtree) to duplicate.
 * @param[in] trg_ctx Target context for duplicated nodes.
 * @param[in] parent Initial parent to connect to.
 * @param[in] options Bitmask of options flags, see @ref dupoptions.
 * @param[out] dup_parent First duplicated parent node, if any.
 * @param[out] local_parent Correct parent to directly connect duplicated @p node to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_dup_get_local_parent(const struct lyd_node *node, const struct ly_ctx *trg_ctx, struct lyd_node *parent,
        uint32_t options, struct lyd_node **dup_parent, struct lyd_node **local_parent)
{
    const struct lyd_node *orig_parent;
    struct lyd_node *iter = NULL;
    ly_bool repeat = 1, ext_parent = 0;

    *dup_parent = NULL;
    *local_parent = NULL;

    if (node->flags & LYD_EXT) {
        ext_parent = 1;
    }
    for (orig_parent = lyd_parent(node); repeat && orig_parent; orig_parent = lyd_parent(orig_parent)) {
        if (ext_parent) {
            /* use the standard context */
            trg_ctx = LYD_CTX(orig_parent);
        }
        if (parent && (LYD_CTX(parent) == LYD_CTX(orig_parent)) && (parent->schema == orig_parent->schema)) {
            /* stop creating parents, connect what we have into the provided parent */
            iter = parent;
            repeat = 0;
        } else if (parent && (LYD_CTX(parent) != LYD_CTX(orig_parent)) &&
                lyd_compare_schema_equal(parent->schema, orig_parent->schema) &&
                lyd_compare_schema_parents_equal(parent, orig_parent)) {
            iter = parent;
            repeat = 0;
        } else {
            iter = NULL;
            LY_CHECK_RET(lyd_dup_r(orig_parent, trg_ctx, NULL, LYD_INSERT_NODE_DEFAULT, &iter, options, &iter));

            /* insert into the previous duplicated parent */
            if (*dup_parent) {
                lyd_insert_node(iter, NULL, *dup_parent, LYD_INSERT_NODE_DEFAULT);
            }

            /* update the last duplicated parent */
            *dup_parent = iter;
        }

        /* set the first parent */
        if (!*local_parent) {
            *local_parent = iter;
        }

        if (orig_parent->flags & LYD_EXT) {
            ext_parent = 1;
        }
    }

    if (repeat && parent) {
        /* given parent and created parents chain actually do not interconnect */
        LOGERR(trg_ctx, LY_EINVAL, "None of the duplicated node \"%s\" schema parents match the provided parent \"%s\".",
                LYD_NAME(node), LYD_NAME(parent));
        return LY_EINVAL;
    }

    if (*dup_parent && parent) {
        /* last insert into a prevously-existing parent */
        lyd_insert_node(parent, NULL, *dup_parent, LYD_INSERT_NODE_DEFAULT);
    }
    return LY_SUCCESS;
}

static LY_ERR
lyd_dup(const struct lyd_node *node, const struct ly_ctx *trg_ctx, struct lyd_node *parent, uint32_t options,
        ly_bool nosiblings, struct lyd_node **dup_p)
{
    LY_ERR rc;
    const struct lyd_node *orig;          /* original node to be duplicated */
    struct lyd_node *first_dup = NULL;    /* the first duplicated node, this is returned */
    struct lyd_node *top = NULL;          /* the most higher created node */
    struct lyd_node *local_parent = NULL; /* the direct parent node for the duplicated node(s) */
    struct lyd_node *dup = NULL;          /* duplicate node */
    struct lyd_node *first_sibling = NULL; /* first sibling node */

    assert(node && trg_ctx);

    if (options & LYD_DUP_WITH_PARENTS) {
        LY_CHECK_GOTO(rc = lyd_dup_get_local_parent(node, trg_ctx, parent, options & (LYD_DUP_WITH_FLAGS | LYD_DUP_NO_META),
                &top, &local_parent), error);
    } else {
        local_parent = parent;
    }

    LY_LIST_FOR(node, orig) {
        if (lysc_is_key(orig->schema)) {
            if (local_parent) {
                /* the key must already exist in the parent */
                rc = lyd_find_sibling_schema(lyd_child(local_parent), orig->schema, &dup);
                LY_CHECK_ERR_GOTO(rc, LOGINT(trg_ctx), error);
            } else {
                assert(!(options & LYD_DUP_WITH_PARENTS));
                /* duplicating a single key, okay, I suppose... */
                rc = lyd_dup_r(orig, trg_ctx, NULL, LYD_INSERT_NODE_DEFAULT, &first_sibling, options, &dup);
                LY_CHECK_GOTO(rc, error);
            }
        } else if (!nosiblings && orig->schema && (orig->schema->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
            /* duplicate the whole (leaf-)list */
            rc = lyd_dup_list(&orig, trg_ctx, local_parent, &first_sibling, options, &dup);
            LY_CHECK_GOTO(rc, error);
            if (!orig) {
                break;
            }
        } else {
            rc = lyd_dup_r(orig, trg_ctx, local_parent,
                    options & LYD_DUP_NO_LYDS ? LYD_INSERT_NODE_LAST_BY_SCHEMA : LYD_INSERT_NODE_DEFAULT,
                    &first_sibling, options, &dup);
            LY_CHECK_GOTO(rc, error);
        }
        first_dup = first_dup ? first_dup : dup;

        if (nosiblings) {
            break;
        }
    }

    if (dup_p) {
        *dup_p = first_dup;
    }
    return LY_SUCCESS;

error:
    if (top) {
        lyd_free_tree(top);
    } else if (first_dup) {
        lyd_free_siblings(first_dup);
    } else {
        lyd_free_siblings(dup);
    }
    return rc;
}

/**
 * @brief Check the context of node and parent when duplicating nodes.
 *
 * @param[in] node Node to duplicate.
 * @param[in] parent Parent of the duplicated node(s).
 * @return LY_ERR value.
 */
static LY_ERR
lyd_dup_ctx_check(const struct lyd_node *node, const struct lyd_node_inner *parent)
{
    const struct lyd_node *iter;

    if (!node || !parent) {
        return LY_SUCCESS;
    }

    if ((LYD_CTX(node) != LYD_CTX(parent))) {
        /* try to find top-level ext data parent */
        for (iter = node; iter && !(iter->flags & LYD_EXT); iter = lyd_parent(iter)) {}

        if (!iter || !lyd_parent(iter) || (LYD_CTX(lyd_parent(iter)) != LYD_CTX(parent))) {
            LOGERR(LYD_CTX(node), LY_EINVAL, "Different contexts used in node duplication.");
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_dup_single(const struct lyd_node *node, struct lyd_node_inner *parent, uint32_t options, struct lyd_node **dup)
{
    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);
    LY_CHECK_RET(lyd_dup_ctx_check(node, parent));

    return lyd_dup(node, LYD_CTX(node), (struct lyd_node *)parent, options, 1, dup);
}

LIBYANG_API_DEF LY_ERR
lyd_dup_single_to_ctx(const struct lyd_node *node, const struct ly_ctx *trg_ctx, struct lyd_node_inner *parent,
        uint32_t options, struct lyd_node **dup)
{
    LY_CHECK_ARG_RET(trg_ctx, node, trg_ctx, LY_EINVAL);

    return lyd_dup(node, trg_ctx, (struct lyd_node *)parent, options, 1, dup);
}

LIBYANG_API_DEF LY_ERR
lyd_dup_siblings(const struct lyd_node *node, struct lyd_node_inner *parent, uint32_t options, struct lyd_node **dup)
{
    LY_CHECK_ARG_RET(NULL, node, LY_EINVAL);
    LY_CHECK_RET(lyd_dup_ctx_check(node, parent));

    return lyd_dup(node, LYD_CTX(node), (struct lyd_node *)parent, options, 0, dup);
}

LIBYANG_API_DEF LY_ERR
lyd_dup_siblings_to_ctx(const struct lyd_node *node, const struct ly_ctx *trg_ctx, struct lyd_node_inner *parent,
        uint32_t options, struct lyd_node **dup)
{
    LY_CHECK_ARG_RET(trg_ctx, node, trg_ctx, LY_EINVAL);

    return lyd_dup(node, trg_ctx, (struct lyd_node *)parent, options, 0, dup);
}

LY_ERR
lyd_dup_meta_single_to_ctx(const struct ly_ctx *parent_ctx, const struct lyd_meta *meta, struct lyd_node *parent,
        struct lyd_meta **dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *mt, *last;
    const struct lysc_type *ant_type;
    struct lys_module *mod;
    const char *val_can;

    LY_CHECK_ARG_RET(NULL, meta, parent, LY_EINVAL);

    /* create a copy */
    mt = calloc(1, sizeof *mt);
    LY_CHECK_ERR_RET(!mt, LOGMEM(LYD_CTX(parent)), LY_EMEM);

    if (parent_ctx != meta->annotation->module->ctx) {
        /* different contexts */
        mod = ly_ctx_get_module(parent_ctx, meta->annotation->module->name, meta->annotation->module->revision);

        /* annotation */
        mt->annotation = lyd_get_meta_annotation(mod, meta->name, strlen(meta->name));
        lyplg_ext_get_storage(mt->annotation, LY_STMT_TYPE, sizeof ant_type, (const void **)&ant_type);
        LY_CHECK_ERR_GOTO((ret = mt->annotation ? LY_SUCCESS : LY_EINVAL), LOGERR(parent_ctx, LY_EINVAL,
                "Annotation for metadata %s not found, value duplication failed.", meta->name), finish);

        /* duplicate callback expect only the same contexts, so use the store callback */
        val_can = lyd_value_get_canonical(meta->annotation->module->ctx, &meta->value);
        ret = lyd_value_store(parent_ctx, &mt->value, ant_type, val_can, strlen(val_can), 1, 1, NULL,
                LY_VALUE_CANON, NULL, LYD_HINT_DATA, parent->schema, NULL);
    } else {
        /* annotation */
        mt->annotation = meta->annotation;
        /* duplication of value */
        ret = meta->value.realtype->plugin->duplicate(parent_ctx, &meta->value, &mt->value);
    }
    LY_CHECK_ERR_GOTO(ret, LOGERR(LYD_CTX(parent), LY_EINT, "Value duplication failed."), finish);
    LY_CHECK_GOTO(ret = lydict_insert(parent_ctx, meta->name, 0, &mt->name), finish);

    /* insert as the last attribute */
    mt->parent = parent;
    if (parent->meta) {
        for (last = parent->meta; last->next; last = last->next) {}
        last->next = mt;
    } else {
        parent->meta = mt;
    }

finish:
    if (ret) {
        lyd_free_meta_single(mt);
    } else if (dup) {
        *dup = mt;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_dup_meta_single(const struct lyd_meta *meta, struct lyd_node *node, struct lyd_meta **dup)
{
    LY_CHECK_ARG_RET(NULL, meta, LY_EINVAL);

    /* log to node context but value must always use the annotation context */
    return lyd_dup_meta_single_to_ctx(meta->annotation->module->ctx, meta, node, dup);
}

/**
 * @brief Merge a source sibling into target siblings.
 *
 * @param[in,out] first_trg First target sibling, is updated if top-level.
 * @param[in] parent_trg Target parent.
 * @param[in,out] sibling_src_p Source sibling to merge, set to NULL if spent.
 * @param[in] merge_cb Optional merge callback.
 * @param[in] cb_data Arbitrary callback data.
 * @param[in] options Merge options.
 * @param[in] lyds Pool of lyds data which can be reused.
 * @param[in,out] leader_p Cached first instance of target (leaf-)list.
 * @param[in,out] dup_inst Duplicate instance cache for all @p first_trg siblings.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_merge_sibling_r(struct lyd_node **first_trg, struct lyd_node *parent_trg,
        const struct lyd_node **sibling_src_p, lyd_merge_cb merge_cb, void *cb_data, uint16_t options,
        struct lyds_pool *lyds, struct lyd_node **leader_p, struct ly_ht **dup_inst)
{
    const struct lyd_node *child_src, *tmp, *sibling_src;
    struct lyd_node *match_trg, *dup_src, *elem, *leader;
    struct lyd_node_opaq *opaq_trg, *opaq_src;
    struct lysc_type *type;
    const struct lysc_node *schema;
    struct ly_ht *child_dup_inst = NULL;
    LY_ERR r;
    ly_bool first_inst = 0;

    sibling_src = *sibling_src_p;
    if (!sibling_src->schema) {
        /* try to find the same opaque node */
        r = lyd_find_sibling_opaq_next(*first_trg, LYD_NAME(sibling_src), &match_trg);
    } else if (sibling_src->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* try to find the exact instance */
        r = lyd_find_sibling_first(*first_trg, sibling_src, &match_trg);
    } else {
        /* try to simply find the node, there cannot be more instances */
        r = lyd_find_sibling_val(*first_trg, sibling_src->schema, NULL, 0, &match_trg);
    }
    LY_CHECK_RET(r && (r != LY_ENOTFOUND), r);

    if (match_trg) {
        /* update match as needed */
        LY_CHECK_RET(lyd_dup_inst_next(&match_trg, *first_trg, dup_inst));
    } else {
        /* first instance of this node */
        first_inst = 1;
    }

    if (match_trg) {
        /* call callback */
        if (merge_cb) {
            LY_CHECK_RET(merge_cb(match_trg, sibling_src, cb_data));
        }

        /* node found, make sure even value matches for all node types */
        if (!match_trg->schema) {
            if (lyd_compare_single(sibling_src, match_trg, 0)) {
                /* update value */
                opaq_trg = (struct lyd_node_opaq *)match_trg;
                opaq_src = (struct lyd_node_opaq *)sibling_src;

                lydict_remove(LYD_CTX(opaq_trg), opaq_trg->value);
                lydict_insert(LYD_CTX(opaq_trg), opaq_src->value, 0, &opaq_trg->value);
                opaq_trg->hints = opaq_src->hints;

                ly_free_prefix_data(opaq_trg->format, opaq_trg->val_prefix_data);
                opaq_trg->format = opaq_src->format;
                ly_dup_prefix_data(LYD_CTX(opaq_trg), opaq_src->format, opaq_src->val_prefix_data,
                        &opaq_trg->val_prefix_data);
            }
        } else if ((match_trg->schema->nodetype == LYS_LEAF) &&
                lyd_compare_single(sibling_src, match_trg, LYD_COMPARE_DEFAULTS)) {
            /* since they are different, they cannot both be default */
            assert(!(sibling_src->flags & LYD_DEFAULT) || !(match_trg->flags & LYD_DEFAULT));

            /* update value (or only LYD_DEFAULT flag) only if flag set or the source node is not default */
            if ((options & LYD_MERGE_DEFAULTS) || !(sibling_src->flags & LYD_DEFAULT)) {
                type = ((struct lysc_node_leaf *)match_trg->schema)->type;
                type->plugin->free(LYD_CTX(match_trg), &((struct lyd_node_term *)match_trg)->value);
                LY_CHECK_RET(type->plugin->duplicate(LYD_CTX(match_trg), &((struct lyd_node_term *)sibling_src)->value,
                        &((struct lyd_node_term *)match_trg)->value));

                /* copy flags and add LYD_NEW */
                match_trg->flags = sibling_src->flags | ((options & LYD_MERGE_WITH_FLAGS) ? 0 : LYD_NEW);
            }
        } else if ((match_trg->schema->nodetype & LYS_ANYDATA) && lyd_compare_single(sibling_src, match_trg, 0)) {
            /* update value */
            LY_CHECK_RET(lyd_any_copy_value(match_trg, &((struct lyd_node_any *)sibling_src)->value,
                    ((struct lyd_node_any *)sibling_src)->value_type));

            /* copy flags and add LYD_NEW */
            match_trg->flags = sibling_src->flags | ((options & LYD_MERGE_WITH_FLAGS) ? 0 : LYD_NEW);
        }

        /* check descendants, recursively */
        r = LY_SUCCESS;
        leader = NULL;
        schema = NULL;
        LY_LIST_FOR_SAFE(lyd_child_no_keys(sibling_src), tmp, child_src) {
            if ((options & LYD_MERGE_DESTRUCT) && (schema != child_src->schema) && LYDS_NODE_IS_LEADER(child_src)) {
                schema = child_src->schema;
                /* unlink lyds data and add them to the pool */
                lyds_pool_add((struct lyd_node *)child_src, lyds);
            }

            r = lyd_merge_sibling_r(lyd_node_child_p(match_trg), match_trg, &child_src,
                    merge_cb, cb_data, options, lyds, &leader, &child_dup_inst);
            if (r) {
                break;
            }
        }

        lyd_dup_inst_free(child_dup_inst);
        LY_CHECK_RET(r);
    } else {
        /* node not found, merge it */
        if (options & LYD_MERGE_DESTRUCT) {
            dup_src = (struct lyd_node *)sibling_src;
            lyd_unlink_ignore_lyds(NULL, dup_src);
            /* spend it */
            *sibling_src_p = NULL;
        } else {
            LY_CHECK_RET(lyd_dup_single(sibling_src, NULL, LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS, &dup_src));
        }

        if (!(options & LYD_MERGE_WITH_FLAGS)) {
            /* set LYD_NEW for all the new nodes, required for validation */
            LYD_TREE_DFS_BEGIN(dup_src, elem) {
                elem->flags |= LYD_NEW;
                LYD_TREE_DFS_END(dup_src, elem);
            }
        }

        if (lyds->rbn) {
            /* insert node and try to reuse free lyds data */
            lyds_insert2(parent_trg, first_trg, leader_p, dup_src, lyds);
        } else {
            /* generic insert node */
            lyd_insert_node(parent_trg, first_trg, dup_src, LYD_INSERT_NODE_DEFAULT);
        }

        if (first_inst) {
            /* remember not to find this instance next time */
            LY_CHECK_RET(lyd_dup_inst_next(&dup_src, *first_trg, dup_inst));
        }

        /* call callback, no source node */
        if (merge_cb) {
            LY_CHECK_RET(merge_cb(dup_src, NULL, cb_data));
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
lyd_merge(struct lyd_node **target, const struct lyd_node *source, const struct lys_module *mod,
        lyd_merge_cb merge_cb, void *cb_data, uint16_t options, ly_bool nosiblings)
{
    const struct lyd_node *sibling_src, *tmp;
    const struct lysc_node *schema;
    struct lyd_node *leader;
    struct ly_ht *dup_inst = NULL;
    ly_bool first;
    LY_ERR ret = LY_SUCCESS;
    struct lyds_pool lyds = {0};

    LY_CHECK_ARG_RET(NULL, target, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(*target ? LYD_CTX(*target) : NULL, source ? LYD_CTX(source) : NULL, mod ? mod->ctx : NULL,
            LY_EINVAL);

    if (!source) {
        /* nothing to merge */
        return LY_SUCCESS;
    }

    if ((*target && lysc_data_parent((*target)->schema)) || lysc_data_parent(source->schema)) {
        LOGERR(LYD_CTX(source), LY_EINVAL, "Invalid arguments - can merge only 2 top-level subtrees (%s()).", __func__);
        return LY_EINVAL;
    }

    leader = NULL;
    schema = NULL;
    LY_LIST_FOR_SAFE(source, tmp, sibling_src) {
        if (mod && (lyd_owner_module(sibling_src) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        if ((options & LYD_MERGE_DESTRUCT) && (schema != sibling_src->schema) && LYDS_NODE_IS_LEADER(sibling_src)) {
            schema = sibling_src->schema;
            /* unlink lyds data and add them to the pool */
            lyds_pool_add((struct lyd_node *)sibling_src, &lyds);
        }

        first = (sibling_src == source) ? 1 : 0;
        ret = lyd_merge_sibling_r(target, NULL, &sibling_src, merge_cb, cb_data, options,
                &lyds, &leader, &dup_inst);
        if (ret) {
            break;
        }
        if (first && !sibling_src) {
            /* source was spent (unlinked), move to the next node */
            source = tmp;
        }

        if (nosiblings) {
            break;
        }
    }
    lyds_pool_clean(&lyds);

    if (options & LYD_MERGE_DESTRUCT) {
        /* free any leftover source data that were not merged */
        lyd_free_siblings((struct lyd_node *)source);
    }

    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_merge_tree(struct lyd_node **target, const struct lyd_node *source, uint16_t options)
{
    return lyd_merge(target, source, NULL, NULL, NULL, options, 1);
}

LIBYANG_API_DEF LY_ERR
lyd_merge_siblings(struct lyd_node **target, const struct lyd_node *source, uint16_t options)
{
    return lyd_merge(target, source, NULL, NULL, NULL, options, 0);
}

LIBYANG_API_DEF LY_ERR
lyd_merge_module(struct lyd_node **target, const struct lyd_node *source, const struct lys_module *mod,
        lyd_merge_cb merge_cb, void *cb_data, uint16_t options)
{
    return lyd_merge(target, source, mod, merge_cb, cb_data, options, 0);
}

static LY_ERR
lyd_path_str_enlarge(char **buffer, size_t *buflen, size_t reqlen, ly_bool is_static)
{
    /* ending \0 */
    ++reqlen;

    if (reqlen > *buflen) {
        if (is_static) {
            return LY_EINCOMPLETE;
        }

        *buffer = ly_realloc(*buffer, reqlen * sizeof **buffer);
        if (!*buffer) {
            return LY_EMEM;
        }

        *buflen = reqlen;
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    const struct lyd_node *key;
    size_t len;
    const char *val;
    char quot;

    for (key = lyd_child(node); key && key->schema && (key->schema->flags & LYS_KEY); key = key->next) {
        val = lyd_get_value(key);
        len = 1 + strlen(key->schema->name) + 2 + strlen(val) + 2;
        LY_CHECK_RET(lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static));

        quot = '\'';
        if (strchr(val, '\'')) {
            quot = '"';
        }
        *bufused += sprintf(*buffer + *bufused, "[%s=%c%s%c]", key->schema->name, quot, val, quot);
    }

    return LY_SUCCESS;
}

/**
 * @brief Append leaf-list value predicate to path.
 *
 * @param[in] node Node to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
static LY_ERR
lyd_path_leaflist_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    size_t len;
    const char *val;
    char quot;

    val = lyd_get_value(node);
    len = 4 + strlen(val) + 2; /* "[.='" + val + "']" */
    LY_CHECK_RET(lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static));

    quot = '\'';
    if (strchr(val, '\'')) {
        quot = '"';
    }
    *bufused += sprintf(*buffer + *bufused, "[.=%c%s%c]", quot, val, quot);

    return LY_SUCCESS;
}

/**
 * @brief Append node position (relative to its other instances) predicate to path.
 *
 * @param[in] node Node to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
static LY_ERR
lyd_path_position_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static)
{
    size_t len;
    uint32_t pos;
    char *val = NULL;
    LY_ERR rc;

    pos = lyd_list_pos(node);
    if (asprintf(&val, "%" PRIu32, pos) == -1) {
        return LY_EMEM;
    }

    len = 1 + strlen(val) + 1;
    rc = lyd_path_str_enlarge(buffer, buflen, *bufused + len, is_static);
    if (rc != LY_SUCCESS) {
        goto cleanup;
    }

    *bufused += sprintf(*buffer + *bufused, "[%s]", val);

cleanup:
    free(val);
    return rc;
}

LIBYANG_API_DEF char *
lyd_path(const struct lyd_node *node, LYD_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    ly_bool is_static = 0;
    uint32_t i, depth;
    size_t bufused = 0, len;
    const struct lyd_node *iter, *parent;
    const struct lys_module *mod, *prev_mod;
    LY_ERR rc = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, node, NULL);
    if (buffer) {
        LY_CHECK_ARG_RET(LYD_CTX(node), buflen > 1, NULL);
        is_static = 1;
    } else {
        buflen = 0;
    }

    switch (pathtype) {
    case LYD_PATH_STD:
    case LYD_PATH_STD_NO_LAST_PRED:
        depth = 1;
        for (iter = node; iter->parent; iter = lyd_parent(iter)) {
            ++depth;
        }

        goto iter_print;
        while (depth) {
            /* find the right node */
            for (iter = node, i = 1; i < depth; iter = lyd_parent(iter), ++i) {}
iter_print:
            /* get the module */
            mod = lyd_node_module(iter);
            parent = lyd_parent(iter);
            prev_mod = lyd_node_module(parent);
            if (prev_mod == mod) {
                mod = NULL;
            }

            /* realloc string */
            len = 1 + (mod ? strlen(mod->name) + 1 : 0) + (iter->schema ? strlen(iter->schema->name) :
                    strlen(((struct lyd_node_opaq *)iter)->name.name));
            rc = lyd_path_str_enlarge(&buffer, &buflen, bufused + len, is_static);
            if (rc != LY_SUCCESS) {
                break;
            }

            /* print next node */
            bufused += sprintf(buffer + bufused, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "", LYD_NAME(iter));

            /* do not always print the last (first) predicate */
            if (iter->schema && ((depth > 1) || (pathtype == LYD_PATH_STD))) {
                switch (iter->schema->nodetype) {
                case LYS_LIST:
                    if (iter->schema->flags & LYS_KEYLESS) {
                        /* print its position */
                        rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, is_static);
                    } else {
                        /* print all list keys in predicates */
                        rc = lyd_path_list_predicate(iter, &buffer, &buflen, &bufused, is_static);
                    }
                    break;
                case LYS_LEAFLIST:
                    if (iter->schema->flags & LYS_CONFIG_W) {
                        /* print leaf-list value */
                        rc = lyd_path_leaflist_predicate(iter, &buffer, &buflen, &bufused, is_static);
                    } else {
                        /* print its position */
                        rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, is_static);
                    }
                    break;
                default:
                    /* nothing to print more */
                    break;
                }
            }
            if (rc != LY_SUCCESS) {
                break;
            }

            --depth;
        }
        break;
    }

    return buffer;
}

char *
lyd_path_set(const struct ly_set *dnodes, LYD_PATH_TYPE pathtype)
{
    uint32_t depth;
    size_t bufused = 0, buflen = 0, len;
    char *buffer = NULL;
    const struct lyd_node *iter, *parent;
    const struct lys_module *mod, *prev_mod;
    LY_ERR rc = LY_SUCCESS;

    switch (pathtype) {
    case LYD_PATH_STD:
    case LYD_PATH_STD_NO_LAST_PRED:
        for (depth = 1; depth <= dnodes->count; ++depth) {
            /* current node */
            iter = dnodes->dnodes[depth - 1];
            mod = lyd_node_module(iter);

            /* parent */
            parent = (depth > 1) ? dnodes->dnodes[depth - 2] : NULL;
            assert(!parent || !iter->schema || !parent->schema || (parent->schema->nodetype & LYD_NODE_ANY) ||
                    (lysc_data_parent(iter->schema) == parent->schema) ||
                    (!lysc_data_parent(iter->schema) && (LYD_CTX(iter) != LYD_CTX(parent))) ||
                    (parent->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)));

            /* get module to print, if any */
            prev_mod = lyd_node_module(parent);
            if (prev_mod == mod) {
                mod = NULL;
            }

            /* realloc string */
            len = 1 + (mod ? strlen(mod->name) + 1 : 0) + (iter->schema ? strlen(iter->schema->name) :
                    strlen(((struct lyd_node_opaq *)iter)->name.name));
            if ((rc = lyd_path_str_enlarge(&buffer, &buflen, bufused + len, 0))) {
                break;
            }

            /* print next node */
            bufused += sprintf(buffer + bufused, "/%s%s%s", mod ? mod->name : "", mod ? ":" : "", LYD_NAME(iter));

            /* do not always print the last (first) predicate */
            if (iter->schema && ((depth > 1) || (pathtype == LYD_PATH_STD))) {
                switch (iter->schema->nodetype) {
                case LYS_LIST:
                    if (iter->schema->flags & LYS_KEYLESS) {
                        /* print its position */
                        rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, 0);
                    } else {
                        /* print all list keys in predicates */
                        rc = lyd_path_list_predicate(iter, &buffer, &buflen, &bufused, 0);
                    }
                    break;
                case LYS_LEAFLIST:
                    if (iter->schema->flags & LYS_CONFIG_W) {
                        /* print leaf-list value */
                        rc = lyd_path_leaflist_predicate(iter, &buffer, &buflen, &bufused, 0);
                    } else {
                        /* print its position */
                        rc = lyd_path_position_predicate(iter, &buffer, &buflen, &bufused, 0);
                    }
                    break;
                default:
                    /* nothing to print more */
                    break;
                }
            }
            if (rc) {
                break;
            }
        }
        break;
    }

    return buffer;
}

LIBYANG_API_DEF struct lyd_meta *
lyd_find_meta(const struct lyd_meta *first, const struct lys_module *module, const char *name)
{
    struct lyd_meta *ret = NULL;
    const struct ly_ctx *ctx;
    const char *prefix, *tmp;
    char *str;
    size_t pref_len, name_len;

    LY_CHECK_ARG_RET(NULL, module || strchr(name, ':'), name, NULL);
    LY_CHECK_CTX_EQUAL_RET(first ? first->annotation->module->ctx : NULL, module ? module->ctx : NULL, NULL);

    if (!first) {
        return NULL;
    }

    ctx = first->annotation->module->ctx;

    /* parse the name */
    tmp = name;
    if (ly_parse_nodeid(&tmp, &prefix, &pref_len, &name, &name_len) || tmp[0]) {
        LOGERR(ctx, LY_EINVAL, "Metadata name \"%s\" is not valid.", name);
        return NULL;
    }

    /* find the module */
    if (prefix) {
        str = strndup(prefix, pref_len);
        module = ly_ctx_get_module_latest(ctx, str);
        free(str);
        LY_CHECK_ERR_RET(!module, LOGERR(ctx, LY_EINVAL, "Module \"%.*s\" not found.", (int)pref_len, prefix), NULL);
    }

    /* find the metadata */
    LY_LIST_FOR(first, first) {
        if ((first->annotation->module == module) && !strcmp(first->name, name)) {
            ret = (struct lyd_meta *)first;
            break;
        }
    }

    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_find_sibling_first(const struct lyd_node *siblings, const struct lyd_node *target, struct lyd_node **match)
{
    struct lyd_node **match_p, *iter, *dup = NULL;
    struct lyd_node_inner *parent;
    ly_bool found;

    LY_CHECK_ARG_RET(NULL, target, LY_EINVAL);
    if (!siblings) {
        /* no data */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (LYD_CTX(siblings) != LYD_CTX(target)) {
        /* create a duplicate in this context */
        LY_CHECK_RET(lyd_dup_single_to_ctx(target, LYD_CTX(siblings), NULL, 0, &dup));
        target = dup;
    }

    if ((siblings->schema && target->schema && (lysc_data_parent(siblings->schema) != lysc_data_parent(target->schema)))) {
        /* schema mismatch */
        lyd_free_tree(dup);
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    /* get first sibling */
    siblings = lyd_first_sibling(siblings);

    parent = siblings->parent;
    if (target->schema && parent && parent->schema && parent->children_ht) {
        assert(target->hash);

        if (lysc_is_dup_inst_list(target->schema)) {
            /* we must search the instances from beginning to find the first matching one */
            found = 0;
            LYD_LIST_FOR_INST(siblings, target->schema, iter) {
                if (!lyd_compare_single(target, iter, LYD_COMPARE_FULL_RECURSION)) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                siblings = iter;
            } else {
                siblings = NULL;
            }
        } else {
            /* find by hash */
            if (!lyht_find(parent->children_ht, &target, target->hash, (void **)&match_p)) {
                siblings = *match_p;
            } else {
                /* not found */
                siblings = NULL;
            }
        }
    } else {
        /* no children hash table or cannot be used */
        for ( ; siblings; siblings = siblings->next) {
            if (lysc_is_dup_inst_list(target->schema)) {
                if (!lyd_compare_single(siblings, target, LYD_COMPARE_FULL_RECURSION)) {
                    break;
                }
            } else {
                if (!lyd_compare_single(siblings, target, 0)) {
                    break;
                }
            }
        }
    }

    lyd_free_tree(dup);
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

LIBYANG_API_DEF LY_ERR
lyd_find_sibling_val(const struct lyd_node *siblings, const struct lysc_node *schema, const char *key_or_value,
        size_t val_len, struct lyd_node **match)
{
    LY_ERR rc;
    struct lyd_node *target = NULL;
    const struct lyd_node *parent;

    LY_CHECK_ARG_RET(NULL, schema, !(schema->nodetype & (LYS_CHOICE | LYS_CASE)), LY_EINVAL);
    if (!siblings) {
        /* no data */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if ((LYD_CTX(siblings) != schema->module->ctx)) {
        /* parent of ext nodes is useless */
        parent = (siblings->flags & LYD_EXT) ? NULL : lyd_parent(siblings);
        if (lyd_find_schema_ctx(schema, LYD_CTX(siblings), parent, 0, &schema)) {
            /* no schema node in siblings so certainly no data node either */
            if (match) {
                *match = NULL;
            }
            return LY_ENOTFOUND;
        }
    }

    if (siblings->schema && (lysc_data_parent(siblings->schema) != lysc_data_parent(schema))) {
        /* schema mismatch */
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (key_or_value && !val_len) {
        val_len = strlen(key_or_value);
    }

    if ((schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && key_or_value) {
        /* create a data node and find the instance */
        if (schema->nodetype == LYS_LEAFLIST) {
            /* target used attributes: schema, hash, value */
            rc = lyd_create_term(schema, key_or_value, val_len, 0, 1, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, NULL, &target);
            LY_CHECK_RET(rc);
        } else {
            /* target used attributes: schema, hash, child (all keys) */
            LY_CHECK_RET(lyd_create_list2(schema, key_or_value, val_len, 1, &target));
        }

        /* find it */
        rc = lyd_find_sibling_first(siblings, target, match);
    } else {
        /* find the first schema node instance */
        rc = lyd_find_sibling_schema(siblings, schema, match);
    }

    lyd_free_tree(target);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_find_sibling_dup_inst_set(const struct lyd_node *siblings, const struct lyd_node *target, struct ly_set **set)
{
    struct lyd_node **match_p, *first, *iter;
    struct lyd_node_inner *parent;
    uint32_t comp_opts;

    LY_CHECK_ARG_RET(NULL, target, set, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(siblings ? LYD_CTX(siblings) : NULL, LYD_CTX(target), LY_EINVAL);

    LY_CHECK_RET(ly_set_new(set));

    if (!siblings || (siblings->schema && (lysc_data_parent(siblings->schema) != lysc_data_parent(target->schema)))) {
        /* no data or schema mismatch */
        return LY_ENOTFOUND;
    }

    /* set options */
    comp_opts = (lysc_is_dup_inst_list(target->schema) ? LYD_COMPARE_FULL_RECURSION : 0);

    /* get first sibling */
    siblings = lyd_first_sibling(siblings);

    parent = siblings->parent;
    if (parent && parent->schema && parent->children_ht) {
        assert(target->hash);

        /* find the first instance */
        lyd_find_sibling_first(siblings, target, &first);
        if (first) {
            /* add it so that it is the first in the set */
            if (ly_set_add(*set, first, 1, NULL)) {
                goto error;
            }

            /* find by hash */
            if (!lyht_find(parent->children_ht, &target, target->hash, (void **)&match_p)) {
                iter = *match_p;
            } else {
                /* not found */
                iter = NULL;
            }
            while (iter) {
                /* add all found nodes into the set */
                if ((iter != first) && !lyd_compare_single(iter, target, comp_opts) && ly_set_add(*set, iter, 1, NULL)) {
                    goto error;
                }

                /* find next instance */
                if (lyht_find_next(parent->children_ht, &iter, iter->hash, (void **)&match_p)) {
                    iter = NULL;
                } else {
                    iter = *match_p;
                }
            }
        }
    } else {
        /* no children hash table */
        LY_LIST_FOR(siblings, siblings) {
            if (!lyd_compare_single(target, siblings, comp_opts)) {
                ly_set_add(*set, (void *)siblings, 1, NULL);
            }
        }
    }

    if (!(*set)->count) {
        return LY_ENOTFOUND;
    }
    return LY_SUCCESS;

error:
    ly_set_free(*set, NULL);
    *set = NULL;
    return LY_EMEM;
}

LIBYANG_API_DEF LY_ERR
lyd_find_sibling_opaq_next(const struct lyd_node *first, const char *name, struct lyd_node **match)
{
    LY_CHECK_ARG_RET(NULL, name, LY_EINVAL);

    if (first && first->schema) {
        first = first->prev;
        if (first->schema) {
            /* no opaque nodes */
            first = NULL;
        } else {
            /* opaque nodes are at the end, find quickly the first */
            while (!first->prev->schema) {
                first = first->prev;
            }
        }
    }

    for ( ; first; first = first->next) {
        assert(!first->schema);
        if (!strcmp(LYD_NAME(first), name)) {
            break;
        }
    }

    if (match) {
        *match = (struct lyd_node *)first;
    }
    return first ? LY_SUCCESS : LY_ENOTFOUND;
}

LIBYANG_API_DEF LY_ERR
lyd_find_xpath(const struct lyd_node *ctx_node, const char *xpath, struct ly_set **set)
{
    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);

    return lyd_find_xpath3(ctx_node, ctx_node, xpath, LY_VALUE_JSON, NULL, NULL, set);
}

LIBYANG_API_DEF LY_ERR
lyd_find_xpath2(const struct lyd_node *ctx_node, const char *xpath, const struct lyxp_var *vars, struct ly_set **set)
{
    LY_CHECK_ARG_RET(NULL, ctx_node, xpath, set, LY_EINVAL);

    return lyd_find_xpath3(ctx_node, ctx_node, xpath, LY_VALUE_JSON, NULL, vars, set);
}

LIBYANG_API_DEF LY_ERR
lyd_find_xpath3(const struct lyd_node *ctx_node, const struct lyd_node *tree, const char *xpath, LY_VALUE_FORMAT format,
        void *prefix_data, const struct lyxp_var *vars, struct ly_set **set)
{
    LY_CHECK_ARG_RET(NULL, tree, xpath, set, LY_EINVAL);

    *set = NULL;

    return lyd_eval_xpath4(ctx_node, tree, NULL, xpath, format, prefix_data, vars, NULL, set, NULL, NULL, NULL);
}

LIBYANG_API_DEF LY_ERR
lyd_eval_xpath(const struct lyd_node *ctx_node, const char *xpath, ly_bool *result)
{
    return lyd_eval_xpath3(ctx_node, NULL, xpath, LY_VALUE_JSON, NULL, NULL, result);
}

LIBYANG_API_DEF LY_ERR
lyd_eval_xpath2(const struct lyd_node *ctx_node, const char *xpath, const struct lyxp_var *vars, ly_bool *result)
{
    return lyd_eval_xpath3(ctx_node, NULL, xpath, LY_VALUE_JSON, NULL, vars, result);
}

LIBYANG_API_DEF LY_ERR
lyd_eval_xpath3(const struct lyd_node *ctx_node, const struct lys_module *cur_mod, const char *xpath,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lyxp_var *vars, ly_bool *result)
{
    return lyd_eval_xpath4(ctx_node, ctx_node, cur_mod, xpath, format, prefix_data, vars, NULL, NULL, NULL, NULL, result);
}

LIBYANG_API_DEF LY_ERR
lyd_eval_xpath4(const struct lyd_node *ctx_node, const struct lyd_node *tree, const struct lys_module *cur_mod,
        const char *xpath, LY_VALUE_FORMAT format, void *prefix_data, const struct lyxp_var *vars, LY_XPATH_TYPE *ret_type,
        struct ly_set **node_set, char **string, long double *number, ly_bool *boolean)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, tree, xpath, ((ret_type && node_set && string && number && boolean) ||
            (node_set && !string && !number && !boolean) || (!node_set && string && !number && !boolean) ||
            (!node_set && !string && number && !boolean) || (!node_set && !string && !number && boolean)), LY_EINVAL);

    /* parse expression */
    ret = lyxp_expr_parse((struct ly_ctx *)LYD_CTX(tree), xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* evaluate expression */
    ret = lyxp_eval(LYD_CTX(tree), exp, cur_mod, format, prefix_data, ctx_node, ctx_node, tree, vars, &xp_set,
            LYXP_IGNORE_WHEN);
    LY_CHECK_GOTO(ret, cleanup);

    /* return expected result type without or with casting */
    if (node_set) {
        /* node set */
        if (xp_set.type == LYXP_SET_NODE_SET) {
            /* transform into a set */
            LY_CHECK_GOTO(ret = ly_set_new(node_set), cleanup);
            (*node_set)->objs = malloc(xp_set.used * sizeof *(*node_set)->objs);
            LY_CHECK_ERR_GOTO(!(*node_set)->objs, LOGMEM(LYD_CTX(tree)); ret = LY_EMEM, cleanup);
            (*node_set)->size = xp_set.used;
            for (i = 0; i < xp_set.used; ++i) {
                if (xp_set.val.nodes[i].type == LYXP_NODE_ELEM) {
                    ret = ly_set_add(*node_set, xp_set.val.nodes[i].node, 1, NULL);
                    LY_CHECK_GOTO(ret, cleanup);
                }
            }
            if (ret_type) {
                *ret_type = LY_XPATH_NODE_SET;
            }
        } else if (!string && !number && !boolean) {
            LOGERR(LYD_CTX(tree), LY_EINVAL, "XPath \"%s\" result is not a node set.", xpath);
            ret = LY_EINVAL;
            goto cleanup;
        }
    }

    if (string) {
        if ((xp_set.type != LYXP_SET_STRING) && !node_set) {
            /* cast into string */
            LY_CHECK_GOTO(ret = lyxp_set_cast(&xp_set, LYXP_SET_STRING), cleanup);
        }
        if (xp_set.type == LYXP_SET_STRING) {
            /* string */
            *string = xp_set.val.str;
            xp_set.val.str = NULL;
            if (ret_type) {
                *ret_type = LY_XPATH_STRING;
            }
        }
    }

    if (number) {
        if ((xp_set.type != LYXP_SET_NUMBER) && !node_set) {
            /* cast into number */
            LY_CHECK_GOTO(ret = lyxp_set_cast(&xp_set, LYXP_SET_NUMBER), cleanup);
        }
        if (xp_set.type == LYXP_SET_NUMBER) {
            /* number */
            *number = xp_set.val.num;
            if (ret_type) {
                *ret_type = LY_XPATH_NUMBER;
            }
        }
    }

    if (boolean) {
        if ((xp_set.type != LYXP_SET_BOOLEAN) && !node_set) {
            /* cast into boolean */
            LY_CHECK_GOTO(ret = lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN), cleanup);
        }
        if (xp_set.type == LYXP_SET_BOOLEAN) {
            /* boolean */
            *boolean = xp_set.val.bln;
            if (ret_type) {
                *ret_type = LY_XPATH_BOOLEAN;
            }
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free((struct ly_ctx *)LYD_CTX(tree), exp);
    return ret;
}

/**
 * @brief Hash table node equal callback.
 */
static ly_bool
lyd_trim_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lyd_node *node1, *node2;

    node1 = *(struct lyd_node **)val1_p;
    node2 = *(struct lyd_node **)val2_p;

    return node1 == node2;
}

LIBYANG_API_DEF LY_ERR
lyd_trim_xpath(struct lyd_node **tree, const char *xpath, const struct lyxp_var *vars)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx = NULL;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    struct lyd_node *node, *parent;
    struct lyxp_set_hash_node hnode;
    struct ly_ht *parent_ht = NULL;
    struct ly_set free_set = {0};
    uint32_t i, hash;
    ly_bool is_result;

    LY_CHECK_ARG_RET(NULL, tree, xpath, LY_EINVAL);

    if (!*tree) {
        /* nothing to do */
        goto cleanup;
    }

    *tree = lyd_first_sibling(*tree);
    ctx = (struct ly_ctx *)LYD_CTX(*tree);

    /* parse expression */
    ret = lyxp_expr_parse(ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* evaluate expression */
    ret = lyxp_eval(ctx, exp, NULL, LY_VALUE_JSON, NULL, *tree, *tree, *tree, vars, &xp_set, LYXP_IGNORE_WHEN);
    LY_CHECK_GOTO(ret, cleanup);

    /* create hash table for all the parents of results */
    parent_ht = lyht_new(32, sizeof node, lyd_trim_equal_cb, NULL, 1);
    LY_CHECK_GOTO(!parent_ht, cleanup);

    for (i = 0; i < xp_set.used; ++i) {
        if (xp_set.val.nodes[i].type != LYXP_NODE_ELEM) {
            /* ignore */
            continue;
        }

        for (parent = lyd_parent(xp_set.val.nodes[i].node); parent; parent = lyd_parent(parent)) {
            /* add the parent into parent_ht */
            ret = lyht_insert(parent_ht, &parent, parent->hash, NULL);
            if (ret == LY_EEXIST) {
                /* shared parent, we are done */
                break;
            }
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

    hnode.type = LYXP_NODE_ELEM;
    LY_LIST_FOR(*tree, parent) {
        LYD_TREE_DFS_BEGIN(parent, node) {
            if (lysc_is_key(node->schema)) {
                /* ignore */
                goto next_iter;
            }

            /* check the results */
            is_result = 0;
            if (xp_set.ht) {
                hnode.node = node;
                hash = lyht_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
                hash = lyht_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
                hash = lyht_hash_multi(hash, NULL, 0);

                if (!lyht_find(xp_set.ht, &hnode, hash, NULL)) {
                    is_result = 1;
                }
            } else {
                /* not enough elements for a hash table */
                for (i = 0; i < xp_set.used; ++i) {
                    if (xp_set.val.nodes[i].type != LYXP_NODE_ELEM) {
                        /* ignore */
                        continue;
                    }

                    if (xp_set.val.nodes[i].node == node) {
                        is_result = 1;
                        break;
                    }
                }
            }

            if (is_result) {
                /* keep the whole subtree if the node is in the results */
                LYD_TREE_DFS_continue = 1;
            } else if (lyht_find(parent_ht, &node, node->hash, NULL)) {
                /* free the whole subtree if the node is not even among the selected parents */
                ret = ly_set_add(&free_set, node, 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);
                LYD_TREE_DFS_continue = 1;
            } /* else keep the parent node because a subtree is in the results */

next_iter:
            LYD_TREE_DFS_END(parent, node);
        }
    }

    /* free */
    for (i = 0; i < free_set.count; ++i) {
        node = free_set.dnodes[i];
        if (*tree == node) {
            *tree = (*tree)->next;
        }
        lyd_free_tree(node);
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free(ctx, exp);
    lyht_free(parent_ht, NULL);
    ly_set_erase(&free_set, NULL);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_find_path(const struct lyd_node *ctx_node, const char *path, ly_bool output, struct lyd_node **match)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *expr = NULL;
    struct ly_path *lypath = NULL;

    LY_CHECK_ARG_RET(NULL, ctx_node, ctx_node->schema, path, LY_EINVAL);

    /* parse the path */
    ret = ly_path_parse(LYD_CTX(ctx_node), ctx_node->schema, path, strlen(path), 0, LY_PATH_BEGIN_EITHER,
            LY_PATH_PREFIX_FIRST, LY_PATH_PRED_SIMPLE, &expr);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile the path */
    ret = ly_path_compile(LYD_CTX(ctx_node), NULL, ctx_node->schema, NULL, expr,
            output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, 0, LY_VALUE_JSON, NULL, &lypath);
    LY_CHECK_GOTO(ret, cleanup);

    /* evaluate the path */
    ret = ly_path_eval_partial(lypath, ctx_node, NULL, 0, NULL, match);

cleanup:
    lyxp_expr_free(LYD_CTX(ctx_node), expr);
    ly_path_free(LYD_CTX(ctx_node), lypath);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_find_target(const struct ly_path *path, const struct lyd_node *tree, struct lyd_node **match)
{
    LY_ERR ret;
    struct lyd_node *m;

    LY_CHECK_ARG_RET(NULL, path, LY_EINVAL);

    ret = ly_path_eval(path, tree, NULL, &m);
    if (ret) {
        if (match) {
            *match = NULL;
        }
        return LY_ENOTFOUND;
    }

    if (match) {
        *match = m;
    }
    return LY_SUCCESS;
}

LY_ERR
lyd_get_or_create_leafref_links_record(const struct lyd_node_term *node, struct lyd_leafref_links_rec **record, ly_bool create)
{
    struct ly_ht *ht;
    uint32_t hash;
    struct lyd_leafref_links_rec rec = {0};

    assert(node);
    assert(record);

    *record = NULL;

    if (!(ly_ctx_get_options(LYD_CTX(node)) & LY_CTX_LEAFREF_LINKING)) {
        return LY_EDENIED;
    }

    rec.node = node;
    ht = LYD_CTX(node)->leafref_links_ht;
    hash = lyht_hash((const char *)&node, sizeof node);

    if (lyht_find(ht, &rec, hash, (void **)record) == LY_ENOTFOUND) {
        if (create) {
            LY_CHECK_RET(lyht_insert_no_check(ht, &rec, hash, (void **)record));
        } else {
            return LY_ENOTFOUND;
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_leafref_get_links(const struct lyd_node_term *node, const struct lyd_leafref_links_rec **record)
{
    LY_CHECK_ARG_RET(NULL, node, record, LY_EINVAL);

    return lyd_get_or_create_leafref_links_record(node, (struct lyd_leafref_links_rec **)record, 0);
}

LY_ERR
lyd_link_leafref_node(const struct lyd_node_term *node, const struct lyd_node_term *leafref_node)
{
    const struct lyd_node_term **item = NULL;
    struct lyd_leafref_links_rec *rec;
    LY_ARRAY_COUNT_TYPE u;

    assert(node);
    assert(leafref_node);

    if (!(ly_ctx_get_options(LYD_CTX(node)) & LY_CTX_LEAFREF_LINKING)) {
        return LY_EDENIED;
    }

    /* add leafref node into the list of target node */
    LY_CHECK_RET(lyd_get_or_create_leafref_links_record(node, &rec, 1));
    LY_ARRAY_FOR(rec->leafref_nodes, u) {
        if (rec->leafref_nodes[u] == leafref_node) {
            return LY_SUCCESS;
        }
    }

    LY_ARRAY_NEW_RET(LYD_CTX(node), rec->leafref_nodes, item, LY_EMEM);
    *item = leafref_node;

    /* add target node into the list of leafref node*/
    LY_CHECK_RET(lyd_get_or_create_leafref_links_record(leafref_node, &rec, 1));
    LY_ARRAY_FOR(rec->target_nodes, u) {
        if (rec->target_nodes[u] == node) {
            return LY_SUCCESS;
        }
    }

    LY_ARRAY_NEW_RET(LYD_CTX(node), rec->target_nodes, item, LY_EMEM);
    *item = node;

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_leafref_link_node_tree(const struct lyd_node *tree)
{
    const struct lyd_node *sibling, *elem;
    struct ly_set *targets = NULL;
    char *errmsg;
    struct lyd_node_term *leafref_node;
    struct lysc_node_leaf *leaf_schema;
    struct lysc_type_leafref *lref;
    LY_ERR ret = LY_SUCCESS;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, tree, LY_EINVAL);

    if (!(ly_ctx_get_options(LYD_CTX(tree)) & LY_CTX_LEAFREF_LINKING)) {
        return LY_EDENIED;
    }

    LY_LIST_FOR(tree, sibling) {
        LYD_TREE_DFS_BEGIN(sibling, elem) {
            if (elem->schema && (elem->schema->nodetype & LYD_NODE_TERM)) {
                leafref_node = (struct lyd_node_term *)elem;
                leaf_schema = (struct lysc_node_leaf *)elem->schema;

                if (leaf_schema->type->basetype == LY_TYPE_LEAFREF) {
                    lref = (struct lysc_type_leafref *)leaf_schema->type;
                    ly_set_free(targets, NULL);
                    if (lyplg_type_resolve_leafref(lref, elem, &leafref_node->value, tree, &targets, &errmsg)) {
                        /* leafref target not found */
                        free(errmsg);
                    } else {
                        /* leafref target found, link it */
                        for (i = 0; i < targets->count; ++i) {
                            if (targets->dnodes[i]->schema->nodetype & LYD_NODE_TERM) {
                                ret = lyd_link_leafref_node((struct lyd_node_term *)targets->dnodes[i], leafref_node);
                                LY_CHECK_GOTO(ret, cleanup);
                            }
                        }
                    }
                }
            }
            LYD_TREE_DFS_END(sibling, elem);
        }
    }

cleanup:
    ly_set_free(targets, NULL);
    return ret;
}

LY_ERR
lyd_unlink_leafref_node(const struct lyd_node_term *node, const struct lyd_node_term *leafref_node)
{
    LY_ERR ret;
    struct lyd_leafref_links_rec *rec;

    assert(node);
    assert(leafref_node);

    if (!(ly_ctx_get_options(LYD_CTX(node)) & LY_CTX_LEAFREF_LINKING)) {
        return LY_EDENIED;
    }

    /* remove link from target node to leafref node */
    ret = lyd_get_or_create_leafref_links_record(node, &rec, 0);
    if (ret == LY_SUCCESS) {
        LY_ARRAY_REMOVE_VALUE(rec->leafref_nodes, leafref_node);
        if ((LY_ARRAY_COUNT(rec->leafref_nodes) == 0) && (LY_ARRAY_COUNT(rec->target_nodes) == 0)) {
            lyd_free_leafref_nodes(node);
        }
    } else if (ret != LY_ENOTFOUND) {
        return ret;
    }

    /* remove link from leafref node to target node */
    ret = lyd_get_or_create_leafref_links_record(leafref_node, &rec, 0);
    if (ret == LY_SUCCESS) {
        LY_ARRAY_REMOVE_VALUE(rec->target_nodes, node);
        if ((LY_ARRAY_COUNT(rec->leafref_nodes) == 0) && (LY_ARRAY_COUNT(rec->target_nodes) == 0)) {
            lyd_free_leafref_nodes(leafref_node);
        }
    } else if (ret != LY_ENOTFOUND) {
        return ret;
    }

    return LY_SUCCESS;
}
