/**
 * @file parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief JSON data parser for libyang
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in_internal.h"
#include "json.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"

/**
 * @brief Free the JSON data parser context.
 *
 * JSON implementation of lyd_ctx_free_clb().
 */
static void
lyd_json_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_json_ctx *ctx = (struct lyd_json_ctx *)lydctx;

    if (lydctx) {
        lyd_ctx_free(lydctx);
        lyjson_ctx_free(ctx->jsonctx);
        free(ctx);
    }
}

/**
 * @brief Pass the responsibility for releasing the dynamic values to @p dst.
 *
 * @param[in] jsonctx JSON context which contains the dynamic value.
 * @param[in,out] dst Pointer to which the responsibility will be submited.
 * If the pointer is already pointing to some allocated memory, it is released beforehand.
 */
static void
lyjson_ctx_give_dynamic_value(struct lyjson_ctx *jsonctx, char **dst)
{
    assert(jsonctx && dst);

    if (!jsonctx->dynamic) {
        return;
    }

    if (dst) {
        free(*dst);
    }
    *dst = NULL;

    /* give the dynamic value */
    *dst = (char *)jsonctx->value;

    /* responsibility for the release is now passed to dst */
    jsonctx->dynamic = 0;
}

/**
 * @brief Parse JSON member-name as [\@][prefix:][name]
 *
 * \@ - metadata flag, maps to 1 in @p is_meta_p
 * prefix - name of the module of the data node
 * name - name of the data node
 *
 * All the output parameter are mandatory. Function only parse the member-name, all the appropriate checks are up to the caller.
 *
 * @param[in] value String to parse
 * @param[in] value_len Length of the @p str.
 * @param[out] name_p Pointer to the beginning of the parsed name.
 * @param[out] name_len_p Pointer to the length of the parsed name.
 * @param[out] prefix_p Pointer to the beginning of the parsed prefix. If the member-name does not contain prefix, result is NULL.
 * @param[out] prefix_len_p Pointer to the length of the parsed prefix. If the member-name does not contain prefix, result is 0.
 * @param[out] is_meta_p Pointer to the metadata flag, set to 1 if the member-name contains \@, 0 otherwise.
 */
static void
lydjson_parse_name(const char *value, size_t value_len, const char **name_p, size_t *name_len_p, const char **prefix_p,
        size_t *prefix_len_p, ly_bool *is_meta_p)
{
    const char *name, *prefix = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0;

    name = memchr(value, ':', value_len);
    if (name != NULL) {
        prefix = value;
        if (*prefix == '@') {
            is_meta = 1;
            prefix++;
        }
        prefix_len = name - prefix;
        name++;
        name_len = value_len - (prefix_len + 1) - is_meta;
    } else {
        name = value;
        if (name[0] == '@') {
            is_meta = 1;
            name++;
        }
        name_len = value_len - is_meta;
    }

    *name_p = name;
    *name_len_p = name_len;
    *prefix_p = prefix;
    *prefix_len_p = prefix_len;
    *is_meta_p = is_meta;
}

/**
 * @brief Get correct prefix (module_name) inside the @p node.
 *
 * @param[in] node Data node to get inherited prefix.
 * @param[in] local_prefix Local prefix to replace the inherited prefix.
 * @param[in] local_prefix_len Length of the @p local_prefix string. In case of 0, the inherited prefix is taken.
 * @param[out] prefix_p Pointer to the resulting prefix string, Note that the result can be NULL in case of no local prefix
 * and no context @p node to get inherited prefix.
 * @param[out] prefix_len_p Pointer to the length of the resulting @p prefix_p string. Note that the result can be 0 in case
 * of no local prefix and no context @p node to get inherited prefix.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_get_node_prefix(struct lyd_node *node, const char *local_prefix, size_t local_prefix_len, const char **prefix_p,
        size_t *prefix_len_p)
{
    struct lyd_node_opaq *onode;
    const char *module_name = NULL;

    assert(prefix_p && prefix_len_p);

    if (local_prefix_len) {
        *prefix_p = local_prefix;
        *prefix_len_p = local_prefix_len;
        return LY_SUCCESS;
    }

    while (node) {
        if (node->schema) {
            module_name = node->schema->module->name;
            break;
        }
        onode = (struct lyd_node_opaq *)node;
        if (onode->name.module_name) {
            module_name = onode->name.module_name;
            break;
        } else if (onode->name.prefix) {
            module_name = onode->name.prefix;
            break;
        }
        node = lyd_parent(node);
    }

    *prefix_p = module_name;
    *prefix_len_p = ly_strlen(module_name);
    return LY_SUCCESS;
}

/**
 * @brief Skip the current JSON item (based on status).
 *
 * The JSON context is moved to the last "status" of the JSON item so to completely
 * finish the skip, one more JSON context move is required.
 *
 * @param[in] jsonctx JSON context with the input data to skip.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_data_skip(struct lyjson_ctx *jsonctx)
{
    enum LYJSON_PARSER_STATUS status, current;
    uint32_t depth;

    status = lyjson_ctx_status(jsonctx);
    depth = lyjson_ctx_depth(jsonctx);

    switch (status) {
    case LYJSON_OBJECT:
        ++depth;

        /* skip until object closes */
        do {
            LY_CHECK_RET(lyjson_ctx_next(jsonctx, &current));
        } while ((current != LYJSON_OBJECT_CLOSED) || (depth != lyjson_ctx_depth(jsonctx)));
        break;
    case LYJSON_ARRAY:
        ++depth;

        /* skip until array closes */
        do {
            LY_CHECK_RET(lyjson_ctx_next(jsonctx, &current));
        } while ((current != LYJSON_ARRAY_CLOSED) || (depth != lyjson_ctx_depth(jsonctx)));
        break;
    case LYJSON_OBJECT_NAME:
        /* just get to the value */
        LY_CHECK_RET(lyjson_ctx_next(jsonctx, &current));
        if ((current == LYJSON_OBJECT) || (current == LYJSON_ARRAY)) {
            LY_CHECK_RET(lydjson_data_skip(jsonctx));
        }
        break;
    default:
        /* no other status really expected, just go to next */
        LY_CHECK_RET(lyjson_ctx_next(jsonctx, &current));
        break;
    }

    return LY_SUCCESS;
}

/**
 * @brief Get schema node corresponding to the input parameters.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] is_attr Flag if the reference to the node is an attribute, for logging only.
 * @param[in] prefix Requested node's prefix (module name).
 * @param[in] prefix_len Length of the @p prefix.
 * @param[in] name Requested node's name.
 * @param[in] name_len Length of the @p name.
 * @param[in] parent Parent of the node being processed, can be NULL in case of top-level.
 * @param[out] snode Found schema node corresponding to the input parameters. If NULL, parse as an opaque node.
 * @param[out] ext Extension instance that provided @p snode, if any.
 * @return LY_SUCCES on success.
 * @return LY_ENOT if the whole object was parsed (skipped or as an extension).
 * @return LY_ERR on error.
 */
static LY_ERR
lydjson_get_snode(struct lyd_json_ctx *lydctx, ly_bool is_attr, const char *prefix, size_t prefix_len, const char *name,
        size_t name_len, struct lyd_node *parent, const struct lysc_node **snode, struct lysc_ext_instance **ext)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lys_module *mod = NULL;
    uint32_t getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    *snode = NULL;
    *ext = NULL;

    /* get the element module, prefer parent context because of extensions */
    if (prefix_len) {
        mod = ly_ctx_get_module_implemented2(parent ? LYD_CTX(parent) : lydctx->jsonctx->ctx, prefix, prefix_len);
    } else if (parent) {
        if (parent->schema) {
            mod = parent->schema->module;
        }
    } else if (!(lydctx->int_opts & LYD_INTOPT_ANY)) {
        LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX_JSON, "Top-level JSON object member \"%.*s\" must be namespace-qualified.",
                (int)(is_attr ? name_len + 1 : name_len), is_attr ? name - 1 : name);
        ret = LY_EVALID;
        goto cleanup;
    }
    if (!mod) {
        /* check for extension data */
        r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_JSON, NULL, name, name_len, snode, ext);
        if (r != LY_ENOT) {
            /* success or error */
            ret = r;
            goto cleanup;
        }

        /* unknown module */
        if (lydctx->parse_opts & LYD_PARSE_STRICT) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "No module named \"%.*s\" in the context.", (int)prefix_len, prefix);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    /* get the schema node */
    if (mod && (!parent || parent->schema)) {
        if (!parent && lydctx->ext) {
            *snode = lysc_ext_find_node(lydctx->ext, mod, name, name_len, 0, getnext_opts);
        } else {
            *snode = lys_find_child(lyd_parser_node_schema(parent), mod, name, name_len, 0, getnext_opts);
        }
        if (!*snode) {
            /* check for extension data */
            r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_JSON, NULL, name, name_len, snode, ext);
            if (r != LY_ENOT) {
                /* success or error */
                ret = r;
                goto cleanup;
            }

            /* unknown data node */
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                if (parent) {
                    LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found as a child of \"%s\" node.",
                            (int)name_len, name, LYD_NAME(parent));
                } else if (lydctx->ext) {
                    if (lydctx->ext->argument) {
                        LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE,
                                "Node \"%.*s\" not found in the \"%s\" %s extension instance.",
                                (int)name_len, name, lydctx->ext->argument, lydctx->ext->def->name);
                    } else {
                        LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the %s extension instance.",
                                (int)name_len, name, lydctx->ext->def->name);
                    }
                } else {
                    LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" module.",
                            (int)name_len, name, mod->name);
                }
                ret = LY_EVALID;
                goto cleanup;
            }
        } else {
            /* check that schema node is valid and can be used */
            ret = lyd_parser_check_schema((struct lyd_ctx *)lydctx, *snode);
        }
    }

cleanup:
    return ret;
}

/**
 * @brief Get the hint for the data type parsers according to the current JSON parser context.
 *
 * @param[in] jsonctx JSON parser context. The context is supposed to be on a value.
 * @param[in,out] status Pointer to the current context status,
 * in some circumstances the function manipulates with the context so the status is updated.
 * @param[out] type_hint_p Pointer to the variable to store the result.
 * @return LY_SUCCESS in case of success.
 * @return LY_EINVAL in case of invalid context status not referring to a value.
 */
static LY_ERR
lydjson_value_type_hint(struct lyjson_ctx *jsonctx, enum LYJSON_PARSER_STATUS *status_p, uint32_t *type_hint_p)
{
    *type_hint_p = 0;

    if (*status_p == LYJSON_ARRAY) {
        /* only [null] */
        LY_CHECK_RET(lyjson_ctx_next(jsonctx, status_p));
        if (*status_p != LYJSON_NULL) {
            LOGVAL(jsonctx->ctx, LYVE_SYNTAX_JSON,
                    "Expected JSON name/value or special name/[null], but input data contains name/[%s].",
                    lyjson_token2str(*status_p));
            return LY_EINVAL;
        }

        LY_CHECK_RET(lyjson_ctx_next(jsonctx, NULL));
        if (lyjson_ctx_status(jsonctx) != LYJSON_ARRAY_CLOSED) {
            LOGVAL(jsonctx->ctx, LYVE_SYNTAX_JSON, "Expected array end, but input data contains %s.",
                    lyjson_token2str(*status_p));
            return LY_EINVAL;
        }

        *type_hint_p = LYD_VALHINT_EMPTY;
    } else if (*status_p == LYJSON_STRING) {
        *type_hint_p = LYD_VALHINT_STRING | LYD_VALHINT_NUM64;
    } else if (*status_p == LYJSON_NUMBER) {
        *type_hint_p = LYD_VALHINT_DECNUM;
    } else if ((*status_p == LYJSON_FALSE) || (*status_p == LYJSON_TRUE)) {
        *type_hint_p = LYD_VALHINT_BOOLEAN;
    } else if (*status_p == LYJSON_NULL) {
        *type_hint_p = 0;
    } else {
        LOGVAL(jsonctx->ctx, LYVE_SYNTAX_JSON, "Unexpected input data %s.", lyjson_token2str(*status_p));
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check that the input data are parseable as the @p list.
 *
 * Checks for all the list's keys. Function does not revert the context state.
 *
 * @param[in] jsonctx JSON parser context.
 * @param[in] list List schema node corresponding to the input data object.
 * @return LY_SUCCESS in case the data are ok for the @p list
 * @return LY_ENOT in case the input data are not sufficient to fully parse the list instance.
 */
static LY_ERR
lydjson_check_list(struct lyjson_ctx *jsonctx, const struct lysc_node *list)
{
    LY_ERR rc = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS status = lyjson_ctx_status(jsonctx);
    struct ly_set key_set = {0};
    const struct lysc_node *snode;
    uint32_t i, hints;

    assert(list && (list->nodetype == LYS_LIST));

    /* get all keys into a set (keys do not have if-features or anything) */
    snode = NULL;
    while ((snode = lys_getnext(snode, list, NULL, 0)) && (snode->flags & LYS_KEY)) {
        rc = ly_set_add(&key_set, (void *)snode, 1, NULL);
        LY_CHECK_GOTO(rc, cleanup);
    }
    if (!key_set.count) {
        /* no keys */
        goto cleanup;
    }

    if (status == LYJSON_OBJECT) {
        do {
            const char *name, *prefix;
            size_t name_len, prefix_len;
            ly_bool is_attr;

            /* match the key */
            LY_CHECK_GOTO(rc = lyjson_ctx_next(jsonctx, &status), cleanup);
            if (status != LYJSON_OBJECT_NAME) {
                break;
            }
            snode = NULL;
            lydjson_parse_name(jsonctx->value, jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_attr);

            if (!is_attr && !prefix) {
                for (i = 0; i < key_set.count; ++i) {
                    if (!ly_strncmp(key_set.snodes[i]->name, name, name_len)) {
                        snode = key_set.snodes[i];
                        break;
                    }
                }

                /* get the value */
                LY_CHECK_GOTO(rc = lyjson_ctx_next(jsonctx, &status), cleanup);

                if (snode) {
                    /* we have the key, validate the value */
                    if ((status < LYJSON_NUMBER) || (status > LYJSON_NULL)) {
                        /* not a terminal */
                        rc = LY_ENOT;
                        goto cleanup;
                    }

                    rc = lydjson_value_type_hint(jsonctx, &status, &hints);
                    LY_CHECK_GOTO(rc, cleanup);
                    rc = ly_value_validate(NULL, snode, jsonctx->value, jsonctx->value_len, LY_VALUE_JSON, NULL, hints);
                    LY_CHECK_GOTO(rc, cleanup);

                    /* key with a valid value, remove from the set */
                    ly_set_rm_index(&key_set, i, NULL);
                }

                /* next object */
                LY_CHECK_GOTO(rc = lyjson_ctx_next(jsonctx, &status), cleanup);
            } else {
                /* skip the uninteresting object */
                LY_CHECK_GOTO(rc = lydjson_data_skip(jsonctx), cleanup);
                LY_CHECK_GOTO(rc = lyjson_ctx_next(jsonctx, &status), cleanup);
            }
        } while (key_set.count && (status == LYJSON_OBJECT_NEXT));
    }

    if (key_set.count) {
        /* some keys are missing/did not validate */
        rc = LY_ENOT;
    }

cleanup:
    ly_set_erase(&key_set, NULL);
    return rc;
}

/**
 * @brief Check in advance if the input data are parsable according to the provided @p snode.
 *
 * Note that the checks are done only in case the LYD_PARSE_OPAQ is allowed. Otherwise the same checking
 * is naturally done when the data are really parsed.
 *
 * @param[in] lydctx JSON data parser context. When the function returns, the context is in the same state
 * as before calling, despite it is necessary to process input data for checking.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[out] type_hint_p Pointer to a variable to store detected value type hint in case of leaf or leaf-list.
 * @return LY_SUCCESS in case the data are ok for the @p snode or the LYD_PARSE_OPAQ is not enabled.
 * @return LY_ENOT in case the input data are not sufficient to fully parse the list instance
 * @return LY_EINVAL in case of invalid leaf JSON encoding
 * and they are expected to be parsed as opaq nodes.
 */
static LY_ERR
lydjson_data_check_opaq(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, uint32_t *type_hint_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyjson_ctx *jsonctx = lydctx->jsonctx;
    enum LYJSON_PARSER_STATUS status;

    assert(snode);

    if (!(snode->nodetype & (LYD_NODE_TERM | LYS_LIST))) {
        /* can always be parsed as a data node if we have the schema node */
        return LY_SUCCESS;
    }

    /* backup parser */
    lyjson_ctx_backup(jsonctx);
    status = lyjson_ctx_status(jsonctx);

    if (lydctx->parse_opts & LYD_PARSE_OPAQ) {
        /* check if the node is parseable. if not, NULL the snode to announce that it is supposed to be parsed
         * as an opaq node */
        switch (snode->nodetype) {
        case LYS_LEAFLIST:
        case LYS_LEAF:
            /* value may not be valid in which case we parse it as an opaque node */
            if ((ret = lydjson_value_type_hint(jsonctx, &status, type_hint_p))) {
                break;
            }
            if (ly_value_validate(NULL, snode, jsonctx->value, jsonctx->value_len, LY_VALUE_JSON, NULL, *type_hint_p)) {
                ret = LY_ENOT;
            }
            break;
        case LYS_LIST:
            /* lists may not have all its keys */
            if (lydjson_check_list(jsonctx, snode)) {
                /* invalid list, parse as opaque if it misses/has invalid some keys */
                ret = LY_ENOT;
            }
            break;
        }
    } else if (snode->nodetype & LYD_NODE_TERM) {
        ret = lydjson_value_type_hint(jsonctx, &status, type_hint_p);
    }

    /* restore parser */
    lyjson_ctx_restore(jsonctx);

    return ret;
}

/**
 * @brief Join the forward-referencing metadata with their target data nodes.
 *
 * Note that JSON encoding for YANG data allows forward-referencing metadata only for leafs/leaf-lists.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in,out] first_p Pointer to the first sibling node variable (top-level or in a particular parent node)
 * as a starting point to search for the metadata's target data node
 * @return LY_SUCCESS on success
 * @return LY_EVALID in case there are some metadata with unresolved target data node instance
 */
static LY_ERR
lydjson_metadata_finish(struct lyd_json_ctx *lydctx, struct lyd_node **first_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node, *attr, *next, *meta_iter;
    struct lysc_ext_instance *ext;
    uint64_t instance = 0;
    const char *prev = NULL;
    uint32_t log_location_items = 0;

    /* finish linking metadata */
    LY_LIST_FOR_SAFE(*first_p, next, attr) {
        struct lyd_node_opaq *meta_container = (struct lyd_node_opaq *)attr;
        uint64_t match = 0;
        ly_bool is_attr;
        const char *name, *prefix;
        size_t name_len, prefix_len;
        const struct lysc_node *snode;

        if (attr->schema || (meta_container->name.name[0] != '@')) {
            /* not an opaq metadata node */
            continue;
        }

        LOG_LOCSET(NULL, attr, NULL, NULL);
        log_location_items++;

        if (prev != meta_container->name.name) {
            /* metas' names are stored in dictionary, so checking pointers must works */
            lydict_remove(lydctx->jsonctx->ctx, prev);
            LY_CHECK_GOTO(ret = lydict_insert(lydctx->jsonctx->ctx, meta_container->name.name, 0, &prev), cleanup);
            instance = 1;
        } else {
            instance++;
        }

        /* find the corresponding data node */
        LY_LIST_FOR(*first_p, node) {
            if (!node->schema) {
                /* opaq node - we are going to put into it just a generic attribute. */
                if (strcmp(&meta_container->name.name[1], ((struct lyd_node_opaq *)node)->name.name)) {
                    continue;
                }

                if (((struct lyd_node_opaq *)node)->hints & LYD_NODEHINT_LIST) {
                    LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX, "Metadata container references a sibling list node %s.",
                            ((struct lyd_node_opaq *)node)->name.name);
                    ret = LY_EVALID;
                    goto cleanup;
                }

                /* match */
                match++;
                if (match != instance) {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter) {
                    /* convert opaq node to a attribute of the opaq node */
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;

                    ret = lyd_create_attr(node, NULL, lydctx->jsonctx->ctx, meta->name.name, strlen(meta->name.name),
                            meta->name.prefix, ly_strlen(meta->name.prefix), meta->name.module_name,
                            ly_strlen(meta->name.module_name), meta->value, ly_strlen(meta->value), NULL, LY_VALUE_JSON,
                            NULL, meta->hints);
                    LY_CHECK_GOTO(ret, cleanup);
                }

                /* done */
                break;
            } else {
                /* this is the second time we are resolving the schema node, so it must succeed,
                 * but remember that snode can be still NULL */
                lydjson_parse_name(meta_container->name.name, strlen(meta_container->name.name), &name, &name_len,
                        &prefix, &prefix_len, &is_attr);
                assert(is_attr);
                lydjson_get_snode(lydctx, is_attr, prefix, prefix_len, name, name_len, lyd_parent(*first_p), &snode, &ext);

                if (snode != node->schema) {
                    continue;
                }

                /* match */
                match++;
                if (match != instance) {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter) {
                    /* convert opaq node to a metadata of the node */
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;
                    struct lys_module *mod = NULL;

                    mod = ly_ctx_get_module_implemented(lydctx->jsonctx->ctx, meta->name.prefix);
                    if (mod) {
                        ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, NULL, mod,
                                meta->name.name, strlen(meta->name.name), meta->value, ly_strlen(meta->value),
                                NULL, LY_VALUE_JSON, NULL, meta->hints, node->schema);
                        LY_CHECK_GOTO(ret, cleanup);
                    } else if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                        if (meta->name.prefix) {
                            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE,
                                    "Unknown (or not implemented) YANG module \"%s\" of metadata \"%s%s%s\".",
                                    meta->name.prefix, meta->name.prefix, ly_strlen(meta->name.prefix) ? ":" : "",
                                    meta->name.name);
                        } else {
                            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing YANG module of metadata \"%s\".",
                                    meta->name.name);
                        }
                        ret = LY_EVALID;
                        goto cleanup;
                    }
                }

                /* add/correct flags */
                ret = lyd_parse_set_data_flags(node, &node->meta, (struct lyd_ctx *)lydctx, ext);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            }
        }

        if (match != instance) {
            /* there is no corresponding data node for the metadata */
            if (instance > 1) {
                LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE,
                        "Missing JSON data instance #%" PRIu64 " to be coupled with %s metadata.",
                        instance, meta_container->name.name);
            } else {
                LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing JSON data instance to be coupled with %s metadata.",
                        meta_container->name.name);
            }
            ret = LY_EVALID;
        } else {
            /* remove the opaq attr */
            if (attr == (*first_p)) {
                *first_p = attr->next;
            }
            lyd_free_tree(attr);
        }

        LOG_LOCBACK(0, log_location_items, 0, 0);
        log_location_items = 0;
    }

cleanup:
    lydict_remove(lydctx->jsonctx->ctx, prev);

    LOG_LOCBACK(0, log_location_items, 0, 0);
    return ret;
}

/**
 * @brief Parse a metadata member.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] snode Schema node of the metadata parent.
 * @param[in] node Parent node in case the metadata is not forward-referencing (only LYD_NODE_TERM)
 * so the data node does not exists. In such a case the metadata is stored in the context for the later
 * processing by lydjson_metadata_finish().
 * @return LY_SUCCESS on success
 * @return Various LY_ERR values in case of failure.
 */
static LY_ERR
lydjson_metadata(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS, r;
    enum LYJSON_PARSER_STATUS status;
    const char *expected;
    ly_bool in_parent = 0;
    const char *name, *prefix = NULL;
    char *dynamic_prefname = NULL;
    size_t name_len, prefix_len = 0;
    struct lys_module *mod;
    const struct ly_ctx *ctx = lydctx->jsonctx->ctx;
    ly_bool is_attr = 0;
    struct lyd_node *prev = node;
    uint32_t instance = 0, val_hints;
    uint16_t nodetype;

    assert(snode || node);

    nodetype = snode ? snode->nodetype : LYS_CONTAINER;
    LOG_LOCSET(snode, NULL, NULL, NULL);

    /* move to the second item in the name/X pair */
    LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);

    /* check attribute encoding */
    switch (nodetype) {
    case LYS_LEAFLIST:
        expected = "@name/array of objects/nulls";
        LY_CHECK_GOTO(status != LYJSON_ARRAY, representation_error);

next_entry:
        if (status == LYJSON_ARRAY_CLOSED) {
            /* no more metadata */
            goto cleanup;
        }

        /* move into the array/next item */
        LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);
        instance++;
        LY_CHECK_GOTO((status != LYJSON_OBJECT) && (status != LYJSON_NULL), representation_error);

        if (!node || (node->schema != prev->schema)) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing JSON data instance #%u of %s:%s to be coupled with metadata.",
                    instance, prev->schema->module->name, prev->schema->name);
            rc = LY_EVALID;
            goto cleanup;
        }

        if (status == LYJSON_NULL) {
            /* continue with the next entry in the leaf-list array */
            prev = node;
            node = node->next;

            LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);
            goto next_entry;
        }
        break;
    case LYS_LEAF:
    case LYS_ANYXML:
        expected = "@name/object";

        LY_CHECK_GOTO(status != LYJSON_OBJECT, representation_error);
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_ANYDATA:
    case LYS_NOTIF:
    case LYS_ACTION:
    case LYS_RPC:
        in_parent = 1;
        expected = "@/object";
        LY_CHECK_GOTO(status != LYJSON_OBJECT, representation_error);
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* process all the members inside a single metadata object */
    assert(status == LYJSON_OBJECT);
    do {
        LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);
        LY_CHECK_GOTO(status != LYJSON_OBJECT_NAME, representation_error);

        lydjson_parse_name(lydctx->jsonctx->value, lydctx->jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_attr);
        lyjson_ctx_give_dynamic_value(lydctx->jsonctx, &dynamic_prefname);

        if (!name_len) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Metadata in JSON found with an empty name, followed by: %.10s", name);
            rc = LY_EVALID;
            goto cleanup;
        } else if (!prefix_len) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Metadata in JSON must be namespace-qualified, missing prefix for \"%.*s\".",
                    (int)lydctx->jsonctx->value_len, lydctx->jsonctx->value);
            rc = LY_EVALID;
            goto cleanup;
        } else if (is_attr) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Invalid format of the Metadata identifier in JSON, unexpected '@' in \"%.*s\"",
                    (int)lydctx->jsonctx->value_len, lydctx->jsonctx->value);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* get the element module */
        mod = ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
        if (!mod) {
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                LOGVAL(ctx, LYVE_REFERENCE, "Prefix \"%.*s\" of the metadata \"%.*s\" does not match any module in the context.",
                        (int)prefix_len, prefix, (int)name_len, name);
                rc = LY_EVALID;
                goto cleanup;
            }
            if (node->schema) {
                /* skip element with children */
                LY_CHECK_GOTO(rc = lydjson_data_skip(lydctx->jsonctx), cleanup);
                status = lyjson_ctx_status(lydctx->jsonctx);
                /* end of the item */
                continue;
            }
            assert(lydctx->parse_opts & LYD_PARSE_OPAQ);
        }

        /* get the value */
        LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);

        /* get value hints */
        LY_CHECK_GOTO(rc = lydjson_value_type_hint(lydctx->jsonctx, &status, &val_hints), cleanup);

        if (node->schema) {
            /* create metadata */
            rc = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, NULL, mod, name, name_len, lydctx->jsonctx->value,
                    lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic, LY_VALUE_JSON, NULL, val_hints, node->schema);
            LY_CHECK_GOTO(rc, cleanup);

            /* add/correct flags */
            rc = lyd_parse_set_data_flags(node, &node->meta, (struct lyd_ctx *)lydctx, NULL);
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            /* create attribute */
            const char *module_name;
            size_t module_name_len;

            lydjson_get_node_prefix(node, prefix, prefix_len, &module_name, &module_name_len);

            /* attr2 is always changed to the created attribute */
            rc = lyd_create_attr(node, NULL, lydctx->jsonctx->ctx, name, name_len, prefix, prefix_len, module_name,
                    module_name_len, lydctx->jsonctx->value, lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic,
                    LY_VALUE_JSON, NULL, val_hints);
            LY_CHECK_GOTO(rc, cleanup);
        }

        /* next member */
        LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);
    } while (status == LYJSON_OBJECT_NEXT);
    LY_CHECK_GOTO(status != LYJSON_OBJECT_CLOSED, representation_error);

    if (nodetype == LYS_LEAFLIST) {
        /* continue by processing another metadata object for the following
         * leaf-list instance since they are always instantiated in JSON array */
        prev = node;
        node = node->next;

        LY_CHECK_GOTO(rc = lyjson_ctx_next(lydctx->jsonctx, &status), cleanup);
        goto next_entry;
    }

    /* success */
    goto cleanup;

representation_error:
    LOGVAL(ctx, LYVE_SYNTAX_JSON,
            "The attribute(s) of %s \"%s\" is expected to be represented as JSON %s, but input data contains @%s/%s.",
            lys_nodetype2str(nodetype), node ? LYD_NAME(node) : LYD_NAME(prev), expected, lyjson_token2str(status),
            in_parent ? "" : "name");
    rc = LY_EVALID;

cleanup:
    if ((rc == LY_EVALID) && (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR)) {
        /* try to skip the invalid data */
        if ((r = lydjson_data_skip(lydctx->jsonctx))) {
            rc = r;
        }
    }
    free(dynamic_prefname);
    LOG_LOCBACK(1, 0, 0, 0);
    return rc;
}

/**
 * @brief Eat the node pointed by @p node_p by inserting it into @p parent and maintain the @p first_p pointing
 * to the first child node.
 *
 * @param[in] parent Parent node to insert to, can be NULL in case of top-level (or provided first_p).
 * @param[in,out] first_p Pointer to the first sibling node in case of top-level.
 * @param[in,out] node_p pointer to the new node to insert, after the insert is done, pointer is set to NULL.
 * @param[in] last If set, always insert at the end.
 * @param[in] ext Extension instance of @p node_p, if any.
 */
static void
lydjson_maintain_children(struct lyd_node *parent, struct lyd_node **first_p, struct lyd_node **node_p, ly_bool last,
        struct lysc_ext_instance *ext)
{
    if (!*node_p) {
        return;
    }

    /* insert, keep first pointer correct */
    if (ext) {
        lyplg_ext_insert(parent, *node_p);
    } else {
        lyd_insert_node(parent, first_p, *node_p, last);
    }
    if (first_p) {
        if (parent) {
            *first_p = lyd_child(parent);
        } else {
            while ((*first_p)->prev->next) {
                *first_p = (*first_p)->prev;
            }
        }
    }
    *node_p = NULL;
}

/**
 * @brief Wrapper for ::lyd_create_opaq().
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node, to inherit module name from.
 * @param[in,out] status_inner_p In case of processing JSON array, this parameter points to a standalone
 * context status of the array content. Otherwise, it is supposed to be the same as @p status_p.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_create_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
        struct lyd_node *parent, enum LYJSON_PARSER_STATUS *status_inner_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *value = NULL, *module_name;
    size_t value_len = 0, module_name_len = 0;
    ly_bool dynamic = 0;
    uint32_t type_hint = 0;

    if (*status_inner_p != LYJSON_OBJECT) {
        /* prepare for creating opaq node with a value */
        value = lydctx->jsonctx->value;
        value_len = lydctx->jsonctx->value_len;
        dynamic = lydctx->jsonctx->dynamic;
        lydctx->jsonctx->dynamic = 0;

        LY_CHECK_RET(lydjson_value_type_hint(lydctx->jsonctx, status_inner_p, &type_hint));
    }

    /* get the module name */
    lydjson_get_node_prefix(parent, prefix, prefix_len, &module_name, &module_name_len);
    if (!module_name && !parent && lydctx->any_schema) {
        /* in an anyxml/anydata tree, parsing first node, use the previous any schema node */
        module_name = lydctx->any_schema->module->name;
        module_name_len = strlen(module_name);
    }

    /* create node */
    ret = lyd_create_opaq(lydctx->jsonctx->ctx, name, name_len, prefix, prefix_len, module_name, module_name_len, value,
            value_len, &dynamic, LY_VALUE_JSON, NULL, type_hint, node_p);
    if (dynamic) {
        free((char *)value);
    }

    return ret;
}

static LY_ERR lydjson_subtree_r(struct lyd_json_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed);

/**
 * @brief Parse opaq node from the input.
 *
 * In case of processing array, the whole array is being processed and the resulting @p node_p is the last item of the array.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] status_inner_p In case of processing JSON array, this parameter points to a standalone
 * context status of the array content. Otherwise, it is supposed to be the same as @p status_p.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_parse_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
        struct lyd_node *parent, enum LYJSON_PARSER_STATUS *status_p, enum LYJSON_PARSER_STATUS *status_inner_p,
        struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_GOTO(ret = lydjson_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_inner_p, node_p), cleanup);

    assert(*node_p);
    LOG_LOCSET(NULL, *node_p, NULL, NULL);

    if ((*status_p == LYJSON_ARRAY) && (*status_inner_p == LYJSON_NULL)) {
        /* special array null value */
        ((struct lyd_node_opaq *)*node_p)->hints |= LYD_VALHINT_EMPTY;

        /* must be the only item */
        LY_CHECK_GOTO(ret = lyjson_ctx_next(lydctx->jsonctx, status_inner_p), cleanup);
        if (*status_inner_p != LYJSON_ARRAY_CLOSED) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX, "Array \"null\" member with another member.");
            ret = LY_EVALID;
            goto cleanup;
        }

        goto finish;
    }

    while (*status_p == LYJSON_ARRAY) {
        /* process another instance of the same node */
        if (*status_inner_p == LYJSON_OBJECT) {
            /* array with objects, list */
            ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_LIST;

            /* but first process children of the object in the array */
            do {
                LY_CHECK_GOTO(ret = lydjson_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL), cleanup);
                *status_inner_p = lyjson_ctx_status(lydctx->jsonctx);
            } while (*status_inner_p == LYJSON_OBJECT_NEXT);
        } else {
            /* array with values, leaf-list */
            ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_LEAFLIST;
        }

        LY_CHECK_GOTO(ret = lyjson_ctx_next(lydctx->jsonctx, status_inner_p), cleanup);
        if (*status_inner_p == LYJSON_ARRAY_CLOSED) {
            goto finish;
        }
        assert(*status_inner_p == LYJSON_ARRAY_NEXT);

        /* continue with the next instance */
        LY_CHECK_GOTO(ret = lyjson_ctx_next(lydctx->jsonctx, status_inner_p), cleanup);
        assert(*node_p);
        lydjson_maintain_children(parent, first_p, node_p, lydctx->parse_opts & LYD_PARSE_ORDERED ? 1 : 0, NULL);

        LOG_LOCBACK(0, 1, 0, 0);

        LY_CHECK_GOTO(ret = lydjson_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_inner_p, node_p), cleanup);

        assert(*node_p);
        LOG_LOCSET(NULL, *node_p, NULL, NULL);
    }

    if (*status_p == LYJSON_OBJECT) {
        /* process children */
        do {
            LY_CHECK_GOTO(ret = lydjson_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL), cleanup);
            *status_p = lyjson_ctx_status(lydctx->jsonctx);
        } while (*status_p == LYJSON_OBJECT_NEXT);
    }

finish:
    /* finish linking metadata */
    ret = lydjson_metadata_finish(lydctx, lyd_node_child_p(*node_p));

cleanup:
    if (*node_p) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    return ret;
}

/**
 * @brief Move to the second item in the name/X pair and parse opaq node from the input.
 *
 * This function is basically the wrapper of the ::lydjson_parse_opaq().
 * In addition, it calls the ::json_ctx_next() and prepares the status_inner_p parameter
 * for ::lydjson_parse_opaq().
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_ctx_next_parse_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len, const char *prefix,
        size_t prefix_len, struct lyd_node *parent, enum LYJSON_PARSER_STATUS *status_p,
        struct lyd_node **first_p, struct lyd_node **node_p)
{
    enum LYJSON_PARSER_STATUS status_inner = 0;

    /* move to the second item in the name/X pair */
    LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, status_p));

    if (*status_p == LYJSON_ARRAY) {
        /* move into the array */
        LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, &status_inner));
    } else {
        /* just a flag to pass correct parameters into lydjson_parse_opaq() */
        status_inner = LYJSON_ERROR;
    }

    if (status_inner == LYJSON_ERROR) {
        status_inner = *status_p;
    }

    /* parse opaq node from the input */
    LY_CHECK_RET(lydjson_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_p, &status_inner,
            first_p, node_p));

    return LY_SUCCESS;
}

/**
 * @brief Process the attribute container (starting by @)
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] attr_node The data node referenced by the attribute container, if already known.
 * @param[in] snode The schema node of the data node referenced by the attribute container, if known.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_parse_attribute(struct lyd_json_ctx *lydctx, struct lyd_node *attr_node, const struct lysc_node *snode,
        const char *name, size_t name_len, const char *prefix, size_t prefix_len, struct lyd_node *parent,
        enum LYJSON_PARSER_STATUS *status_p, struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR r;
    const char *opaq_name, *mod_name;
    size_t opaq_name_len;

    if (!snode && !prefix) {
        /* set the prefix */
        if (parent) {
            lydjson_get_node_prefix(parent, NULL, 0, &prefix, &prefix_len);
        } else {
            prefix = "";
            prefix_len = 0;
        }
    }

    /* parse as an attribute to a (opaque) node */
    if (!attr_node) {
        /* try to find the instance */
        LY_LIST_FOR(*first_p, attr_node) {
            if (snode) {
                if (attr_node->schema) {
                    if (attr_node->schema == snode) {
                        break;
                    }
                } else {
                    mod_name = ((struct lyd_node_opaq *)attr_node)->name.module_name;
                    if (!strcmp(LYD_NAME(attr_node), snode->name) && mod_name && !strcmp(mod_name, snode->module->name)) {
                        break;
                    }
                }
            } else {
                if (attr_node->schema) {
                    if (!ly_strncmp(LYD_NAME(attr_node), name, name_len) &&
                            !ly_strncmp(attr_node->schema->module->name, prefix, prefix_len)) {
                        break;
                    }
                } else {
                    mod_name = ((struct lyd_node_opaq *)attr_node)->name.module_name;
                    if (!ly_strncmp(LYD_NAME(attr_node), name, name_len) && mod_name &&
                            !ly_strncmp(mod_name, prefix, prefix_len)) {
                        break;
                    }
                }
            }
        }
    }
    if (!attr_node) {
        /* parse just as an opaq node with the name beginning with @,
         * later we have to check that it belongs to a standard node
         * and it is supposed to be converted to a metadata */
        uint32_t prev_opts;

        /* backup parser options to parse unknown metadata as opaq nodes and try to resolve them later */
        prev_opts = lydctx->parse_opts;
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ;

        opaq_name = prefix ? prefix - 1 : name - 1;
        opaq_name_len = prefix ? prefix_len + name_len + 2 : name_len + 1;
        r = lydjson_ctx_next_parse_opaq(lydctx, opaq_name, opaq_name_len, NULL, 0, parent, status_p, first_p, node_p);

        /* restore the parser options */
        lydctx->parse_opts = prev_opts;
        LY_CHECK_RET(r);
    } else {
        LY_CHECK_RET(lydjson_metadata(lydctx, snode, attr_node));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a single anydata/anyxml node.
 *
 * @param[in] lydctx JSON data parser context. When the function returns, the context is in the same state
 * as before calling, despite it is necessary to process input data for checking.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in,out] status JSON parser status, is updated.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed,
 * @return LY_ENOT in case of invalid JSON encoding,
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydjson_parse_any(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, struct lysc_ext_instance *ext,
        enum LYJSON_PARSER_STATUS *status, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t prev_parse_opts = lydctx->parse_opts, prev_int_opts = lydctx->int_opts;
    struct ly_in in_start;
    char *val = NULL;
    const char *end;
    struct lyd_node *child = NULL;
    ly_bool log_node = 0;

    assert(snode->nodetype & LYD_NODE_ANY);

    *node = NULL;

    /* status check according to allowed JSON types */
    if (snode->nodetype == LYS_ANYXML) {
        LY_CHECK_RET((*status != LYJSON_OBJECT) && (*status != LYJSON_ARRAY) && (*status != LYJSON_NUMBER) &&
                (*status != LYJSON_STRING) && (*status != LYJSON_FALSE) && (*status != LYJSON_TRUE) &&
                (*status != LYJSON_NULL), LY_ENOT);
    } else {
        LY_CHECK_RET(*status != LYJSON_OBJECT, LY_ENOT);
    }

    /* create any node */
    switch (*status) {
    case LYJSON_OBJECT:
        /* create node */
        r = lyd_create_any(snode, NULL, LYD_ANYDATA_DATATREE, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        assert(*node);
        LOG_LOCSET(NULL, *node, NULL, NULL);
        log_node = 1;

        /* parse any data tree with correct options, first backup the current options and then make the parser
         * process data as opaq nodes */
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ | (ext ? LYD_PARSE_ONLY : 0);
        lydctx->int_opts |= LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS;
        lydctx->any_schema = snode;

        /* process the anydata content */
        do {
            r = lydjson_subtree_r(lydctx, NULL, &child, NULL);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

            *status = lyjson_ctx_status(lydctx->jsonctx);
        } while (*status == LYJSON_OBJECT_NEXT);

        /* finish linking metadata */
        r = lydjson_metadata_finish(lydctx, &child);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* assign the data tree */
        ((struct lyd_node_any *)*node)->value.tree = child;
        child = NULL;
        break;
    case LYJSON_ARRAY:
        /* skip until the array end */
        in_start = *lydctx->jsonctx->in;
        LY_CHECK_GOTO(rc = lydjson_data_skip(lydctx->jsonctx), cleanup);

        /* return back by all the WS */
        end = lydctx->jsonctx->in->current;
        while (is_jsonws(end[-1])) {
            --end;
        }

        /* make a copy of the whole array and store it */
        if (asprintf(&val, "[%.*s", (int)(end - in_start.current), in_start.current) == -1) {
            LOGMEM(lydctx->jsonctx->ctx);
            rc = LY_EMEM;
            goto cleanup;
        }
        r = lyd_create_any(snode, val, LYD_ANYDATA_JSON, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        val = NULL;
        break;
    case LYJSON_STRING:
        /* string value */
        if (lydctx->jsonctx->dynamic) {
            LY_CHECK_GOTO(rc = lyd_create_any(snode, lydctx->jsonctx->value, LYD_ANYDATA_STRING, 1, node), cleanup);
            lydctx->jsonctx->dynamic = 0;
        } else {
            val = strndup(lydctx->jsonctx->value, lydctx->jsonctx->value_len);
            LY_CHECK_ERR_GOTO(!val, LOGMEM(lydctx->jsonctx->ctx); rc = LY_EMEM, cleanup);

            r = lyd_create_any(snode, val, LYD_ANYDATA_STRING, 1, node);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
            val = NULL;
        }
        break;
    case LYJSON_NUMBER:
    case LYJSON_FALSE:
    case LYJSON_TRUE:
        /* JSON value */
        assert(!lydctx->jsonctx->dynamic);
        val = strndup(lydctx->jsonctx->value, lydctx->jsonctx->value_len);
        LY_CHECK_ERR_GOTO(!val, LOGMEM(lydctx->jsonctx->ctx); rc = LY_EMEM, cleanup);

        r = lyd_create_any(snode, val, LYD_ANYDATA_JSON, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        val = NULL;
        break;
    case LYJSON_NULL:
        /* no value */
        r = lyd_create_any(snode, NULL, LYD_ANYDATA_JSON, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        break;
    default:
        LOGINT(lydctx->jsonctx->ctx);
        rc = LY_EINT;
        goto cleanup;
    }

cleanup:
    if (log_node) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    lydctx->parse_opts = prev_parse_opts;
    lydctx->int_opts = prev_int_opts;
    lydctx->any_schema = NULL;
    free(val);
    lyd_free_tree(child);
    return rc;
}

/**
 * @brief Parse a single instance of an inner node.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in,out] status JSON parser status, is updated.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed,
 * @return LY_ENOT in case of invalid JSON encoding,
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydjson_parse_instance_inner(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, struct lysc_ext_instance *ext,
        enum LYJSON_PARSER_STATUS *status, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t prev_parse_opts = lydctx->parse_opts;

    LY_CHECK_RET(*status != LYJSON_OBJECT, LY_ENOT);

    /* create inner node */
    LY_CHECK_RET(lyd_create_inner(snode, node));

    /* use it for logging */
    LOG_LOCSET(NULL, *node, NULL, NULL);

    if (ext) {
        /* only parse these extension data and validate afterwards */
        lydctx->parse_opts |= LYD_PARSE_ONLY;
    }

    /* process children */
    do {
        r = lydjson_subtree_r(lydctx, *node, lyd_node_child_p(*node), NULL);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        *status = lyjson_ctx_status(lydctx->jsonctx);
    } while (*status == LYJSON_OBJECT_NEXT);

    /* finish linking metadata */
    r = lydjson_metadata_finish(lydctx, lyd_node_child_p(*node));
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    if (snode->nodetype == LYS_LIST) {
        /* check all keys exist */
        r = lyd_parse_check_keys(*node);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    if (!(lydctx->parse_opts & LYD_PARSE_ONLY) && !rc) {
        /* new node validation, autodelete CANNOT occur (it can if multi-error), all nodes are new */
        r = lyd_validate_new(lyd_node_child_p(*node), snode, NULL, lydctx->val_opts, NULL);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        /* add any missing default children */
        r = lyd_new_implicit_r(*node, lyd_node_child_p(*node), NULL, NULL, &lydctx->node_when, &lydctx->node_types,
                &lydctx->ext_node, (lydctx->val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, NULL);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

cleanup:
    lydctx->parse_opts = prev_parse_opts;
    LOG_LOCBACK(0, 1, 0, 0);
    if (!(*node)->hash) {
        /* list without keys is unusable */
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse a single instance of a node.
 *
 * @param[in] lydctx JSON data parser context. When the function returns, the context is in the same state
 * as before calling, despite it is necessary to process input data for checking.
 * @param[in] parent Data parent of the subtree, must be set if @p first is not.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling, must be set if @p parent is not.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in] name Parsed JSON node name.
 * @param[in] name_len Lenght of @p name.
 * @param[in] prefix Parsed JSON node prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in,out] status JSON parser status, is updated.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed,
 * @return LY_ENOT in case of invalid JSON encoding,
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydjson_parse_instance(struct lyd_json_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
        const struct lysc_node *snode, struct lysc_ext_instance *ext, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, enum LYJSON_PARSER_STATUS *status, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t type_hints = 0;

    LOG_LOCSET(snode, NULL, NULL, NULL);

    r = lydjson_data_check_opaq(lydctx, snode, &type_hints);
    if (r == LY_SUCCESS) {
        assert(snode->nodetype & (LYD_NODE_TERM | LYD_NODE_INNER | LYD_NODE_ANY));
        if (snode->nodetype & LYD_NODE_TERM) {
            if ((*status != LYJSON_ARRAY) && (*status != LYJSON_NUMBER) && (*status != LYJSON_STRING) &&
                    (*status != LYJSON_FALSE) && (*status != LYJSON_TRUE) && (*status != LYJSON_NULL)) {
                rc = LY_ENOT;
                goto cleanup;
            }

            /* create terminal node */
            r = lyd_parser_create_term((struct lyd_ctx *)lydctx, snode, lydctx->jsonctx->value,
                    lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic, LY_VALUE_JSON, NULL, type_hints, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

            /* move JSON parser */
            if (*status == LYJSON_ARRAY) {
                /* only [null], 2 more moves are needed */
                r = lyjson_ctx_next(lydctx->jsonctx, status);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
                assert(*status == LYJSON_NULL);

                r = lyjson_ctx_next(lydctx->jsonctx, status);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
                assert(*status == LYJSON_ARRAY_CLOSED);
            }
        } else if (snode->nodetype & LYD_NODE_INNER) {
            /* create inner node */
            r = lydjson_parse_instance_inner(lydctx, snode, ext, status, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        } else {
            /* create any node */
            r = lydjson_parse_any(lydctx, snode, ext, status, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }
        LY_CHECK_GOTO(!*node, cleanup);

        /* add/correct flags */
        r = lyd_parse_set_data_flags(*node, &(*node)->meta, (struct lyd_ctx *)lydctx, ext);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        if (!(lydctx->parse_opts & LYD_PARSE_ONLY)) {
            /* store for ext instance node validation, if needed */
            r = lyd_validate_node_ext(*node, &lydctx->ext_node);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }
    } else if (r == LY_ENOT) {
        /* parse it again as an opaq node */
        r = lydjson_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status, status, first_p, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        if (snode->nodetype == LYS_LIST) {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LIST;
        } else if (snode->nodetype == LYS_LEAFLIST) {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LEAFLIST;
        }
    } else {
        /* error */
        rc = r;
        goto cleanup;
    }

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    return rc;
}

/**
 * @brief Parse JSON subtree. All leaf-list and list instances of a node are considered one subtree.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] parent Data parent of the subtree, must be set if @p first is not.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling, must be set if @p parent is not.
 * @param[in,out] parsed Optional set to add all the parsed siblings into.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_subtree_r(struct lyd_json_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR r, rc = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS status = lyjson_ctx_status(lydctx->jsonctx);
    const char *name, *prefix = NULL, *expected = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0, parse_subtree;
    const struct lysc_node *snode = NULL;
    struct lysc_ext_instance *ext = NULL;
    struct lyd_node *node = NULL, *attr_node = NULL;
    const struct ly_ctx *ctx = lydctx->jsonctx->ctx;
    char *value = NULL;

    assert(parent || first_p);
    assert((status == LYJSON_OBJECT) || (status == LYJSON_OBJECT_NEXT));

    parse_subtree = lydctx->parse_opts & LYD_PARSE_SUBTREE ? 1 : 0;
    /* all descendants should be parsed */
    lydctx->parse_opts &= ~LYD_PARSE_SUBTREE;

    r = lyjson_ctx_next(lydctx->jsonctx, &status);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    if (status == LYJSON_OBJECT_CLOSED) {
        /* empty object, fine... */
        goto cleanup;
    }

    /* process the node name */
    assert(status == LYJSON_OBJECT_NAME);
    lydjson_parse_name(lydctx->jsonctx->value, lydctx->jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_meta);
    lyjson_ctx_give_dynamic_value(lydctx->jsonctx, &value);

    if ((lydctx->int_opts & LYD_INTOPT_EVENTTIME) && !parent && !is_meta && name_len && !prefix_len &&
            !ly_strncmp("eventTime", name, name_len)) {
        /* parse eventTime */
        r = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        if (status != LYJSON_STRING) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX_JSON, "Expecting JSON %s but %s found.", lyjson_token2str(LYJSON_STRING),
                    lyjson_token2str(status));
            rc = LY_EVALID;
            goto cleanup;
        }

        /* create node */
        r = lyd_create_opaq(lydctx->jsonctx->ctx, name, name_len, prefix, prefix_len, prefix, prefix_len,
                lydctx->jsonctx->value, lydctx->jsonctx->value_len, NULL, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, &node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* validate the value */
        r = lyd_parser_notif_eventtime_validate(node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        goto node_parsed;
    } else if (!is_meta || name_len || prefix_len) {
        /* get the schema node */
        r = lydjson_get_snode(lydctx, is_meta, prefix, prefix_len, name, name_len, parent, &snode, &ext);
        if (r == LY_ENOT) {
            /* data parsed */
            goto cleanup;
        } else if ((r == LY_EVALID) && (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR)) {
            rc = r;

            /* skip the invalid data */
            if ((r = lydjson_data_skip(lydctx->jsonctx))) {
                rc = r;
            }
            r = lyjson_ctx_next(lydctx->jsonctx, &status);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
            goto cleanup;
        } else if (r) {
            /* error */
            rc = r;
            goto cleanup;
        }

        if (!snode) {
            /* we will not be parsing it as metadata */
            is_meta = 0;
        }
    }

    if (is_meta) {
        /* parse as metadata */
        if (!name_len && !prefix_len && !parent) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON,
                    "Invalid metadata format - \"@\" can be used only inside anydata, container or list entries.");
            r = LY_EVALID;
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        } else if (!name_len && !prefix_len) {
            /* parent's metadata without a name - use the schema from the parent */
            attr_node = parent;
            snode = attr_node->schema;
        }
        r = lydjson_parse_attribute(lydctx, attr_node, snode, name, name_len, prefix, prefix_len, parent, &status,
                first_p, &node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    } else if (!snode) {
        if (!(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
            /* skip element with children */
            r = lydjson_data_skip(lydctx->jsonctx);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        } else {
            /* parse as an opaq node */

            /* opaq node cannot have an empty string as the name. */
            if (name_len == 0) {
                LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX_JSON, "JSON object member name cannot be a zero-length string.");
                r = LY_EVALID;
                LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
            }

            /* move to the second item in the name/X pair and parse opaq */
            r = lydjson_ctx_next_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, &status, first_p, &node);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }
    } else {
        /* parse as a standard lyd_node but it can still turn out to be an opaque node */

        /* move to the second item in the name/X pair */
        r = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* set expected representation */
        switch (snode->nodetype) {
        case LYS_LEAFLIST:
            expected = "name/array of values";
            break;
        case LYS_LIST:
            expected = "name/array of objects";
            break;
        case LYS_LEAF:
            if (status == LYJSON_ARRAY) {
                expected = "name/[null]";
            } else {
                expected = "name/value";
            }
            break;
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_ACTION:
        case LYS_RPC:
        case LYS_ANYDATA:
            expected = "name/object";
            break;
        case LYS_ANYXML:
            if (status == LYJSON_ARRAY) {
                expected = "name/array";
            } else {
                expected = "name/value";
            }
            break;
        }

        /* check the representation according to the nodetype and then continue with the content */
        switch (snode->nodetype) {
        case LYS_LEAFLIST:
        case LYS_LIST:
            LY_CHECK_GOTO(status != LYJSON_ARRAY, representation_error);

            /* process all the values/objects */
            do {
                /* move into array/next value */
                r = lyjson_ctx_next(lydctx->jsonctx, &status);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
                if (status == LYJSON_ARRAY_CLOSED) {
                    /* empty array, fine... */
                    break;
                }

                r = lydjson_parse_instance(lydctx, parent, first_p, snode, ext, name, name_len, prefix, prefix_len,
                        &status, &node);
                if (r == LY_ENOT) {
                    goto representation_error;
                }
                LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

                lydjson_maintain_children(parent, first_p, &node, lydctx->parse_opts & LYD_PARSE_ORDERED ? 1 : 0, ext);

                /* move after the item(s) */
                r = lyjson_ctx_next(lydctx->jsonctx, &status);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
            } while (status == LYJSON_ARRAY_NEXT);

            break;
        case LYS_LEAF:
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_ACTION:
        case LYS_RPC:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            /* process the value/object */
            r = lydjson_parse_instance(lydctx, parent, first_p, snode, ext, name, name_len, prefix, prefix_len,
                    &status, &node);
            if (r == LY_ENOT) {
                goto representation_error;
            }
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

            if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
                /* rememeber the RPC/action/notification */
                lydctx->op_node = node;
            }
            break;
        }
    }

node_parsed:
    /* rememeber a successfully parsed node */
    if (parsed && node) {
        ly_set_add(parsed, node, 1, NULL);
    }

    /* finally connect the parsed node, is zeroed */
    lydjson_maintain_children(parent, first_p, &node, lydctx->parse_opts & LYD_PARSE_ORDERED ? 1 : 0, ext);

    if (!parse_subtree) {
        /* move after the item(s) */
        r = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

    /* success */
    goto cleanup;

representation_error:
    LOGVAL(ctx, LYVE_SYNTAX_JSON, "Expecting JSON %s but %s \"%s\" is represented in input data as name/%s.",
            expected, lys_nodetype2str(snode->nodetype), snode->name, lyjson_token2str(status));
    rc = LY_EVALID;
    if (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) {
        /* try to skip the invalid data */
        if ((r = lydjson_data_skip(lydctx->jsonctx))) {
            rc = r;
        }
    }

cleanup:
    free(value);
    lyd_free_tree(node);
    return rc;
}

/**
 * @brief Common start of JSON parser processing different types of the input data.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_parse_json_init(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts,
        struct lyd_json_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_json_ctx *lydctx;
    enum LYJSON_PARSER_STATUS status;

    assert(lydctx_p);

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_json_ctx_free;

    LY_CHECK_ERR_RET(ret = lyjson_ctx_new(ctx, in, &lydctx->jsonctx), free(lydctx), ret);
    status = lyjson_ctx_status(lydctx->jsonctx);

    /* parse_opts & LYD_PARSE_SUBTREE not implemented */
    if (status != LYJSON_OBJECT) {
        /* expecting top-level object */
        LOGVAL(ctx, LYVE_SYNTAX_JSON, "Expected top-level JSON object, but %s found.", lyjson_token2str(status));
        *lydctx_p = NULL;
        lyd_json_ctx_free((struct lyd_ctx *)lydctx);
        return LY_EVALID;
    }

    *lydctx_p = lydctx;
    return LY_SUCCESS;
}

LY_ERR
lyd_parse_json(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_json_ctx *lydctx = NULL;
    enum LYJSON_PARSER_STATUS status;

    rc = lyd_parse_json_init(ctx, in, parse_opts, val_opts, &lydctx);
    LY_CHECK_GOTO(rc, cleanup);

    lydctx->int_opts = int_opts;
    lydctx->ext = ext;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    /* read subtree(s) */
    do {
        r = lydjson_subtree_r(lydctx, parent, first_p, parsed);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        status = lyjson_ctx_status(lydctx->jsonctx);

        if (!(int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    } while (status == LYJSON_OBJECT_NEXT);

    if ((int_opts & LYD_INTOPT_NO_SIBLINGS) && lydctx->jsonctx->in->current[0] && (status != LYJSON_OBJECT_CLOSED)) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    /* finish linking metadata */
    r = lydjson_metadata_finish(lydctx, parent ? lyd_node_child_p(parent) : first_p);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    if (parse_opts & LYD_PARSE_SUBTREE) {
        /* check for a sibling object */
        assert(subtree_sibling);
        if (status == LYJSON_OBJECT_NEXT) {
            *subtree_sibling = 1;

            /* move to the next object */
            ly_in_skip(lydctx->jsonctx->in, 1);
        } else {
            *subtree_sibling = 0;
        }
    }

cleanup:
    /* there should be no unresolved types stored */
    assert(!(parse_opts & LYD_PARSE_ONLY) || !lydctx || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    if (rc && (!lydctx || !(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID))) {
        lyd_json_ctx_free((struct lyd_ctx *)lydctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;

        /* the JSON context is no more needed, freeing it also stops logging line numbers which would be confusing now */
        lyjson_ctx_free(lydctx->jsonctx);
        lydctx->jsonctx = NULL;
    }
    return rc;
}

/**
 * @brief Parse a specific JSON object into an opaque node.
 *
 * @param[in] jsonctx JSON parser context.
 * @param[in] name Name of the object.
 * @param[in] module Module name of the object, NULL if none expected.
 * @param[out] evnp Parsed envelope (opaque node).
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the specified object did not match.
 * @return LY_ERR value on error.
 */
static LY_ERR
lydjson_envelope(struct lyjson_ctx *jsonctx, const char *name, const char *module, struct lyd_node **envp)
{
    LY_ERR rc = LY_SUCCESS, r;
    enum LYJSON_PARSER_STATUS status = lyjson_ctx_status(jsonctx);
    const char *nam, *prefix;
    size_t nam_len, prefix_len;
    ly_bool is_meta;

    assert(status == LYJSON_OBJECT);

    *envp = NULL;

    r = lyjson_ctx_next(jsonctx, &status);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    if (status == LYJSON_OBJECT_CLOSED) {
        LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Empty JSON object.");
        rc = LY_EVALID;
        goto cleanup;
    }

    /* process the name */
    assert(status == LYJSON_OBJECT_NAME);
    lydjson_parse_name(jsonctx->value, jsonctx->value_len, &nam, &nam_len, &prefix, &prefix_len, &is_meta);
    if (is_meta) {
        LOGVAL(jsonctx->ctx, LYVE_DATA, "Unexpected metadata.");
        rc = LY_EVALID;
        goto cleanup;
    } else if (module && ly_strncmp(module, prefix, prefix_len)) {
        LOGVAL(jsonctx->ctx, LYVE_DATA, "Unexpected module \"%.*s\" instead of \"%s\".", (int)prefix_len, prefix, module);
        rc = LY_EVALID;
        goto cleanup;
    } else if (ly_strncmp(name, nam, nam_len)) {
        LOGVAL(jsonctx->ctx, LYVE_DATA, "Unexpected object \"%.*s\" instead of \"%s\".", (int)nam_len, nam, name);
        rc = LY_EVALID;
        goto cleanup;
    }

    r = lyjson_ctx_next(jsonctx, &status);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    /* create node */
    rc = lyd_create_opaq(jsonctx->ctx, name, strlen(name), prefix, prefix_len, prefix, prefix_len, NULL, 0, NULL,
            LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, envp);
    LY_CHECK_GOTO(rc, cleanup);

cleanup:
    if (rc) {
        lyd_free_tree(*envp);
        *envp = NULL;
    }
    return rc;
}

LY_ERR
lyd_parse_json_restconf(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct lyd_node **envp, struct ly_set *parsed, struct lyd_ctx **lydctx_p)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lyd_json_ctx *lydctx = NULL;
    struct lyd_node *node;
    uint32_t i, int_opts = 0, close_elem = 0;

    assert(ctx && in && lydctx_p);
    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));

    assert((data_type == LYD_TYPE_RPC_RESTCONF) || (data_type == LYD_TYPE_NOTIF_RESTCONF) ||
            (data_type == LYD_TYPE_REPLY_RESTCONF));
    assert(!(parse_opts & LYD_PARSE_SUBTREE));

    /* init context */
    rc = lyd_parse_json_init(ctx, in, parse_opts, val_opts, &lydctx);
    LY_CHECK_GOTO(rc, cleanup);
    lydctx->ext = ext;

    switch (data_type) {
    case LYD_TYPE_RPC_RESTCONF:
        assert(parent);

        /* parse "input" */
        rc = lydjson_envelope(lydctx->jsonctx, "input", lyd_node_module(parent)->name, envp);
        LY_CHECK_GOTO(rc, cleanup);

        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_RPC | LYD_INTOPT_ACTION;
        close_elem = 1;
        break;
    case LYD_TYPE_NOTIF_RESTCONF:
        assert(!parent);

        /* parse "notification" */
        rc = lydjson_envelope(lydctx->jsonctx, "notification", "ietf-restconf", envp);
        LY_CHECK_GOTO(rc, cleanup);

        /* RESTCONF notification and eventTime */
        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_NOTIF | LYD_INTOPT_EVENTTIME;
        close_elem = 1;
        break;
    case LYD_TYPE_REPLY_RESTCONF:
        assert(parent);

        /* parse "output" */
        rc = lydjson_envelope(lydctx->jsonctx, "output", lyd_node_module(parent)->name, envp);
        LY_CHECK_GOTO(rc, cleanup);

        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_REPLY;
        close_elem = 1;
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    lydctx->int_opts = int_opts;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    /* read subtree(s) */
    do {
        r = lydjson_subtree_r(lydctx, parent, first_p, parsed);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    } while (lyjson_ctx_status(lydctx->jsonctx) == LYJSON_OBJECT_NEXT);

    /* close all opened elements */
    for (i = 0; i < close_elem; ++i) {
        if (lyjson_ctx_status(lydctx->jsonctx) != LYJSON_OBJECT_CLOSED) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Expecting JSON %s but %s found.", lyjson_token2str(LYJSON_OBJECT_CLOSED),
                    lyjson_token2str(lyjson_ctx_status(lydctx->jsonctx)));
            rc = LY_EVALID;
            goto cleanup;
        }

        r = lyjson_ctx_next(lydctx->jsonctx, NULL);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }
    if (int_opts & LYD_INTOPT_EVENTTIME) {
        /* parse as a child of the envelope */
        node = (*first_p)->prev;
        if (node->schema) {
            LOGVAL(ctx, LYVE_DATA, "Missing notification \"eventTime\" node.");
            r = LY_EVALID;
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        } else {
            /* can be the only opaque node and an operation had to be parsed */
            assert(!strcmp(LYD_NAME(node), "eventTime") && (*first_p)->next);
            lyd_unlink(node);
            assert(*envp);
            lyd_insert_child(*envp, node);
        }
    }

    /* finish linking metadata */
    r = lydjson_metadata_finish(lydctx, parent ? lyd_node_child_p(parent) : first_p);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_opts & LYD_PARSE_ONLY) || !lydctx || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    if (rc) {
        lyd_json_ctx_free((struct lyd_ctx *)lydctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;

        /* the JSON context is no more needed, freeing it also stops logging line numbers which would be confusing now */
        lyjson_ctx_free(lydctx->jsonctx);
        lydctx->jsonctx = NULL;
    }
    return rc;
}
